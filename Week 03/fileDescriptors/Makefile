# Compiler
CC= g++  # C++ compiler for linking

# Compiler flags
CFLAGS = -Wall

# Source files
SRC = main.c

# Object files
OBJ = main.o

# Executable name
EXEC = output

# Default target
all: $(EXEC)  # The 'all' target depends on the executable

# Rule to create the executable from the object file
$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC)

# Rule to compile the source file into an object file
$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $(SRC) 

# Clean target to remove object files and the executable
clean:
	rm -rf *.o $(EXEC)

# .PHONY declaration
# Declaring 'all' and 'clean' as phony targets to prevent conflicts with files
# that may share the same name. This ensures that the commands for these targets
# will always run regardless of whether a file with the same name exists.
# 
# Without .PHONY:
# - If a file named 'all' exists, running 'make all' will do nothing, as
#   make will see it as up to date.
# - If a file named 'clean' exists, running 'make clean' will also do nothing,
#   since make will think it doesn't need to execute the clean commands.
.PHONY: all clean
