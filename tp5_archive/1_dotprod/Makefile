CC=gcc
CFLAGS=-O3 -pthread

dotprod_1: dotprod_ref.c dotprod_1.c
	$(CC) $(CFLAGS) -c dotprod_ref.c
	$(CC) $(CFLAGS) -c dotprod_1.c
	$(CC) $(CFLAGS) -o $@ dotprod_ref.o dotprod_1.o

dotprod_2: dotprod_ref.c dotprod_2.c
	$(CC) $(CFLAGS) -c dotprod_ref.c
	$(CC) $(CFLAGS) -c dotprod_2.c
	$(CC) $(CFLAGS) -o $@ dotprod_ref.o dotprod_2.o

clean:
	rm -f *.o dotprod_1 dotprod_2
