#include "Logger.h"
#include <arpa/inet.h>
#include <cstring>
#include <ctime>
#include <fcntl.h> // For non-blocking socket
#include <fstream>
#include <iostream>
#include <mutex>
#include <thread>
#include <unistd.h>

static int sockfd;
static struct sockaddr_in server_addr;
static LOG_LEVEL filter_level;
static std::mutex log_mutex;
static bool is_running = true;
static std::ofstream log_file("client_log.txt");

void receive_thread_func() {
  std::cout << "Receive thread started" << std::endl; // Debug print
  char buf[1024];
  while (is_running) {
    memset(buf, 0, sizeof(buf));
    int len = recvfrom(sockfd, buf, sizeof(buf), 0, NULL, NULL);
    if (len < 0 && (errno == EWOULDBLOCK || errno == EAGAIN)) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }
    std::cout << "Received " << len << " bytes" << std::endl; // Debug print
    if (len > 0) {
      buf[len] = '\0'; // Ensure null-termination
      std::string command(buf);
      std::cout << "Command received: " << command << std::endl; // Debug print
      if (command.find("Set Log Level=") != std::string::npos) {
        int level = std::stoi(command.substr(14));
        std::cout << "Received command to set log level to " << level
                  << std::endl; // Debug print
        SetLogLevel(static_cast<LOG_LEVEL>(level));
        std::cout << "Log level set to " << level << std::endl; // Debug print
      }
    }
  }
}

int InitializeLog() {
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    std::cerr << "Failed to create socket" << std::endl;
    return -1;
  }
  std::cout << "Socket created: " << sockfd << std::endl; // Debug print

  // Set socket to non-blocking
  int flags = fcntl(sockfd, F_GETFL, 0);
  fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(8080);
  inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

  std::thread t(receive_thread_func);
  t.detach(); // Detach the thread to run independently
  std::cout << "Receive thread detached" << std::endl; // Debug print
  return 0;
}

void SetLogLevel(LOG_LEVEL level) {
  std::lock_guard<std::mutex> lock(log_mutex);
  filter_level = level; // Direct assignment now works correctly
  std::cout << "Filter level updated to " << static_cast<int>(level)
            << std::endl;
  log_file << "Filter level updated to " << static_cast<int>(level)
           << std::endl;
}

void Log(LOG_LEVEL level, const char *file, const char *func, int line,
         const char *message) {
  // Add bounds checking to prevent potential out-of-bounds access
  if (level < LOG_LEVEL::DEBUG || level > LOG_LEVEL::CRITICAL) {
    std::cerr << "Invalid log level" << std::endl;
    return;
  }

  std::cout << "Current filter level: " << static_cast<int>(filter_level)
            << ", Log level: " << static_cast<int>(level) << std::endl;

  if (level < filter_level) {
    std::cout << "Log level " << static_cast<int>(level)
              << " is lower than filter level "
              << static_cast<int>(filter_level) << ", skipping log."
              << std::endl;
    return;
  }

  std::lock_guard<std::mutex> lock(log_mutex);

  time_t now = time(0);
  char *dt = ctime(&now);
  dt[strlen(dt) - 1] = '\0';

  char buf[1024];
  const char *levelStr[] = {"DEBUG", "WARNING", "ERROR", "CRITICAL"};
  int len = snprintf(buf, sizeof(buf), "%s %s %s:%s:%d %s", dt, levelStr[level],
                     file, func, line, message);
  if (len >= sizeof(buf)) {
    std::cerr << "Log message too long, truncated" << std::endl;
    return;
  }

  log_file << buf << std::endl; // Write to file
  sendto(sockfd, buf, len, 0, (struct sockaddr *)&server_addr,
         sizeof(server_addr));
}

void ExitLog() {
  std::lock_guard<std::mutex> lock(log_mutex);
  is_running = false;
  close(sockfd);
  log_file.close();
}