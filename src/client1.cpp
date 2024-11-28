#include <Client.hpp>
#include <iostream>

int main() {
    Client client{};
    client.add_tcp_port(4001);
    client.add_tcp_port(4002);

    if (const auto conn_result = client.connect_tcp();
        conn_result.has_error()) {
        std::cerr << "Failed to connect to port: " << conn_result.error()
                  << std::endl;
        return -1;
    }

    if (const auto epoll_result = client.setup_epoll();
        epoll_result.has_error()) {
        std::cerr << "Failed to setup epoll: " << epoll_result.error()
                  << std::endl;
        return -1;
    }

    return 0;
}