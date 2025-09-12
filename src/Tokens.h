#pragma once

#include <optional>
#include <string>

namespace Delta
{
    enum class TokenType
    {
        if_,            // if
        elif,           // elif
        else_,          // else
        exit,           // exit
        while_,         // while
        data_type,      // int64, float32, bool, etc.
        equals,         // =
        int_literal,    // integer like 10, etc
        float_literal,  // floats (32bit) like 3.14f, etc
        double_literal, // doubles (64bit) like 3.14, etc
        string_literal, // strings like "hello world"
        apostrophe,     // '
        quotes,         // "
        semicolon,      // ;
        open_paren,     // (
        close_paren,    // )
        open_curly,     // {
        close_curly,    // }
        open_square,    // [
        close_square,   // ]
        identifier,     // ANY variable name, etc
        plus,           // +
        minus,          // -
        star,           // *
        slash,          // /
        comma,          // ,
        const_,         // const
        return_,        // return
        greater,        // >
        less,           // <
        greater_equals, // >=
        less_equals,    // <=
        double_equals,  // ==
        and_            // &
    };

    struct Token
    {
        TokenType type;
        int line;
        std::optional<std::string> value; // For literals
    };
}