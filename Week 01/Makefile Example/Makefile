# Anytime that main.o or output.o changes, compile both of them into a single executable called output
output: main.o Message.o 
	g++ main.o Message.o -o output 

# Go ahead and compile all the code in main.cpp to generate main.o whenever main.cpp changes
# -c flag says, don't try to create an executable but instead, just compile these files into an object file
# By default, -c will keep the name of the file, but will replace the .cpp with .o
main.o: main.cpp
	g++ -c main.cpp

Message.o: Message.cpp MESSAGE.h
	g++ -c Message.cpp

# Everything that ends with .o and output file will be removed when the clean command is run
clean:
	rm *.o output