#include <arpa/inet.h> // For inet_pton and network functions
#include <cstring>     // For memset and string operations
#include <fcntl.h>     // For file control options
#include <fstream>     // For file operations
#include <iostream>    // For standard I/O
#include <mutex>       // For thread synchronization
#include <signal.h>    // For signal handling
#include <sys/stat.h>  // For file permissions
#include <thread>      // For threading support
#include <unistd.h>    // For POSIX operating system API

// ========== STATIC VARIABLES ==========
static int sockfd;                     // Socket file descriptor
static struct sockaddr_in server_addr; // Server address structure
static std::mutex log_mutex;           // Mutex for thread safety
static bool is_running = true;         // Flag to control thread execution
static std::ofstream log_file;         // Server log file stream

// ========== THREAD FUNCTIONS ==========

void receive_thread_func() {
  // Buffer to store received data from the logger
  char buf[1024];

  while (is_running) {
    // Clear the buffer before each receive operation
    memset(buf, 0, sizeof(buf));
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Attempt to receive data from the socket
    int len = recvfrom(sockfd, buf, sizeof(buf), 0,
                       (struct sockaddr *)&client_addr, &client_len);

    // Handle receive errors
    if (len < 0) {
      if (errno != EWOULDBLOCK && errno != EAGAIN) {
        std::cerr << "Error receiving: " << strerror(errno) << std::endl;
      }
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }

    // Write received log message to file
    if (len > 0) {
      std::lock_guard<std::mutex> lock(log_mutex);
      log_file << buf;
      log_file.flush();
    }
  }
}

// ========== SIGNAL HANDLERS ==========

void shutdown_handler(int signum) {
  is_running = false; // Signal threads to stop
  close(sockfd);      // Close the socket
  log_file.close();   // Close the log file
  exit(0);            // Exit the program
}

// ========== MENU OPERATIONS ==========

// Handle setting log level
void handle_set_log_level() {
  int level;
  std::cout << "Enter log level (0-DEBUG, 1-WARNING, 2-ERROR, 3-CRITICAL): ";
  std::cin >> level;

  char buf[1024];
  memset(buf, 0, sizeof(buf));
  int len = snprintf(buf, sizeof(buf), "Set Log Level=%d", level);

  // Debug print
  std::cout << "Sending command: " << buf << std::endl;

  struct sockaddr_in client_addr;
  memset(&client_addr, 0, sizeof(client_addr));
  client_addr.sin_family = AF_INET;
  client_addr.sin_port = htons(8081); // Send to logger's port
  inet_pton(AF_INET, "127.0.0.1", &client_addr.sin_addr);

  ssize_t sent = sendto(sockfd, buf, len, 0, (struct sockaddr *)&client_addr,
                        sizeof(client_addr));

  if (sent < 0) {
    std::cerr << "Failed to send command. Error: " << strerror(errno)
              << std::endl;
  } else {
    std::cout << "Sent " << sent << " bytes" << std::endl;
  }
}

// Handle dumping log file contents
void handle_dump_log() {
  std::lock_guard<std::mutex> lock(log_mutex);
  std::ifstream read_file("server_log.txt");
  std::string line;
  while (std::getline(read_file, line)) {
    std::cout << line << std::endl;
  }
  std::cout << "Press any key to continue:";
  std::cin.ignore();
  std::cin.get();
}

// ========== MAIN FUNCTION ==========

int main() {
  // Set up signal handler for graceful shutdown
  signal(SIGINT, shutdown_handler);

  // Open log file with rw-rw-rw- permissions
  mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
  log_file.open("server_log.txt", std::ios::out | std::ios::app);
  chmod("server_log.txt", mode);

  if (!log_file.is_open()) {
    std::cerr << "Failed to open server log file" << std::endl;
    return -1;
  }

  // Create UDP socket
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    std::cerr << "Failed to create socket" << std::endl;
    return -1;
  }

  // Set socket to non-blocking
  int flags = fcntl(sockfd, F_GETFL, 0);
  fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

  // Setup server address
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(8080);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  // Bind socket
  if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    std::cerr << "Failed to bind socket" << std::endl;
    return -1;
  }

  // Start receive thread
  std::thread receive_thread(receive_thread_func);

  // Main menu loop
  while (is_running) {
    std::cout
        << "1. Set the log level\n2. Dump the log file here\n0. Shut down\n";
    int choice;
    std::cin >> choice;

    if (choice == 1) {
      handle_set_log_level();
    } else if (choice == 2) {
      handle_dump_log();
    } else if (choice == 0) {
      is_running = false;
    }
  }

  // Clean up
  receive_thread.join();
  return 0;
}