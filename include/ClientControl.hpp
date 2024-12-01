#ifndef CLIENT_CONTROL_HPP
#define CLIENT_CONTROL_HPP
#include <utility>

#include "Client.hpp"

class ClientControl : public Client {
   public:
    static constexpr uint16_t FREQUENCY_PROP = 0xFF;  // Frequency property code
    static constexpr uint16_t AMPLITUDE_PROP = 0xAA;  // Amplitude property code

    static Result<ClientControl> create();

    void set_control_port(int port);

    Result<void> send_data(uint16_t property, uint16_t value);

    void set_behavior_trigger_index(int index);

   protected:
    // static constexpr int OUTPUT3_INDEX = 2;
    int behavior_trigger_index;
    int control_port = 4000;

    ClientControl();

    Result<void> on_receive(int socket_index, const std::string &data) override;

    static std::pair<uint16_t, uint16_t> compute_behavior(float out3_value);

    static Result<int> create_udp_socket();
};

#endif  // CLIENT_CONTROL_HPP