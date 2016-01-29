#Makefile for compiing & executing the program
#OS-Labs at Technische Universität Darmstadt

CFLAG= -Wall -std=gnu99 -I../1.1
CC=gcc
RM= rm -f 


all: producer-consumer

producer-consumer: 
	$(CC) $(CFLAG) -o producer-consumer producer_consumer.c

clean:
	$(RM) producer-consumer
