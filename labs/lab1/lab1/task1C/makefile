all: enc

enc: encoder.o
	gcc -g -Wall -o enc encoder.o

encoder.o: encoder.c
	gcc -g -Wall -c -o encoder.o encoder.c

.PHONY: clean
clean: 
	rm -f *.o enc


