#ifndef CLIENT_CONTROL_HPP
#define CLIENT_CONTROL_HPP
#include <optional>
#include <utility>

#include "Client.hpp"

class ClientControl : public Client {
   public:
    static Result<ClientControl> create();

   protected:
    static constexpr int PRINT_INTERVAL_MS = 20;
    static constexpr int OUTPUT3_INDEX = 2;

    ClientControl();

    std::pair<uint16_t, uint16_t> compute_behavior(float out3_value);

    Result<void> on_receive_data(int socket_index, const std::string &data);
};

#endif  // CLIENT_CONTROL_HPP