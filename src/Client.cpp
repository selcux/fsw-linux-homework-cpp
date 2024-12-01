#include "Client.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <csignal>
#include <string>

std::atomic<bool> Client::running(true);

void Client::signal_handler(int) { running = false; }

Result<Client> Client::create() {
    if (const auto setup_result = setup_signal_handling();
        setup_result.has_error()) {
        return setup_result.error();
    }

    return Client{};
}

Client::~Client() { Client::cleanup(); }

void Client::add_tcp_port(int port) { tcp_ports.insert(port); }

Result<void> Client::connect_tcp() {
    for (const auto port : tcp_ports) {
        auto sock_result = create_tcp_socket();
        if (sock_result.has_error()) {
            return sock_result.error();
        }

        const int sock = sock_result.value();

        auto addr_result = get_server_sockaddr(port);
        if (addr_result.has_error()) {
            return addr_result.error();
        }

        auto addr = addr_result.value();

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

Result<void> Client::on_receive(const int socket_index,
                                const std::string &data) {
    return Result<void>::success();
}

Result<int> Client::create_tcp_socket() {
    const auto sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return ClientError::make_error(
            ClientError::Code::SocketCreationFailed,
            "create_tcp_socket() failed with errno: " + std::to_string(errno));
    }

    return sock;
}

Result<void> Client::set_flags(const int fd, const int flag) {
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

    for (const auto socket : tcp_sockets) {
        epoll_event event{};
        event.events = EPOLLIN;
        event.data.fd = socket;

        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket, &event) < 0) {
            ClientError::make_error(ClientError::Code::EpollCreationFailed,
                                    "Socket " + std::to_string(socket) +
                                        " failed to be added to epoll.");
        }
    }

    // Create received_data as the same size with tcp_sockets
    received_data.resize(tcp_sockets.size());
    reset_data();  // Reset received_data

    return Result<void>::success();
}

Result<void> Client::listen_and_receive() {
    constexpr int BUFFER_SIZE = 256;

    std::array<epoll_event, MAX_EVENTS> events{};
    char buffer[BUFFER_SIZE];

    // Wait for the first data before starting timer
    bool first_data_received = false;
    auto next_print = std::chrono::steady_clock::now();

    while (running) {
        auto now = std::chrono::steady_clock::now();

        auto timeout = std::chrono::duration_cast<std::chrono::milliseconds>(
                           next_print - now)
                           .count();
        timeout = std::max(1L, timeout);

        int num_events =
            epoll_wait(epoll_fd, events.data(), MAX_EVENTS, timeout);

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
                std::fill_n(buffer, BUFFER_SIZE, 0);
                const int bytes_read =
                    read(events[i].data.fd, buffer, BUFFER_SIZE - 1);

                if (bytes_read > 0) {
                    first_data_received = true;
                    // Find which socket this is
                    for (auto j = 0; j < tcp_sockets.size(); ++j) {
                        if (tcp_sockets[j] == events[i].data.fd) {
                            // Remove the newline if present
                            if (buffer[bytes_read - 1] == '\n') {
                                buffer[bytes_read - 1] = '\0';
                            }
                            std::string str_buffer(buffer);

                            received_data[j] = str_buffer;

                            if (const auto recv_result =
                                    on_receive(j, str_buffer);
                                recv_result.has_error()) {
                                return recv_result.error();
                            }
                            break;
                        }
                    }
                }
            }
        }

        if (!first_data_received) {
            continue;
        }

        now = std::chrono::steady_clock::now();
        if (now >= next_print) {
            // Get the current timestamp in milliseconds
            const auto ms =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now().time_since_epoch())
                    .count();

            print_json(ms, received_data);

            reset_data();

            // Calculate next print time based on the previous target
            next_print += std::chrono::milliseconds(interval_ms);

            // If we've fallen behind, catch up to current time
            if (now > next_print) {
                next_print = now;
            }
        }
    }

    cleanup();

    return Result<void>::success();
}

void Client::set_server_addr(std::string addr) { server_addr = addr; }

Result<void> Client::run() {
    if (const auto conn_result = connect_tcp(); conn_result.has_error()) {
        return conn_result.error();
    }

    if (const auto epoll_result = setup_epoll(); epoll_result.has_error()) {
        return epoll_result.error();
    }

    if (const auto event_loop_result = listen_and_receive();
        event_loop_result.has_error()) {
        return event_loop_result.error();
    }

    return Result<void>::success();
}

void Client::set_interval_ms(int ms) { interval_ms = ms; }

void Client::reset_data() {
    std::fill(received_data.begin(), received_data.end(), "--");
}

void Client::print_json(int64_t timestamp,
                        const std::vector<std::string> &data) {
    // Join data into comma separated string
    // with fields as out1, out2, out3, ...
    std::string data_str;
    for (size_t i = 0; i < data.size(); ++i) {
        data_str += "\"out" + std::to_string(i + 1) + "\": \"" + data[i] + "\"";
        if (i < data.size() - 1) {
            data_str += ", ";
        }
    }

    std::cout << "{\"timestamp\": " << timestamp << ", " << data_str << "}"
              << std::endl;
}

Result<void> Client::setup_signal_handling() {
    struct sigaction sa {};
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, nullptr) < 0 ||
        sigaction(SIGTERM, &sa, nullptr) < 0) {
        return ClientError::make_error(
            ClientError::Code::SignalHandlingSetupFailed,
            "Failed to set up signal handling");
    }

    return Result<void>::success();
}

void Client::cleanup() {
    // Close all TCP sockets
    for (const auto socket : tcp_sockets) {
        close(socket);
    }
    tcp_sockets.clear();

    // Close epoll fd
    if (epoll_fd >= 0) {
        close(epoll_fd);
        epoll_fd = -1;
    }
}

Result<sockaddr_in> Client::get_server_sockaddr(const int port) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (server_addr.empty()) {
        set_server_addr(DEFAULT_SERVER_ADDR);
    }

    if (inet_pton(AF_INET, server_addr.c_str(), &addr.sin_addr) <= 0) {
        return ClientError::make_error(
            ClientError::Code::InvalidAddress,
            "Invalid address for server: " + server_addr);
    }

    return addr;
}
