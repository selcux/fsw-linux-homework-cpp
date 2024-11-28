#ifndef CLIENT_CONTROL_HPP
#define CLIENT_CONTROL_HPP
#include "Client.hpp"

class ClientControl : public Client {
   public:
    static Result<ClientControl> create();

   protected:
    static constexpr int PRINT_INTERVAL_MS = 20;

    ClientControl();
};

#endif  // CLIENT_CONTROL_HPP