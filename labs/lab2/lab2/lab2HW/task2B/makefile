all: exe

exe: main.o
	gcc -g  -Wall -o exe main.o

main.o: main.c
	gcc -g -Wall -c -o main.o main.c

.PHONY: clean
clean: 
	rm -f *.o exe