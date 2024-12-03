CXX = g++
CXXFLAGS = -std=c++11 -pthread

LogServer: LogServer.o
	$(CXX) $(CXXFLAGS) -o LogServer LogServer.o

LogServer.o: LogServer.cpp
	$(CXX) $(CXXFLAGS) -c LogServer.cpp

all: LogServer

clean:
	rm -f *.o LogServer