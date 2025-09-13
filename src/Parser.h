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

    private:
        std::optional<NodeScope *> parseScope();
        std::optional<NodeIfPred *> parseIfPred();
        std::optional<NodeStatement *> parseStatement();
        std::optional<NodeExpression *> parseExpression(int min_prec = 0);
        std::optional<NodeExpressionTerm *> parseTerm();

        std::optional<NodeFunctionDeclaration *> parseFunctionDeclaration();
        std::optional<NodeExternalDeclaration *> parseExternalDeclaration();
        std::optional<std::vector<NodeParameter *>> parseParameterList();
        std::optional<NodeParameter *> parseParameter();
        std::optional<std::vector<NodeExpression *>> parseArgumentList();

        std::optional<Token> peek(int count = 1) const;
        Token consume();
        Token try_consume(TokenType type, const std::string &c, int line, int row = 0);
        std::optional<Token> try_consume(TokenType type);

    private:
        const std::vector<Token> m_tokens;
        size_t m_position = 0;
        ArenaAllocator m_allocator;
    };
}