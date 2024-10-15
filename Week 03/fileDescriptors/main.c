/*
In this program, we will be using file descriptors and UNIX system calls to:

    * Read from a file
    * Write to a file
*/

#define _CRT_SECURE_NO_WARNINGS
#include <fcntl.h>    // Import the file control header for system call flags
#include <stdio.h>    // For standard I/O operations like perror()
#include <sys/stat.h> // For defining file status and permissions
#include <unistd.h>   // For read(), write(), close() functions

const int BUFFER_SIZE = 50; // Size of the buffer used for reading/writing

int main(void) {
  int returnValue = 0; // Variable to track the return value for error handling
  ssize_t numRead,
      numWrite; // Variables to store the number of bytes read/written
  char buf[BUFFER_SIZE + 1]; // Buffer to hold the data being read/written
  // Store the names of the input and output file in variables
  char inFile[] = "/home/aryank1511/Desktop/testfile.txt";
  char newFile[] = "/home/aryank1511/Desktop/newfile.txt";

  // Attempt to open the input file in read-only mode
  int fd = open(inFile, O_RDONLY);
  if (fd < 0) {
    fprintf(stderr, "Error opening file: %s\n", inFile);
    // Print the error description using perror
    perror("open");
    return 1; // Exit with error if opening fails
  }

  // Prepare to open the output file with the appropriate flags and permissions
  int openFlags =
      O_CREAT | O_WRONLY | O_TRUNC; // Create, write-only, truncate if exists
  mode_t filePerms =
      S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH; // rw-rw-rw-

  // Open the output file with the specified flags and permissions
  int fdOut = open(newFile, openFlags, filePerms);
  if (fdOut < 0) {
    fprintf(stderr, "Error opening file: %s\n", newFile);
    perror("open");
    close(fd); // Close the input file before exiting
    return 1;  // Exit with error if opening fails
  }

  // Read from the input file and write to the output file
  do {
    numRead = read(fd, buf, BUFFER_SIZE); // Read data into buffer
    if (numRead < 0) {                    // Check for read errors
      perror("read");
      returnValue = 1; // Set return value to indicate error
      break;           // Exit the loop on error
    }

    // Write the buffer to standard output (file descriptor 1)
    numWrite = write(1, buf, numRead); // Write to standard output (Terminal)
    if (numWrite < 0) {                // Check for write errors
      perror("write to stdout");
      returnValue = 1; // Set return value to indicate error
      break;           // Exit the loop on error
    }

    // Write the buffer to the output file
    numWrite = write(fdOut, buf, numRead); // Write to the output file
    if (numWrite < 0) { // Check if write to output file was successful
      fprintf(
          stderr,
          "Could not write to the output file. numRead: %ld, numWrite: %ld\n",
          numRead, numWrite);
      perror("write to output file");
      returnValue = 1; // Set return value to indicate error
      break;           // Exit the loop on error
    }

  } while (numRead > 0); // Continue until no more bytes are read

  // Close file descriptors
  close(fd);    // Close the input file
  close(fdOut); // Close the output file
  puts("DONE"); // Indicate completion

  return returnValue; // Return 0 on success or 1 on error
}
