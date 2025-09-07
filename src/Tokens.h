#pragma once

#include <optional>
#include <string>

namespace Delta
{
    enum class TokenType
    {
        if_,         // if
        elif,        // elif
        else_,       // else
        exit,        // exit
        data_type,   // int64, float32, bool, etc.
        equals,      // =
        int_literal, // integer like 10, etc
        semicolon,   // ;
        open_paren,  // (
        close_paren, // )
        open_curly,  // {
        close_curly, // }
        identifier,  // ANY variable name, etc
        plus,        // +
        minus,       // -
        star,        // *
        slash        // /
    };

    struct Token
    {
        TokenType type;
        int line;
        std::optional<std::string> value; // For literals
    };
}