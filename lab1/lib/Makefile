CC=g++
CFLAGS=-I
CFLAGS+=-Wall
CFLAGS+=-c
AR=ar

pidUtil: .
	$(CC) $(CFLAGS) pidUtil.cpp -o pidUtil.o

lib: .
	$(AR) rcs libPidUtil.a pidUtil.o

clean:
	rm -f *.o *.a

all: pidUtil lib
