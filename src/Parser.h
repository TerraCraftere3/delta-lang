#pragma once

#include <vector>
#include "Tokens.h"
#include "Nodes.h"

namespace Delta
{
    class Parser
    {
    public:
        Parser(std::vector<Token> tokens);
        std::optional<NodeExit> parse();

    private:
        std::optional<NodeExpression> parseExpression();
        std::optional<Token> peek(int count = 1) const;
        Token consume();

    private:
        const std::vector<Token> m_tokens;
        size_t m_position = 0;
    };
}