all: skeleton

skeleton: skeleton.o 
	gcc -m32 -g -Wall -nostdlib -o skeleton skeleton.o 

skeleton.o: skeleton.s
	nasm -f elf skeleton.s -o skeleton.o

.PHONY: clean
clean: 
	rm -f *.o skeleton