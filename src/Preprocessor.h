#pragma once

#include <vector>
#include "Tokens.h"
#include "Nodes.h"
#include "Arena.h"

namespace Delta
{
    class Preprocessor
    {
    public:
        Preprocessor(std::vector<Token> tokens);
        std::vector<Token> process();

    private:
        std::optional<Token> peek(int count = 1) const;
        Token consume();
        Token try_consume(TokenType type, const std::string &c, int line, int row = 0);
        std::optional<Token> try_consume(TokenType type);

    private:
        const std::vector<Token> m_tokens;
        std::vector<Token> m_output;
        size_t m_position = 0;
    };
}