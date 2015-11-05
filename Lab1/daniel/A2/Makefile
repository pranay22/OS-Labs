CC=gcc
CFLAGS=-Wall -pthread
LDFLAGS=-pthread
SOURCES=task_1_2.c
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=task_1_2

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
