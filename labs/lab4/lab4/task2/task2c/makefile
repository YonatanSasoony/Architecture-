all: run

run: start.o util.o task2c.o
	ld -m elf_i386 start.o task2c.o util.o -o run 

start.o: start.s
	nasm -f elf32 start.s -o start.o
	nasm -f elf start.s -l sample.lst
	
util.o: util.c
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o

task2c.o: task2c.c
	gcc -m32 -g -Wall -ansi -c -nostdlib -fno-stack-protector task2c.c -o task2c.o

.PHONY: clean

clean: 
	rm -f *.o run

	