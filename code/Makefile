CC=gcc
CFLAGS=-Wall -Werror -O3 -std=c99
LIBS=-pthread

SOURCES=io.c main.c test.c utils.c stopwatch.c fw.c

OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=main

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LIBS) $(OBJECTS) -o $@

run: clean
	./run_all.sh

test: clean $(EXECUTABLE)
	./main -t

clean:
	rm -f $(EXECUTABLE) $(OBJECTS) log.txt output_data/*
