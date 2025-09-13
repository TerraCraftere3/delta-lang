#include "Preprocessor.h"
#include "Files.h"
#include "Tokenizer.h"

namespace Delta
{
    Preprocessor::Preprocessor(std::vector<Token> tokens)
        : m_tokens(tokens)
    {
    }

    // -----------------------------------------------------
    //                  PUBLIC FUNCTIONS
    // -----------------------------------------------------

    std::vector<Token> Preprocessor::process(std::vector<std::string> includeDirs)
    {
        while (peek(1).has_value())
        {
            if (peek(1).value().type == TokenType::hashtag &&
                peek(2).has_value() && peek(2).value().type == TokenType::include &&
                peek(3).has_value() && peek(3).value().type == TokenType::less &&
                peek(4).has_value() && peek(4).value().type == TokenType::identifier &&
                peek(5).has_value() && peek(5).value().type == TokenType::greater)
            {
                consume();                    // #
                consume();                    // include
                consume();                    // <
                auto filenameTok = consume(); // identifier
                consume();                    // >

                std::string filename = filenameTok.value.value();

                std::string filepath;
                for (auto &dir : includeDirs)
                {
                    std::string candidate = dir + FILE_SEPARATOR + filename;
                    if (Files::fileExists(candidate))
                    {
                        filepath = candidate;
                        break;
                    }
                    if (Files::fileExists(candidate + ".dlt"))
                    {
                        filepath = candidate + ".dlt";
                        break;
                    }
                }

                if (filepath.empty())
                {
                    LOG_ERROR("Include file not found: {}", filename);
                    exit(EXIT_FAILURE);
                }

                LOG_TRACE("Including {}", filepath);
                Tokenizer tokenizer(Files::readFile(filepath));
                auto tokens = tokenizer.tokenize();

                Preprocessor subPreproc(tokens);
                auto includedTokens = subPreproc.process(includeDirs);

                m_output.insert(m_output.end(), includedTokens.begin(), includedTokens.end());
            }
            else
            {
                m_output.push_back(consume());
            }
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
        if (peek(1).has_value() && peek(1).value().type == type)
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
        if (peek(1).has_value() && peek(1).value().type == type)
        {
            return consume();
        }
        else
        {
            return std::nullopt;
        }
    }
}