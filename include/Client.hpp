#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <atomic>
#include <set>
#include <vector>

#include "Result.hpp"

class Client {
   public:
    static Result<Client> create();

    ~Client();

    void add_tcp_port(int port);

    Result<void> connect_tcp();

    Result<void> setup_epoll();

    Result<void> run_and_receive();

   protected:
    static constexpr int MAX_EVENTS = 10;

    int epoll_fd;
    std::set<int> tcp_ports;
    std::vector<int> tcp_sockets;
    std::vector<std::string> received_data;

    // Signal handling
    static std::atomic<bool> running;
    static void signal_handler(int);

    Client() = default;

    virtual int get_interval() const;

    virtual Result<void> on_receive(int socket_index, const std::string &data);

    static Result<int> create_tcp_socket();

    static Result<void> set_flags(int fd, int flag);

    void reset_data();

    static void print_json(int64_t timestamp,
                           const std::vector<std::string> &received_data);

    static Result<void> setup_signal_handling();

    virtual void cleanup();
};

#endif  // CLIENT_HPP