CC=g++
CFLAGS=-I
CFLAGS+=-fPIC
CFLAGS+=-Wall
CFLAGS+=-c
LIBFLAGS=-shared
LIBFLAGS+=-o
FILES=Bye.cpp
FILES+=Hello.cpp
OBJFILES=Bye.o
OBJFILES+=Hello.o

Test: $(FILES)
	$(CC) $(CFLAGS) $(FILES)

lib: $(OBJFILES)
	$(CC) $(LIBFLAGS) libTest.so $(OBJFILES)

install:
	cp libTest.so /usr/lib
	cp *.h ../libTest

clean:
	rm -f *.o *.so

all: Test lib
