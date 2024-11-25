#include <iostream>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

using namespace std;

// ========== CONSTANTS ==========

// Path for the UNIX socket
const char *socketPath = "/tmp/networkMonitor";

// Buffer size for message communication
const int bufferSize = 256;

// Maximum amount of inerface monitors that can connect to this interface
const int maxConnections = 10;

// ========== GLOBAL VARIABLES ==========

// Flag to indicate whether the program is running
bool isRunning = true;

// Maintaining a vector of child PIDs since it is easier to clean them up later
vector<pid_t> childPIDs;

// ========== FUNCTION DEFINITIONS ==========

int initSocketConnection();
void monitorNetworkInterfaces(const vector<string> &interfaceList,
                              vector<pid_t> &childProcessIDs);
void acceptMonitorConnections(int masterSocket, fd_set &masterSet,
                              int &maxSocket, int monitorSockets[],
                              int &activeMonitors);
void processInterfaceMonitorData(int activeInterfaceMonitors,
                                 int monitorSockets[], fd_set &readSet);
void cleanupResources(int masterSocket, int activeMonitors,
                      int monitorSockets[], fd_set &masterSet,
                      vector<pid_t> &childProcessIDs);
static void signalHandler(const int signal);

// ========== CORE FUNCTIONS ==========

// Set up the socket for inter-process communication
int initSocketConnection() {
  // Structure to hold socket address information
  struct sockaddr_un socketAddress;
  // Descriptor for the master socket
  int masterSocket;

  // Create the master socket
  // Use UNIX domain sockets (AF_UNIX) with a stream type (SOCK_STREAM)
  masterSocket = socket(AF_UNIX, SOCK_STREAM, 0);

  // Check if socket creation failed
  if (masterSocket < 0) {
    cerr << "[networkMonitor.cpp] An error occurred while creating the master "
            "socket: "
         << strerror(errno) << endl;
    return EXIT_FAILURE;
  }

  // Configure the address structure for the socket
  // Zero out the structure
  memset(&socketAddress, 0, sizeof(socketAddress));

  // Set the address family to UNIX (local IPC)
  socketAddress.sun_family = AF_UNIX;

  // Copy the socket path into the address structure, ensuring no overflow
  strncpy(socketAddress.sun_path, socketPath,
          sizeof(socketAddress.sun_path) - 1);

  // Remove any existing socket file with the same path to prevent conflicts
  unlink(socketPath);

  // Bind the master socket to the specified path
  if (bind(masterSocket, (struct sockaddr *)&socketAddress,
           sizeof(socketAddress)) == -1) {
    cerr << "[networkMonitor.cpp] Error binding socket: " << strerror(errno)
         << endl;
    close(masterSocket);
    return EXIT_FAILURE;
  }

  // Return the master socket descriptor on success
  return masterSocket;
}

// Function to fork a child process and run the monitor program for a specific
// network interface
pid_t startMonitoringForInterface(const string &networkInterface) {
  pid_t processID = fork();

  if (processID == 0) {
    // Child Process
    // Execute the monitoring program for the network interface
    if (execlp("./intfMonitor", "./intfMonitor", networkInterface.c_str(),
               nullptr) == -1) {
      cerr << "[networkMonitor.cpp] Failed to execute intfMonitor for "
              "interface '"
           << networkInterface << "': " << strerror(errno) << endl;
      exit(EXIT_FAILURE);
    }
  } else if (processID < 0) {
    // Fork Failed
    cerr << "[networkMonitor.cpp] Fork failed for interface '"
         << networkInterface << "': " << strerror(errno) << endl;
    return -1;
  }

  // Parent Process
  // Return the PID of the child process
  return processID;
}

// Function to monitor multiple network interfaces
void monitorNetworkInterfaces(const vector<string> &interfaceList,
                              vector<pid_t> &childProcessIDs) {
  for (const auto &interfaceName : interfaceList) {
    // Start monitoring and capture the child's PID
    pid_t childPID = startMonitoringForInterface(interfaceName);
    if (childPID > 0) {
      // If fork was successful, store the child's PID
      childProcessIDs.push_back(childPID);
    } else {
      // Log error for failure to fork
      cerr << "[networkMonitor.cpp] Skipping monitoring for interface: "
           << interfaceName << endl;
    }
  }
}

// Accept connections from interface monitors
void acceptMonitorConnections(
    int masterSocket,     // The server socket accepting new connections
    fd_set &masterSet,    // The set of file descriptors for select()
    int &maxSocket,       // The highest-numbered socket, used by select()
    int monitorSockets[], // Array to store monitor sockets
    int &activeMonitors   // Counter for active monitor connections
) {
  // Buffer for reading and writing messages
  char buffer[bufferSize];

  // Accept a new connection from an interface monitor
  monitorSockets[activeMonitors] = accept(masterSocket, nullptr, nullptr);

  // Exit the function as the connection failed
  if (monitorSockets[activeMonitors] < 0) {
    cerr << "[networkMonitor.cpp] Error accepting connection from interface "
            "monitor: "
         << strerror(errno) << endl;
    return;
  }

  // Add the newly accepted socket to the master file descriptor set
  FD_SET(monitorSockets[activeMonitors], &masterSet);

  // Read the initial message (expected to be "Ready") from the client
  int bytesRead = read(monitorSockets[activeMonitors], buffer, bufferSize - 1);
  if (bytesRead < 0) {
    cerr << "[networkMonitor.cpp] Error reading from interface monitor: "
         << strerror(errno) << endl;
    // Close the faulty socket and exit
    close(monitorSockets[activeMonitors]);
    return;
  }

  // Null-terminate the buffer to safely use it as a C-style string
  buffer[bytesRead] = '\0';

  // Check if the received message is "ready_to_monitor"
  if (strcmp(buffer, "ready_to_monitor") == 0) {

    // Prepare the response message
    snprintf(buffer, bufferSize, "start_monitoring");

    // Send the message to the client
    if (write(monitorSockets[activeMonitors], buffer, strlen(buffer) + 1) ==
        -1) {
      cerr << "[networkMonitor.cpp] Error writing to interface monitor: "
           << strerror(errno) << endl;
      // Close the faulty socket and exit
      close(monitorSockets[activeMonitors]);
      return;
    }
  } else {
    cerr << "[networkMonitor.cpp] Unexpected message from interface monitor: "
         << buffer << endl;
    // Close the faulty socket and exit
    close(monitorSockets[activeMonitors]);
    return;
  }

  // Update the maximum socket number for select()
  maxSocket = max(maxSocket, monitorSockets[activeMonitors]);

  // Increment the count of active monitors
  ++activeMonitors;
}

// Process data received from the interface monitors
void processInterfaceMonitorData(
    int activeInterfaceMonitors, // Number of active interface monitors
    int monitorSockets[],        // Array of sockets for each interface monitor
    fd_set &readSet              // Set of file descriptors ready for reading
) {
  char buffer[bufferSize]; // Buffer to store incoming data from monitors

  // Iterate through all active interface monitor sockets
  for (int i = 0; i < activeInterfaceMonitors; ++i) {
    // Check if the current socket is ready for reading
    if (FD_ISSET(monitorSockets[i], &readSet)) {
      // Clear the buffer to avoid carrying over old data
      bzero(buffer, bufferSize);

      // Read data from the current monitor socket
      int bytesRead = read(monitorSockets[i], buffer, bufferSize);

      if (bytesRead > 0) {
        // Successfully read data; print it to the console
        cout << "[networkMonitor.cpp] Interface monitor [" << i
             << "] - Data received: \n"
             << buffer << endl;
      } else if (bytesRead == -1) {
        // Error while reading from the socket
        cerr << "[networkMonitor.cpp] networkMonitor - Error reading data from "
                "interface monitor ["
             << i << "]: " << strerror(errno) << endl;
      } else if (bytesRead == 0) {
        // Connection closed by the client
        cerr << "[networkMonitor.cpp] networkMonitor - Interface monitor [" << i
             << "] has closed the connection." << endl;

        // Close the socket and mark it as inactive
        close(monitorSockets[i]);
        monitorSockets[i] = -1; // Optionally mark the socket as invalid
      }
    }
  }
}

// Clean up resources and notify interface monitors before shutdown
void cleanupResources(
    int masterSocket,     // The master socket to be closed
    int activeMonitors,   // Number of active monitor sockets
    int monitorSockets[], // Array of monitor sockets to be cleaned up
    fd_set &masterSet,    // Set of file descriptors for active sockets
    vector<pid_t> &childProcessIDs) {
  // Terminate child processes gracefully
  for (pid_t pid : childProcessIDs) {
    cout
        << "[networkMonitor.cpp] Sending SIGUSR1 signal to child process (PID: "
        << pid << ")..." << endl;

    if (kill(pid, SIGUSR1) == -1) {
      cerr << "[networkMonitor.cpp] Failed to send SIGUSR1 to process " << pid
           << ": " << strerror(errno) << endl;
    }
  }

  // Wait for all child processes to terminate
  while (wait(nullptr) > 0) {
    cout << "[networkMonitor.cpp] A child process has exited." << endl;
  }

  // Handle monitor sockets
  for (int i = 0; i < activeMonitors; ++i) {
    // Remove socket from the master set
    FD_CLR(monitorSockets[i], &masterSet);

    // Close the monitor socket
    close(monitorSockets[i]);
    cout << "[networkMonitor.cpp] Closed monitor socket " << monitorSockets[i]
         << endl;
  }

  // Close the master socket
  close(masterSocket);
  cout << "[networkMonitor.cpp] Master socket closed." << endl;

  // Unlink the socket path to remove the file created for the UNIX domain
  // socket
  if (unlink(socketPath) == -1) {
    cerr << "[networkMonitor.cpp] Failed to unlink socket path: "
         << strerror(errno) << endl;
  } else {
    cout << "[networkMonitor.cpp] Socket path unlinked successfully." << endl;
  }
}

// ========== UTILITY FUNCTIONS ==========

// Handle termination signal (e.g., Ctrl+C)
static void signalHandler(const int signal) {
  // Set the running flag to false to exit the main loop if SIGINT (Ctrl+C) is
  // received
  if (signal == SIGINT) {
    cout << endl << "[networkMonitor.cpp] CTRL-C - shutting down" << endl;
    isRunning = false;
  } else {
    cout << endl << "[networkMonitor.cpp] undefined signal" << endl;
  }
}

// ==================== MAIN PROGRAM ====================
int main() {
  // Declare a variable to store the number of interfaces to monitor
  int numInterfaces;
  cout << "Please specify the number of interfaces to monitor: ";
  cin >> numInterfaces;

  // Declare a vector to hold the names of interfaces
  vector<string> interfaceNames(numInterfaces);

  // Get the names of the interfaces to monitor from the user
  for (int i = 0; i < numInterfaces; ++i) {
    cout << "Number " << i + 1 << ": ";
    cin >> interfaceNames[i];
  }

  // Set up the signal handler for graceful termination
  // Handle SIGINT (Ctrl+C)
  // No additional signals blocked during handler
  struct sigaction sigAction;
  sigAction.sa_handler = signalHandler;
  sigemptyset(&sigAction.sa_mask);
  sigAction.sa_flags = 0;

  // Set the signal handler for SIGINT (Ctrl+C)
  if (sigaction(SIGINT, &sigAction, nullptr) < 0) {
    cerr << "[networkMonitor.cpp] Error setting up signal handler: "
         << strerror(errno) << endl;
    // Exit if signal handler setup fails
    exit(EXIT_FAILURE);
  }

  // Set up the master socket to accept incoming connections
  int masterSocket = initSocketConnection();

  // Initially, the master socket is the
  // highest-numbered socket
  int maxSocket = masterSocket;
  fd_set masterSet, readSet;

  // Initialize the read set for select()
  FD_ZERO(&readSet);

  // Initialize the master set for select()
  FD_ZERO(&masterSet);

  // Add the master socket to the master set
  FD_SET(masterSocket, &masterSet);

  // Array to hold monitor sockets
  int monitorSockets[maxConnections];

  // Keeps track of the number of active monitor connections
  int activeMonitors = 0;

  // Create child processes for each interface to monitor
  monitorNetworkInterfaces(interfaceNames, childPIDs);

  // Start listening for incoming connections on the master socket
  if (listen(masterSocket, maxConnections) == -1) {
    cerr << "[networkMonitor.cpp] Error starting listener: " << strerror(errno)
         << endl;
    // Cleanup and return failure if listen fails
    cleanupResources(masterSocket, activeMonitors, monitorSockets, masterSet,
                     childPIDs);
    return EXIT_FAILURE;
  }

  // Main event loop for monitoring sockets
  while (isRunning) {
    // Copy the master set to readSet for select()
    readSet = masterSet;

    // Use select() to monitor the sockets for activity
    int result = select(maxSocket + 1, &readSet, nullptr, nullptr, nullptr);

    if (result < 0) {
      // If interrupted by signal, restart select()
      if (errno == EINTR)
        continue;
      cerr << "[networkMonitor.cpp] Error in select: " << strerror(errno)
           << endl;
      // Exit if select() fails
      break;
    }

    if (FD_ISSET(masterSocket, &readSet)) {
      // If there's activity on the master socket, accept a new connection
      acceptMonitorConnections(masterSocket, masterSet, maxSocket,
                               monitorSockets, activeMonitors);
    } else {
      // If there's activity on any child socket, read data from interface
      // monitor
      processInterfaceMonitorData(activeMonitors, monitorSockets, readSet);
    }
  }

  // Cleanup resources when the program exits
  cleanupResources(masterSocket, activeMonitors, monitorSockets, masterSet,
                   childPIDs);

  return EXIT_SUCCESS;
}
