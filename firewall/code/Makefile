CC=gcc
CFLAGS=-Wall -Werror -O3 -std=c99
LIBS=-pthread -lm

SOURCES=utils/*c serial.c

OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=firewall

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LIBS)

test: clean $(EXECUTABLE)
	./$(EXECUTABLE) -t

clean:
	rm -f $(EXECUTABLE) *.o log.txt output_data/*
