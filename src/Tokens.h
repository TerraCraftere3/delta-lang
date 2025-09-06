#pragma once

#include <optional>
#include <string>

namespace Delta
{
    enum class TokenType
    {
        exit,
        let,
        equals,
        int_literal,
        semicolon,
        open_paren,
        close_paren,
        identifier
    };

    struct Token
    {
        TokenType type;
        std::optional<std::string> value; // For literals
    };
}