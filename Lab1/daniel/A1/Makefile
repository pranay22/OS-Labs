CC=gcc
CFLAGS=-Wall
LDFLAGS=
SOURCES=task_1_1.c
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=task_1_1

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
