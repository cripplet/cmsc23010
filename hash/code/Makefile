CC=gcc
CFLAGS=-Wall -Werror -O3 -std=gnu99
DEV_CFLAGS=-Wall -Werror -O0 -g -std=gnu99
LIBS=-pthread -lm

SOURCES=utils/*c main.c hash.c lock.c test.c

OBJECTS=$(SOURCES:.c=.o)
STABLE=hash
DEV=hash_dev

all: clean $(SOURCES) $(STABLE) $(DEV)

$(STABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LIBS)

$(DEV): $(OBJECTS)
	$(CC) $(DEV_CFLAGS) $(OBJECTS) -o $@ $(LIBS)

test: clean $(DEV)
	valgrind --leak-check=full ./$(DEV) 0

run: clean $(STABLE)
	./$(STABLE) $(EXPR)

clean:
	rm -f $(STABLE) $(DEV) *.o *.tsv *.log
