#ifndef RESULT_HPP
#define RESULT_HPP
#include <variant>

#include "ClientError.hpp"

template <typename T, typename E = ClientError>
class Result {
   public:
    Result(T value) : value(std::move(value)) {}
    Result(E error) : error(std::move(error)) {}

    bool has_value() const { return std::holds_alternative<T>(variant); }
    bool has_error() const { return std::holds_alternative<E>(variant); }

    T& value() const { return std::get<T>(variant); }
    const E& error() const { return std::get<E>(variant); }

   private:
    std::variant<T, E> variant;
};

#endif  // RESULT_HPP