#pragma once

#include <string>
#include <vector>
#include <optional>
#include "Tokens.h"

namespace Delta
{
    class Tokenizer
    {
    public:
        Tokenizer(const std::string &source) : m_source(source) {}
        std::vector<Token> tokenize();

    private:
        std::optional<char> peek(int count = 1) const;
        char consume();

    private:
        const std::string m_source;
        size_t m_position = 0;
    };
}