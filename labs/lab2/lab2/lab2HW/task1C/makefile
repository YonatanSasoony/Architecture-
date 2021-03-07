all: exe

exe: arr.o
	gcc -g -m32 -Wall -o exe arr.o

arr.o: arr.c
	gcc -m32 -g -Wall -c -o arr.o arr.c

.PHONY: clean
clean: 
	rm -f *.o exe