# Week 01 Notes

## Introduction to Linux

**Linux is an operating system as well as a Kernel.**

- Linux has its roots in Unix and Multics.
- Linux is not a UNIX decorative. It was written from scratch.

> An **operating system** is simply a collection of software that manages hardware resources and provides an environment where applications can run. The operating system allows applications to store information, send documents to printers, interact with users and other things.

> A **kernel** is the lowest level of easily replaceable software that interfaces with the hardware in your computer. It is responsible for interfacing all of your applications that are running in “user mode” down to the physical hardware, and allowing processes, known as servers, to get information from each other using inter-process communication (IPC).

- Linux distributions take the Linux kernel and combine it with other free software to create complete packages.
  - Debian
  - Ubuntu
  - Fedora
  - CentOS / Red Hat Enterprise Linux (RHEL)
  - openSUSE / SUSE Linux Enterprise
  - Arch Linux

| Aspect                     | Windows                                                                         | Linux                                                                                  |
| -------------------------- | ------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------- |
| **Version Structure**      | Standard versions (e.g., Windows 7, 8, 10)                                      | Numerous distributions (distros) with variations                                       |
| **Installation**           | Lengthy but simple process                                                      | Live booting possible; installation can be complex                                     |
| **Software Compatibility** | Designed for Windows; limited Linux versions                                    | Many free/open source alternatives; use of WINE or VMs for Windows software            |
| **Software Installation**  | Executable files (.exe)                                                         | Software repositories and package managers; some manual installation required          |
| **Support**                | Comprehensive, centralized support from Microsoft; well-organized documentation | Extensive but fragmented community support; relies on forums and tutorials             |
| **Security**               | Full administrator access by default; more common target for malware            | Privileged access control; open source code reviewed by many; less targeted by malware |
| **Performance**            | Can be bloated and sluggish                                                     | Generally faster and less demanding; good for older hardware                           |
| **User-Friendliness**      | Familiar and intuitive; widely used                                             | Can be complex for new users; becomes easier with experience                           |
| **Verdict**                | Convenient, compatible, and familiar; ideal for busy environments               | Flexible, cost-effective, and secure; better for those willing to adapt                |

## What is a Makefile?

A **Makefile** is a special file used by the `make` tool to automate the process of building and managing software projects. It helps streamline and simplify tasks that you might need to repeat often, such as compiling code or generating documentation.

### Why Use a Makefile?

1. **Automation**: A Makefile automates repetitive tasks, so you don’t have to type out complex commands each time you want to build your project.
2. **Consistency**: It ensures that the build process is consistent across different machines or environments.
3. **Efficiency**: It only re-executes tasks that are necessary, which can save time and resources.

#### Basic Structure of a Makefile

A Makefile consists of **rules** that describe how to build a target (like an executable or a file) from source files. Each rule typically looks like this:

```makefile
target: dependencies
    command
```

- **target**: The file or action you want to create or perform (e.g., a compiled executable).
- **dependencies**: The files that the target depends on (e.g., source code files).
- **command**: The shell commands that `make` will run to build the target (e.g., `gcc -o myprogram main.c`).

You can view an example [here](./Makefile).

#### Using a Makefile

To use a Makefile, you typically run the command:

```bash
make
```

This will build the default target, which is usually the first one listed in the Makefile. You can also specify a target explicitly:

If I do not use `make`, this is what I would have to normally run.

```bash
g++ Message.cpp main.cpp -o main
```

Additionally,

```bash
make clean
```

This will execute the `clean` rule, which removes intermediate files and the final executable.

## Linux file system

- `/bin` is the directory that contains binaries, that is, some of the applications and programs you can run.
- The `/boot` directory contains files required for starting your system.
- `/dev` contains device files. Many of these are generated at boot time or even on the fly. For example, if you plug in a new webcam or a USB pendrive into your machine, a new device entry will automagically pop up here.
- `/etc` is the directory where names start to get confusing. /etc gets its name from the earliest Unixes and it was literally "et cetera" because it was the dumping ground for system files administrators were not sure where else to put. Nowadays, it would be more appropriate to say that etc stands for "Everything to configure," as it contains most, if not all system-wide configuration files.
- `/lib` is where libraries live. Libraries are files containing code that your applications can use.
- The `/media` directory is where external storage will be automatically mounted when you plug it in and try to access it.
- The `/mnt` directory is where you would manually mount storage devices or partitions.
- The `/opt` directory is often where software you compile (that is, you build yourself from source code and do not install from your distribution repositories) sometimes lands. Applications will end up in the `/opt/bin` directory and libraries in the `/opt/lib` directory. A slight digression: another place where applications and libraries end up in is `/usr/local`.
- `/proc`, like /dev is a virtual directory. It contains information about your computer, such as information about your CPU and the kernel your Linux system is running.
- `/root` is the home directory of the superuser (also known as the "Administrator") of the system.
- `/run` is another new directory. System processes use it to store temporary data for their own nefarious reasons.
- `/sbin` is similar to `/bin`, but it contains applications that only the superuser (hence the initial s) will need. You can use these applications with the sudo command that temporarily concedes you superuser powers on many distributions.
- The `/usr` directory was where users' home directories were originally kept back in the early days of UNIX. However, now it is `/home`.
  - These days, `/usr` contains a mish-mash of directories which in turn contain applications, libraries, documentation, wallpapers, icons and a long list of other stuff that need to be shared by applications and services.
  - You will also find `bin`, `sbin` and `lib` directories in `/usr`. Originally, the `/bin` directory (hanging off of root) would contain very basic commands, like ls, mv and rm; the kind of commands that would come pre-installed in all UNIX/Linux installations, the bare minimum to run and maintain a system.
  - `/usr/bin` on the other hand would contain stuff the users would install and run to use the system as a work station, things like word processors, web browsers, and other apps.
  - But many modern Linux distributions just put everything into `/usr/bin` and have `/bin` point to `/usr/bin` just in case erasing it completely would break something.
  - So, while Debian, Ubuntu and Mint still keep `/bin` and `/usr/bin` (and `/sbin` and `/usr/sbin`) separate; others, like Arch and its derivatives just have one "real" directory for binaries, /usr/bin, and the rest or \*bins are "fake" directories that point to /usr/bin.
- The `/srv` directory contains data for servers. If you are running a web server from your Linux box, your HTML files for your sites would go into /srv/http (or /srv/www). If you were running an FTP server, your files would go into /srv/ftp.
- `/sys` is another virtual directory like `/proc` and `/dev` and also contains information from devices connected to your computer.
- `/tmp` contains temporary files, usually placed there by applications that you are running. The files and directories often (not always) contain data that an application doesn't need right now, but may need later on.
- `/var` contains things like logs in the /var/log subdirectories. Logs are files that register events that happen on the system. If something fails in the kernel, it will be logged in a file in /var/log; if someone tries to break into your computer from outside, your firewall will also log the attempt here. It also contains spools for tasks. These "tasks" can be the jobs you send to a shared printer when you have to wait because another user is printing a long document, or mail that is waiting to be delivered to users on the system.

## Linux command to check processes

All the directories under the `/proc` directory that start with a digit represent a process.

```bash
ps aux
```

Run the command above to see all the processes that are running.

## Makefile guide

You can find the guide [here](./MakefileCreation.pdf).

Here is an example of a `Makefile` used to create a static library.

```Makefile
CC=g++

# CC is just a variable name.
# It's common practice to name it 'CC', which stands for C Compiler.
# Here, we're assigning the C++ compiler `g++` to the CC variable.

CFLAGS=-I

# The ‘-I’ flag specifies the include path for headers.

CFLAGS+=-Wall

# ‘-Wall’ stands for ‘warnings all` and enables all the compiler's warning messages.

CFLAGS+=-c

# The ‘ -c’ flag stands for ‘compile only’.
# This tells the compiler to compile the source file into an object file(.o).
# An object file is a file that contains machine code generated by a compiler from a source file.
# However, it is not a complete executable yet because it hasn't been linked with other object files or libraries.

AR=ar

# Here, ‘ar’ is the archiver tool for creating static libraries.
# An archiver is used to bundle multiple object files into a single static library.
# This static library can then be linked into other programs during the compilation process.

pidUtil: pidUtil.cpp
    $(CC) $(CFLAGS) pidUtil.cpp -o pidUtil.o

# This uses the `g++` compiler (referenced by `$(CC)`) and the flags `$(CFLAGS)` to compile 'pidUtil.cpp' into an object file 'pidUtil.o'.
# The `-o` flag specifies the output file name, which is 'pidUtil.o'.

lib: pidUtil.o
    $(AR) rcs libPidUtil.a pidUtil.o

# Creates a static library 'libPidUtil.a' from the object file 'pidUtil.o'
# The `rcs` flags mean:
# • ‘r’ : Replace or add files to the archive
# • ‘c’: Create the archive if it doesn't already exist
# • ‘s’: Add an index to the archive, improving lookup times when linking

clean:
    rm -f *.o *.a

# Cleans up object files and static libraries. The ‘rm -f’ command deletes all object files (*.o) and archive files (*.a) in the directory.
# '-f' forces the removal without asking for confirmation, even if files don't exist.

install:
    cp libPidUtil.a ../.
    cp pidUtil.h ../.

# Installs the library and header file to a parent directory.
# The ‘cp’ command copies the static library 'libPidUtil.a' and the header file 'pidUtil.h' to the parent directory.

all: pidUtil lib

# This runs both the `pidUtil` and `lib` targets sequentially,
# ensuring that the object file is compiled before the static library is created.
```
