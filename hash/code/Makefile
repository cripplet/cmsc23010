CC=gcc
CFLAGS=-Wall -Werror -O3 -std=c99
DEV_CFLAGS=-Wall -Werror -std=c99 -g
LIBS=-pthread -lm

SOURCES=utils/*c main.c hash.c lock.c

OBJECTS=$(SOURCES:.c=.o)
STABLE=hash
DEV=hash_dev

all: $(SOURCES) $(STABLE)

$(STABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LIBS)

$(DEV): $(OBJECTS)
	$(CC) $(DEV_CFLAGS) $(OBJECTS) -o $@ $(LIBS)

test: clean $(DEV)
	valgrind --leak-check=full ./$(DEV) 0

run: clean $(DEV)
	./$(DEV) $(EXPR)

clean:
	rm -f $(STABLE) $(DEV) *.o *.tsv *.log
