CC=g++
CFLAGS=-I
CFLAGS+=-Wall
CFLAGS+=-g
#CFLAGS+=-DDEBUG
FILES1=Transducer.cpp
FILES1+=TransducerMain.cpp
FILES2=DataAcquisition.cpp
LIBS=-lpthread

transducer: $(FILES1)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

dataAc: $(FILES2)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -f *.o transducer dataAc

all: transducer dataAc
