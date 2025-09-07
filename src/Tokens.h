#pragma once

#include <optional>
#include <string>

namespace Delta
{
    enum class TokenType
    {
        exit,        // exit
        let,         // let
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
        std::optional<std::string> value; // For literals
    };
}