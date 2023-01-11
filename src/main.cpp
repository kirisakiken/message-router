#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <functional>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <uuid/uuid.h>
#include <thread>

const int kBufferSize = 1024;

class Client {
public:
  Client(int sock) : sock_(sock) {
    uuid_t uuid;
    uuid_generate(uuid);
    char id_str[37];
    uuid_unparse(uuid, id_str);
    id_ = id_str;
    name_ = "Guest[" + std::to_string(sock) + "]";
  }

  int sock() const { return sock_; }
  const std::string& id() const { return id_; }
  const std::string& name() const { return name_; }

private:
  int sock_;
  std::string id_;
  std::string name_;
};

class MessageRouter {
public:
  MessageRouter(int port) {
    sock_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_ < 0) {
      std::cerr << "Error creating socket" << std::endl;
      exit(1);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(sock_, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
      std::cerr << "Error binding socket" << std::endl;
      exit(1);
    }

    listen(sock_, 5);
  }

  [[noreturn]] void Run() {
    while (true) {
      struct sockaddr_in client_addr{};
      socklen_t client_addr_size = sizeof(client_addr);
      int client_sock = accept(sock_, (struct sockaddr*) &client_addr, &client_addr_size);

      if (client_sock < 0) {
        std::cerr << "Error accepting connection" << std::endl;
        continue;
      }

      std::cout << "New client connected" << std::endl;

      Client client(client_sock);
      clients_.insert({client.id(), client});

      BroadcastExcept(client, "[INFO]: " + client.name() + " connected.");
      BroadcastOnly(client, InitialConnectionPayload());

      std::thread t(&MessageRouter::HandleClient, this, client);
      t.detach();
    }
  }

private:
  [[nodiscard]] std::string InitialConnectionPayload() const {
    std::string payload("{\n");
    for (const auto& [id, c] : clients_) {
      payload += "\t\"" + id + "\"" + ",\n";
    }
    payload.erase(payload.end() - 2);
    payload += "}";

    return "[INITIAL MESSAGE]: Welcome. Available clients:\n" + payload;
  }

  void Broadcast(const std::string& message) {
    for (const auto& [_, client] : clients_) {
      send(client.sock(), message.c_str(), message.size(), 0);
    }
  }

  void BroadcastExcept(const Client& sender, const std::string& message) {
    for (const auto& [_, client] : clients_) {
      if (client.id() != sender.id()) {
        send(client.sock(), message.c_str(), message.size(), 0);
      }
    }
  }

  void BroadcastOnly(const Client& client, const std::string& message) {
    send(client.sock(), message.c_str(), message.size(), 0);
  }

  void HandleClient(const Client& client) {
    char buffer[kBufferSize];
    while (true) {
      int bytes_received = recv(client.sock(), buffer, kBufferSize - 1, 0);
      if (bytes_received <= 0) {
        break;
      }
      buffer[bytes_received] = '\0';

      std::stringstream message_stream(buffer);
      std::string message;
      while (std::getline(message_stream, message)) {
        BroadcastExcept(client, "[" + client.name() + "]: " + message);
      }
    }

    std::cout << "Client disconnected: " << client.name() << "|" << client.id() << std::endl;
    clients_.erase(client.id());
    BroadcastExcept(client, "[INFO]: " + client.name() + " disconnected.");
    close(client.sock());
  }

  int sock_;
  std::map<std::string, Client> clients_;
};

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: router PORT" << std::endl;
    return 1;
  }

  int port = std::stoi(argv[1]);
  MessageRouter router(port);
  router.Run();

  return 0;
}
