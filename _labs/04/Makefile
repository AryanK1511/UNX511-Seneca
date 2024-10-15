CC = g++
CFLAGS = -Wall
CFLAGS += -c
SRC = Lab4.cpp
OBJ = Lab4.o
EXEC = output

all: $(EXEC)

output: $(OBJ)
	$(CC) $(OBJ) -o ${EXEC}

Lab04.o: $(SRC)
	$(CC) $(CFLAGS) $(SRC)

clean:
	rm -rf *.o $(EXEC)

.PHONY: all clean