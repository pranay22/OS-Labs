CC=gcc
CFLAGS=-Wall
LDFLAGS=-lrt
SOURCES=task_1_5.c
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=task_1_5

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
