#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h> // For non-blocking socket
#include <fstream>
#include <iostream>
#include <mutex>
#include <signal.h>
#include <thread>
#include <unistd.h>

static int sockfd;
static struct sockaddr_in server_addr;
static std::mutex log_mutex;
static bool is_running = true;
static std::ofstream log_file("server_log.txt");

void receive_thread_func() {
  char buf[1024];
  while (is_running) {
    memset(buf, 0, sizeof(buf));
    int len = recvfrom(sockfd, buf, sizeof(buf), 0, NULL, NULL);
    if (len > 0) {
      buf[len] = '\0';
      std::lock_guard<std::mutex> lock(log_mutex);
      log_file << buf << std::endl;
      log_file.flush();
    }
    // std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

void shutdown_handler(int signum) {
  is_running = false;
  close(sockfd);
  log_file.close();
  exit(0);
}

int main() {
  signal(SIGINT, shutdown_handler);

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    std::cerr << "Failed to create socket" << std::endl;
    return -1;
  }

  // Set socket to non-blocking
  int flags = fcntl(sockfd, F_GETFL, 0);
  fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(8080);
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    std::cerr << "Failed to bind socket" << std::endl;
    return -1;
  }

  std::thread receive_thread(receive_thread_func);

  while (is_running) {
    std::cout
        << "1. Set the log level\n2. Dump the log file here\n0. Shut down\n";
    int choice;
    std::cin >> choice;

    if (choice == 1) {
      int level;
      std::cout
          << "Enter log level (0-DEBUG, 1-WARNING, 2-ERROR, 3-CRITICAL): ";
      std::cin >> level;
      char buf[1024];
      int len = snprintf(buf, sizeof(buf), "Set Log Level=%d", level);
      if (len >= sizeof(buf)) {
        std::cerr << "Log level command too long, truncated" << std::endl;
        continue;
      }
      struct sockaddr_in client_addr;
      memset(&client_addr, 0, sizeof(client_addr));
      client_addr.sin_family = AF_INET;
      client_addr.sin_port =
          htons(8080); // Ensure this matches the logger's port
      inet_pton(
          AF_INET, "127.0.0.1",
          &client_addr.sin_addr); // Ensure this matches the logger's address

      sendto(sockfd, buf, len, 0, (struct sockaddr *)&client_addr,
             sizeof(client_addr));
    } else if (choice == 2) {
      std::lock_guard<std::mutex> lock(log_mutex);
      std::ifstream log_file("server_log.txt");
      std::string line;
      while (std::getline(log_file, line)) {
        std::cout << line << std::endl;
      }
      std::cout << "Press any key to continue:";
      std::cin.ignore();
      std::cin.get();
    } else if (choice == 0) {
      is_running = false;
    }
  }

  receive_thread.join(); // Wait for the receive_thread to finish
  return 0;
}