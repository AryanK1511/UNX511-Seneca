#include "Logger.h"
#include <arpa/inet.h> // For inet_pton and network functions
#include <cstring>     // For memset and string operations
#include <ctime>       // For timestamp functions
#include <fcntl.h>     // For file control options
#include <iostream>    // For standard I/O
#include <mutex>       // For thread synchronization
#include <thread>      // For threading support
#include <unistd.h>    // For POSIX operating system API

// ========== STATIC VARIABLES ==========
static int sockfd;                                // Socket file descriptor
static struct sockaddr_in server_addr;            // Server address structure
static LOG_LEVEL filter_level = LOG_LEVEL::DEBUG; // Current log level filter
static std::mutex log_mutex;                      // Mutex for thread safety
static bool is_running = true; // Flag to control thread execution

// ========== THREAD FUNCTIONS ==========

// Thread function to receive commands from the server
void receive_thread_func() {
  // Buffer to store received data
  char buf[1024];

  while (is_running) {
    // Clear the buffer before each receive operation
    memset(buf, 0, sizeof(buf));

    // Structure to store sender's address information
    struct sockaddr_in sender_addr;
    socklen_t sender_len = sizeof(sender_addr);

    // Attempt to receive data from the socket
    int len = recvfrom(sockfd, buf, sizeof(buf), 0,
                       (struct sockaddr *)&sender_addr, &sender_len);

    // Handle receive errors
    if (len < 0) {
      if (errno != EWOULDBLOCK && errno != EAGAIN) {
        std::cerr << "Error receiving: " << strerror(errno) << std::endl;
      }

      // Wait for 1 second before trying again if no data
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }

    // Process received data
    if (len > 0) {

      // Null terminate the received string
      buf[len] = '\0';

      // Convert received data to string
      std::string command(buf);

      // Check if the command is to set log level
      if (command.find("Set Log Level=") != std::string::npos) {
        try {
          // Extract and parse the log level from command
          int level = std::stoi(command.substr(14));
          if (level >= 0 && level <= 3) {
            SetLogLevel(static_cast<LOG_LEVEL>(level));
          }
        } catch (const std::exception &e) {
          std::cerr << "Error parsing log level" << std::endl;
        }
      }
    }
  }
}

// ========== INITIALIZATION AND CLEANUP ==========

// Initialize the logger
int InitializeLog() {

  // Create a UDP socket
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    std::cerr << "Failed to create socket" << std::endl;
    return -1;
  }

  // Set socket to non-blocking mode
  int flags = fcntl(sockfd, F_GETFL, 0);
  fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

  // Setup local address structure for receiving commands
  struct sockaddr_in local_addr;
  memset(&local_addr, 0, sizeof(local_addr));
  local_addr.sin_family = AF_INET;   // Use IPv4
  local_addr.sin_port = htons(8081); // Listen on port 8081
  local_addr.sin_addr.s_addr =
      INADDR_ANY; // Accept connections on any interface

  // Bind socket to local address to receive commands
  if (bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
    std::cerr << "Failed to bind socket" << std::endl;
    return -1;
  }

  // Setup server address structure for sending logs
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;   // Use IPv4
  server_addr.sin_port = htons(8080); // Server listens on port 8080
  inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr); // Server IP address

  // Debug print to confirm initialization
  std::cout << "Logger initialized, listening on port 8081" << std::endl;

  // Start the receive thread
  std::thread t(receive_thread_func);
  t.detach(); // Detach thread to run independently

  return 0;
}

// Cleanup and shutdown the logger
void ExitLog() {
  std::lock_guard<std::mutex> lock(log_mutex); // Thread-safe shutdown
  is_running = false;                          // Signal thread to stop
  close(sockfd);                               // Close the socket
}

// ========== LOGGING OPERATIONS ==========

// Set the current log level filter
void SetLogLevel(LOG_LEVEL level) {
  std::lock_guard<std::mutex> lock(log_mutex); // Thread-safe operation
  filter_level = level;
}

// Log a message with the given severity level
void Log(LOG_LEVEL level, const char *file, const char *func, int line,
         const char *message) {
  // Check if this log should be filtered out
  if (level < filter_level) {
    return;
  }

  // Lock for thread safety
  std::lock_guard<std::mutex> lock(log_mutex);

  // Create timestamp for the log
  time_t now = time(0);
  char *dt = ctime(&now);
  dt[strlen(dt) - 1] = '\0'; // Remove trailing newline

  // Prepare the log message
  char buf[1024];
  memset(buf, 0, sizeof(buf));
  const char *levelStr[] = {"DEBUG", "WARNING", "ERROR", "CRITICAL"};
  int len =
      snprintf(buf, sizeof(buf), "%s %s %s:%s:%d %s\n", dt,
               levelStr[static_cast<int>(level)], file, func, line, message);

  // Check for buffer overflow
  if (len >= sizeof(buf)) {
    std::cerr << "Log message too long, truncated" << std::endl;
    return;
  }

  // Send log message to server
  sendto(sockfd, buf, len, 0, (struct sockaddr *)&server_addr,
         sizeof(server_addr));
}