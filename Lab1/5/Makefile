#Makefile for compiing & executing the program
#OS-Labs at Technische Universität Darmstadt

CC=gcc
CFLAGS=-Wall
LDFLAGS= -lrt


all: sh-mem
   
sh-mem: shared_memory.c
	$(CC) $(CFLAGS) -o sh-mem shared_memory.c $(LDFLAGS)

clean:
	$(RM) sh-mem
