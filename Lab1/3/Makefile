#Makefile for compiing & executing the program
#OS-Labs at Technische Universität Darmstadt
CC= gcc
CFLAG= -Wall
EXEC= parent2child
RM= rm -f
all: $(EXEC) 

parent2child: parent_2_child.c
	$(CC) $(CFLAG) -o parent2child parent_2_child.c

clean: 
	$(RM) $(EXEC)

