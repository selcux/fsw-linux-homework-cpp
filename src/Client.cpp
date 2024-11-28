#include "Client.hpp"

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

Client::~Client() {
    for (auto socket : tcp_sockets) {
        close(socket);
    }

    if (epoll_fd >= 0) {
        close(epoll_fd);
    }
}

void Client::add_tcp_port(int port) { tcp_ports.insert(port); }

Result<void> Client::connect_tcp() {
    for (const auto port : tcp_ports) {
        std::cout << "Connecting to port: " << port << std::endl;

        auto sock_result = create_socket();
        if (sock_result.has_error()) {
            return sock_result.error();
        }

        const int sock = sock_result.value();

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

        tcp_sockets.insert(sock);
    }

    return Result<void>::success();
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

Result<void> Client::setup_epoll() {
    epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        return ClientError::make_error(
            ClientError::Code::EpollCreationFailed,
            "epoll_create1() failed with errno: " + std::to_string(errno));
    }

    for (auto socket : tcp_sockets) {
        epoll_event event{};
        event.events = EPOLLIN | EPOLLET;
        event.data.fd = socket;

        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket, &event) < 0) {
            ClientError::make_error(ClientError::Code::EpollCreationFailed,
                                    "Socket " + std::to_string(socket) +
                                        " failed to be added to epoll.");
        }
    }

    return Result<void>::success();
}
