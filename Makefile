CC = gcc
CFLAGS = -c

all: main


hash.o: hash.c hash.h
	$(CC) $(CFLAGS) -o $@ $<

segment.o: segment.c segment.h
	$(CC) $(CFLAGS) -o $@ $<

parser.o: parser.c parser.h
	$(CC) $(CFLAGS) -o $@ $<

cpu.o: cpu.c cpu.h
	$(CC) $(CFLAGS) -o $@ $<

addressing.o: addressing.c addressing.h 
	$(CC) $(CFLAGS) -o $@ $<

main.o: main.c hash.h segment.h parser.h
	$(CC) $(CFLAGS) -o $@ $<

main: hash.o parser.o segment.o test.o cpu.o
	$(CC) -o main hash.o parser.o segment.o test.o cpu.o

test.o: test.c hash.h segment.h parser.h cpu.h
	$(CC) $(CFLAGS) -o $@ $<

test: hash.o parser.o segment.o test.o cpu.o
	$(CC) -o test hash.o parser.o segment.o test.o cpu.o

clean:
	rm -f *.o
	rm -f main
	rm -f test