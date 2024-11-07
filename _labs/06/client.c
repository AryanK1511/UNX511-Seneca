#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>


#define SOCKET_PATH "/tmp/lab6"
#define BUFFER_SIZE 32

int main() {
  int client_socket;
  struct sockaddr_un client_addr;
  char buffer[BUFFER_SIZE];

  // Create client socket
  client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
  if (client_socket < 0) {
    perror("Client socket creation failed\n");
    return 1;
  }

  // Configure client address structure
  memset(&client_addr, 0, sizeof(struct sockaddr_un));
  client_addr.sun_family = AF_UNIX;
  strncpy(client_addr.sun_path, SOCKET_PATH, sizeof(client_addr.sun_path) - 1);

  // Connect to the server socket
  if (connect(client_socket, (struct sockaddr *)&client_addr,
              sizeof(struct sockaddr_un)) < 0) {
    perror("Connection to the server socket failed\n");
    close(client_socket);
    return 1;
  }

  while (1) {
    memset(buffer, 0, BUFFER_SIZE);

    // Read command from server
    if (read(client_socket, buffer, BUFFER_SIZE) < 0) {
      perror("Reading from server failed\n");
      break;
    }

    // Handle different commands
    if (strcmp(buffer, "Pid") == 0) {
      printf("A request for the client's pid has been received\n");
      snprintf(buffer, BUFFER_SIZE, "%d", getpid()); // Fill buffer with PID
      if (write(client_socket, buffer, strlen(buffer)) < 0) {
        perror("Sending PID response failed\n");
      }
    } else if (strcmp(buffer, "Sleep") == 0) {
      // Sleep for 5 seconds, then respond
      printf("This client is going to sleep for 5 seconds\n");
      sleep(5);
      strcpy(buffer, "Done");
      if (write(client_socket, buffer, strlen(buffer)) < 0) {
        perror("Sending Done response failed\n");
      }
    } else if (strcmp(buffer, "Quit") == 0) {
      // Quit the client loop and close the connection
      printf("This client is quitting\n");
      break;

    } else {
      printf("Received command is not recognized\n");
    }
  }

  // Close client socket
  close(client_socket);
  return 0;
}
