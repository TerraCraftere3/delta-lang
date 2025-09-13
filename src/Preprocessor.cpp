#include "Preprocessor.h"

namespace Delta
{
    Preprocessor::Preprocessor(std::vector<Token> tokens)
        : m_tokens(tokens)
    {
    }

    // -----------------------------------------------------
    //                  PUBLIC FUNCTIONS
    // -----------------------------------------------------

    std::vector<Token> Preprocessor::process()
    {
        while (peek().has_value())
        {
            m_output.push_back(consume());
        }
        return m_output;
    }

    // -----------------------------------------------------
    //                  PRIVATE FUNCTIONS
    // -----------------------------------------------------

    std::optional<Token> Preprocessor::peek(int count) const
    {
        if (m_position + count > m_tokens.size())
            return std::nullopt;
        return m_tokens.at(m_position + count - 1);
    }

    Token Preprocessor::consume()
    {
        return m_tokens.at(m_position++);
    }

    Token Preprocessor::try_consume(TokenType type, const std::string &c, int line, int row)
    {
        if (peek().has_value() && peek().value().type == type)
        {
            return consume();
        }
        else
        {
            Error::throwExpected(c, line, row);
        }
        return {};
    }

    std::optional<Token> Preprocessor::try_consume(TokenType type)
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