all: encoder

encoder: main.o
	gcc -g -Wall -o encoder main.o

main.o: main.c
	gcc -g -Wall -c -o main.o main.c

.PHONY: clean
clean: 
	rm -f *.o encoder


