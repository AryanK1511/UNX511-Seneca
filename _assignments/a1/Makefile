CC=g++
CFLAGS=-I
CFLAGS+=-Wall
FILES1=intfMonitor.cpp
FILES2=networkMonitor.cpp

all: intfMonitor networkMonitor

intfMonitor: $(FILES1)
	$(CC) $(CFLAGS) -o intfMonitor $(FILES1)

networkmonitor: $(FILES2)
	$(CC) $(CFLAGS) -o networkmonitor $(FILES2)

clean:
	rm -f *.o intfMonitor networkMonitor