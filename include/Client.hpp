#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <set>

#include "Result.hpp"

class Client {
   public:
    ~Client();

    static Result<Client> create();

    void add_tcp_port(int port);

    Result<void> connect_tcp();

   private:
    std::set<int> tcp_ports;
    int epoll_fd;

    Client() = default;

    Result<int> create_socket();
};

#endif  // CLIENT_HPP