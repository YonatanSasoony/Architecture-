all: run

run: task2b.o
	gcc -g  -Wall -o run task2b.o

task2b.o: task2b.c
	gcc -g -Wall -c -o task2b.o task2b.c

.PHONY: clean
clean: 
	rm -f *.o run