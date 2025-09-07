#include "Parser.h"
#include "Tokenizer.h"
#include "Log.h"
#include "Error.h"

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
            auto open_paren = consume();
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
            try_consume(TokenType::close_paren, "')'", open_paren.line);
            try_consume(TokenType::semicolon, "';'", open_paren.line);
        }
        else if (peek().value().type == TokenType::let && peek(2).has_value() && peek(2).value().type == TokenType::identifier && peek(3).has_value() && peek(3).value().type == TokenType::equals)
        {
            consume();
            auto *statement_let = m_allocator.alloc<NodeStatementLet>();
            statement_let->ident = consume();
            auto eq = consume();
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
            try_consume(TokenType::semicolon, "';'", eq.line);
        }
        else if (peek().value().type == TokenType::identifier && peek(2).has_value() && peek(2).value().type == TokenType::equals)
        {
            auto assign = m_allocator.alloc<NodeStatementAssign>();
            assign->ident = consume();
            auto eq = consume(); // equals sign
            if (auto expr = parseExpression())
            {
                assign->expression = expr.value();
            }
            else
            {
                LOG_ERROR("Invalid Expression");
                exit(EXIT_FAILURE);
            }
            try_consume(TokenType::semicolon, "';'", eq.line);
            auto stmt = m_allocator.alloc<NodeStatement>();
            stmt->var = assign;
            statement = stmt;
        }
        else if (peek().value().type == TokenType::open_curly)
        {
            if (auto scope = parseScope())
            {
                auto stmt = m_allocator.alloc<NodeStatement>();
                stmt->var = scope.value();
                statement = stmt;
            }
            else
            {
                LOG_ERROR("Invalid Scope");
                exit(EXIT_FAILURE);
            }
        }
        else if (auto if_ = try_consume(TokenType::if_))
        {
            try_consume(TokenType::open_paren, "'('", if_.value().line);
            auto stmt_if = m_allocator.alloc<NodeStatementIf>();
            if (auto expr = parseExpression())
            {
                stmt_if->expr = expr.value();
            }
            else
            {
                LOG_ERROR("Invalid Expression");
                exit(EXIT_FAILURE);
            }
            try_consume(TokenType::close_paren, "')'", if_.value().line);
            if (auto scope = parseScope())
            {
                stmt_if->scope = scope.value();
            }
            else
            {
                LOG_ERROR("Invalid Scope");
                exit(EXIT_FAILURE);
            }
            stmt_if->pred = parseIfPred();
            auto stmt = m_allocator.alloc<NodeStatement>();
            stmt->var = stmt_if;
            statement = stmt;
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

    std::optional<NodeScope *> Parser::parseScope()
    {
        if (auto open_curly = try_consume(TokenType::open_curly))
        {
            auto scope = m_allocator.alloc<NodeScope>();
            while (auto statement = parseStatement())
            {
                scope->statements.push_back(statement.value());
            }
            try_consume(TokenType::close_curly, "'}'", peek(-1).value().line);
            return scope;
        }
        else
        {
            return std::nullopt;
        }
    }

    std::optional<NodeIfPred *> Parser::parseIfPred()
    {
        if (auto _ = try_consume(TokenType::elif))
        {
            try_consume(TokenType::open_paren, "'('", _.value().line);
            auto elif = m_allocator.alloc<NodeIfPredElif>();
            if (auto expr = parseExpression())
            {
                elif->expr = expr.value();
            }
            else
            {
                LOG_ERROR("Expected Expression");
                exit(EXIT_FAILURE);
            }
            try_consume(TokenType::close_paren, "')'", _.value().line);
            if (auto scope = parseScope())
            {
                elif->scope = scope.value();
            }
            else
                Error::throwExpected("Scope", _.value().line);
            elif->pred = parseIfPred();
            auto pred = m_allocator.alloc<NodeIfPred>();
            pred->var = elif;
            return pred;
        }
        if (auto _ = try_consume(TokenType::else_))
        {
            auto else_ = m_allocator.alloc<NodeIfPredElse>();
            if (auto scope = parseScope())
            {
                else_->scope = scope.value();
            }
            else
                Error::throwExpected("Scope", _.value().line);
            auto pred = m_allocator.alloc<NodeIfPred>();
            pred->var = else_;
            return pred;
        }
        return std::nullopt;
    }

    std::optional<NodeExpression *> Parser::parseExpression(int min_prec)
    {
        std::optional<NodeExpressionTerm *> term_lhs = parseTerm();
        if (!term_lhs.has_value())
        {
            return std::nullopt;
        }

        auto expr_lhs = m_allocator.alloc<NodeExpression>();
        expr_lhs->var = term_lhs.value();

        while (true)
        {
            std::optional<Token> curr_token = peek();
            std::optional<int> prec;
            if (curr_token.has_value())
            {
                prec = Tokenizer::getBinaryOPPrec(curr_token.value().type);
                if (!prec.has_value() || prec.value() < min_prec)
                    break;
            }
            else
            {
                break;
            }

            Token op = consume();

            int next_min_prec = prec.value() + 1;
            auto expr_rhs = parseExpression(next_min_prec);
            if (!expr_rhs.has_value())
            {
                LOG_ERROR("Unable to parse Expression");
                exit(EXIT_FAILURE);
            }

            auto expr = m_allocator.alloc<NodeExpressionBinary>();
            auto expr_lhs2 = m_allocator.alloc<NodeExpression>();
            if (op.type == TokenType::plus)
            {
                auto add = m_allocator.alloc<NodeExpressionBinaryAddition>();
                expr_lhs2->var = expr_lhs->var;
                add->left = expr_lhs2;
                add->right = expr_rhs.value();
                expr->var = add;
            }
            else if (op.type == TokenType::minus)
            {
                auto sub = m_allocator.alloc<NodeExpressionBinarySubtraction>();
                expr_lhs2->var = expr_lhs->var;
                sub->left = expr_lhs2;
                sub->right = expr_rhs.value();
                expr->var = sub;
            }
            else if (op.type == TokenType::slash)
            {
                auto div = m_allocator.alloc<NodeExpressionBinaryDivision>();
                expr_lhs2->var = expr_lhs->var;
                div->left = expr_lhs2;
                div->right = expr_rhs.value();
                expr->var = div;
            }
            else if (op.type == TokenType::star)
            {
                auto mult = m_allocator.alloc<NodeExpressionBinaryMultiplication>();
                expr_lhs2->var = expr_lhs->var;
                mult->left = expr_lhs2;
                mult->right = expr_rhs.value();
                expr->var = mult;
            }
            else
            {
                assert(false);
            }
            expr_lhs->var = expr;
        }

        return expr_lhs;
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
        else if (auto open_paren = try_consume(TokenType::open_paren))
        {
            auto expr = parseExpression();
            if (!expr.has_value())
            {
                LOG_ERROR("Expected Expression");
                exit(EXIT_FAILURE);
            }
            try_consume(TokenType::close_paren, "')'", open_paren.value().line);
            auto node_term_paren = m_allocator.alloc<NodeTermParen>();
            node_term_paren->expr = expr.value();
            auto node_term = m_allocator.alloc<NodeExpressionTerm>();
            node_term->var = node_term_paren;
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

    Token Parser::try_consume(TokenType type, const std::string &c, int line, int row)
    {
        if (peek().has_value() && peek().value().type == type)
        {
            return consume();
        }
        else
        {
            Error::throwExpected(c, line, row);
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