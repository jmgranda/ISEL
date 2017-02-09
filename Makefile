CC=gcc
CFLAGS=-g -Wall -O -I. -DNDEBUG
LDFLAGS=-L.
LDLIBS=-lwiringPi -lpthread -lrt -lreadline

all: libwiringPi.a main

main: main.o cofm.o purse.o fsm.o task.o interp.o reactor.o

test: main
	@for i in test/*.test ; do \
	  printf "%-30s" $$i ; \
	  ./main < $$i 2>&1 > $$i.out ; \
	  if cmp $$i.out $$i.exp 2>&1 >/dev/null; then \
	    echo OK ; \
	  else \
	    echo FAIL ; \
	  fi ; \
	done
	

clean:
	$(RM) *.o *~ main libwiringPi.a

libwiringPi.a: wiringPi.o
	ar rcs $@ $^



