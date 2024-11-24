#include <iostream>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#define SOCKET_PATH "/tmp/networkMonitor"
#define BUFFER_SIZE 256
#define MAX_CONNECTIONS 10

using namespace std;

bool isRunning = true;

// Handle termination signal
static void handleSignal(int signal) {
  if (signal == SIGINT) {
    cout << endl << "networkMonitor: ctrl-C - shutting down" << endl;
    isRunning = false;
  } else {
    cout << endl << "networkMonitor: undefined signal" << endl;
  }
}

// Set up the socket for communication
int setupSocket() {
  struct sockaddr_un socketAddress;
  int masterSocket;

  // Create master socket
  masterSocket = socket(AF_UNIX, SOCK_STREAM, 0);
  if (masterSocket < 0) {
    cerr << "Unable to create master socket: " << strerror(errno) << endl;
    return EXIT_FAILURE;
  }

  // Configure address structure
  memset(&socketAddress, 0, sizeof(socketAddress));
  socketAddress.sun_family = AF_UNIX;
  strncpy(socketAddress.sun_path, SOCKET_PATH,
          sizeof(socketAddress.sun_path) - 1);

  unlink(SOCKET_PATH);

  // Bind master socket to socket path
  if (bind(masterSocket, (struct sockaddr *)&socketAddress,
           sizeof(socketAddress)) == -1) {
    cerr << "Error binding socket: " << strerror(errno) << endl;
    return EXIT_FAILURE;
  }

  return masterSocket;
}

// Fork child processes to monitor each interface
void monitorInterfaces(const vector<string> &interfaceNames) {
  for (const auto &interfaceName : interfaceNames) {
    if (fork() == 0) {
      if (execlp("./intfMonitor", "./intfMonitor", interfaceName.c_str(),
                 nullptr) == -1) {
        cerr << "Failed to execute intfMonitor for " << interfaceName << " : "
             << strerror(errno) << endl;
      }
    }
  }
}

// Handle incoming interface monitor connections
void acceptMonitorConnections(int masterSocket, fd_set &masterSet,
                              int &maxSocket, int monitorSockets[],
                              int &activeMonitors) {
  char buffer[BUFFER_SIZE];
  int bytesRead;

  monitorSockets[activeMonitors] = accept(masterSocket, nullptr, nullptr);
  if (monitorSockets[activeMonitors] < 0) {
    cerr << "Error accepting interface monitor: " << strerror(errno) << endl;
  }

  FD_SET(monitorSockets[activeMonitors], &masterSet);
  bytesRead = read(monitorSockets[activeMonitors], buffer, BUFFER_SIZE);

  if (bytesRead < 0) {
    cerr << "Error reading from interface monitor: " << strerror(errno) << endl;
  } else {
    buffer[bytesRead] = '\0';

    // Write "Monitor" to the buffer if Interface Monitor is ready
    if (strcmp(buffer, "Ready") == 0) {
      snprintf(buffer, BUFFER_SIZE, "Monitor");
      if (write(monitorSockets[activeMonitors], buffer, strlen(buffer) + 1) ==
          -1) {
        cerr << "Error writing to interface monitor: " << strerror(errno)
             << endl;
      }
    }
  }

  maxSocket = max(maxSocket, monitorSockets[activeMonitors]);
  ++activeMonitors;
}

// Process data from connected interface monitors
void processInterfaceMonitorData(int activeInterfaceMonitors,
                                 int monitorSockets[], fd_set &readSet,
                                 char buffer[]) {
  for (int i = 0; i < activeInterfaceMonitors; ++i) {
    if (FD_ISSET(monitorSockets[i], &readSet)) {
      bzero(buffer, BUFFER_SIZE);
      int bytesRead = read(monitorSockets[i], buffer, BUFFER_SIZE);
      if (bytesRead > 0) {
        cout << buffer << endl;
      } else if (bytesRead == -1) {
        cerr << "Error reading data from interface monitor: " << strerror(errno)
             << endl;
      }
    }
  }
}

// Clean up and notify interface monitors before shutting down
void cleanUpAndNotifyInterfaceMonitors(int activeInterfaceMonitors,
                                       int monitorSockets[],
                                       fd_set &masterSet) {
  char buffer[BUFFER_SIZE];

  for (int i = 0; i < activeInterfaceMonitors; ++i) {
    cout << "Sending quit command to Interface Monitor " << i + 1 << endl;

    // Send the "Quit" message to the monitor
    snprintf(buffer, BUFFER_SIZE, "Quit");
    if (write(monitorSockets[i], buffer, strlen(buffer) + 1) == -1) {
      cerr << "Error writing 'Quit' command to Interface Monitor " << i + 1
           << ": " << strerror(errno) << endl;
    }

    // Close the socket
    if (close(monitorSockets[i]) == -1) {
      cerr << "Error closing socket for Interface Monitor " << i + 1 << ": "
           << strerror(errno) << endl;
    } else {
      cout << "Successfully closed socket for Interface Monitor " << i + 1
           << endl;
    }

    // Remove the socket from the master set
    FD_CLR(monitorSockets[i], &masterSet);
  }

  // Unlink the socket path
  if (unlink(SOCKET_PATH) == -1) {
    cerr << "Error unlinking the socket path: " << strerror(errno) << endl;
  } else {
    cout << "Socket path unlinked successfully." << endl;
  }

  cout << "Cleanup completed." << endl;
}

// Main logic of the program
int main() {
  int numInterfaces;
  cout << "Enter the number of interfaces to monitor: ";
  cin >> numInterfaces;

  vector<string> interfaceNames(numInterfaces);
  for (int i = 0; i < numInterfaces; ++i) {
    cout << "Interface #" << i + 1 << ": ";
    cin >> interfaceNames[i];
  }
  cout << endl;

  // Set up signal handler
  struct sigaction sigAction;
  sigAction.sa_handler = handleSignal;
  sigemptyset(&sigAction.sa_mask);
  sigAction.sa_flags = 0;
  sigaction(SIGINT, &sigAction, nullptr);

  // Set up socket
  int masterSocket = setupSocket();
  int maxSocket = masterSocket;
  fd_set masterSet, readSet;
  FD_ZERO(&readSet);
  FD_ZERO(&masterSet);
  FD_SET(masterSocket, &masterSet);
  int monitorSockets[MAX_CONNECTIONS];
  int activeInterfaceMonitors = 0;

  // Start monitoring interfaces
  if (fork() == 0) {
    monitorInterfaces(interfaceNames);
  } else {
    if (listen(masterSocket, MAX_CONNECTIONS) == -1) {
      cerr << "Error starting listener: " << strerror(errno) << endl;
      return EXIT_FAILURE;
    }

    char buffer[BUFFER_SIZE];
    while (isRunning) {
      readSet = masterSet;
      int result = select(maxSocket + 1, &readSet, nullptr, nullptr, nullptr);

      if (result < 0) {
        cerr << "Error in select: " << strerror(errno) << endl;
      } else {
        if (FD_ISSET(masterSocket, &readSet)) {
          acceptMonitorConnections(masterSocket, masterSet, maxSocket,
                                   monitorSockets, activeInterfaceMonitors);
        } else {
          // Handle interface monitor data reading from the already connected
          // socket
          processInterfaceMonitorData(activeInterfaceMonitors, monitorSockets,
                                      readSet, buffer);
        }
      }
      sleep(1);
    }

    // Clean up and notify interface monitors
    cleanUpAndNotifyInterfaceMonitors(activeInterfaceMonitors, monitorSockets,
                                      masterSet);

    // Wait for all child processes to finish
    while (wait(nullptr) > 0)
      ;

    close(masterSocket);
    unlink(SOCKET_PATH);
  }

  return EXIT_SUCCESS;
}
