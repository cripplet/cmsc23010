CC=gcc
CFLAGS=-Wall -Werror -O3 -std=c99
LIBS=-pthread -lm

SOURCES=utils/*c main.c serial.c queue.c test.c parallel.c worker.c dispatcher.c serial_queue.c

OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=firewall

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LIBS)

test: clean $(EXECUTABLE)
	./$(EXECUTABLE) -

run: clean $(EXECUTABLE)
	./$(EXECUTABLE) 100 16 4 0 1 1
	./$(EXECUTABLE) 100 16 4 0 1 2
	./$(EXECUTABLE) 100 16 4 0 1 3

clean:
	rm -f $(EXECUTABLE) *.o log.txt output_data/*
