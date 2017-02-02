CC=gcc
CFLAGS=-g -Wall -O -I. -DNDEBUG
LDFLAGS=-L.
LDLIBS=-lwiringPi -lpthread -lrt

all: main

main: main.o fsm.o task.o

clean:
	$(RM) *.o *~ main libwiringPi.a

libwiringPi.a: wiringPi.o
	ar rcs $@ $^



