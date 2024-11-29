#include "ClientControl.hpp"

Result<ClientControl> ClientControl::create() {
    if (const auto setup_result = setup_signal_handling();
        setup_result.has_error()) {
        return setup_result.error();
    }

    auto client_control = ClientControl{};

    client_control.on_receive =
        std::bind(&ClientControl::on_receive_data, &client_control,
                  std::placeholders::_1, std::placeholders::_2);

    return client_control;
}

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

    return Result<void>::success();
}
