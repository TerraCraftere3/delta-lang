#pragma once

#include <vector>
#include "Tokens.h"
#include "Nodes.h"
#include "Arena.h"

namespace Delta
{
    class Parser
    {
    public:
        Parser(std::vector<Token> tokens);
        std::optional<NodeProgram> parseProgram();
        std::optional<NodeStatement *> parseStatement();
        std::optional<NodeExpression *> parseExpression(int min_prec = 0);
        std::optional<NodeExpressionBinary *> parseBinaryExpression();
        std::optional<NodeExpressionTerm *> parseTerm();

    private:
        std::optional<Token> peek(int count = 1) const;
        Token consume();
        Token try_consume(TokenType type, const std::string &error_msg);
        std::optional<Token> try_consume(TokenType type);

    private:
        const std::vector<Token> m_tokens;
        size_t m_position = 0;
        ArenaAllocator m_allocator;
    };
}