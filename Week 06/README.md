# Week 06 Notes

## fork(), exit(), wait(), execve()

These are system calls in Unix-like operating systems that deal with process creation, termination, synchronization, and execution. Here's a brief explanation of each:

1. **`fork()`**:

   - The `fork()` system call is used to create a new process. It creates a child process that runs as a copy of the parent process.
   - After a `fork()`, both the parent and the child processes continue executing from the point where the fork occurred.
   - Returns:
     - `0` in the child process
     - The child's Process ID (PID) in the parent process
     - `-1` if the fork failed

   Example:

   ```c
   pid_t pid = fork();
   if (pid == 0) {
       // Child process
   } else if (pid > 0) {
       // Parent process
   } else {
       // Fork failed
   }
   ```

2. **`exit()`**:

   - The `exit()` system call terminates a process. When a process calls `exit()`, it ends, and its resources are released.
   - It takes an exit status code, which can be retrieved by its parent using `wait()`.
   - Example: `exit(0);` where `0` typically indicates successful termination.

3. **`wait()`**:

   - The `wait()` system call makes a parent process wait until any of its child processes have terminated.
   - It retrieves the exit status of the terminated child and prevents zombie processes (processes that have terminated but have not yet been waited on).
   - Example:

   ```c
   int status;
   pid_t pid = wait(&status);
   ```

4. **`execve()`**:

   - `execve()` is part of the `exec` family of functions that replaces the current process image with a new program.
   - Unlike `fork()`, which creates a new process, `execve()` loads a new program into the current process's memory and starts it.
   - Example:

   ```c
   char *argv[] = { "/bin/ls", "-l", NULL };
   execve("/bin/ls", argv, NULL);
   ```

These system calls are often used together in scenarios where a parent process creates child processes, waits for them to finish, and the child processes might execute different programs.

## Multithreading and forking

### Similarities

- **Concurrency:** Both in `fork()` and multithreading, multiple tasks (in this case, parent and child processes) appear to execute concurrently, meaning the execution of one can interleave with the execution of the other, as managed by the operating system's scheduler.
- **Interleaved Output:** Just like in multithreading where multiple threads can run at the same time and interleave their outputs or operations, in `fork()`, both the parent and child processes can interleave their outputs.

### Differences

- **Separate Processes vs. Shared Memory:**

  - In `fork()`, the parent and child processes are separate, each having its own **memory space**. When the child is created, it gets a **copy of the parent's memory**. Changes made in one process (like modifying variables) do **not affect the other**.
  - In multithreading, threads within the same process share the **same memory space**. This means that variables, resources, and memory are **shared** between threads, and changes made by one thread can directly affect others.

- **Communication:**

  - Processes created via `fork()` need inter-process communication (IPC) mechanisms (e.g., pipes, message queues, shared memory) to communicate or share data between the parent and child.
  - Threads, on the other hand, can directly share data because they run in the same memory space.

- **Overhead:**

  - Forking a new process (with `fork()`) is generally more resource-intensive because the OS needs to duplicate the process and allocate separate memory.
  - Creating a new thread in a multithreaded program typically has lower overhead because threads share the same memory and resources.

- The behavior you observe (concurrent execution) is similar to multithreading, but `fork()` deals with **separate processes** that don't share memory, while multithreading deals with **multiple threads** within a single process that share the same memory space.

## Comapring different approaches

- [`fork1.cpp`](./examples/forking/fork1.cpp)
- [`fork2.cpp`](./examples/forking/fork2.cpp)

### **First File: Single Child Process (`fork1.cpp`)**

```cpp
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
  int data = 1;
  pid_t childPid;

  std::cout << "\npid:" << getpid() << " Hello World" << std::endl;
  childPid = fork();

  if (childPid == 0) { // the child
    std::cout << "Child: pid:" << getpid() << std::endl;
    for (int i = 0; i < 10; ++i) {
      sleep(1);
      std::cout << "Child [" << getpid() << "]: i:" << i << " data:" << data << std::endl;
    }
  } else { // the parent
    std::cout << "Parent: pid:" << getpid() << std::endl;
    for (int i = 0; i < 10; ++i) {
      sleep(1);
      ++data;
      std::cout << "Parent [" << getpid() << "]: i:" << i << " data:" << data << std::endl;
    }
  }

  std::cout << "pid:" << getpid() << " This is a common area" << std::endl;
  std::cout << "pid:" << getpid() << " Finished!" << std::endl;

  return 0;
}
```

#### Key Features of the First File

1. **Single `fork()` Call**:

   - This file creates **one child process** with a single `fork()`.
   - After the `fork()`, the parent and child both execute their respective branches of the `if` statement.

2. **Output**:

   - Both the parent and child processes run a loop for 10 iterations, printing their respective process IDs (`getpid()`) and the value of a shared variable (`data`).
   - The child process doesn’t modify the variable `data` but prints it in its loop, while the parent process increments it in each iteration (`++data`).
   - The output you're seeing from the fork() execution shows that the parent and child processes run concurrently, and there's no strict or guaranteed order in which their outputs are printed. The order of execution can vary depending on the operating system’s process scheduling.

3. **Common Area**:

   - After completing their loops, both the parent and child process print "This is a common area" and "Finished!"—illustrating that code after the `fork()` is executed by both processes.

4. **Concurrency**:
   - The parent and child processes run concurrently, and because of the `sleep(1)` calls, their output interleaves (though their exact execution timing is up to the OS).

#### Behavior

- **Process Separation**: Both the parent and child processes operate independently after the `fork()`, with separate memory spaces. Thus, changes in `data` in the parent don’t affect the child's `data`.
- **Single Fork**: Only two processes exist in total—the original parent and the one child created by the `fork()`.

Yes, the behavior you're seeing with `fork()` in terms of interleaving output from the parent and child processes is **similar to multithreading**, but there are some key differences:

### **Second File: Multiple Child Processes (`fork1.cpp`)**

```cpp
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

const int NO_OF_CHILDREN = 4;

int main() {
  pid_t childId;
  bool isParent = true;
  int childNo;

  std::cout << "\nparent [" << getpid() << "]: BEGIN" << std::endl << std::endl;

  for (childNo = 0; childNo < NO_OF_CHILDREN && isParent; ++childNo) {
    childId = fork();
    sleep(1);
    if (childId == 0)
      isParent = false;
  }

  if (isParent) {
    int status;
    pid_t pid = 0;
    while (pid >= 0) {
      pid = wait(&status);
      status = status >> 8; // Bits 8-15
      std::cout << "pid:" << pid << std::endl;
      if (pid >= 0)
        std::cout << "parent [" << getpid() << "]: child [" << pid << "] " << status << " has terminated." << std::endl;
      else
        std::cout << "parent [" << getpid() << "]: no more children." << std::endl;
    }
  } else {
    for (int data = 1; data <= 10; ++data) {
      sleep(1);
      std::cout << "child " << std::to_string(childNo) << "[" << getpid() << "]: data:" << data << std::endl;
    }
  }

  std::cout << "[" << getpid() << "]:  This is a common area" << std::endl;
  std::cout << "[" << getpid() << "]:  Finished!" << std::endl;

  if (!isParent)
    exit(childNo);
  else
    exit(0);
}
```

#### Key Features of the Second File

1. **Multiple `fork()` Calls**:

   - This file creates **four child processes** by iterating over a loop (`NO_OF_CHILDREN = 4`). In each iteration, a `fork()` call spawns a new child process.

2. **Child Identification**:

   - Each child process is identified by `childNo`, which is passed via the loop. After each `fork()`, only the child process that was just created continues to run the child-specific code, while the parent goes on to create another child.
   - The `isParent` flag ensures that only the parent continues forking new child processes, while children exit the loop.

3. **Waiting for All Children**:

   - The parent process uses `wait()` in a loop to wait for each child to terminate. It outputs the PID of the terminated child and any exit status (shifted from the status byte).
   - The child processes exit using their `childNo` as the exit code.

4. **Child Execution**:

   - Each child runs a loop that prints its process ID and `data` for 10 iterations. The `sleep(1)` ensures that each child’s output is spread over time.

5. **Common Area**:
   - Similar to the first program, both the parent and child processes print "This is a common area" and "Finished!" at the end.

#### Behavior

- **Multiple Children**: This program creates multiple child processes (4 in this case). Each child runs concurrently with its siblings and the parent.
- **Process Synchronization**: The parent uses `wait()` to synchronize with its children, ensuring it knows when each child process has finished.
- **Fork Loop**: The parent forks multiple times, but the children do not. The loop ensures that each new child forks from the original parent process.
- **Exit Status**: Each child exits with a unique exit code (its `childNo`), which is captured by the parent using `wait()`.

### Summary of Differences

| Feature                       | First File (Single Child)                                   | Second File (Multiple Children)                           |
| ----------------------------- | ----------------------------------------------------------- | --------------------------------------------------------- |
| **Number of Child Processes** | One                                                         | Four (or more, depending on `NO_OF_CHILDREN`)             |
| **Fork Loop**                 | Single `fork()` outside of loop                             | Multiple `fork()` calls inside a loop                     |
| **Parent-Child Structure**    | Parent and child run concurrently but independently         | Parent spawns multiple children and waits for each        |
| **Wait Mechanism**            | No explicit `wait()` used                                   | Parent waits for all children to terminate using `wait()` |
| **Data Modification**         | Parent increments `data`, but child does not                | Each child runs its own loop independently                |
| **Exit Status**               | Both processes run to completion without exiting explicitly | Child processes exit with their `childNo` as the status   |
| **Concurrency**               | Two processes run concurrently                              | Five processes (parent + 4 children) run concurrently     |

The second program demonstrates more advanced process control with multiple child processes and synchronization, while the first program offers a simpler example with only one child and parent process running concurrently.

## Signals

A signal is a software interrupt sent to a process by the operating system to notify it of various events, such as termination requests, illegal operations, or external user commands (e.g., pressing Ctrl+C). Signals can be handled or ignored by processes.

### Signal Handler Overview

A **signal handler** is a function that executes in response to a specific signal. When a signal is raised, the operating system interrupts the current execution of the process and transfers control to the signal handler. Once the signal handler completes, the process resumes execution from where it left off.

### Key Concepts

1. **Signal Types**:

   - Common signals include:

     - **SIGINT**: Sent when a user interrupts the process (Ctrl+C).
     - **SIGUSR1** and **SIGUSR2**: User-defined signals that can be used for custom inter-process communication.
     - **SIGTERM**: Requests termination of the process.
     - **SIGKILL**: Forces termination of the process (cannot be caught or ignored).

     You can also read the [signals guide](./signals.pdf).

2. **Signal Handler Function**:

   - The handler is typically a function that takes the signal number as an argument.
   - `void sigHandler(int signum)` is the common form of a signal handler. Inside this function, the program can define how to handle specific signals.

3. **Installing a Signal Handler**:

   - Signal handlers are installed using the `signal()` system call:

     ```cpp
     sighandler_t signal(int signum, sighandler_t handler);
     ```

   - `signal()` sets up a handler for the given signal (`signum`), returning `SIG_ERR` if the handler setup fails.

4. **Signals in the Example Code** ([code](./examples/signals/recvsig2.cpp)):

   - **SIGINT**: The handler prints a message and refuses to terminate the process, instead counting for 5 more seconds before completing.
   - **SIGUSR1** and **SIGUSR2**: These are custom signals that can be used for user-defined behavior. In the example, the handler for both signals performs similar actions as with `SIGINT`, refusing immediate termination and counting for 5 seconds.
   - **SIG_ERR**: Indicates an error in registering a signal handler.

5. **Default vs Custom Handling**:

   - If no signal handler is provided, the system’s default action for that signal is executed (e.g., termination for `SIGINT` or `SIGKILL`).
   - By defining custom handlers, you can control the response to specific signals (e.g., ignore `SIGINT` or perform a graceful shutdown on `SIGTERM`).

### Example Explanation

In the provided code:

- The process prints its process ID (`PID`) and waits for signals.
- Signal handlers are installed for **SIGINT**, **SIGUSR1**, and **SIGUSR2** using the `signal()` function.
- When one of these signals is received, the `sigHandler()` function is called, which performs specific actions based on the signal type.
  - For **SIGINT**, **SIGUSR1**, and **SIGUSR2**, the handler prints a message, then counts for 5 seconds before returning control to the process.
  - This allows the program to delay termination or perform other actions when these signals are received.

### Signal Handling Best Practices

- **Avoid Long Tasks in Handlers**: Keep signal handlers short and simple, as signals interrupt the normal flow of the program.
- **Reentrancy**: Signal handlers should be reentrant, meaning they can be interrupted and safely called again without causing problems (e.g., avoid calling non-reentrant functions like `printf` inside handlers).
- **Handling Multiple Signals**: Ensure your handlers are designed to manage multiple signals efficiently, especially when multiple signals might arrive before the handler completes.
- **Graceful Shutdown**: Use signals like **SIGTERM** or **SIGINT** to perform a graceful shutdown, cleaning up resources before exiting the process.

## Using `sigaction`

Both `sigaction` and `signal` are used to handle signals in Linux, but they have important differences in terms of functionality and reliability. Below is a detailed comparison between the two:

### `signal()`

`signal()` is a basic and older method for setting up signal handlers in Unix-like systems. It works by associating a signal number with a signal handler.

```c++
sighandler_t signal(int signum, sighandler_t handler);
```

- **`signum`**: The signal number (e.g., `SIGINT`, `SIGTERM`).
- **`handler`**: A pointer to the signal handling function or a predefined action (`SIG_IGN` for ignore, `SIG_DFL` for default behavior).

#### Example

```c++
signal(SIGINT, sigHandler);
```

#### Pros

- Simple and easy to use.
- Sufficient for handling signals in small programs.

#### Cons

- **Portability issues**: The behavior of `signal()` can differ between Unix systems and may not always be predictable.
- **Handler reset**: On some systems, `signal()` resets the signal handler to its default after it catches a signal. You would have to call `signal()` again inside the handler to reset it.
- **Limited control**: It offers limited options for managing how signals are delivered and how handlers execute, making it less flexible for complex applications.

### `sigaction()`

`sigaction()` is a more advanced and reliable way to handle signals. It allows you to specify a set of behaviors for how a signal is handled and offers more control over signal handling than `signal()`.

```c++
int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
```

- **`signum`**: The signal number (e.g., `SIGINT`, `SIGTERM`).
- **`act`**: A pointer to a `sigaction` structure that defines the new signal handler and its properties.
- **`oldact`**: A pointer to a `sigaction` structure to store the previous action for the signal.

The `sigaction` structure provides more control over signal handling:

```c++
struct sigaction {
    void (*sa_handler)(int);       // Pointer to the handler function
    sigset_t sa_mask;              // Signals to block during the handler execution
    int sa_flags;                  // Flags to control behavior
};
```

```c++
struct sigaction sa;
sa.sa_handler = sigHandler;
sigemptyset(&sa.sa_mask);          // No signals blocked during handler execution
sa.sa_flags = SA_RESTART;          // Restart interrupted system calls
sigaction(SIGINT, &sa, NULL);      // Set up handler for SIGINT
```

#### Pros

- **More control**: `sigaction()` allows you to fine-tune signal handling with various flags, such as whether to restart interrupted system calls (`SA_RESTART`) or whether to queue multiple signals.
- **Reliability**: Unlike `signal()`, `sigaction()` does not reset the signal handler after it is invoked, providing more consistent behavior.
- **Signal blocking**: You can specify signals to block during the execution of the handler, preventing other signals from interrupting critical sections.
- **Flags**: It supports various flags for advanced control:
  - `SA_RESTART`: Automatically restarts interrupted system calls.
  - `SA_NOCLDWAIT`: Prevents `zombie` processes for child processes.
  - `SA_NODEFER`: Allows signals to be received even while the handler for that signal is running.

#### Cons

- More complex to use compared to `signal()` due to additional structures and flags.

### Key Differences

| Feature                      | `signal()`                     | `sigaction()`                                                |
| ---------------------------- | ------------------------------ | ------------------------------------------------------------ |
| **Resetting of handlers**    | May reset to default after use | Does not reset, more reliable                                |
| **Portability**              | Behavior varies across systems | More standardized across Unix                                |
| **Blocking other signals**   | Not possible                   | Allows blocking of specific signals during handler execution |
| **Advanced control (flags)** | Limited                        | Supports flags like `SA_RESTART`, `SA_NODEFER`, etc.         |
| **Handling system calls**    | May not restart system calls   | Can restart system calls (`SA_RESTART`)                      |
| **Complexity**               | Simple and easy to use         | More complex and requires a `sigaction` structure            |

- **Use `signal()`**: When you need a simple and basic signal handler for small, non-critical programs.
- **Use `sigaction()`**: For more advanced, reliable, and portable signal handling, especially in complex programs that require fine control over signal behavior, blocking, or restarting system calls.

## Author

[Aryan Khurana](https://www.github.com/AryanK1511)
