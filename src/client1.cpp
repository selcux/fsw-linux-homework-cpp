#include <Client.hpp>
#include <iostream>

int main() {
    auto client_result = Client::create();
    if (client_result.has_error()) {
        std::cerr << "Failed to create client: " << client_result.error()
                  << std::endl;
    }

    auto client = client_result.value();
    client.add_tcp_port(4001);
    client.add_tcp_port(4002);

    if (const auto conn_result = client.connect_tcp();
        conn_result.has_error()) {
        std::cerr << "Failed to connect to port: " << conn_result.error()
                  << std::endl;
    }

    std::cout << "Connection successful" << std::endl;

    return 0;
}