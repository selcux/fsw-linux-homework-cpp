#include "ClientControl.hpp"

int main() {
    auto client_result = ClientControl::create();
    if (client_result.has_error()) {
        std::cerr << "Failed to create client: " << client_result.error()
                  << std::endl;
        return -1;
    }

    auto client = client_result.value();

    // Get SERVER_ADDR env variable
    const char* server_addr = std::getenv("SERVER_ADDR");
    if (server_addr != nullptr) {
        client.set_server_addr(server_addr);
    }
    
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