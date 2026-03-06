# Project 9

## Building 
Command Line:
* `make` to build. Executable called pc will be produced
* `make clean` to clean up all build products.

## Files
* `pc.c` - main code for program
* `eventbuf.c` - shared event buffer implementation.
* `eventbuf.h` - header for event buffer

## Data
* There's a shared `eventbuf` FIFO queue that holds integer events produced by producer threads and consumed by consumer threads.

## Functions

* `main()`
    * `producer()` -  thread function for producers
    * `consumer()` - thread function for consumers
    * `sem_open_temp()` - Helper to create and immediately unlink a named semaphore. 
       