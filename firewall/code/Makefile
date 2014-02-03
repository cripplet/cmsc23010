CC=gcc
CFLAGS=-Wall -Werror -O3 -std=c99
LIBS=-pthread -lm

SOURCES=utils/*c main.c serial.c queue.c test.c

OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=firewall

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LIBS)

test: clean $(EXECUTABLE)
	./$(EXECUTABLE) -

clean:
	rm -f $(EXECUTABLE) *.o log.txt output_data/*
