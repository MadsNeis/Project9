#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

#include "eventbuf.h"

struct eventbuf *eb;

sem_t *mutex;
sem_t *items;
sem_t *spaces;

int producers_done = 0;

int num_consumers;
int events_per_producer;

sem_t *sem_open_temp(const char *name, int value)
{
    sem_t *sem;

    if ((sem = sem_open(name, O_CREAT, 0600, value)) == SEM_FAILED)
        return SEM_FAILED;

    if (sem_unlink(name) == -1) {
        sem_close(sem);
        return SEM_FAILED;
    }

    return sem;
}

void *producer(void *arg)
{
    int id = *(int *)arg;

    for (int i = 0; i < events_per_producer; i++) {
        int event = id * 100 + i;

        sem_wait(spaces);
        sem_wait(mutex);

        eventbuf_add(eb, event);
        printf("P%d: adding event %d\n", id, event);

        sem_post(mutex);
        sem_post(items);
    }

    printf("P%d: exiting\n", id);

    return NULL;
}

void *consumer(void *arg)
{
    int id = *(int *)arg;

    while (1) {
        sem_wait(items);
        sem_wait(mutex);

        if (producers_done && eventbuf_empty(eb)) {
            sem_post(mutex);
            break;
        }

        int event = eventbuf_get(eb);
        printf("C%d: got event %d\n", id, event);

        sem_post(mutex);
        sem_post(spaces);
    }

    printf("C%d: exiting\n", id);

    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 5) {
        fprintf(stderr, "usage: pc producers consumers events_per_producer max_outstanding\n");
        return 1;
    }

    int num_producers = atoi(argv[1]);
    num_consumers = atoi(argv[2]);
    events_per_producer = atoi(argv[3]);
    int max_outstanding = atoi(argv[4]);

    eb = eventbuf_create();

    mutex  = sem_open_temp("pc-mutex",  1);
    items  = sem_open_temp("pc-items",  0);
    spaces = sem_open_temp("pc-spaces", max_outstanding);

    pthread_t *producer_threads = malloc(num_producers * sizeof(pthread_t));
    pthread_t *consumer_threads = malloc(num_consumers * sizeof(pthread_t));
    int *producer_ids = malloc(num_producers * sizeof(int));
    int *consumer_ids = malloc(num_consumers * sizeof(int));

    for (int i = 0; i < num_producers; i++) {
        producer_ids[i] = i;
        pthread_create(&producer_threads[i], NULL, producer, &producer_ids[i]);
    }

    for (int i = 0; i < num_consumers; i++) {
        consumer_ids[i] = i;
        pthread_create(&consumer_threads[i], NULL, consumer, &consumer_ids[i]);
    }

    for (int i = 0; i < num_producers; i++)
        pthread_join(producer_threads[i], NULL);

    producers_done = 1;

    for (int i = 0; i < num_consumers; i++)
        sem_post(items);

    for (int i = 0; i < num_consumers; i++)
        pthread_join(consumer_threads[i], NULL);

    eventbuf_free(eb);

    sem_close(mutex);
    sem_close(items);
    sem_close(spaces);

    free(producer_threads);
    free(consumer_threads);
    free(producer_ids);
    free(consumer_ids);

    return 0;
}