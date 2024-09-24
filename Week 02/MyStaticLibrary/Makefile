CC=g++
CFLAGS=-I
CFLAGS+=-Wall
CFLAGS+=-c
AR=ar
FILES+=Bye.cpp
FILES+=Hello.cpp
OBJFILES+=Bye.o
OBJFILES+=Hello.o

Test: $(FILES)
	$(CC) $(CFLAGS) $(FILES)

lib: $(OBJFILES)
	$(AR) rcs libTest.a $(OBJFILES)

install: 
	cp libTest.a ../libTest
	cp *.h ../libTest

clean:
	rm -f *.o *.a

all: Test lib