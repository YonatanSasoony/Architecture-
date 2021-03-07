all: digit_cnt

digit_cnt: digit_cnt.o 
	gcc -g -m32 -lm -Wall -o digit_cnt digit_cnt.o

digit_cnt.o: digit_cnt.c
	gcc -g -m32 -fno-pie -Wall -c -o digit_cnt.o digit_cnt.c

.PHONY: clean
clean: 
	rm -f *.o digit_cnt