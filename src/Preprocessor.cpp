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

    PreprocessorResult Preprocessor::process(std::vector<std::string> includeDirs)
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
                    std::string candidate = dir + FILE_SEPARATOR + filename + ".dlt";
                    if (Files::fileExists(candidate))
                    {
                        filepath = candidate;
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
                auto subResult = subPreproc.process(includeDirs);

                // merge tokens
                m_output.insert(m_output.end(), subResult.tokens.begin(), subResult.tokens.end());

                // merge macros
                for (auto &[name, repl] : subResult.macros)
                {
                    m_definitions[name] = repl;
                }
            }
            else if (peek(1).value().type == TokenType::hashtag &&
                     peek(2).has_value() && peek(2).value().type == TokenType::define &&
                     peek(3).has_value() && peek(3).value().type == TokenType::identifier)
            {
                consume();                // #
                consume();                // define
                auto nameTok = consume(); // identifier
                std::string name = nameTok.value.value();

                // Collect replacement tokens until end of line
                std::vector<Token> replacement;
                while (peek(1).has_value() && peek(1).value().line == nameTok.line)
                {
                    replacement.push_back(consume());
                }

                m_definitions[name] = replacement;
                continue; // don't push anything to m_output
            }

            else
            {
                auto tok = consume();
                if (tok.type == TokenType::identifier)
                {
                    auto it = m_definitions.find(tok.value.value());
                    if (it != m_definitions.end())
                    {
                        m_output.insert(m_output.end(), it->second.begin(), it->second.end());
                        continue;
                    }
                }
                m_output.push_back(tok);
            }
        }
        PreprocessorResult result;
        result.tokens = m_output;
        result.macros = m_definitions;
        return result;
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