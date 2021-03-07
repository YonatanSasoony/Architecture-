all: run

run: lab3_bubblesort.o
	gcc -g  -Wall -o run lab3_bubblesort.o

lab3_bubblesort.o: lab3_bubblesort.c
	gcc -g -Wall -c -o lab3_bubblesort.o lab3_bubblesort.c

.PHONY: clean
clean: 
	rm -f *.o run