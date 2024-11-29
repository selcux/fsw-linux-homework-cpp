#include "ClientControl.hpp"

#include <netinet/in.h>
#include <sys/socket.h>

#include <cstring>

Result<ClientControl> ClientControl::create() {
    if (const auto setup_result = setup_signal_handling();
        setup_result.has_error()) {
        return setup_result.error();
    }

    auto client_control = ClientControl{};

    client_control.on_receive =
        std::bind(&ClientControl::on_receive_data, &client_control,
                  std::placeholders::_1, std::placeholders::_2);

    return std::move(client_control);
}

void ClientControl::set_control_port(int port) { control_port = port; }

ClientControl::ClientControl() : Client() {}

std::pair<uint16_t, uint16_t> ClientControl::compute_behavior(
    float out3_value) {
    if (out3_value < 3.0f) {
        return {1, 8000};
    }

    return {2, 4000};
}

Result<void> ClientControl::on_receive_data(int socket_index,
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
    std::cout << "Sending frequency: " << frequency << " and amplitude: ";
    if (const auto send_result = send_data(FREQUENCY_PROP, frequency);
        send_result.has_error()) {
        return send_result.error();
    }

    std::cout << amplitude << " to server." << std::endl;
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

    auto sock_result = create_udp_socket();
    if (sock_result.has_error()) {
        return sock_result.error();
    }

    const auto sock = sock_result.value();

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(control_port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    char buffer[BUFFER_SIZE];
    std::fill_n(buffer, BUFFER_SIZE, 0);

    std::array<uint16_t, FIELD_COUNT> command = {WRITE_OP, OBJ1, property,
                                                 value};

    // Convert each 16-bit value to big-endian and store it in the buffer
    for (int i = 0; i < FIELD_COUNT; ++i) {
        uint16_t field = htons(command[i]);
        std::memcpy(buffer + i * FIELD_LEN, &field, FIELD_LEN);
    }

    const auto send_result =
        sendto(sock, buffer, BUFFER_SIZE, 0, reinterpret_cast<sockaddr*>(&addr),
               sizeof(addr));

    return Result<void>::success();
}
