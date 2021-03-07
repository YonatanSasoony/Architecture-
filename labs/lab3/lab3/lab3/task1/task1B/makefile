all: run

run: main.o
	gcc -g  -Wall -o run main.o

main.o: main.c
	gcc -g -Wall -c -o main.o main.c

.PHONY: clean
clean: 
	rm -f *.o run