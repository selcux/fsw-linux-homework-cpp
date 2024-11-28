#ifndef RESULT_HPP
#define RESULT_HPP
#include <variant>

#include "ClientError.hpp"

template <typename T, typename E = ClientError>
class Result {
   public:
    Result(T value) : variant(std::move(value)) {}
    Result(E error) : variant(std::move(error)) {}

    bool has_value() const { return std::holds_alternative<T>(variant); }
    bool has_error() const { return std::holds_alternative<E>(variant); }

    T& value() { return std::get<T>(variant); }
    const E& error() const { return std::get<E>(variant); }

   protected:
    std::variant<T, E> variant;
};

// Specialization for Result<void>
template <typename E>
class Result<void, E> {
   public:
    Result(std::monostate) : variant(std::monostate{}) {}
    Result(E error) : variant(std::move(error)) {}

    bool has_value() const {
        return std::holds_alternative<std::monostate>(variant);
    }
    bool has_error() const { return std::holds_alternative<E>(variant); }

    const E& error() const { return std::get<E>(variant); }

    static const std::monostate success() { return std::monostate{}; }

   protected:
    std::variant<std::monostate, E> variant;
};

#endif  // RESULT_HPP