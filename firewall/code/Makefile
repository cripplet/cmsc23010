CC=gcc
CFLAGS=-Wall -Werror -O3 -std=c99
LIBS=-pthread -lm

SOURCES=utils/*c main.c serial.c queue.c test.c parallel.c worker.c dispatcher.c serial_queue.c result.c

OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=firewall

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LIBS)

test: clean $(EXECUTABLE)
	./$(EXECUTABLE) -

run: clean $(EXECUTABLE)
	echo "uniform packet, 16 threads" >> log.txt
	./$(EXECUTABLE) 131072 16 8000 1 1 1
	./$(EXECUTABLE) 131072 16 8000 1 1 2
	./$(EXECUTABLE) 131072 16 8000 1 1 3
	echo "exponential packet, 16 threads" >> log.txt
	./$(EXECUTABLE) 131072 16 8000 0 1 1
	./$(EXECUTABLE) 131072 16 8000 0 1 2
	./$(EXECUTABLE) 131072 16 8000 0 1 3
	echo "uniform packet, 8 threads" >> log.txt
	./$(EXECUTABLE) 131072 8 8000 1 1 1
	./$(EXECUTABLE) 131072 8 8000 1 1 2
	./$(EXECUTABLE) 131072 8 8000 1 1 3
	echo "exponential packet, 8 threads" >> log.txt
	./$(EXECUTABLE) 131072 8 8000 0 1 1
	./$(EXECUTABLE) 131072 8 8000 0 1 2
	./$(EXECUTABLE) 131072 8 8000 0 1 3

clean:
	rm -f $(EXECUTABLE) *.o output_data/*.out log.txt *.stdout
