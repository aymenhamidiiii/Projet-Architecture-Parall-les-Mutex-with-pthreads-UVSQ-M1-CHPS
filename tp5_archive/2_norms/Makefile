CC=gcc
CFLAGS=-O3 -pthread -lm

frobenius: frobenius.c
	$(CC) $(CFLAGS) -o $@ frobenius.c -lm

max: max.c
	$(CC) $(CFLAGS) -o $@ max.c

clean:
	rm -f *.o frobenius max
