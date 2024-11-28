#include "ClientControl.hpp"

Result<ClientControl> ClientControl::create() {
    if (const auto setup_result = setup_signal_handling();
        setup_result.has_error()) {
        return setup_result.error();
    }

    return ClientControl{};
}

ClientControl::ClientControl() : Client() {}
