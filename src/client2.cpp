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

    if (const auto run_result = client.run(); run_result.has_error()) {
        std::cerr << "Failed to run client: " << run_result.error()
                  << std::endl;
        return -1;
    }

    return 0;
}