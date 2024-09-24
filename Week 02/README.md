# Week 02 Notes

## Shared Libraries vs Static Libraries

Shared:

- Windows: \*.dll
- Mac: \*dylib
- Linux: \*.so

Application only references that library excatly when it needs to at runtime. Shared lib needs to be there in order for an app to run. This is because all the code lives in the shared library. So app crashes without shared library. Since you dont copy paste the code, this keeps your application really small.

Static:

- Windows: \*.lib
- Mac and Linux: \*.a

Program utilizes static library at compile time. This happens before running. You take exactly what you need from static library, take all the code and move that into the application. Here, the code now lives in your application in contrast to haveing the code reference to the library. If you are depended on alot of libraries, this will make your application very bg very fast.

## Creating a Static Library

Refer to the [`Makefile` for an example Static Library](./MyStaticLibrary/Makefile) that I created.

Here's a detailed explanation of each line in my `Makefile`:

```makefile
CC=g++
```

This sets the C++ compiler to `g++`. It defines the compiler that will be used for compiling the source files.

```makefile
CFLAGS=-I
CFLAGS+=-Wall
CFLAGS+=-c
```

- `CFLAGS=-I`: The `-I` flag specifies directories where the compiler will search for header files. It’s currently empty, which means no extra directories are added. You can specify a path like `-I/path/to/include` if needed.
- `CFLAGS+=-Wall`: The `-Wall` flag tells the compiler to show **all warnings**. This helps catch potential issues early.
- `CFLAGS+=-c`: The `-c` flag tells the compiler to compile source files into object files (`.o`), but **not link** them. This is useful for building libraries.

```makefile
AR=ar
```

This sets the archive utility to `ar`, which is used to create static libraries (e.g., `libTest.a`). Static libraries are collections of object files.

```makefile
FILES+=Bye.cpp
FILES+=Hello.cpp
```

- `FILES+=Bye.cpp`: Adds the `Bye.cpp` file to the list of files to be compiled.
- `FILES+=Hello.cpp`: Adds the `Hello.cpp` file to the list.

These are your source files that will be compiled into object files.

```makefile
OBJFILES+=Bye.o
OBJFILES+=Hello.o
```

- `OBJFILES+=Bye.o`: Adds `Bye.o` (the compiled object file from `Bye.cpp`) to the list of object files.
- `OBJFILES+=Hello.o`: Adds `Hello.o` (the compiled object file from `Hello.cpp`) to the list of object files.

Object files are intermediary compiled outputs before linking.

```makefile
Test: $(FILES)
	$(CC) $(CFLAGS) $(FILES)
```

- **Target `Test`:** This defines a rule named `Test`, which depends on the `$(FILES)` (source files).
- `$(CC) $(CFLAGS) $(FILES)`: This line invokes the `g++` compiler with the flags specified in `CFLAGS` to compile the `$(FILES)` source files.

**Issue:** This rule attempts to compile all the `.cpp` files in one step without creating individual object files. Since `-c` is specified in `CFLAGS`, the object files will be generated but not linked into a binary. This rule is not very useful unless your goal is just to generate the `.o` files.

```makefile
lib: $(OBJFILES)
	$(AR) rcs libTest.a $(OBJFILES)
```

- **Target `lib`:** This rule defines how to build the static library `libTest.a` from the object files (`$(OBJFILES)`).
- `$(AR) rcs libTest.a $(OBJFILES)`:

  - `ar` creates an archive (static library) with the `.o` files.
  - `r` means to replace older files if they exist in the archive.
  - `c` creates a new archive if one doesn’t exist.
  - `s` adds an index to the library for faster linking.

  The result is the `libTest.a` static library, which is a collection of compiled object files.

```makefile
install:
	cp libTest.a ../libTest
	cp *.h ../libTest
```

- **Target `install`:** This rule defines how to "install" your static library and header files.
- `cp libTest.a ../libTest`: This copies the generated static library `libTest.a` into the `../libTest` directory.
- `cp *.h ../libTest`: This copies all header files (`*.h`) into the `../libTest` directory.

This assumes that header files are present in the current directory. You may want to explicitly specify the header files instead of using `*.h`.

```makefile
clean:
	rm -f *.o *.a
```

- **Target `clean`:** This rule cleans up the build directory by removing compiled object files (`*.o`) and the static library (`*.a`).
- `rm -f *.o *.a`: Removes all files ending with `.o` (object files) and `.a` (archive/static libraries). The `-f` flag ensures that no error is raised if the files don't exist.

```makefile
all: Test lib
```

- **Target `all`:** This is the default target that is run when you type `make` without specifying a target.
- It depends on the `Test` and `lib` targets, meaning that it will first try to build the object files and static library when invoked.

### How to Run the Makefile

```bash
make clean all install
```

### Some Additional Commands

#### `ar tv ../libTest/libTest.a`

- `ar` is a tool for creating, modifying, and extracting from archives (e.g., static libraries).
- `t` is the option that lists the contents of an archive.
- `v` gives a verbose output, showing detailed information.
- `../libTest/libTest.a` is the path to the static library we're inspecting (`libTest.a`).
- This command lists the contents of the static library `libTest.a`. In this case, it shows the object files (`Bye.o` and `Hello.o`) that were archived inside the static library.
- The output shows file permissions (`rw-r--r--`), file size, and timestamps (in this case, placeholder timestamps).

**Output Example:**

```bash
rw-r--r-- 0/0   9000 Jan  1 00:00 1970 Bye.o
rw-r--r-- 0/0  11544 Jan  1 00:00 1970 Hello.o
```

This provides an overview of what object files (`Bye.o` and `Hello.o`) were included in the static library and other metadata.

#### `nm ../libTest/libTest.a`

- `nm` is a tool used to display the symbol table of object files. It shows the functions and variables defined or referenced in an object file.
- `../libTest/libTest.a` specifies the archive file to analyze.
- This command displays the symbol table of the object files contained in the static library `libTest.a`. It shows functions defined in the `Bye.o` and `Hello.o` object files, along with their attributes.
- Symbols marked as `T` represent text (code) that is defined within the file.
- Symbols marked as `U` represent undefined references, i.e., symbols that are used but not defined within that object file (to be linked later).
- Other symbols like `V` (weak symbol), `W` (weak references), and `n` (local symbols) provide further information about the code and data within the object files.

**Sample Output Breakdown:**

For `Bye.o`:

```bash
0000000000000000 T _Z14print_bye_nameNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE
```

The symbol `print_bye_name` is a function defined in the `Bye.o` object file. The `T` indicates that this is code (text section), and the name-mangling shows it works with C++ strings (`std::string`).

For `Hello.o`:

```bash
0000000000000000 T _Z17print_hello_worldB5cxx11v
```

This is the `print_hello_world` function defined in the `Hello.o` object file.

This command helps you see which functions are defined and referenced within the object files in the static library. It’s useful for checking which symbols are available for linking and which are missing.

### Using a static library

You can checkout my `Makefile` to do this [here](./Makefile).

#### Makefile Explanation

```makefile
CC=g++                         # Define the compiler to use (g++).
CFLAGS=-I./libTest              # Add the directory where the library header files are located (./libTest).
CFLAGS+=-Wall                   # Enable all compiler warnings to ensure code quality.
FILES=staticLibraryTest.cpp     # The source file that will be compiled.
LIBS=./libTest/libTest.a        # Full path to the static library libTest.a that will be linked.

LibTest: $(FILES)               # Target to build the program.
	$(CC) $(CFLAGS) $(FILES) -o staticLibraryTest $(LIBS)  # Compile and link the static library to create the executable.

clean:                          # Target to clean up the generated files.
	rm -f *.o staticLibraryTest  # Remove all object files and the compiled program.

all: LibTest                    # A convenience target to build the program by running `make all`.
```

#### Breakdown of Commands

1. **Variables:**

   - `CC=g++`: Specifies that the `g++` compiler is used to compile the source code.
   - `CFLAGS=-I./libTest`: Adds the `./libTest` directory to the include path, which allows the compiler to find header files for the static library.
   - `FILES=staticLibraryTest.cpp`: Defines the source file to be compiled.
   - `LIBS=./libTest/libTest.a`: Points to the static library (`libTest.a`) that will be linked with the compiled code. Can also use `LIBS=-L./libTest -lTest`. This will automatically prefix the work lib for you.

2. **LibTest Target:**

   - This target compiles `staticLibraryTest.cpp` and links it with `libTest.a` to generate the executable `staticLibraryTest`.
   - `$(CC) $(CFLAGS) $(FILES) -o staticLibraryTest $(LIBS)`: The command compiles the source file, adds any flags (such as include paths with `CFLAGS`), and links it with the static library `libTest.a`.

3. **clean Target:**

   - Removes the compiled output (`staticLibraryTest`) and any object files (`*.o`).
   - This is useful for resetting the build environment.

4. **all Target:**
   - A generic target that just calls `LibTest`. Running `make all` will trigger the build.

#### Example `staticLibraryTest.cpp`

```cpp
#include "Lib.h"     // Include the header file for the static library.
#include <iostream>  // Include the standard IO library.

using namespace std;

int main() {
    cout << print_hello_world() << endl;  // Call the function from the static library and print the result.
    return 0;
}
```

- This program includes the static library's header (`Lib.h`) and the standard I/O library.
- `print_hello_world()` is a function defined in the static library (`libTest.a`), and this function is called and its result is printed.

#### Steps to Use Static Libraries

Once you are done with making the static library. Use `make clean all` to make this code as well and then run the executable.

## Explanation of Creating and Using Shared Libraries

In the provided example, the focus is on creating and using **shared libraries**. The process is similar to creating static libraries but with a few key differences. Let's break this down step by step.

### Step 1: Compiling Position Independent Code (`fPIC`)

To create a shared library, you first need to compile your source files into **Position Independent Code (PIC)** using the `-fPIC` option. PIC is required for shared libraries to allow the code to be loaded at different memory addresses in different programs.

#### Example

```bash
g++ -I -fPIC -Wall -c Conversions.cpp General.cpp Geometry.cpp
```

- `-I`: Include the directory for header files.
- `-fPIC`: Compiles position-independent code, which is essential for shared libraries.
- `-Wall`: Enables all compiler's warning messages.
- `-c`: Tells the compiler to generate object files (`.o`) instead of executable binaries.

This command will generate the object files `Conversions.o`, `General.o`, and `Geometry.o`.

### Step 2: Creating the Shared Library

The next step is to link these object files into a shared library using the `-shared` option.

#### Example:

```bash
g++ -shared -o libMath.so Conversions.o General.o Geometry.o
```

- `-shared`: Tells the compiler to create a shared library.
- `-o libMath.so`: Specifies the output name of the shared library (`libMath.so`).
- `Conversions.o General.o Geometry.o`: The object files that will be linked into the shared library.

### Step 3: Installing the Shared Library

Once the shared library (`libMath.so`) is created, it must be placed in a location where the system can find it.

#### Installing to `/usr/lib`:

```bash
sudo cp libMath.so /usr/lib
```

- You need **root permissions** (hence the `sudo` command) to place files in `/usr/lib`, where system-wide libraries are stored.

Alternatively, if you store the shared library in a custom location, you need to tell the system where to find it:

```bash
export LD_LIBRARY_PATH=/path/to/your/library:$LD_LIBRARY_PATH
sudo ldconfig
```

- `LD_LIBRARY_PATH`: This environment variable tells the system where to look for shared libraries.
- `ldconfig`: Updates the system's knowledge of available libraries. This is necessary after adding a new library.

### Step 4: Header Files

Usually, the header files (`*.h`) associated with the shared library are placed in a separate directory, like `/usr/include`, to be easily accessible by programs that need to use the library.

#### Copying Header Files

```bash
sudo cp *.h /usr/include
```

This allows other developers or programs to include your library headers with `#include <yourheader.h>`.

### `Makefile` Breakdown

The provided `Makefile` handles both **compiling source files** and **creating a shared library**. You can check out my `Makefile` [here](./MySharedLibrary/Makefile).

#### Variables

```makefile
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
```

- `CC=g++`: Specifies the compiler.
- `CFLAGS`: Compiler flags, including:
  - `-I`: Include path for headers.
  - `-fPIC`: Required for shared libraries.
  - `-Wall`: Enables all warnings.
  - `-c`: Compiles source files into object files.
- `LIBFLAGS`: Flags used for creating the shared library, including:
  - `-shared`: Create a shared library.
  - `-o`: Specify the output file name.
- `FILES`: Source files to be compiled.
- `OBJFILES`: Object files generated from the source files.

#### Targets

```makefile
Test: $(FILES)
	$(CC) $(CFLAGS) $(FILES)
```

- The `Test` target compiles the source files (`Bye.cpp` and `Hello.cpp`) into object files (`Bye.o` and `Hello.o`).

```makefile
lib: $(OBJFILES)
	$(CC) $(LIBFLAGS) libTest.so $(OBJFILES)
```

- The `lib` target creates the shared library `libTest.so` from the object files `Bye.o` and `Hello.o`.

```makefile
install:
	cp libTest.so /usr/lib
	cp *.h ../libTest
```

- The `install` target copies the shared library `libTest.so` to `/usr/lib` and the header files to a directory called `../libTest`.

```makefile
clean:
	rm -f *.o *.so
```

- The `clean` target removes the generated object files and shared library (`*.o`, `*.so`).

```makefile
all: Test lib
```

- The `all` target is a convenient way to run both the `Test` and `lib` targets in sequence, compiling the source code and creating the shared library.

### Using the Shared Library in Another Program

Once the shared library is created and installed, you can compile a program that uses this library.
You can checkout my `Makefile` for this [here](./sharedMakefile).

#### Example `Makefile` for Using the Shared Library

```makefile
CC=g++
CFLAGS=-I./libTest
CFLAGS+=-Wall
FILES=sharedLibraryTest.cpp
LIBS=-L/usr/lib -lTest

LibTest: $(FILES)
	$(CC) $(CFLAGS) $(FILES) -o sharedLibraryTest $(LIBS)

clean:
	rm -f *.o sharedLibraryTest

all: LibTest
```

- `CFLAGS=-I./libTest`: Includes the path to your header files.
- `LIBS=-L/usr/lib -lTest`: Links the shared library `libTest.so` from `/usr/lib`.

To compile and link against the shared library, you would run:

```bash
make -f YourMakefile clean all
```

This will create an executable that uses the shared library (`libTest.so`).
