#include "Client.hpp"

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <cstring>
#include <string>

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

        if (connect(sock, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) <
            0) {
            return ClientError::make_error(
                ClientError::Code::ConnectFailed,
                "Connecting to port " + std::to_string(port) + " failed.");
        }

        // Set non-blocking
        if (const auto set_result = set_flags(sock, O_NONBLOCK);
            set_result.has_error()) {
            return set_result.error();
        }

        tcp_sockets.emplace_back(sock);
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

    return sock;
}

Result<void> Client::set_flags(int fd, int flag) {
    int flags = fcntl(fd, F_GETFL);
    if (flags == -1) {
        return ClientError::make_error(
            ClientError::Code::GetFlagsFailed,
            "Could not get the flags of the file descriptor " +
                std::to_string(fd));
    }

    flags |= flag;
    if (fcntl(fd, F_SETFL, flags) == -1) {
        return ClientError::make_error(
            ClientError::Code::SetFlagsFailed,
            "Could not set the flags of the file descriptor " +
                std::to_string(fd));
    }

    return Result<void>::success();
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

    // Create received_data as the same size with tcp_sockets
    received_data.resize(tcp_sockets.size());

    return Result<void>::success();
}

Result<void> Client::run_and_receive() {
    std::array<epoll_event, MAX_EVENTS> events;
    char buffer[BUFFER_SIZE];
    running = true;

    while (running) {
        int num_events = epoll_wait(epoll_fd, events.data(), MAX_EVENTS, 1000);

        if (num_events < 0) {
            // Handle interrupted system call
            if (errno == EINTR) {
                continue;  // Restart if interrupted by a signal
            }

            return ClientError::make_error(ClientError::Code::EpollWaitFailed,
                                           "Failed waiting for events");
        }

        for (int i = 0; i < num_events; ++i) {
            if (events[i].events & EPOLLIN) {
                std::memset(buffer, 0, BUFFER_SIZE);
                int bytes_read =
                    read(events[i].data.fd, buffer, BUFFER_SIZE - 1);

                if (bytes_read > 0) {
                    // Find which socket this is
                    for (auto j = 0; j < tcp_sockets.size(); ++j) {
                        if (tcp_sockets[j] == events[i].data.fd) {
                            received_data[j] = buffer;
                            std::cout << j << ". " << tcp_sockets[j] << ": "
                                      << received_data[j] << std::endl;
                            break;
                        }
                    }
                }
            }
        }
    }

    return Result<void>::success();
}

void Client::print_json(int64_t timestamp,
                        const std::vector<std::string> &data) {
    // Join data into comma separated string
    // with fields as out1, out2, out3, ...
    std::string data_str;
    for (size_t i = 0; i < data.size(); ++i) {
        data_str += "out" + std::to_string(i + 1) + ": " + data[i];
        if (i < data.size() - 1) {
            data_str += ", ";
        }
    }

    std::cout << "{\"timestamp\": " << timestamp << ", " << data_str << "}"
              << std::endl;
}