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

#define SOCKET_PATH "/tmp/networkMonitor"
#define BUFFER_SIZE 512
#define MAX_INTERFACE_NAME_LEN 32

using namespace std;

bool isRunning = true;

string networkInterfaceStats;
int socketFileDescriptor = -1;

// Configure network interface flags
int configureInterfaceFlags(const char *interfaceName, short flagSettings) {
  struct ifreq interfaceRequest;
  memset(&interfaceRequest, 0, sizeof(interfaceRequest));
  strncpy(interfaceRequest.ifr_name, interfaceName, IFNAMSIZ);
  interfaceRequest.ifr_flags = flagSettings;

  int localSocket = socket(AF_INET, SOCK_DGRAM, 0);
  if (localSocket < 0) {
    cerr << "Socket creation failed: " << strerror(errno) << endl;
    EXIT_FAILURE;
  }

  int result = ioctl(localSocket, SIOCSIFFLAGS, &interfaceRequest);
  if (result < 0) {
    cerr << "Failed to set flags for interface '" << interfaceName
         << "': " << strerror(errno) << endl;
  } else {
    cout << "Interface '" << interfaceName << "' is now active." << endl;
  }

  close(localSocket);
  return result;
}

// Get all required stats from the interfaces
void getNetworkInterfaceStats(const char *interface, string &interfaceData) {
  string operstate;
  int carrier_up_count = 0, carrier_down_count = 0;
  int rx_bytes = 0, rx_dropped = 0, rx_errors = 0, rx_packets = 0;
  int tx_bytes = 0, tx_dropped = 0, tx_errors = 0, tx_packets = 0;
  char statPath[BUFFER_SIZE];
  ifstream infile;

  sprintf(statPath, "/sys/class/net/%s/operstate", interface);
  infile.open(statPath);
  if (infile.is_open()) {
    infile >> operstate;
    infile.close();
  }

  sprintf(statPath, "/sys/class/net/%s/carrier_up_count", interface);
  infile.open(statPath);
  if (infile.is_open()) {
    infile >> carrier_up_count;
    infile.close();
  }

  sprintf(statPath, "/sys/class/net/%s/carrier_down_count", interface);
  infile.open(statPath);
  if (infile.is_open()) {
    infile >> carrier_up_count;
    infile.close();
  }

  sprintf(statPath, "/sys/class/net/%s/statistics/rx_bytes", interface);
  infile.open(statPath);
  if (infile.is_open()) {
    infile >> rx_bytes;
    infile.close();
  }

  sprintf(statPath, "/sys/class/net/%s/statistics/rx_dropped", interface);
  infile.open(statPath);
  if (infile.is_open()) {
    infile >> rx_dropped;
    infile.close();
  }

  sprintf(statPath, "/sys/class/net/%s/statistics/rx_errors", interface);
  infile.open(statPath);
  if (infile.is_open()) {
    infile >> rx_errors;
    infile.close();
  }

  sprintf(statPath, "/sys/class/net/%s/statistics/rx_packets", interface);
  infile.open(statPath);
  if (infile.is_open()) {
    infile >> rx_packets;
    infile.close();
  }

  sprintf(statPath, "/sys/class/net/%s/statistics/tx_bytes", interface);
  infile.open(statPath);
  if (infile.is_open()) {
    infile >> tx_bytes;
    infile.close();
  }

  sprintf(statPath, "/sys/class/net/%s/statistics/tx_dropped", interface);
  infile.open(statPath);
  if (infile.is_open()) {
    infile >> tx_dropped;
    infile.close();
  }

  sprintf(statPath, "/sys/class/net/%s/statistics/tx_errors", interface);
  infile.open(statPath);
  if (infile.is_open()) {
    infile >> tx_errors;
    infile.close();
  }

  sprintf(statPath, "/sys/class/net/%s/statistics/tx_packets", interface);
  infile.open(statPath);
  if (infile.is_open()) {
    infile >> tx_packets;
    infile.close();
  }

  if (operstate == "down") {
    configureInterfaceFlags(interface, IFF_UP);
  }

  string tmpInterface = string(interface);
  string tmpOperstate = string(operstate);

  interfaceData = "Interface: " + tmpInterface + " state: " + tmpOperstate +
                  " up_count: " + to_string(carrier_up_count) +
                  " down_count: " + to_string(carrier_down_count) + "\n" +
                  "rx_bytes: " + to_string(rx_bytes) +
                  " rx_dropped: " + to_string(rx_dropped) +
                  " rx_errors: " + to_string(rx_errors) +
                  " rx_packets: " + to_string(rx_packets) + "\n" +
                  "tx_bytes: " + to_string(tx_bytes) +
                  " tx_dropped: " + to_string(tx_dropped) +
                  " tx_errors: " + to_string(tx_errors) +
                  " tx_packets: " + to_string(tx_packets) + "\n";
}

// Handle the main loop
void monitorInterface(const char *interfaceName, int clientSocket) {
  char buffer[BUFFER_SIZE];
  memset(buffer, 0, BUFFER_SIZE);

  while (isRunning) {
    // Get and send interface statistics
    getNetworkInterfaceStats(interfaceName, networkInterfaceStats);
    strncpy(buffer, networkInterfaceStats.c_str(), BUFFER_SIZE - 1);

    if (write(clientSocket, buffer, strlen(buffer)) < 0) {
      cerr << "Failed to send data: " << strerror(errno) << endl;
      break;
    }

    sleep(1);
  }
}

int main(int argc, char *argv[]) {
  struct sockaddr_un socketAddress;
  int clientSocket;

  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " <network-interface>" << endl;
    return EXIT_FAILURE;
  }

  char networkInterface[MAX_INTERFACE_NAME_LEN];
  strncpy(networkInterface, argv[1], MAX_INTERFACE_NAME_LEN - 1);

  // Create interface socket
  clientSocket = socket(AF_UNIX, SOCK_STREAM, 0);
  if (clientSocket < 0) {
    cerr << "Unable to create interface socket: " << strerror(errno) << endl;
    return EXIT_FAILURE;
  }

  // Configure address structure
  struct sockaddr_un serverAddress = {};
  socketAddress.sun_family = AF_UNIX;
  strncpy(socketAddress.sun_path, SOCKET_PATH,
          sizeof(socketAddress.sun_path) - 1);

  if (connect(clientSocket, (struct sockaddr *)&socketAddress,
              sizeof(socketAddress)) < 0) {
    cerr << "Connection to Network Monitor failed: " << strerror(errno) << endl;
    close(clientSocket);
    return EXIT_FAILURE;
  }

  write(clientSocket, "Ready", 5);

  char buffer[BUFFER_SIZE];
  int bytesRead = read(clientSocket, buffer, BUFFER_SIZE - 1);
  if (bytesRead < 0) {
    cerr << "Failed to read data from server: " << strerror(errno) << endl;
    close(clientSocket);
    return EXIT_FAILURE;
  }

  buffer[bytesRead] = '\0'; // Null-terminate the string

  if (strcmp(buffer, "Monitor") != 0) {
    cerr << "Unexpected message received, expected 'monitor', got: " << buffer
         << endl;
    close(clientSocket);
    return EXIT_FAILURE;
  }

  monitorInterface(networkInterface, clientSocket);

  close(clientSocket);
  unlink(SOCKET_PATH);

  return EXIT_SUCCESS;
}
