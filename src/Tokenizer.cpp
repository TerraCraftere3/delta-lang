#include "Tokenizer.h"

#include "Types.h"
#include "Log.h"

namespace Delta
{
    std::vector<Token> Tokenizer::tokenize()
    {
        std::vector<Token> tokens;
        std::string buf;
        int line_count = 1;
        while (peek().has_value())
        {
            if (std::isalpha(peek().value()))
            {
                buf.push_back(consume());
                while (peek().has_value() && std::isalnum(peek().value()))
                {
                    buf.push_back(consume());
                }
                if (buf == "true")
                {
                    tokens.push_back({TokenType::int_literal, line_count, "1"});
                    buf.clear();
                }
                else if (buf == "false")
                {
                    tokens.push_back({TokenType::int_literal, line_count, "0"});
                    buf.clear();
                }
                else if (buf == "const")
                {
                    tokens.push_back({TokenType::const_, line_count});
                    buf.clear();
                }
                else if (buf == "return")
                {
                    tokens.push_back({TokenType::return_, line_count});
                    buf.clear();
                }
                else if (buf == "exit")
                {
                    tokens.push_back({TokenType::exit, line_count});
                    buf.clear();
                }
                else if (isValidDataType(buf))
                {
                    tokens.push_back({TokenType::data_type, line_count, buf});
                    buf.clear();
                }
                else if (buf == "if")
                {
                    tokens.push_back({TokenType::if_, line_count});
                    buf.clear();
                }
                else if (buf == "elif")
                {
                    tokens.push_back({TokenType::elif, line_count});
                    buf.clear();
                }
                else if (buf == "else")
                {
                    tokens.push_back({TokenType::else_, line_count});
                    buf.clear();
                }
                else
                {
                    tokens.push_back({TokenType::identifier, line_count, buf});
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
                tokens.push_back({TokenType::int_literal, line_count, buf});
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
            else if (peek().value() == '=' && peek(2).has_value() && peek(2).value() == '=')
            {
                consume(); // '='
                consume(); // '='
                tokens.push_back({TokenType::double_equals, line_count});
            }
            else if (peek().value() == '>' && peek(2).has_value() && peek(2).value() == '=')
            {
                consume(); // '>'
                consume(); // '='
                tokens.push_back({TokenType::greater_equals, line_count});
            }
            else if (peek().value() == '<' && peek(2).has_value() && peek(2).value() == '=')
            {
                consume(); // '<'
                consume(); // '='
                tokens.push_back({TokenType::less_equals, line_count});
            }
            else if (peek().value() == '>')
            {
                consume();
                tokens.push_back({TokenType::greater, line_count});
            }
            else if (peek().value() == '<')
            {
                consume();
                tokens.push_back({TokenType::less, line_count});
            }
            else if (peek().value() == ',')
            {
                consume();
                tokens.push_back({TokenType::comma, line_count});
            }
            else if (peek().value() == ';')
            {
                consume();
                tokens.push_back({TokenType::semicolon, line_count});
            }
            else if (peek().value() == '(')
            {
                consume();
                tokens.push_back({TokenType::open_paren, line_count});
            }
            else if (peek().value() == ')')
            {
                consume();
                tokens.push_back({TokenType::close_paren, line_count});
            }
            else if (peek().value() == '=')
            {
                consume();
                tokens.push_back({TokenType::equals, line_count});
            }
            else if (peek().value() == '+')
            {
                consume();
                tokens.push_back({TokenType::plus, line_count});
            }
            else if (peek().value() == '-')
            {
                consume();
                tokens.push_back({TokenType::minus, line_count});
            }
            else if (peek().value() == '*')
            {
                consume();
                tokens.push_back({TokenType::star, line_count});
            }
            else if (peek().value() == '/')
            {
                consume();
                tokens.push_back({TokenType::slash, line_count});
            }
            else if (peek().value() == '{')
            {
                consume();
                tokens.push_back({TokenType::open_curly, line_count});
            }
            else if (peek().value() == '}')
            {
                consume();
                tokens.push_back({TokenType::close_curly, line_count});
            }
            else if (peek().value() == '\n')
            {
                line_count++;
                consume();
            }
            else if (std::isspace(peek().value()))
            {
                consume();
            }
            else
            {
                LOG_ERROR("Unexpected character '{}' on line {}", peek().value(), line_count);
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
        case TokenType::greater:
        case TokenType::greater_equals:
        case TokenType::less:
        case TokenType::less_equals:
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
        case TokenType::star:  // *
        case TokenType::slash: // /
            return 2;
        case TokenType::plus:  // +
        case TokenType::minus: // -
            return 1;
        case TokenType::greater:        // >
        case TokenType::greater_equals: // >=
        case TokenType::less:           // <
        case TokenType::less_equals:    // <=
        case TokenType::double_equals:  // ==
            return 0;
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
