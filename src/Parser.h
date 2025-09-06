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
        std::optional<NodeProgram> parseProgram();
        std::optional<NodeStatement> parseStatement();
        std::optional<NodeExpression> parseExpression();

    private:
        std::optional<Token> peek(int count = 1) const;
        Token consume();

    private:
        const std::vector<Token> m_tokens;
        size_t m_position = 0;
    };
}