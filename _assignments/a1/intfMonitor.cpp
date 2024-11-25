#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <net/if.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;

// ========== CONSTANTS ==========

// Path for the UNIX socket
const char *socketPath = "/tmp/networkMonitor";

// Buffer size for message communication
const int bufferSize = 256;

// Maximum interface name length
const int maxIfNameLen = 32;

// ========== GLOBAL VARIABLES ==========

// Flag to indicate whether monitoring is active
bool isMonitoringActive = true;

// String to store network interface statistics
string networkInterfaceStatistics;

// ========== FUNCTION DEFINITIONS ==========

int createSocketForInterface();
int bringInterfaceUp(const char *interfaceName);
void collectInterfaceStats(const char *interface, string &interfaceData);
void monitorNetworkInterface(const char *interfaceName, int socket);
static void signalHandler(int signal);

// ========== CORE FUNCTIONS ==========

// Create and connect to the UNIX domain socket
int createSocketForInterface() {
  // Structure to hold the socket address
  struct sockaddr_un socketAddr;

  // Create the socket with the UNIX domain and STREAM type
  int socketFd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (socketFd < 0) {
    // If socket creation fails, print error and exit
    cerr << "[intfMonitor.cpp] Unable to create socket: " << strerror(errno)
         << endl;
    exit(EXIT_FAILURE);
  }

  // Zero out the socket address structure to avoid uninitialized data
  memset(&socketAddr, 0, sizeof(socketAddr));
  socketAddr.sun_family = AF_UNIX;
  strncpy(socketAddr.sun_path, socketPath, sizeof(socketAddr.sun_path) - 1);
  socketAddr.sun_path[sizeof(socketAddr.sun_path) - 1] = '\0';

  // Attempt to connect the socket to the given address
  if (connect(socketFd, (struct sockaddr *)&socketAddr, sizeof(socketAddr)) <
      0) {
    // If connection fails, print error, close socket, and exit
    cerr << "[intfMonitor.cpp] Connection failed: " << strerror(errno) << endl;
    close(socketFd);
    exit(EXIT_FAILURE);
  }

  // Return the file descriptor for the connected socket
  return socketFd;
}

// Bring up the network interface if it's down
int bringInterfaceUp(const char *interfaceName) {
  struct ifreq interfaceRequest; // Structure for storing interface details

  // Zero out the structure to avoid using uninitialized data
  memset(&interfaceRequest, 0, sizeof(interfaceRequest));

  // Copy the provided interface name into the structure (ensuring it's
  // null-terminated)
  strncpy(interfaceRequest.ifr_name, interfaceName, IFNAMSIZ);
  interfaceRequest.ifr_name[IFNAMSIZ - 1] = '\0'; // Ensure null termination

  // Set the interface flags to bring it up
  interfaceRequest.ifr_flags = IFF_UP;

  // Create a socket for the ioctl call (use AF_INET for network-related
  // operations)
  int socketFd = socket(AF_INET, SOCK_DGRAM, 0);
  if (socketFd < 0) {
    // If socket creation fails, print error and return failure
    cerr << "[intfMonitor.cpp] Socket creation failed: " << strerror(errno)
         << endl;
    return EXIT_FAILURE; // Return failure code
  }

  // Attempt to bring the interface up using the ioctl system call
  int result = ioctl(socketFd, SIOCSIFFLAGS, &interfaceRequest);
  if (result < 0) {
    // If ioctl fails, print error with the interface name and return failure
    cerr << "[intfMonitor.cpp] Failed to bring interface up: '" << interfaceName
         << "' - " << strerror(errno) << endl;
  }

  // Close the socket after the operation
  close(socketFd);

  // Return the result of the ioctl call (0 on success, -1 on failure)
  return result;
}

// Collect statistics for the specified network interface
void collectInterfaceStats(const char *interface, string &interfaceData) {
  string operstate;
  int carrierUpCount = 0, carrierDownCount = 0;
  int txBytes = 0, rxBytes = 0;
  int rxDropped = 0, rxErrors = 0;
  int txPackets = 0, rxPackets = 0;
  int txDropped = 0, txErrors = 0;
  char statPath[bufferSize];
  ifstream infile;

  // Read interface operational state
  snprintf(statPath, sizeof(statPath), "/sys/class/net/%s/operstate",
           interface);
  infile.open(statPath);
  if (infile.is_open()) {
    infile >> operstate;
    infile.close();
  }

  // Read carrier up count
  snprintf(statPath, sizeof(statPath), "/sys/class/net/%s/carrier_up_count",
           interface);
  infile.open(statPath);
  if (infile.is_open()) {
    infile >> carrierUpCount;
    infile.close();
  }

  // Read carrier down count
  snprintf(statPath, sizeof(statPath), "/sys/class/net/%s/carrier_down_count",
           interface);
  infile.open(statPath);
  if (infile.is_open()) {
    infile >> carrierDownCount;
    infile.close();
  }

  // Read transmit bytes
  snprintf(statPath, sizeof(statPath), "/sys/class/net/%s/statistics/tx_bytes",
           interface);
  infile.open(statPath);
  if (infile.is_open()) {
    infile >> txBytes;
    infile.close();
  }

  // Read receive bytes
  snprintf(statPath, sizeof(statPath), "/sys/class/net/%s/statistics/rx_bytes",
           interface);
  infile.open(statPath);
  if (infile.is_open()) {
    infile >> rxBytes;
    infile.close();
  }

  // Read receive dropped packets
  snprintf(statPath, sizeof(statPath),
           "/sys/class/net/%s/statistics/rx_dropped", interface);
  infile.open(statPath);
  if (infile.is_open()) {
    infile >> rxDropped;
    infile.close();
  }

  // Read receive errors
  snprintf(statPath, sizeof(statPath), "/sys/class/net/%s/statistics/rx_errors",
           interface);
  infile.open(statPath);
  if (infile.is_open()) {
    infile >> rxErrors;
    infile.close();
  }

  // Read transmit packets
  snprintf(statPath, sizeof(statPath),
           "/sys/class/net/%s/statistics/tx_packets", interface);
  infile.open(statPath);
  if (infile.is_open()) {
    infile >> txPackets;
    infile.close();
  }

  // Read transmit dropped packets
  snprintf(statPath, sizeof(statPath),
           "/sys/class/net/%s/statistics/tx_dropped", interface);
  infile.open(statPath);
  if (infile.is_open()) {
    infile >> txDropped;
    infile.close();
  }

  // Read transmit errors
  snprintf(statPath, sizeof(statPath), "/sys/class/net/%s/statistics/tx_errors",
           interface);
  infile.open(statPath);
  if (infile.is_open()) {
    infile >> txErrors;
    infile.close();
  }

  // Read receive packets
  snprintf(statPath, sizeof(statPath),
           "/sys/class/net/%s/statistics/rx_packets", interface);
  infile.open(statPath);
  if (infile.is_open()) {
    infile >> rxPackets;
    infile.close();
  }

  // Check interface state and attempt to bring it up if down
  if (operstate == "down") {
    cout << "[intfMonitor.cpp] Interface " << interface << " xxxxx DOWN xxxxx"
         << endl;
    bringInterfaceUp(interface);
  }

  // Format the collected statistics
  interfaceData = "Interface: " + string(interface) + " state: " + operstate +
                  " up_count: " + to_string(carrierUpCount) +
                  " down_count: " + to_string(carrierDownCount) + "\n" +
                  " rx_bytes: " + to_string(rxBytes) +
                  " rx_dropped: " + to_string(rxDropped) +
                  " rx_errors: " + to_string(rxErrors) +
                  " rx_packets: " + to_string(rxPackets) + "\n" +
                  " tx_bytes: " + to_string(txBytes) +
                  " tx_dropped: " + to_string(txDropped) +
                  " tx_errors: " + to_string(txErrors) +
                  " tx_packets: " + to_string(txPackets) + "\n";
}

// Monitor and send interface statistics
void monitorNetworkInterface(const char *interfaceName, int socket) {
  // Buffer to hold the data to be sent
  char buffer[bufferSize];

  // Clear the buffer before use
  memset(buffer, 0, sizeof(buffer));

  // Collect network interface statistics (assuming this function is implemented
  // elsewhere)
  collectInterfaceStats(interfaceName, networkInterfaceStatistics);

  // Ensure that the networkInterfaceStatistics is properly copied into the
  // buffer, and make sure not to overflow the buffer
  // Ensure null termination in case the string is too long
  strncpy(buffer, networkInterfaceStatistics.c_str(), bufferSize - 1);
  buffer[bufferSize - 1] = '\0';

  // Send the data over the socket
  // If the write operation fails, print an error message
  if (write(socket, buffer, strlen(buffer)) < 0) {
    cerr << "[intfMonitor.cpp] Failed to send data: " << strerror(errno)
         << endl;
  }
}

// =========== UTILITY FUNCTIONS ==========

// Handle incoming signals
static void signalHandler(int signal) {
  if (signal == SIGUSR1) {
    cout << "[intfMonitor.cpp] Shutting down" << endl;
    isMonitoringActive = false;
  }
}

// ==================== MAIN PROGRAM ====================
int main(int argc, char *argv[]) {
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " <network-interface>" << endl;
    return EXIT_FAILURE;
  }

  // Store the network interface name
  char networkInterface[maxIfNameLen];
  strncpy(networkInterface, argv[1], maxIfNameLen - 1);

  // Set up signal handler
  struct sigaction sigAction;
  sigAction.sa_handler = signalHandler;
  sigemptyset(&sigAction.sa_mask);
  sigAction.sa_flags = 0;
  sigaction(SIGUSR1, &sigAction, nullptr);

  // Ignore SIGINT signal
  struct sigaction ignoreSigAction;
  ignoreSigAction.sa_handler = SIG_IGN;
  sigemptyset(&ignoreSigAction.sa_mask);
  ignoreSigAction.sa_flags = 0;
  if (sigaction(SIGINT, &ignoreSigAction, nullptr) < 0) {
    cerr << "[intfMonitor.cpp] Failed to block SIGINT: " << strerror(errno)
         << endl;
    return EXIT_FAILURE;
  }

  // Create and initialize socket connection
  int socketFd = createSocketForInterface();
  write(socketFd, "ready_to_monitor", 16);

  // Wait for start monitoring command
  char buffer[bufferSize];
  int bytesRead = read(socketFd, buffer, bufferSize - 1);
  if (bytesRead < 0) {
    cerr << "[intfMonitor.cpp] Failed to read data: " << strerror(errno)
         << endl;
    close(socketFd);
    return EXIT_FAILURE;
  }
  buffer[bytesRead] = '\0';

  // Verify correct start command received
  if (strcmp(buffer, "start_monitoring") != 0) {
    cerr << "[intfMonitor.cpp] Unexpected message received: " << buffer << endl;
    close(socketFd);
    return EXIT_FAILURE;
  }

  // Main monitoring loop
  while (isMonitoringActive) {
    monitorNetworkInterface(networkInterface, socketFd);
    sleep(1);
  }

  // Clean up and exit
  close(socketFd);
  return EXIT_SUCCESS;
}