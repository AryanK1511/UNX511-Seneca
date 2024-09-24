CC=g++ 
CFLAGS=-I./libTest
CFLAGS+=-Wall 
FILES=staticLibraryTest.cpp 
LIBS=./libTest/libTest.a 
 
LibTest: $(FILES) 
	$(CC) $(CFLAGS) $(FILES) -o staticLibraryTest $(LIBS) 
 
clean: 
	rm -f *.o staticLibraryTest
 
all: LibTest