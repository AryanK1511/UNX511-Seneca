CC = g++
CFLAGS = -Wall
TARGET = output

$(TARGET): main.o
	$(CC) main.o -o $(TARGET)

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp

clean:
	rm -f *.o $(TARGET)
