all: mypipe

mypipe: mypipe.o
	gcc -g  -Wall -o mypipe mypipe.o

mypipe.o: mypipe.c
	gcc -g -Wall -c -o mypipe.o mypipe.c

.PHONY: clean
clean: 
	rm -f *.o mypipe