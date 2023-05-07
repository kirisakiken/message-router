#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

int main(int argc, char** argv) {
  int server_file_descriptor;
  int new_socket; // stands for incoming client connection
  int value_read; // stands for value read from newSocket client

  struct sockaddr_in address;
  int opt = 1;
  int address_len = sizeof(address);
  char buffer[1024] = {0};

  // Construct Socket File Descriptor
  server_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
  if (server_file_descriptor == 0) {
    perror("[ERROR]: Failed to create socket file descriptor.");
    exit(EXIT_FAILURE);
  }

  // Set Socket Options
  int set_sock_opts_result = setsockopt(
      server_file_descriptor,
      SOL_SOCKET,
      SO_REUSEADDR | SO_REUSEPORT,
      &opt,
      sizeof(opt)
      );
  if (set_sock_opts_result) {
    perror("[ERROR]: Failed to set socket options.");
    exit(EXIT_FAILURE);
  }

  // Set Address Info
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(8080);

  // Bind the socket to Address/Port
  int bind_result = bind(server_file_descriptor, (struct sockaddr*) &address, sizeof(address));
  if (bind_result < 0) {
    perror("[ERROR]: Failed to bind socket.");
    exit(EXIT_FAILURE);
  }

  // Start listening for connections
  int listen_result = listen(server_file_descriptor, 3); // max 3
  if (listen_result < 0) {
    perror("[ERROR]: Error while listening for incoming connections.");
    exit(EXIT_FAILURE);
  }

  // Accept Incoming connections
  new_socket = accept(server_file_descriptor, (struct sockaddr*) &address, (socklen_t*) &address_len);
  if (new_socket < 0) {
    perror("[ERROR]: Error while accepting incoming connection.");
    exit(EXIT_FAILURE);
  }

  // Read data from client
  value_read = read(new_socket, buffer, 1024);
  std::cout << "[CLIENT MESSAGE]: " << buffer << std::endl;

  // Write data to client
  const char *sample_server_message = "this is server message!";
  send(new_socket, sample_server_message, strlen(sample_server_message), 0);

  // Close allocations
  close(new_socket);
  close(server_file_descriptor);

  return 0;
}