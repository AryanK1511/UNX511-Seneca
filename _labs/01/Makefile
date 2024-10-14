output: main.o Process.o ProcessManager.o
	g++ main.o Process.o ProcessManager.o -o output 

main.o: main.cpp
	g++ -c main.cpp

Process.o: Process.cpp Process.h
	g++ -c Process.cpp

ProcessManager.o: ProcessManager.cpp ProcessManager.h
	g++ -c ProcessManager.cpp

clean:
	rm *.o output