#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>


#define SOCKET_PATH "/tmp/lab6"
#define BUFFER_SIZE 32

int main() {
  int server_socket, client_socket;
  struct sockaddr_un server_addr;
  char buffer[BUFFER_SIZE];

  // Create server socket
  server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
  if (server_socket < 0) {
    perror("Server socket creation failed\n");
    return 1;
  }

  // Configure server address structure
  memset(&server_addr, 0, sizeof(struct sockaddr_un));
  server_addr.sun_family = AF_UNIX;
  strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

  // Bind server socket to socket path
  if (bind(server_socket, (struct sockaddr *)&server_addr,
           sizeof(struct sockaddr_un)) < 0) {
    perror("Server bind failed\n");
    close(server_socket);
    return 1;
  }

  // Listen for incoming connections
  if (listen(server_socket, 1) < 0) {
    perror("Server listen failed\n");
    close(server_socket);
    unlink(SOCKET_PATH);
    return 1;
  }

  printf("Waiting for the client...\n");

  // Accept client connection
  client_socket = accept(server_socket, NULL, NULL);
  if (client_socket < 0) {
    perror("Server accept failed\n");
    close(server_socket);
    unlink(SOCKET_PATH);
    return 1;
  }

  printf("Client connected to the server...\n");
  printf("server: accept()\n");

  // Send Pid command
  strcpy(buffer, "Pid");
  if (write(client_socket, buffer, strlen(buffer)) < 0) {
    perror("Sending Pid command failed\n");
  }
  printf("The server requests the client's pid\n");

  // Receive client response
  memset(buffer, 0, BUFFER_SIZE);
  if (read(client_socket, buffer, BUFFER_SIZE) < 0) {
    perror("Error receiving response from client\n");
  }
  printf("server: This client has pid %s\n", buffer);

  // Send Sleep command
  strcpy(buffer, "Sleep");
  if (write(client_socket, buffer, strlen(buffer)) < 0) {
    perror("Sending Sleep command failed\n");
  }
  printf("The server requests the client to sleep\n");

  // Receive client response (Done)
  memset(buffer, 0, BUFFER_SIZE);
  if (read(client_socket, buffer, BUFFER_SIZE) < 0) {
    perror("Receiving response from client failed\n");
  }

  // Send Quit command
  strcpy(buffer, "Quit");
  if (write(client_socket, buffer, strlen(buffer)) < 0) {
    perror("Sending Quit command failed\n");
  }
  printf("The server requests the client to quit\n");

  // Close the client socket
  close(client_socket);
  close(server_socket);
  unlink(SOCKET_PATH);

  return 0;
}
