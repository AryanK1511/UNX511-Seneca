# Week 03 Notes

A **file descriptor** is a low-level programming concept used in Unix-like operating systems (including Linux) to uniquely identify an open file or other input/output resources (such as sockets or pipes). It acts as an index to an entry in a file descriptor table maintained by the operating system.

- A file descriptor is an integer that represents an open file or resource in the operating system.
- It allows programs to perform input and output operations on files and other resources using standardized system calls instead of having to use the `stdio` C library.

- Every process in a Unix-like system has three default file descriptors:

  - **0**: Standard Input (stdin)
  - **1**: Standard Output (stdout)
  - **2**: Standard Error (stderr)

- When a file is opened, the operating system allocates a file descriptor and maintains a file descriptor table for each process.
- The table contains entries for each open file, including the file's location, mode (read/write), and other attributes.
- Every process has its own separate file descriptor table. This means that the same file descriptor number (e.g., FD 3) in different processes can refer to entirely different files or resources.

  - Process A could have FD 3 referring to /home/user/fileA.txt.
  - Process B could have FD 3 referring to /var/log/syslog.

- File descriptors are used with various system calls to perform operations like reading, writing, and closing files:

  - **open()**: Opens a file and returns its file descriptor.
  - **read()**: Reads data from a file using its file descriptor.
  - **write()**: Writes data to a file using its file descriptor.
  - **close()**: Closes the file descriptor, releasing the resource.

  ```c
  #include <stdio.h>
  #include <unistd.h>
  #include <fcntl.h>

  int main() {
      int fd;
      char buffer[100];

      // Open a file
      fd = open("example.txt", O_RDONLY); // O_RDONLY for read-only mode
      if (fd == -1) {
          perror("Error opening file");
          return 1;
      }

      // Read from the file
      ssize_t bytesRead = read(fd, buffer, sizeof(buffer) - 1);
      if (bytesRead == -1) {
          perror("Error reading file");
          close(fd);
          return 1;
      }
      buffer[bytesRead] = '\0'; // Null-terminate the string

      // Print the read data
      printf("Data read from file: %s\n", buffer);

      // Close the file
      close(fd);
      return 0;
  }
  ```

- When a file is opened in a Linux process using the open() system call, a new file descriptor (FD) is created specifically for that process. Each process has its own file descriptor table, so even if the same file is opened by multiple processes, each process gets a unique FD. Running a program creates a new process with its own FDs, and even if the file is already open in another process, calling open() will generate a new FD in the new process. Processes can share access to the same file, but their file descriptors are independent of each other.

- File descriptors are reusable. Once a file descriptor is closed, the same integer can be reused for another open file or resource.

- There is a limit to the number of file descriptors that a process can have open at any time, which is typically controlled by system parameters. You can check and modify these limits using commands like `ulimit`.

  - To check the current limit of open file descriptors for your shell session (soft limit), run:

  ```bash
  ulimit -n
  ```

  This will display the maximum number of file descriptors your shell can open at once.

  - **Increasing the Limit**

    - **Temporarily** (for the current shell session):
      You can increase the limit for the current shell session by specifying a higher value:

    ```bash
    ulimit -n 4096
    ```

    This will increase the limit to 4096 file descriptors for the current session. However, this change will not persist after you log out or restart the session.

    - **Permanently** (system-wide or per-user):
      To set a higher limit permanently, you need to modify system configuration files.

    - **Edit `/etc/security/limits.conf`**:
      Add lines like the following to set limits for specific users or groups:

      ```bash
      <username> soft nofile 4096
      <username> hard nofile 8192
      ```

      - `soft nofile`: The soft limit, which can be modified by the user.
      - `hard nofile`: The hard limit, which can only be modified by the system administrator.

    - **Edit `/etc/pam.d/common-session` (for some systems)**:
      Add the following line if not present, to ensure that the changes in `limits.conf` take effect:

      ```bash
      session required pam_limits.so
      ```

    - **Reboot or re-login**: After making changes to the configuration files, you may need to log out and back in or reboot the system for the changes to take effect.

  - Checking System-Wide Limits

  To check the system-wide limit for all processes, you can check `/proc/sys/fs/file-max`:

  ```bash
  cat /proc/sys/fs/file-max
  ```

  This value represents the maximum number of file descriptors that the system can allocate across all processes.

## Universality of system calls

The universality of system calls refers to the concept that certain system calls in an operating system provide a consistent and standardized interface for interacting with the underlying hardware and system resources. This concept is particularly prominent in Unix-like operating systems, where system calls serve as the primary means for user-level applications to request services from the kernel, which operates in a privileged mode.

## How to check find file descriptors

### 1. **Using the `ls` command**

You can list the file descriptors of a specific process by navigating to the `/proc/[PID]/fd` directory, where `[PID]` is the process ID of the target process:

```bash
ls -l /proc/[PID]/fd
```

This command will display a list of file descriptors associated with the specified process, along with symbolic links showing which files they point to.

### 2. **Using the `lsof` command**

The `lsof` (List Open Files) command is another powerful tool to display all open files and their corresponding file descriptors for all processes or a specific one:

To list all open files for all processes:

```bash
lsof
```

To filter by a specific process, use its PID:

```bash
lsof -p [PID]
```

### 3. **Using the `/proc` filesystem**

You can also get a more detailed view of the process's file descriptors by examining the `/proc/[PID]/fdinfo/[FD]` files, which give additional information about each file descriptor.

Example:

```bash
cat /proc/[PID]/fdinfo/[FD]
```

## Common File Access Modes and Flags

Access modes refer to the ways in which a program can interact with files or devices in a file system. These modes determine how the program can read from or write to the file, as well as whether the file can be executed.

| Access Mode   | Description                                                                                                                                                  |
| ------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `O_RDONLY`    | Open for reading only                                                                                                                                        |
| `O_WRONLY`    | Open for writing only                                                                                                                                        |
| `O_RDWR`      | Open for reading and writing                                                                                                                                 |
| `O_CREAT`     | Create file if it does not exist                                                                                                                             |
| `O_TRUNC`     | Truncate file to zero length                                                                                                                                 |
| `O_APPEND`    | Write data to the end of the file                                                                                                                            |
| `O_NONBLOCK`  | Open in non-blocking mode                                                                                                                                    |
| `O_EXCL`      | Fail if the file already exists. This also means that only the process that has this can access the file. If the file was created before, it wont be opened. |
| `O_SYNC`      | Write synchronously                                                                                                                                          |
| `O_DIRECTORY` | File descriptor refers to a directory                                                                                                                        |

## `size_t` vs `ssize_t`

| Feature        | `size_t`                       | `ssize_t`                     |
| -------------- | ------------------------------ | ----------------------------- |
| Sign           | Unsigned                       | Signed                        |
| Range          | Non-negative values only       | Negative and positive values  |
| Usage          | Size of objects, array indices | I/O operations (size, errors) |
| Typical Header | `<stddef.h>`                   | `<unistd.h>`                  |

## Changing the offset

Changing the offset in a file or data stream typically involves seeking to a specific position before reading from or writing to that location. In C, this is often done using the `lseek()` function for file descriptors or `fseek()` for file pointers. Here are some notes and examples on how to change the offset effectively:

### 1. Using `lseek()`

`lseek()` is used to reposition the file offset of the open file associated with the file descriptor.

- **Syntax**:

  ```c
  off_t lseek(int fd, off_t offset, int whence);
  ```

  - **Parameters**:
    - `fd`: The file descriptor of the open file.
    - `offset`: The number of bytes to move the file pointer. This can be positive or negative.
    - `whence`: Specifies the reference point for the offset. It can be one of the following:
      - `SEEK_SET`: Set the offset to `offset` bytes from the beginning of the file.
      - `SEEK_CUR`: Set the offset to its current location plus `offset`.
      - `SEEK_END`: Set the offset to the size of the file plus `offset`.

- **Return Value**: On success, `lseek()` returns the resulting offset location measured in bytes from the beginning of the file. On failure, it returns `-1` and sets `errno`.

- **Example**:

  ```c
  #include <stdio.h>
  #include <unistd.h>
  #include <fcntl.h>
  #include <errno.h>

  int main() {
      int fd = open("example.txt", O_RDONLY);
      if (fd == -1) {
          perror("Failed to open file");
          return 1;
      }

      // Move to the 10th byte from the start
      off_t newOffset = lseek(fd, 10, SEEK_SET);
      if (newOffset == -1) {
          perror("lseek failed");
          close(fd);
          return 1;
      }

      printf("New offset: %lld\n", (long long)newOffset);

      // Read from the new offset
      char buffer[20];
      ssize_t bytesRead = read(fd, buffer, sizeof(buffer) - 1);
      if (bytesRead == -1) {
          perror("Failed to read from file");
          close(fd);
          return 1;
      }
      buffer[bytesRead] = '\0'; // Null-terminate the string
      printf("Data read: %s\n", buffer);

      close(fd);
      return 0;
  }
  ```

#### 2. Using `fseek()`

- **Purpose**: `fseek()` is used to set the position of the next read or write operation on a file stream.

- **Syntax**:

  ```c
  int fseek(FILE *stream, long offset, int whence);
  ```

  - **Parameters**:
    - `stream`: Pointer to a `FILE` object that identifies the stream.
    - `offset`: The number of bytes to move the file pointer.
    - `whence`: Similar to `lseek()`, it specifies the reference point:
      - `SEEK_SET`: Beginning of the file.
      - `SEEK_CUR`: Current position.
      - `SEEK_END`: End of the file.

- **Return Value**: On success, `fseek()` returns `0`. On failure, it returns a non-zero value.

- **Example**:

  ```c
  #include <stdio.h>

  int main() {
      FILE *file = fopen("example.txt", "r");
      if (file == NULL) {
          perror("Failed to open file");
          return 1;
      }

      // Move to the 10th byte from the start
      if (fseek(file, 10, SEEK_SET) != 0) {
          perror("fseek failed");
          fclose(file);
          return 1;
      }

      // Read from the new offset
      char buffer[20];
      size_t bytesRead = fread(buffer, 1, sizeof(buffer) - 1, file);
      buffer[bytesRead] = '\0'; // Null-terminate the string
      printf("Data read: %s\n", buffer);

      fclose(file);
      return 0;
  }
  ```

### Key Points

- **Seek Operations**:

  - Use `lseek()` with file descriptors for lower-level file operations (typically with system calls). Used with Linux system calls.
  - Use `fseek()` with file pointers for higher-level I/O operations in C standard library functions. Used with file pointers / streams when you are using standard C library.

- **Error Handling**:

  - Always check for errors after calling `lseek()` or `fseek()`. Use `perror()` or check `errno` to diagnose issues.

- **Offset Calculation**:
  - Be mindful of the file size when changing offsets, especially when using `SEEK_END` to avoid moving beyond the file bounds.
  - When moving backward with negative offsets, ensure the current position is greater than the absolute value of the offset.

## Points to note

- Always close unneeded file descriptors explicitly.

  ```c
  if (close(fd) == -1)
      errExit("close");
  ```

Your `gdb` and `Valgrind` usage guide is quite comprehensive! Here’s a brief summary and a few additional tips that could enhance the guide:

## GDB and Valgrind Usage Guide

### GDB (GNU Debugger)

`gdb` is a debugger for programs written in languages like C and C++. It helps you track program execution, inspect and modify variables at runtime, and diagnose errors.

1. **Compile for Debugging**:

   ```bash
   gcc -g myprogram.c -o myprogram
   ```

2. **Start `gdb`**:

   ```bash
   gdb ./myprogram
   ```

3. **Run the Program**:

   ```bash
   (gdb) run [args]
   ```

4. **Set Breakpoints**:

   ```bash
   (gdb) break main
   (gdb) break filename.c:42
   ```

5. **Continue Execution**:

   ```bash
   (gdb) continue
   ```

6. **Inspect Variables**:

   ```bash
   (gdb) print var_name
   (gdb) info locals
   ```

7. **Step Through Code**:

   ```bash
   (gdb) step  # Step into functions
   (gdb) next  # Step over functions
   ```

8. **Inspect Call Stack**:

   ```bash
   (gdb) backtrace
   (gdb) frame n
   ```

9. **Examine Memory**:

   ```bash
   (gdb) x address
   ```

10. **Modify Variables**:

    ```bash
    (gdb) set var var_name = value
    ```

11. **Analyze Crashes (Core Dumps)**:

    ```bash
    gdb ./myprogram core
    ```

12. **Exit `gdb`**:

    ```bash
    (gdb) quit
    ```

### Valgrind

Valgrind is a tool for detecting memory management issues in C/C++ programs, such as memory leaks and invalid memory access.

1. **Compile Your Program**:

   ```bash
   gcc -g myprogram.c -o myprogram
   ```

2. **Run Your Program with Valgrind**:

   ```bash
   valgrind ./myprogram
   ```

3. **Interpreting Output**:

   Look for invalid reads/writes, memory leaks, and uninitialized memory usage.

4. **Memory Leak Detection**:

   ```bash
   valgrind --leak-check=full ./myprogram
   ```

#### Key Valgrind Tools

1. **Memcheck**: Detects memory leaks and invalid accesses.

   ```bash
   valgrind ./myprogram
   ```

2. **Callgrind**: Profiles program performance.

   ```bash
   valgrind --tool=callgrind ./myprogram
   ```

3. **Helgrind**: Detects threading issues.

   ```bash
   valgrind --tool=helgrind ./myprogram
   ```

4. **Massif**: Monitors memory usage.

   ```bash
   valgrind --tool=massif ./myprogram
   ```

#### Important Valgrind Options

- **`--leak-check=full`**: Detailed memory leak reports.
- **`--track-origins=yes`**: Identify uninitialized memory sources.
- **`--log-file`**: Redirect output to a file.
