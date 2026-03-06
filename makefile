
cc = gcc
CFLAGS = -Wall -Wextra -Werror

pc: pc.c eventbuf.c
	$(cc) $(CFLAGS) -o pc pc.c eventbuf.c -lpthread

clean:
	rm -f pc
