#ifndef RESULTTYPE_HPP
#define RESULTTYPE_HPP
#include <variant>

#include "ClientError.hpp"

using ResultVoidErr = std::variant<void, ClientError>;
using ResultIntErr = std::variant<int, ClientError>;

#endif  // RESULTTYPE_HPP