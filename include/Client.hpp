#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <set>

#include "Result.hpp"

class Client {
   public:
    Client() = default;

    ~Client();

    void add_tcp_port(int port);

    Result<void> connect_tcp();

    Result<void> setup_epoll();

   private:
    std::set<int> tcp_ports;
    int epoll_fd;
    std::set<int> tcp_sockets;

    Result<int> create_socket();

    Result<void> set_flags(int fd, int flag);
};

#endif  // CLIENT_HPP