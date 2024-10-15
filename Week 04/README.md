# Week 04 Notes

There is virtual memory and physical memory which means the memory that a process thinks that it is using vs the memory that it is actually using.
File descriptors work similarly to virtual memory in the sense that:

- A file descriptor is an abstraction (usually an integer) that a process thinks it is using to refer to an open file, socket, or other resources (such as pipes).
- The actual file or resource is managed by the operating system behind the scenes. The process doesn't directly handle the physical aspects of files (like where the file is on disk or its specific system-level details).

**In essence:**

- Virtual memory is an abstraction for the memory a process uses.
- File descriptors are an abstraction for accessing files or resources.

Both of these concepts hide the complexities of the underlying system, allowing the process to work with a simpler, cleaner interface while the OS manages the actual hardware resources in the background.

## Duplicating file Descriptors

The functions `dup()` and `dup2()` are used to duplicate file descriptors in Unix-like operating systems. They allow a process to create a copy of an existing file descriptor, which can be useful for redirecting input/output.

### 1. `dup(int oldfd)`

- Creates a copy of the file descriptor `oldfd`.
- Returns a new file descriptor (referred to as `newfd`) that is the lowest available file descriptor greater than or equal to `oldfd`. The new file descriptor refers to the same open file description as `oldfd`, meaning they share the same file offset, access mode (read/write), and flags.
- If the function fails, it returns `-1` and sets `errno` to indicate the error.

```c
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

int main() {
    int fd = open("file.txt", O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    int newfd = dup(fd);  // Duplicates the file descriptor
    if (newfd < 0) {
        perror("dup");
        return 1;
    }

    printf("Old FD: %d, New FD: %d\n", fd, newfd);
    close(fd);  // Closing the old file descriptor does not affect the new one
    close(newfd);

    return 0;
}
```

In the above example, `newfd` is a duplicate of `fd`, and both can be used interchangeably to read from the file.

### 2. `dup2(int oldfd, int newfd)`

- This function duplicates `oldfd` into `newfd`. If `newfd` is already open, it is closed before being reused. Unlike `dup()`, `dup2()` allows you to specify the exact file descriptor `newfd` to be used for the duplicate.
- Returns `newfd` on success, or `-1` on error (sets `errno`).
- If `oldfd` and `newfd` are the same, `dup2()` does nothing and returns `newfd` without closing it.

```c
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

int main() {
    int fd = open("file.txt", O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    int newfd = 5;  // Specify the file descriptor to duplicate to
    if (dup2(fd, newfd) < 0) {
        perror("dup2");
        return 1;
    }

    printf("Old FD: %d, Duplicated FD: %d\n", fd, newfd);
    close(fd);
    close(newfd);

    return 0;
}
```

In this example, `dup2(fd, newfd)` duplicates `fd` into `newfd`. If `newfd` was already open, it would be closed before duplicating `fd` into it.

### Key Differences Between `dup()` and `dup2()`

1. **File Descriptor Choice**:

   - `dup()`: Returns the lowest available file descriptor greater than or equal to `oldfd`.
   - `dup2()`: Duplicates `oldfd` into `newfd`, where `newfd` is explicitly specified.

2. **File Descriptor Reuse**:

   - `dup()`: Automatically assigns the next available file descriptor.
   - `dup2()`: Closes `newfd` if it is already in use before duplicating `oldfd` into it.

3. **Efficiency**:

   - `dup2()` is often more efficient if you need to specify exactly which file descriptor to reuse (e.g., when duplicating to standard input/output or error descriptors like `0`, `1`, or `2`).

#### Redirecting `stdout` to a file using `dup2`:

```c
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    int fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    dup2(fd, 1);  // Duplicates fd into stdout (1)

    // Now, anything written to stdout goes to "output.txt"
    printf("This will go to the file instead of the terminal!\n");

    close(fd);  // Closing fd does not affect stdout (1)

    return 0;
}
```

Here, `dup2(fd, 1)` redirects `stdout` (file descriptor `1`) to `fd`, so the output of `printf()` is written to the file instead of the terminal.

## `fcntl()` Function in C

The `fcntl()` (file control) function in C is used to manipulate file descriptors in various ways. It provides control over the behavior of file descriptors and open files, such as duplicating file descriptors, changing file descriptor flags, and setting or retrieving file descriptor attributes.

```c
#include <fcntl.h>

int fcntl(int fd, int cmd, ... /* arg */ );
```

- `fd`: The file descriptor you want to operate on.
- `cmd`: The command you want to perform (explained below).
- `arg`: Optional third argument used by some commands. If not needed, it can be omitted.

### Return values

- On success: The return value depends on the command.
- On error: Returns `-1` and sets `errno`.

### Common `fcntl` Commands:

Some of the most common commands you can pass as `cmd` are:

#### 1. `F_DUPFD` (Duplicate File Descriptor)

- Duplicates the file descriptor `fd` into the lowest-numbered available file descriptor greater than or equal to `arg`.
- Similar to the `dup()` system call, but you can specify a minimum value for the new descriptor.

```c
int newfd = fcntl(fd, F_DUPFD, 10); // Duplicates fd into a new file descriptor >= 10
```

#### 2. `F_GETFD` (Get File Descriptor Flags)

- Retrieves the file descriptor flags for `fd`.
- Typically used to check if the `FD_CLOEXEC` flag (which closes the file descriptor on `exec` calls) is set.

```c
int flags = fcntl(fd, F_GETFD); // Get the file descriptor flags
```

#### 3. `F_SETFD` (Set File Descriptor Flags)

- Sets the file descriptor flags for `fd`. The most commonly used flag is `FD_CLOEXEC`, which closes the file descriptor when a new program is executed via `exec()`.

```c
fcntl(fd, F_SETFD, FD_CLOEXEC); // Set the close-on-exec flag
```

#### 4. `F_GETFL` (Get File Status Flags)

- Retrieves the file status flags for `fd`. These flags are set when the file descriptor is opened, such as `O_RDONLY`, `O_WRONLY`, `O_RDWR`, or `O_NONBLOCK`.

```c
int flags = fcntl(fd, F_GETFL); // Get the file status flags
```

#### 5. `F_SETFL` (Set File Status Flags)

- Sets the file status flags for `fd`. You can change flags like `O_APPEND`, `O_NONBLOCK`, etc. For example, to make a file descriptor non-blocking, you can set the `O_NONBLOCK` flag.

```c
int flags = fcntl(fd, F_GETFL);  // Get current flags
flags |= O_NONBLOCK;             // Add the non-blocking flag
fcntl(fd, F_SETFL, flags);       // Set the new flags
```

## `readv()`, `writev()`, and `pread()`

These functions are part of the POSIX standard and deal with performing input/output operations using vectors, which allow you to read/write multiple buffers with a single system call.

### 1. **`readv()`** — Reading into Multiple Buffers

`readv()` allows you to read data from a file descriptor into multiple buffers (gathering input). This is useful when you want to read data into several non-contiguous memory locations with a single system call, improving efficiency compared to multiple calls to `read()`.

```c
#include <sys/uio.h>
ssize_t readv(int fd, const struct iovec *iov, int iovcnt);
```

- `fd`: The file descriptor to read from.
- `iov`: An array of `iovec` structures, which describe the memory buffers to read into.
- `iovcnt`: The number of buffers in the `iov` array.

The `iovec` structure is defined as:

```c
struct iovec {
    void *iov_base;  // Pointer to the buffer
    size_t iov_len;  // Length of the buffer
};
```

#### Return Value

- On success: The total number of bytes read.
- On error: `-1`, and `errno` is set appropriately.

```c
#include <sys/uio.h>
#include <unistd.h>

int fd = open("file.txt", O_RDONLY);
if (fd < 0) {
    perror("open");
    return -1;
}

// Prepare two buffers to read into
char buf1[10], buf2[20];
struct iovec iov[2];

iov[0].iov_base = buf1;
iov[0].iov_len = sizeof(buf1);
iov[1].iov_base = buf2;
iov[1].iov_len = sizeof(buf2);

ssize_t bytesRead = readv(fd, iov, 2);
```

### 2. **`writev()`** — Writing from Multiple Buffers

`writev()` allows you to write data from multiple buffers to a file descriptor in a single system call (scattering output). This is useful when data is spread across different memory regions, and you want to write it all at once.

```c
#include <sys/uio.h>
ssize_t writev(int fd, const struct iovec *iov, int iovcnt);
```

- `fd`: The file descriptor to write to.
- `iov`: An array of `iovec` structures that describe the memory buffers to write from.
- `iovcnt`: The number of buffers in the `iov` array.

```c
#include <sys/uio.h>
#include <unistd.h>

int fd = open("file.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
if (fd < 0) {
    perror("open");
    return -1;
}

// Prepare two buffers to write from
char buf1[] = "Hello, ";
char buf2[] = "world!\n";
struct iovec iov[2];

iov[0].iov_base = buf1;
iov[0].iov_len = sizeof(buf1) - 1;
iov[1].iov_base = buf2;
iov[1].iov_len = sizeof(buf2) - 1;

ssize_t bytesWritten = writev(fd, iov, 2);  // Writes both buffers at once
```

### 3. **`pread()`** — Reading from a Specific Offset Without Changing the File Offset

`pread()` is a variation of the `read()` system call, but it reads data from a specific offset in a file without changing the current file offset. This is useful for random-access operations, such as reading from different parts of a file concurrently in a multi-threaded application.

```c
#include <unistd.h>
ssize_t pread(int fd, void *buf, size_t count, off_t offset);
```

- `fd`: The file descriptor to read from.
- `buf`: Buffer to store the data.
- `count`: Number of bytes to read.
- `offset`: The position in the file from where to start reading.

#### Return Value

- On success: The number of bytes read.
- On error: `-1`, and `errno` is set appropriately.

```c
#include <unistd.h>

int fd = open("file.txt", O_RDONLY);
if (fd < 0) {
    perror("open");
    return -1;
}

char buf[100];
ssize_t bytesRead = pread(fd, buf, sizeof(buf), 50);  // Read 100 bytes from offset 50
```

### 4. **`pwrite()`** — Writing to a Specific Offset Without Changing the File Offset

`pwrite()` works similarly to `pread()`, but it writes data to a specific offset without changing the file's current position.

```c
#include <unistd.h>
ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset);
```

```c
#include <unistd.h>

int fd = open("file.txt", O_WRONLY);
if (fd < 0) {
    perror("open");
    return -1;
}

char buf[] = "Hello at offset!";
ssize_t bytesWritten = pwrite(fd, buf, sizeof(buf) - 1, 100);  // Write at offset 100
```

### Key Differences and Use Cases

1. **`readv()`/`writev()`**:

   - Used for **scatter/gather I/O**: you can read/write from/to multiple buffers in one call.
   - Helps improve efficiency by reducing the number of system calls.

2. **`pread()`/`pwrite()`**:
   - Allows random access without modifying the file offset (good for concurrent access).
   - Avoids seeking the file pointer, making it useful in **multithreaded applications**.

## `netdevice` Overview (Networking Devices in Linux)

In Linux, network devices (also referred to as `netdevices`) represent any interface through which a computer can communicate over a network. Network devices can be physical interfaces, such as Ethernet or wireless network adapters, or virtual interfaces used for tunneling or bridging network traffic.

The `netdevice` layer is part of the Linux kernel networking subsystem, which manages and abstracts these networking interfaces. The kernel provides a way to create, manage, and interact with network devices through the **`netdevice`** API, allowing users to send and receive packets through them.

### Common Types of Network Devices

1. **Physical Network Interfaces**:

   - **Ethernet (e.g., `eth0`, `eth1`)**: Wired network interface, typically used for LAN connections.
   - **Wireless (e.g., `wlan0`, `wlp2s0`)**: Wi-Fi interfaces.
   - **Loopback (`lo`)**: A virtual network interface used to communicate with the host itself. Always exists and is used by processes to communicate within the machine.

2. **Virtual Network Interfaces**:

   - **Bridge (e.g., `br0`)**: Aggregates multiple network interfaces, allowing them to act as a single interface.
   - **Bonding (e.g., `bond0`)**: Combines multiple interfaces for redundancy or increased bandwidth.
   - **Tunnels (e.g., `tun0`, `tap0`)**: Used for tunneling traffic between networks (e.g., VPN).

3. **Software-based Interfaces**:
   - **VLAN (e.g., `eth0.100`)**: Virtual LAN interfaces.
   - **VXLAN**: A virtual extensible LAN for overlaying networks over Layer 2.

### Important `netdevice` Operations

1. **Create/Remove Devices**:

   - Devices can be added or removed dynamically using commands like `ip link add` or `ip link delete`, or by using interfaces in the kernel.

2. **Configure Network Devices**:

   - Devices can be configured using `ifconfig` or more modern tools like `ip`:
     - `ip link set dev eth0 up` (bring a device up)
     - `ip addr add 192.168.1.10/24 dev eth0` (set an IP address)
     - `ip link set dev eth0 mtu 1500` (set Maximum Transmission Unit size)

3. **Monitor Network Devices**:
   - Network device statistics (packets, bytes sent/received, errors) can be viewed using commands like `ip -s link`, `ethtool`, or `netstat`.

### `struct net_device`

At the kernel level, each network device in Linux is represented by a **`struct net_device`**. This structure stores all the information about the network device, including its name, type, hardware address, status, and operational properties.

#### Key Fields in `struct net_device`:

- **`name`**: Name of the network interface (e.g., `eth0`).
- **`flags`**: Operational flags (e.g., whether the interface is up or down).
- **`mtu`**: Maximum transmission unit (size of the largest packet the device can send).
- **`addr_len`**: Length of the hardware address (e.g., for Ethernet, it's 6 bytes).
- **`broadcast`**: Broadcast address for the device.
- **`hard_header_len`**: Length of the link-layer header (e.g., Ethernet headers).
- **`ifindex`**: Interface index number (used internally to identify devices).
- **`netdev_ops`**: Pointers to functions that implement operations on the device (e.g., open, close, transmit packets).

```c
struct net_device {
    /* device name, e.g. "eth0" */
    char name[IFNAMSIZ];

    /* device index, assigned by kernel */
    int ifindex;

    /* network namespace this device is inside */
    struct net *nd_net;

    /* Maximum transmission unit (MTU) */
    unsigned int mtu;

    /* Link layer type (e.g., Ethernet, loopback, etc.) */
    unsigned short type;

    /* Flags (IFF_UP, IFF_RUNNING, etc.) */
    unsigned int flags;

    /* Interface hardware address */
    unsigned char dev_addr[MAX_ADDR_LEN];

    /* Broadcast address */
    unsigned char broadcast[MAX_ADDR_LEN];

    /* Address length */
    unsigned char addr_len;

    /* Interface hardware statistics */
    struct net_device_stats stats;

    /* Information about physical interface */
    struct {
        unsigned short mtu;
        unsigned char addr[MAX_ADDR_LEN];
        unsigned char addr_len;
    } phys_info;

    /* Traffic control */
    struct Qdisc *qdisc;

    /* Network device operations */
    const struct net_device_ops *netdev_ops;

    /* Driver-specific data */
    void *priv;

    /* List of multicast addresses to listen for */
    struct dev_mc_list *mc_list;

    /* Total number of multicast addresses to listen for */
    int mc_count;

    /* IP fragmentation offload state */
    unsigned int gso_max_size;
    unsigned int gso_max_segs;

    /* Device state (e.g., up, down, running) */
    unsigned long state;

    /* Device's transmit queue lock */
    spinlock_t tx_queue_lock;

    /* List of transmit queues */
    struct netdev_queue *tx_queues;

    /* Memory management */
    struct sk_buff_head dev_queue;

    /* Multicast promiscuous mode (whether device listens to multicast packets) */
    unsigned char allmulti;

    /* Promiscuous mode (whether device listens to all packets) */
    unsigned char promiscuity;

    /* Device memory region (e.g., for NIC registers) */
    unsigned long base_addr;

    /* IRQ (interrupt request line) number */
    unsigned int irq;

    /* Link-layer header length */
    unsigned short hard_header_len;

    /* Needed for next-hop routing */
    unsigned short min_header_len;

    /* Hardware-related information */
    struct {
        unsigned short mtu;
        unsigned char addr[MAX_ADDR_LEN];
        unsigned char addr_len;
    } hw_info;

    /* Multicast and broadcast address list */
    struct list_head mc_list_head;

    /* Backlog of incoming packets */
    struct sk_buff_head rx_queue;

    /* Number of packets dropped */
    unsigned int dropped;

    /* Total length of packets sent (in bytes) */
    unsigned long tx_bytes;

    /* Total length of packets received (in bytes) */
    unsigned long rx_bytes;

    /* Number of packets sent */
    unsigned long tx_packets;

    /* Number of packets received */
    unsigned long rx_packets;

    /* Error counts for transmit/receive */
    unsigned long tx_errors;
    unsigned long rx_errors;

    /* Flags for hardware offloading (e.g., checksum offloading) */
    unsigned int hw_features;

    /* Device's hardware timestamping options */
    struct hwtstamp_config *tstamp;

    /* Per-device sysfs directories */
    struct device dev;

    /* Reassembly queue for fragmented packets */
    struct frag_queue frag_list;

    /* Link-layer broadcast address */
    unsigned char broadcast[MAX_ADDR_LEN];

    /* Interface queue lock */
    struct Qdisc *tx_queue;

    /* Timestamps for packet drops */
    struct timespec drop_time;

    /* CPU-affinity mask for interrupts */
    cpumask_t irq_affinity;

    /* More fields depending on kernel version */
};
```

### Kernel Interaction with Network Devices

- **`register_netdev()`**: Registers a network device with the kernel.
- **`unregister_netdev()`**: Unregisters (removes) a network device.
- **`netdev_alloc()`**: Allocates memory for a new network device.

These functions are typically used when developing network drivers in the kernel to define and manage network devices.
