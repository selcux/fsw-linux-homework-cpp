#include "Client.hpp"

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

Client::~Client() {
    if (epoll_fd >= 0) {
        close(epoll_fd);
    }
}

Result<Client> Client::create() {
    try {
        Client client{};

        client.epoll_fd = epoll_create1(0);
        if (client.epoll_fd < 0) {
            return ClientError::make_error(
                ClientError::Code::EpollCreationFailed,
                "epoll_create1() failed with errno: " + std::to_string(errno));
        }

        return client;
    } catch (const std::exception& e) {
        return ClientError::make_error(
            ClientError::Code::Unknown,
            std::string("Unexpected error: ") + e.what());
    }
}

void Client::add_tcp_port(int port) { tcp_ports.insert(port); }

Result<void> Client::connect_tcp() {
    auto sock_result = this->create_socket();
    if (sock_result.has_error()) {
        return sock_result.error();
    }

    const int sock = sock_result.value();

    for (const auto port : tcp_ports) {
        std::cout << "Connecting to port: " << port << std::endl;

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);

        if (connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) <
            0) {
            return ClientError::make_error(
                ClientError::Code::ConnectFailed,
                "Connecting to port " + std::to_string(port) + " failed.");
        }
    }
}

Result<int> Client::create_socket() {
    const auto sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return ClientError::make_error(
            ClientError::Code::SocketCreationFailed,
            "socket() failed with errno: " + std::to_string(errno));
    }

    // Set non-blocking
    int flags = fcntl(sock, F_GETFL);
    if (flags == -1) {
        return ClientError::make_error(
            ClientError::Code::GetFlagsFailed,
            "Could not get socket flags with errno: " + std::to_string(errno));
    }

    flags |= O_NONBLOCK;
    if (fcntl(sock, F_SETFL, flags) == -1) {
        return ClientError::make_error(
            ClientError::Code::SetFlagsFailed,
            "Could not set socket flags with errno: " + std::to_string(errno));
    }

    return sock;
}
