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

## `mode_t` and File Permissions

In Unix-like operating systems, file permissions are controlled through a data type called `mode_t`. This data type defines the access rights and types of files or directories. The `mode_t` type is typically used in functions that deal with file creation, manipulation, and permission settings, such as `open()`, `chmod()`, and `stat()`.

### Overview of `mode_t`

- **Data Type**: `mode_t` is an unsigned integer that encodes the file permissions and file type information.

- **File Permission Bits**: Each permission bit represents a specific type of access for a file or directory. The permissions are usually divided into three categories: **user**, **group**, and **others**.

### Permission Bits

1. **User Permissions** (Owner)

   - **Read (`S_IRUSR`, value: 0400)**: Permission to read the file.
   - **Write (`S_IWUSR`, value: 0200)**: Permission to write to the file.
   - **Execute (`S_IXUSR`, value: 0100)**: Permission to execute the file (if it's executable).

2. **Group Permissions**

   - **Read (`S_IRGRP`, value: 0040)**: Permission to read the file for users in the file's group.
   - **Write (`S_IWGRP`, value: 0020)**: Permission to write to the file for users in the file's group.
   - **Execute (`S_IXGRP`, value: 0010)**: Permission to execute the file for users in the file's group.

3. **Others Permissions**
   - **Read (`S_IROTH`, value: 0004)**: Permission to read the file for all other users.
   - **Write (`S_IWOTH`, value: 0002)**: Permission to write to the file for all other users.
   - **Execute (`S_IXOTH`, value: 0001)**: Permission to execute the file for all other users.

### File Type Bits

In addition to permission bits, `mode_t` can also indicate the type of the file:

- **Regular File (`S_IFREG`, value: 0100000)**: A regular file.
- **Directory (`S_IFDIR`, value: 0040000)**: A directory.
- **Character Device (`S_IFCHR`, value: 0020000)**: A character device file.
- **Block Device (`S_IFBLK`, value: 0060000)**: A block device file.
- **FIFO (named pipe) (`S_IFIFO`, value: 0010000)**: A FIFO file.
- **Socket (`S_IFSOCK`, value: 0140000)**: A socket file.
- **Symbolic Link (`S_IFLNK`, value: 0120000)**: A symbolic link.

### Setting Permissions

File permissions can be set using the `chmod` command or programmatically using the `chmod()` function. The permissions can be specified using symbolic (e.g., `u+rwx`) or numeric (octal) representations.

#### Symbolic Representation

- **Add permission**: Use `+`
- **Remove permission**: Use `-`
- **Set permission explicitly**: Use `=`

For example:

- `chmod u+x file.txt` adds execute permission for the user.
- `chmod g-w file.txt` removes write permission for the group.
- `chmod o=r file.txt` sets read permission for others only.

#### Numeric (Octal) Representation

Permissions can also be represented in octal format, where each digit corresponds to a set of permissions:

- Example: `chmod 755 file.txt`
  - `7` (user): `rwx` (read, write, execute)
  - `5` (group): `r-x` (read and execute)
  - `5` (others): `r-x` (read and execute)

### Checking Permissions

You can check a file's permissions using the `ls -l` command in the terminal. The output will show the permissions in the first column, indicating the file type and access rights:

```bash
-rwxr-xr-- 1 user group 4096 Oct 14 12:34 file.txt
```

- The first character indicates the file type (`-` for regular file, `d` for directory, etc.).
- The next three characters indicate the user permissions, followed by group and others.

### Example of Using `mode_t` in C

Here’s a simple example of how to create a file with specific permissions using the `open()` system call:

```c
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    const char *filename = "example.txt";

    // Create a file with read and write permissions for the user, and read permission for group and others
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    int fd = open(filename, O_CREAT | O_WRONLY, mode);
    if (fd == -1) {
        perror("Error creating file");
        return 1;
    }

    // Write some data to the file (optional)
    const char *data = "Hello, World!";
    write(fd, data, sizeof(data));

    // Close the file
    close(fd);
    return 0;
}
```

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

## Overview of `ioctl`

- The `ioctl` system call is used to manipulate the underlying device parameters of special files. It provides a means of sending control commands to devices and modifying their behavior.

- **Prototype**: The basic syntax for `ioctl` in C is:

  ```c
  int ioctl(int fd, unsigned long request, ...);
  ```

  - `fd`: A file descriptor for the device or file.
  - `request`: An operation request code that specifies the action to be performed.
  - `...`: Optional arguments (depends on the request).

1. **File Descriptor**:

   - Before using `ioctl`, you need to open the device file (e.g., `/dev/something`) using the `open` system call, which returns a file descriptor.

2. **Request Codes**:

   - Each request code is defined by the driver and often follows a naming convention (e.g., `MY_IOCTL_CMD`). Common requests include:
     - `TIOCGWINSZ`: Get window size for terminal.
     - `TIOCSPGRP`: Set foreground process group for terminal.
     - `FIONREAD`: Get the number of bytes available for reading.
   - Request codes are usually defined in header files (e.g., `<linux/ioctl.h>`).
   - Read the [documentation](https://man7.org/linux/man-pages/man7/netdevice.7.html) to find the appropriate requests.

3. **Data Types**:

   - The third argument may require a pointer to a structure or variable to pass data between user space and kernel space. This can include:

     - Pointers to structures for retrieving or setting multiple values.
     - Simple data types (like integers) for single values.

   - You will have to read the docs to understand what to put here.

### Usage

1. **Opening a Device**:

   ```c
   int fd = open("/dev/mydevice", O_RDWR);
   if (fd < 0) {
       perror("Failed to open device");
       return -1;
   }
   ```

2. **Using `ioctl`**:

   ```c
   int result;
   struct winsize w;
   result = ioctl(fd, TIOCGWINSZ, &w); // Get terminal size
   if (result < 0) {
       perror("ioctl failed");
   }
   printf("Rows: %d, Columns: %d\n", w.ws_row, w.ws_col);
   ```

3. **Closing a Device**:

   ```c
   close(fd);
   ```

### Common Use Cases

- **Device Control**: Adjust settings on devices (e.g., baud rates for serial ports).
- **Getting/Setting Parameters**: Fetching the status of devices (e.g., reading sensor data).
- **Communication**: Sending custom commands to the device for specific tasks.

### Error Handling

- On failure, `ioctl` returns `-1` and sets the global variable `errno` to indicate the error. Common error codes include:
  - `EINVAL`: Invalid request code.
  - `ENOTTY`: Not a typewriter; the file descriptor is not associated with a terminal.
  - `ENOTSUP`: Operation not supported.

### Notes on Implementation

- `ioctl` can be considered less portable because the request codes and expected parameters depend on the device driver implementation.
- It's crucial to check the documentation or source code of the specific device driver for the correct usage of `ioctl` requests.

### Summary

- **`ioctl`** is a powerful system call used for device control and configuration.
- It operates on file descriptors and allows interaction with device drivers through custom request codes.
- Proper error handling is essential to manage device communication effectively.

### Example Code Snippet

Here’s a simple example demonstrating how to use `ioctl` to get the window size of a terminal:

```c
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/tty.h>

int main() {
    int fd = open("/dev/tty", O_RDWR); // Open terminal device
    if (fd < 0) {
        perror("Failed to open terminal");
        return EXIT_FAILURE;
    }

    struct winsize w;
    if (ioctl(fd, TIOCGWINSZ, &w) == -1) { // Get window size
        perror("ioctl failed");
        close(fd);
        return EXIT_FAILURE;
    }

    printf("Rows: %d, Columns: %d\n", w.ws_row, w.ws_col);
    close(fd); // Close the device
    return EXIT_SUCCESS;
}
```
