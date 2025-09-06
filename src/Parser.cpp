#include "Parser.h"
#include "Log.h"

namespace Delta
{
    Parser::Parser(std::vector<Token> tokens) : m_tokens(tokens),
                                                m_allocator(1024 * 1024 * 16) // 16 MB
    {
    }

    std::optional<NodeStatement *> Parser::parseStatement()
    {
        std::optional<NodeStatement *> statement = std::nullopt;
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
                auto *statement_exit = m_allocator.alloc<NodeStatementExit>();
                statement_exit->expression = node_expr.value();
                auto *stmt = m_allocator.alloc<NodeStatement>();
                stmt->var = statement_exit;
                statement = stmt;
            }
            else
            {
                LOG_ERROR("Invalid Expression");
                exit(EXIT_FAILURE);
            }
            try_consume(TokenType::close_paren, "Expected ')'");
            try_consume(TokenType::semicolon, "Expected ';'");
        }
        else if (peek().value().type == TokenType::let && peek(2).has_value() && peek(2).value().type == TokenType::identifier && peek(3).has_value() && peek(3).value().type == TokenType::equals)
        {
            consume();
            auto *statement_let = m_allocator.alloc<NodeStatementLet>();
            statement_let->ident = consume();
            consume();
            if (auto node_expr = parseExpression())
            {
                statement_let->expression = node_expr.value();
                auto *stmt = m_allocator.alloc<NodeStatement>();
                stmt->var = statement_let;
                statement = stmt;
            }
            else
            {
                LOG_ERROR("Invalid Expression");
                exit(EXIT_FAILURE);
            }
            try_consume(TokenType::semicolon, "Expected ';'");
        }

        return statement;
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

    std::optional<NodeExpression *> Parser::parseExpression()
    {
        if (auto term = parseTerm())
        {
            if (peek().has_value() && peek().value().type == TokenType::add)
            {
                auto bin_expr = m_allocator.alloc<NodeExpressionBinary>();
                if (try_consume(TokenType::add))
                {
                    auto bin_expr_add = m_allocator.alloc<NodeExpressionBinaryAddition>();
                    auto lhs_expr = m_allocator.alloc<NodeExpression>();
                    lhs_expr->var = term.value();
                    bin_expr_add->left = lhs_expr;
                    if (auto rhs = parseExpression())
                    {
                        bin_expr_add->right = rhs.value();
                        bin_expr->add = bin_expr_add;
                        auto node_expr = m_allocator.alloc<NodeExpression>();
                        node_expr->var = bin_expr;
                        return node_expr;
                    }
                    else
                    {
                        LOG_ERROR("Invalid Expression");
                        exit(EXIT_FAILURE);
                    }
                }
                else
                {
                    LOG_ERROR("Unsupported binary operator");
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                auto expr = m_allocator.alloc<NodeExpression>();
                expr->var = term.value();
                return expr;
            }
        }
        else
        {
            return std::nullopt;
        }
    }

    std::optional<NodeExpressionBinary *> Parser::parseBinaryExpression()
    {
        if (auto lhs = parseExpression())
        {
        }
        else
        {
            return std::nullopt;
        }
    }

    std::optional<NodeExpressionTerm *> Parser::parseTerm()
    {
        if (auto int_lit = try_consume(TokenType::int_literal))
        {
            auto term_int_lit = m_allocator.alloc<NodeTermIntegerLiteral>();
            term_int_lit->int_literal = int_lit.value();
            auto node_term = m_allocator.alloc<NodeExpressionTerm>();
            node_term->var = term_int_lit;
            return node_term;
        }
        else if (auto id = try_consume(TokenType::identifier))
        {
            auto term_ident = m_allocator.alloc<NodeTermIdentifier>();
            term_ident->ident = id.value();
            auto node_term = m_allocator.alloc<NodeExpressionTerm>();
            node_term->var = term_ident;
            return node_term;
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

    Token Parser::try_consume(TokenType type, const std::string &error_msg)
    {
        if (peek().has_value() && peek().value().type == type)
        {
            return consume();
        }
        else
        {
            LOG_ERROR(error_msg);
            exit(EXIT_FAILURE);
        }
    }
    std::optional<Token> Parser::try_consume(TokenType type)
    {
        if (peek().has_value() && peek().value().type == type)
        {
            return consume();
        }
        else
        {
            return std::nullopt;
        }
    }
}