CC=gcc
CFLAGS=-Wall -Werror -O3 -std=c99
DEV_CFLAGS=-Wall -Werror -g -std=c99
LIBS=-pthread -lm

SOURCES=utils/*c main.c hash.c

OBJECTS=$(SOURCES:.c=.o)
STABLE=hash
DEV=hash_dev

all: $(SOURCES) $(STABLE)

$(STABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LIBS)

$(STABLE): $(OBJECTS)
	$(CC) $(DEV_CFLAGS) $(OBJECTS) -o $@ $(LIBS)

test: clean $(STABLE)
	valgrind ./$(STABLE) 0

run: clean $(DEV)
	./$(DEV) $(EXPR)

clean:
	rm -f $(STABLE) $(DEV) *.o *.tsv *.log
