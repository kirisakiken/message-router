#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <vector>

const int opt = 1;
const int max_clients = 30;

int main(int argc, char** argv) {
  int master_socket, client_sockets[max_clients];
  int max_sd;
  int activity, val_read;
  fd_set read_fds;
  int address_len;
  char buffer[1025];

  // mock server message
  char* message = "(Server): Connected.\n";

  // init socket config
  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(8080);
  std::cout << "[INFO]: Initialized socket config" << std::endl;

  // init all available sockets as 0
  for (int& s : client_sockets)
    s = 0;

  std::cout << "[INFO]: Initialized client sockets. Available sockets: " << max_clients << std::endl;

  // init master socket
  master_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (master_socket == 0) {
    std::cerr << "[ERROR]: Failed to initialize master socket" << std::endl;
    exit(EXIT_FAILURE);
  }
  std::cout << "[INFO]: Initialized master socket" << std::endl;

  // enable multiple connections for master socket
  int set_sock_opt_result = setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
  if (set_sock_opt_result < 0) {
    std::cerr << "[ERROR]: Failed to configure master socket" << std::endl;
    exit(EXIT_FAILURE);
  }
  std::cout << "[INFO]: Configured multi connection options" << std::endl;

  // bind master socket
  int bind_result = bind(master_socket, (struct sockaddr*)&address, sizeof(address));
  if (bind_result < 0) {
    std::cerr << "[ERROR]: Failed to bind master socket" << std::endl;
    exit(EXIT_FAILURE);
  }
  std::cout << "[INFO]: Bound master socket" << std::endl;

  // configure max pending connections
  int listen_result = listen(master_socket, 3);
  if (listen_result < 0) {
    std::cerr << "[ERROR]: Failed to configure max pending connections of master socket" << std::endl;
    exit(EXIT_FAILURE);
  }
  std::cout << "[INFO]: Configured max pending connections" << std::endl;

  std::cout << "[INFO]: Listening for incoming connections . . ." << std::endl;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
  while (true) {
    // clear socket set
    FD_ZERO(&read_fds);

    // add master socket to socket set
    FD_SET(master_socket, &read_fds);
    max_sd = master_socket;

    // add client sockets to socket set
    for (const int& sd : client_sockets) {
      // add client socket to socket set if valid
      if (sd > 0)
        FD_SET(sd, &read_fds);

      // update max sd which is used for select function
      if (sd > max_sd)
        max_sd = sd;
    }

    // wait for socket events, null timeout (infinite wait)
    activity = select(max_sd + 1, &read_fds, nullptr, nullptr, nullptr);
    if ((activity < 0) && (errno != EINTR))
      std::cout << "[WARNING]: Error during socket selection" << std::endl;
    std::cout << "[INFO]: Detected socket event" << std::endl;

    // accept incoming connections (FD_ISSET master is valid, meaning it is an incoming connection)
    bool is_master_set = FD_ISSET(master_socket, &read_fds);
    if (is_master_set) {
      // try accept incoming connection
      int new_socket = accept(master_socket, (struct sockaddr*)&address, (socklen_t*)&address_len);
      if (new_socket < 0) {
        std::cout << "[WARNING]: Failed to accept incoming connection" << std::endl;
        continue;
      }

      std::cout << "[INFO]: Client connected: socket file descriptor: " << new_socket;
      std::cout << " IP: " << inet_ntoa(address.sin_addr) << " PORT: " << ntohs(address.sin_port) << std::endl;

      // welcome message
      size_t message_len = strlen(message);
      size_t send_result = send(new_socket, message, message_len, 0);
      if (send_result != message_len)
        std::cout << "[WARNING]: Error sending welcome message" << std::endl;
      std::cout << "[INFO]: Sent welcome message successfully" << std::endl;

      // store incoming connection
      for (int& client : client_sockets) {
        if (client != 0)
          continue;

        client = new_socket;
        break;
      }
    }
    // else, it is input output from other clients
    else {
      for (int& sd : client_sockets) {
        bool client_is_set = FD_ISSET(sd, &read_fds);
        if (client_is_set) {
          // read incoming payload
          val_read = read(sd, buffer, 1024);
          if (val_read == 0) { // meaning that this is disconnect event
            getpeername(sd, (struct sockaddr*)&address, (socklen_t*)&address_len);
            std::cout << "[INFO]: Client disconnected. IP: " << inet_ntoa(address.sin_addr);
            std::cout << " PORT: " << ntohs(address.sin_port) << std::endl;

            close(sd);
            sd = 0;
            continue;
          }

          // echo incoming message to all except sender
          buffer[val_read] = '\0';
          for (int& c : client_sockets) {
            if (c == sd)
              continue;

            send(c, buffer, strlen(buffer), 0); // TODO: send result check (health check)
          }
        }
      }
    }
  }
#pragma clang diagnostic pop

  return 0;
}