all: run

run : add.o main.o numbers.o
	gcc -g -m32 -Wall -o run add.o main.o numbers.o

add.o: add.s
	nasm -g -f elf -w+all -o add.o add.s

main.o: main.c
	gcc -m32 -g -Wall -c -o main.o main.c 

numbers.o: numbers.c
	gcc -m32 -g -Wall -c -o numbers.o numbers.c 

.PHONY: clean

clean: 
	rm -f *.o hello