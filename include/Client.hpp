#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <netinet/in.h>

#include <atomic>
#include <set>
#include <string>
#include <vector>

#include "Result.hpp"

class Client {
   public:
    static constexpr auto DEFAULT_SERVER_ADDR = "127.0.0.1";

    static Result<Client> create();

    ~Client();

    void add_tcp_port(int port);

    Result<void> connect_tcp();

    Result<void> setup_epoll();

    Result<void> listen_and_receive();

    void set_server_addr(std::string addr);

    Result<void> run();

    void set_interval_ms(int ms);

   protected:
    static constexpr int MAX_EVENTS = 10;

    int epoll_fd;
    std::string server_addr;
    std::set<int> tcp_ports;
    std::vector<int> tcp_sockets;
    std::vector<std::string> received_data;
    int interval_ms = 100;

    // Signal handling
    static std::atomic<bool> running;
    static void signal_handler(int);

    Client() = default;

    virtual Result<void> on_receive(int socket_index, const std::string &data);

    static Result<int> create_tcp_socket();

    static Result<void> set_flags(int fd, int flag);

    void reset_data();

    static void print_json(int64_t timestamp,
                           const std::vector<std::string> &received_data);

    static Result<void> setup_signal_handling();

    virtual void cleanup();

    Result<sockaddr_in> get_server_sockaddr(int port);
};

#endif  // CLIENT_HPP