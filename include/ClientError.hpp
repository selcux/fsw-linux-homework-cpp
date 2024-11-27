#ifndef CLIENTERROR_H
#define CLIENTERROR_H
#include <exception>
#include <iostream>
#include <string>

class ClientError : public ::std::exception {
   public:
    enum class Code {
        Unknown,
        SocketCreationFailed,
        GetFlagsFailed,
        SetFlagsFailed,
        ConnectFailed,
        EpollCreationFailed,
        // ReadError,
        // WriteError
    };

    ClientError(const Code code, std::string message, const int errno_code)
        : code(code), message(std::move(message)), errno_code(errno_code) {}

    const char *what() const noexcept override { return message.c_str(); }

    Code get_code() const { return code; }

    const std::string &get_message() const { return message; }

    int get_errno_code() const { return errno_code; }

    static ClientError make_error(const Code code, const std::string &msg = "",
                                  const int err = errno) {
        return ClientError(code, msg, err);
    }

    friend std::ostream &operator<<(std::ostream &os,
                                    const ClientError &error) {
        os << error.code << " (" << error.errno_code << "):  " << error.message;
        return os;
    }

   private:
    Code code;
    std::string message;
    int errno_code;

    friend std::ostream &operator<<(std::ostream &os, const Code &code) {
        switch (code) {
            case Code::Unknown:
                os << "Unknown";
                break;
            case Code::SocketCreationFailed:
                os << "SocketCreationFailed";
                break;
            case Code::GetFlagsFailed:
                os << "GetFlagsFailed";
                break;
            case Code::SetFlagsFailed:
                os << "SetFlagsFailed";
                break;
            case Code::ConnectFailed:
                os << "ConnectFailed";
                break;
            case Code::EpollCreationFailed:
                os << "EpollCreationFailed";
                break;
        }

        return os;
    }
};

#endif  // CLIENTERROR_H