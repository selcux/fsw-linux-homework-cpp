#include "ClientControl.hpp"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <cstring>
#include <thread>

Result<ClientControl> ClientControl::create() {
    if (const auto setup_result = setup_signal_handling();
        setup_result.has_error()) {
        return setup_result.error();
    }

    auto client_control = ClientControl{};

    return std::move(client_control);
}

void ClientControl::set_control_port(int port) { control_port = port; }

ClientControl::ClientControl() : Client() {}

int ClientControl::get_interval() const { return 20; }

std::pair<uint16_t, uint16_t> ClientControl::compute_behavior(
    const float out3_value) {
    if (out3_value < 3.0f) {
        return {1000, 8000};
    }

    return {2000, 4000};
}

Result<void> ClientControl::on_receive(const int socket_index,
                                       const std::string& data) {
    // Only output 3 is essential
    if (socket_index != OUTPUT3_INDEX) {
        return Result<void>::success();
    }

    // Convert data to float
    float out3_value;
    try {
        out3_value = std::stof(data);
    } catch (const std::invalid_argument& e) {
        return ClientError::make_error(
            ClientError::Code::DataConversionFailed,
            "Failed to convert data " + data +
                " to float: " + std::string(e.what()));
    } catch (const std::out_of_range& e) {
        return ClientError::make_error(
            ClientError::Code::DataConversionFailed,
            "Data " + data + " out of range: " + std::string(e.what()));
    }

    const auto [frequency, amplitude] = compute_behavior(out3_value);

    // Send frequency and amplitude to the server
    if (const auto send_result = send_data(FREQUENCY_PROP, frequency);
        send_result.has_error()) {
        return send_result.error();
    }

    if (const auto send_result = send_data(AMPLITUDE_PROP, amplitude);
        send_result.has_error()) {
        return send_result.error();
    }

    return Result<void>::success();
}

Result<int> ClientControl::create_udp_socket() {
    const auto sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        return ClientError::make_error(
            ClientError::Code::SocketCreationFailed,
            "create_udp_socket() failed with errno: " + std::to_string(errno));
    }

    return sock;
}

Result<void> ClientControl::send_data(uint16_t property, uint16_t value) {
    // Each write message consists of four fields
    constexpr int FIELD_COUNT = 4;
    constexpr int FIELD_LEN = sizeof(uint16_t);
    constexpr int BUFFER_SIZE = FIELD_COUNT * FIELD_LEN;
    constexpr uint16_t WRITE_OP = 2;  // Write operation code
    constexpr uint16_t OBJ1 = 1;      // Object 1 code

    auto addr_result = get_server_sockaddr(control_port);
    if (addr_result.has_error()) {
        return addr_result.error();
    }
    auto addr = addr_result.value();

    auto socket_result = create_udp_socket();
    if (socket_result.has_error()) {
        return socket_result.error();
    }

    const auto control_socket = socket_result.value();

    char buffer[BUFFER_SIZE];
    std::fill_n(buffer, BUFFER_SIZE, 0);

    std::array<uint16_t, FIELD_COUNT> commands = {WRITE_OP, OBJ1, property,
                                                  value};

    // Convert each 16-bit value to big-endian and store it in the buffer
    for (int i = 0; i < FIELD_COUNT; ++i) {
        uint16_t field = htons(commands[i]);
        std::memcpy(buffer + i * FIELD_LEN, &field, FIELD_LEN);
    }

    const auto send_result =
        sendto(control_socket, buffer, BUFFER_SIZE, 0,
               reinterpret_cast<sockaddr*>(&addr), sizeof(addr));

    if (send_result < 0) {
        close(control_socket);

        return ClientError::make_error(
            ClientError::Code::DataTransmissionFailed,
            "Failed to sending data(" + std::to_string(send_result) + ")" +
                " with errno: " + std::to_string(errno));
    }

    if (close(control_socket) < 0) {
        return ClientError::make_error(
            ClientError::Code::SocketCloseFailed,
            "Failed to close socket after sending data.");
    }

    return Result<void>::success();
}
