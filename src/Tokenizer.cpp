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
                    continue;
                }
                else
                {
                    LOG_ERROR("Unexpected identifier: '{}'", buf);
                    exit(EXIT_FAILURE);
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
                continue;
            }
            else if (peek().value() == ';')
            {
                consume();
                tokens.push_back({TokenType::semicolon});
                continue;
            }
            else if (std::isspace(peek().value()))
            {
                consume();
                continue;
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
