CC=gcc
CFLAGS=-Wall -Werror -O3 -std=c99
LIBS=-pthread -lm

SOURCES=utils/*c main.c

OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=locks

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LIBS)

test: clean $(EXECUTABLE)
	./$(EXECUTABLE) -

run: clean $(EXECUTABLE)

clean:
	rm -f $(EXECUTABLE) *.o
