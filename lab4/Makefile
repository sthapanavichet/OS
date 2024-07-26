CC=g++
CFLAGS=-I
CFLAGS+=-Wall
FILES=server.cpp
FILES1+=client.cpp

server: $(FILES)
	$(CC) $(CFLAGS) $^ -o $@

client1: $(FILES1)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f *.o server client

all: server client