CC=gcc
# CFLAGS=-Wall -Werror -O3 -std=c99
CFLAGS=-Wall -Werror -g -std=c99
LIBS=-pthread -lm

SOURCES=utils/*c main.c lock.c expr.c test.c

OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=lock

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LIBS)

test: clean $(EXECUTABLE)
	valgrind ./$(EXECUTABLE) 1

run: clean $(EXECUTABLE)

clean:
	rm -f $(EXECUTABLE) *.o
