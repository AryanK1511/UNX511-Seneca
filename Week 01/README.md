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
