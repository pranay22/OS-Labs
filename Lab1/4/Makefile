#Makefile for compiing & executing the program
#OS-Labs at Technische Universität Darmstadt

CC=gcc
CFLAGS=-Wall
LDFLAGS= -lrt


all: msg_queue
   
sh-mem: msg_queue.c
	$(CC) $(CFLAGS) -o msg_queue msg_queue.c $(LDFLAGS)

clean:
	$(RM) msg_queue


#all: posix-mq.c
#	gcc -Wall -o posix-mq.o posix-mq.c -lrt
#clean:
#	$(RM) posix-mq.o
