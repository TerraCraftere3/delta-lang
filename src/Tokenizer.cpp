#include "Tokenizer.h"

#include "Log.h"

namespace Delta
{
    std::vector<Token> Tokenizer::tokenize()
    {
        std::vector<Token> tokens;
        std::string buf;
        while (peek().has_value())
        {
            if (std::isalpha(peek().value()))
            {
                buf.push_back(consume());
                while (peek().has_value() && std::isalnum(peek().value()))
                {
                    buf.push_back(consume());
                }
                if (buf == "exit")
                {
                    tokens.push_back({TokenType::exit});
                    buf.clear();
                }
                else if (buf == "let")
                {
                    tokens.push_back({TokenType::let});
                    buf.clear();
                }
                else if (buf == "if")
                {
                    tokens.push_back({TokenType::if_});
                    buf.clear();
                }
                else
                {
                    tokens.push_back({TokenType::identifier, buf});
                    buf.clear();
                }
            }
            else if (std::isdigit(peek().value()))
            {
                buf.push_back(consume());
                while (peek().has_value() && std::isdigit(peek().value()))
                {
                    buf.push_back(consume());
                }
                tokens.push_back({TokenType::int_literal, buf});
                buf.clear();
            }
            else if (peek().value() == '/' && peek(2).has_value() && peek(2).value() == '/')
            {
                consume();
                consume();
                while (peek().has_value() && peek().value() != '\n')
                {
                    consume();
                }
            }
            else if (peek().value() == '/' && peek(2).has_value() && peek(2).value() == '*')
            {
                consume();
                consume();
                while (peek().has_value())
                {
                    if (peek().value() == '*' && peek(2).has_value() && peek(2).value() == '/')
                        break;
                    consume();
                }
                if (peek().has_value())
                    consume();
                if (peek().has_value())
                    consume();
            }
            else if (peek().value() == ';')
            {
                consume();
                tokens.push_back({TokenType::semicolon});
            }
            else if (peek().value() == '(')
            {
                consume();
                tokens.push_back({TokenType::open_paren});
            }
            else if (peek().value() == ')')
            {
                consume();
                tokens.push_back({TokenType::close_paren});
            }
            else if (peek().value() == '=')
            {
                consume();
                tokens.push_back({TokenType::equals});
            }
            else if (peek().value() == '+')
            {
                consume();
                tokens.push_back({TokenType::plus});
            }
            else if (peek().value() == '-')
            {
                consume();
                tokens.push_back({TokenType::minus});
            }
            else if (peek().value() == '*')
            {
                consume();
                tokens.push_back({TokenType::star});
            }
            else if (peek().value() == '/')
            {
                consume();
                tokens.push_back({TokenType::slash});
            }
            else if (peek().value() == '{')
            {
                consume();
                tokens.push_back({TokenType::open_curly});
            }
            else if (peek().value() == '}')
            {
                consume();
                tokens.push_back({TokenType::close_curly});
            }
            else if (std::isspace(peek().value()))
            {
                consume();
            }
            else
            {
                LOG_ERROR("Unexpected character: '{}'", peek().value());
                exit(EXIT_FAILURE);
            }
        }
        m_position = 0;
        return tokens;
    }

    bool Tokenizer::isBinaryOP(TokenType type)
    {
        switch (type)
        {
        case TokenType::plus:
        case TokenType::star:
        case TokenType::minus:
        case TokenType::slash:
            return true;
        default:
            return false;
        }
    }

    std::optional<int> Tokenizer::getBinaryOPPrec(TokenType type)
    {
        switch (type)
        {
        case TokenType::plus:
        case TokenType::minus:
            return 0;
        case TokenType::star:
        case TokenType::slash:
            return 1;
        default:
            return std::nullopt;
        }
    }

    std::optional<char> Tokenizer::peek(int count) const
    {
        if (m_position + count > m_source.size())
            return std::nullopt;
        return m_source.at(m_position + count - 1);
    }

    char Tokenizer::consume()
    {
        return m_source.at(m_position++);
    }
} // namespace Delta
