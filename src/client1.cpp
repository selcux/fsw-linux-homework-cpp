#include <Client.hpp>
#include <iostream>

int main() {
    Client client{};
    client.add_tcp_port(4001);
    client.add_tcp_port(4002);
    client.add_tcp_port(4003);

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

    if (const auto event_loop_result = client.run_and_receive();
        event_loop_result.has_error()) {
        std::cerr << "Event loop failed: " << event_loop_result.error()
                  << std::endl;
        return -1;
    }

    return 0;
}