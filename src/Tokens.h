#pragma once

#include <optional>
#include <string>

namespace Delta
{
    enum class TokenType
    {
        exit,
        int_literal,
        semicolon
    };

    struct Token
    {
        TokenType type;
        std::optional<std::string> value; // For literals
    };
}