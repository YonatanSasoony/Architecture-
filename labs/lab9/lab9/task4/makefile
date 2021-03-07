all: skeleton

skeleton: skeleton.o 
	ld -m elf_i386 -o skeleton skeleton.o 

skeleton.o: skeleton.s
	nasm -f elf32 skeleton.s -o skeleton.o

.PHONY: clean
clean: 
	rm -f *.o skeleton