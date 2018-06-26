#Makefile
#this file is used for autmated buiding of c files
CC = gcc
OBJS = list
OUTPUT = list



list: list.c
	gcc -Wall -g -std=c99 list.c -o list

clean:
	rm -f *.o

spotless: clean
	rm -f ${OUTPUT}

test: ${OUTPUT}
	valgrind ./${OUTPUT}

.PHONY: clean spotless test

