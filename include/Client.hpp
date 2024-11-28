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

    Result<void> run_and_receive();

   private:
    static constexpr int MAX_EVENTS = 10;
    static constexpr int BUFFER_SIZE = 256;

    std::set<int> tcp_ports;
    int epoll_fd;
    std::set<int> tcp_sockets;
    bool running;

    Result<int> create_socket();

    Result<void> set_flags(int fd, int flag);
};

#endif  // CLIENT_HPP