# Mid-Term Review

## 1) The History of Linux (for answers see IntroToLinux)

- a. Linux has its roots in which two operating systems?
- b. Is Linux a UNIX-derivative?
- c. How similar is Linux to UNIX?
- d. What is the Linux kernel?
- e. What is a Linux distribution?
- f. How is Linux superior to Windows?
- g. How is Windows superior to Linux?

## 2) The Linux File-System (for answers see LinuxFilesystem)

- a. What do you expect to see under the `/bin` directory?
- b. What do you expect to see under the `/dev` directory?
- c. What do you expect to see under the `/home` directory?
- d. What do you expect to see under the `/lib` directory?
- e. What do you expect to see under the `/proc` directory?
- f. What do you expect to see under the `/var` directory?
- g. What do you expect to see under the `/bin` directory?

## 3) Some common Linux commands

- a. What does `nm` do? (See nm)
- b. What does `ldd` do? (See ldd)
- c. What does `top` do? (See top)
- d. What does `netstat` do? (See netstat)
- e. What does `tail -f` do? (See tail)

## 4) Makefiles

- a. How do you create a Makefile for building an executable from the files `Math.cpp`, `General.cpp`, `Geometry.cpp`, and `Conversion.cpp`? (For the answer, see Makefile)
- b. How do you create a Makefile for building a static library `libMath.a` from the files `General.cpp`, `Geometry.cpp`, and `Conversions.cpp`? (For the answer, see Makefile)
- c. How do you create a Makefile that links in the static library `libMath.a` with its header file to build `MathTest` with the file `MathTest.cpp`? (For the answer, see Makefile)
- d. How do you create a Makefile for building a shared library `libMath.so` from the files `General.cpp`, `Geometry.cpp`, and `Conversions.cpp`? (For the answer, see Makefile)
- e. How do you create a Makefile that links in the static library `libMath.so` with its header file to build `MathTest` with the file `MathTest.cpp`? (For the answer, see Makefile)

## 5) Error Reporting

- a. The file `ErrorMsg.cpp` demonstrates `perror` and `strerror`. What is the difference between `perror` and `strerror`? (See perror and strerror).
- b. In the program `MiniMath.cpp`, what does the following print out?
  ```cpp
  cout << ErrMsg[Err_DivideByZero] << endl << endl; //Answer: “Divide by Zero”
  ```

```

## 6) File Input/Output

- a. Write a program that copies one file to another 16 bytes at a time using Linux system functions. (For the answer, see `SimpleFile.cpp`)
- b. Do the same as in part 5a but start 100 bytes into the input file. (For the answer, see `SeekFile.cpp`)
- c. Which is faster, Linux’s `open/read/write/close` or the C-library’s `fopen/fscanf/fprintf/fclose`? (Answer: it depends. The C-library buffers its reads and writes. If Linux reads and writes a large number of bytes at a time, Linux will be faster. If Linux reads and writes one byte at a time, the C-library will be faster)

## 7) IOCTL’s – Input/Output Control

- a. How can a program running in user-space communicate with the kernel space? (For the answer, see Controlling Hardware with ioctls)
- b. Write code that opens the device driver files `/dev/sda` and `/dev/sda1`, retrieves the disk parameters from each (`HDIO_GETGEO`) and retrieves the device size from each (`BLKGETSIZE`). For the disk parameters, print out the heads, sectors, cylinders, and start. For the device size, print out the block size. (For the answer, see `diskDrive.cpp`)

## 8) Device Drivers

- a. In the file `peripheralWriter.h` identify the following:
  - i. What are `_IOR` and `_IOW`?
- b. In the file `peripheralWriter.c`, identify the following:
  - i. What is the GPL license in `peripheralWriter.c`?
  - ii. Which functions register and unregister the device driver with the kernel?
  - iii. What is the purpose of the `file_operations` structure?
  - iv. What does `register_chrdev()` return?
  - v. What does `copy_to_user()` do?
  - vi. What does `copy_from_user()` do?

## 9) File Descriptors

- a. Write a program that redirects the standard error output channel to the file `Error.log`. (For the answer see `fileDup.cpp`)
- b. Which commands read from a file and writes to a file descriptor at a given offset? (Answer: see `pread` and `pwrite` and `offset.cpp`)
- c. Which commands can read or write an entire data structure to a file descriptor? (Answer: see `readv` and `writev` and `car.cpp`)

## 10) Child Creation (fork, exec)

- a. Describe the six memory segments of a process. (For the answer see `ProcessAddressSpace`).
- b. What command reports the memory map of a process? (Answer: `pmap`)
- c. What does `fork()` do as compared with `vfork()`? (See `ProcessCreationAndTermination`)
- d. What does `exit()` do as compared with `on_exit()`? (See `ProcessCreationAndTermination`)
- e. What does `wait()` do as compared with `waitpid()`? (See `ProcessCreationAndTermination`)
- f. What is the purpose of the `exec()` family of functions? (See `ProcessCreationAndTermination`)
- g. Write a simple program that creates a child. Both parent and child count from 0 to 9 seconds printing out their respective pid’s. (See `fork1.cpp`)
- h. Write a simple program that creates four children. Each child counts from 0 to 9. As each child terminates, the parent reports the pid of the child that terminates. (See `fork2.cpp`)
- i. Write a program that forks a child which execs `ls -al`. (See `exec1.cpp`)
- j. Explain how `on_exit` is used to clean up code in the files `openTest.cpp` and `fopenTest.cpp`.

## 11) Software Interrupts (signals)

- a. Explain what happens when a software interrupt is generated. What happens to the instructions that are being executed, where does the program counter go, when does it return. (For the answer, see `software_interrupt`)
  > FYI – A program counter is a register in a computer processor that contains the address (location) of the instruction being executed at the current time. As each instruction gets fetched, the program counter increases its stored value by 1.
- b. Explain the signals `SIGINT`, `SIGKILL`, `SIGUSR1`, `SIGUSR2`, `SIGTSTP`. (See `signals`)
- c. Which signal is generated by a ctrl-C? (Answer: `SIGINT`)
- d. Which signal is generated by a ctrl-Z? (Answer: `SIGTSTP`)
- e. Which signal will violently kill your program? (Answer: `SIGKILL`)
- f. Which command from the command line can be used to send a signal to a process? (Answer: `kill`)
- g. What do the following commands from the command line do? (For the answer, see Sending_signal_to_Processes)
  - `kill -9 1234`
  - `kill -KILL 1234`
  - `kill -SIGKILL 1234`
- h. How can you register a signal handler in your program? (Answer: see `signal` or `sigaction`)
- i. Write a program that handles the following signals: `SIGINT`, `SIGUSR1`, `SIGUSR2` using `signal`. (For the answer see `recvsig2.cpp`)
- j. Write a program that spawns 4 children, then shuts them down one by one with `SIGINT` (ctrl-C). (For the answer see `sigact.cpp`)
- k. Write a program that cannot be interrupted with a ctrl-C or ctrl-Z. (for the answer see `uninterrupt.cpp`)
- l. Write a program that forks and execs 2 child processes and synchronizes them via a signal. (See `sysmonExec.cpp` and `intfMonitor.cpp`) Try the same with `sigaction`.
```
