all: task0a

task0a: task0a.o
	gcc -g  -Wall -o task0a task0a.o

task0a.o: task0a.c
	gcc -g -Wall -c -o task0a.o task0a.c

.PHONY: clean
clean: 
	rm -f *.o task0a