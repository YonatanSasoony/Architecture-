all: exe

exe: addresses.o
	gcc -g -m32 -Wall -o exe addresses.o

addresses.o: addresses.c
	gcc -m32 -g -Wall -c -o addresses.o addresses.c

.PHONY: clean
clean: 
	rm -f *.o exe