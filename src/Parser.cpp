#include "Parser.h"
#include "Log.h"

namespace Delta
{
    Parser::Parser(std::vector<Token> tokens) : m_tokens(tokens) {}

    std::optional<NodeExit> Parser::parse()
    {
        std::optional<NodeExit> exit_node;
        while (peek().has_value())
        {
            if (peek().value().type == TokenType::exit)
            {
                consume();
                if (auto node_expr = parseExpression()) // has value
                {
                    exit_node = NodeExit{
                        node_expr.value() // Expression
                    };
                }
                else
                {
                    LOG_ERROR("Invalid Expression (No Expression)");
                    exit(EXIT_FAILURE);
                }
                if (peek().has_value() && peek().value().type == TokenType::semicolon)
                {
                    consume();
                }
                else
                {
                    LOG_ERROR("Invalid Exit Node (Expected Semicolon)");
                    exit(EXIT_FAILURE);
                }
            }
        }
        m_position = 0;
        return exit_node;
    }

    std::optional<NodeExpression> Parser::parseExpression()
    {
        if (peek().has_value() && peek().value().type == TokenType::int_literal)
        {
            return NodeExpression{
                consume() // int literal
            };
        }
        return std::nullopt;
    }

    std::optional<Token> Parser::peek(int count) const
    {
        if (m_position + count > m_tokens.size())
            return std::nullopt;
        return m_tokens.at(m_position + count - 1);
    }

    Token Parser::consume()
    {
        return m_tokens.at(m_position++);
    }
}