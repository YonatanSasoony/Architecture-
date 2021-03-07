all: run

run: task1c.o
	gcc -g  -Wall -o run task1c.o

task1c.o: task1c.c
	gcc -g -Wall -c -o task1c.o task1c.c

.PHONY: clean
clean: 
	rm -f *.o run