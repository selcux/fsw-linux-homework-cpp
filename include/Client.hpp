#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <set>
#include <vector>

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

    int epoll_fd;
    bool running;
    std::set<int> tcp_ports;
    std::vector<int> tcp_sockets;
    std::vector<std::string> received_data;

    Result<int> create_socket();

    Result<void> set_flags(int fd, int flag);

    static void print_json(int64_t timestamp,
                           const std::vector<std::string> &received_data);
};

#endif  // CLIENT_HPP