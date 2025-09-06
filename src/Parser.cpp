#include "Parser.h"
#include "Log.h"

namespace Delta
{
    Parser::Parser(std::vector<Token> tokens) : m_tokens(tokens) {}

    std::optional<NodeStatement> Parser::parseStatement()
    {
        std::optional<NodeStatement> statement_node;
        if (!peek().has_value())
        {
            exit(EXIT_FAILURE);
        }
        if (peek().value().type == TokenType::exit && peek(2).has_value() && peek(2).value().type == TokenType::open_paren)
        {
            consume();
            consume();
            if (auto node_expr = parseExpression())
            {
                statement_node = NodeStatement{NodeStatementExit{node_expr.value()}};
            }
            else
            {
                LOG_ERROR("Invalid Expression");
                exit(EXIT_FAILURE);
            }
            if (peek().has_value() && peek().value().type == TokenType::close_paren)
            {
                consume();
            }
            else
            {
                LOG_ERROR("Expected ')'");
                exit(EXIT_FAILURE);
            }
            if (peek().has_value() && peek().value().type == TokenType::semicolon)
            {
                consume();
            }
            else
            {
                LOG_ERROR("Expected ';'");
                exit(EXIT_FAILURE);
            }
        }
        else if (peek().value().type == TokenType::let && peek(2).has_value() && peek(2).value().type == TokenType::identifier && peek(3).has_value() && peek(3).value().type == TokenType::equals)
        {
            consume();
            NodeStatementLet statement_let = NodeStatementLet{consume()};
            consume();
            if (auto node_expr = parseExpression())
            {
                statement_let.expression = node_expr.value();
                statement_node = NodeStatement{statement_let};
            }
            else
            {
                LOG_ERROR("Invalid Expression");
                exit(EXIT_FAILURE);
            }
            if (peek().has_value() && peek().value().type == TokenType::semicolon)
            {
                consume();
            }
            else
            {
                LOG_ERROR("Expected ';'");
                exit(EXIT_FAILURE);
            }
        }

        return statement_node;
    }

    std::optional<NodeProgram> Parser::parseProgram()
    {
        NodeProgram program;
        while (peek().has_value())
        {
            if (auto statement = parseStatement())
            {
                program.statements.push_back(statement.value());
            }
            else
            {
                LOG_ERROR("Invalid Statement");
                exit(EXIT_FAILURE);
            }
        }
        return program;
    }

    std::optional<NodeExpression> Parser::parseExpression()
    {
        if (peek().has_value() && peek().value().type == TokenType::int_literal)
        {
            return NodeExpression{NodeExpressionIntegerLiteral{consume()}};
        }
        else if (peek().has_value() && peek().value().type == TokenType::identifier)
        {
            return NodeExpression{NodeExpressionIdentifier{consume()}};
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