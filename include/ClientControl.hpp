#ifndef CLIENT_CONTROL_HPP
#define CLIENT_CONTROL_HPP
#include <optional>
#include <utility>

#include "Client.hpp"

class ClientControl : public Client {
   public:
    static Result<ClientControl> create();

    void set_control_port(int port);

   protected:
    static constexpr int PRINT_INTERVAL_MS = 20;
    static constexpr int OUTPUT3_INDEX = 2;
    static constexpr uint16_t FREQUENCY_PROP = 0xFF;  // Frequency property code
    static constexpr uint16_t AMPLITUDE_PROP = 0xAA;  // Amplitude property code

    int control_port = 4000;

    ClientControl();

    std::pair<uint16_t, uint16_t> compute_behavior(float out3_value);

    Result<void> on_receive_data(int socket_index, const std::string &data);

    Result<int> create_udp_socket();

    Result<void> send_data(uint16_t property, uint16_t value);
};

#endif  // CLIENT_CONTROL_HPP