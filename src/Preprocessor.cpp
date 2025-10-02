#include "Preprocessor.h"
#include "Files.h"
#include "Tokenizer.h"
#include "Globals.h"

namespace Delta
{
#define SPECIAL_TOKEN(type, value) {type, 0, value}

    Preprocessor::Preprocessor(std::vector<Token> tokens, bool isWasm)
        : m_tokens(tokens)
    {
        if (!isWasm)
        {
#if defined(_WIN32)
            m_definitions["_WIN32"].push_back(SPECIAL_TOKEN(TokenType::int_literal, "1"));
#endif
#if defined(_WIN64)
            m_definitions["_WIN64"].push_back(SPECIAL_TOKEN(TokenType::int_literal, "1"));
#endif
#if defined(__linux__)
            m_definitions["__linux__"].push_back(SPECIAL_TOKEN(TokenType::int_literal, "1"));
#endif
#if defined(__linux)
            m_definitions["__linux"].push_back(SPECIAL_TOKEN(TokenType::int_literal, "1"));
#endif
#if defined(__APPLE__)
            m_definitions["__APPLE__"].push_back(SPECIAL_TOKEN(TokenType::int_literal, "1"));
#endif
        }
        else
        {
            m_definitions["_WASM"].push_back(SPECIAL_TOKEN(TokenType::int_literal, "1"));
        }
        m_definitions["_DLT_CC"].push_back(SPECIAL_TOKEN(TokenType::int_literal, "1"));
        m_definitions["_DLT_CC_VERSION"].push_back(SPECIAL_TOKEN(TokenType::string_literal, COMPILER_VERSION));
        m_definitions["_DLT_CC_NAME"].push_back(SPECIAL_TOKEN(TokenType::string_literal, COMPILER_NAME));
        m_definitions["_DLT_VERSION"].push_back(SPECIAL_TOKEN(TokenType::string_literal, STDLIB_VERSION));
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
            else if (peek(1).value().type == TokenType::hashtag &&
                     peek(2).has_value() &&
                     (peek(2).value().type == TokenType::if_ ||
                      peek(2).value().type == TokenType::elif ||
                      peek(2).value().type == TokenType::else_ ||
                      peek(2).value().type == TokenType::endif))
            {
                consume();                     // #
                auto directiveTok = consume(); // if/elif/else/endif

                if (directiveTok.type == TokenType::if_ || directiveTok.type == TokenType::elif)
                {
                    std::vector<Token> conditionTokens;
                    while (peek(1).has_value() && peek(1).value().line == directiveTok.line)
                    {
                        conditionTokens.push_back(consume());
                    }

                    bool condition = evaluateCondition(conditionTokens);

                    if (condition)
                    {
                        // process inner tokens
                        continue;
                    }
                    else
                    {
                        int nestedIfs = 0;
                        while (peek(1).has_value())
                        {
                            auto nextTok = peek(1).value();
                            if (nextTok.type == TokenType::hashtag)
                            {
                                auto nextDirective = peek(2).value().type;
                                if (nextDirective == TokenType::if_)
                                    nestedIfs++;
                                else if (nextDirective == TokenType::endif)
                                {
                                    if (nestedIfs == 0)
                                        break;
                                    nestedIfs--;
                                }
                                else if ((nextDirective == TokenType::elif || nextDirective == TokenType::else_) && nestedIfs == 0)
                                {
                                    break;
                                }
                            }
                            consume();
                        }
                        continue;
                    }
                }
                else if (directiveTok.type == TokenType::else_ || directiveTok.type == TokenType::endif)
                {
                    // Skip these tokens; #endif just marks the end
                    continue;
                }
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

    bool Preprocessor::evaluateCondition(const std::vector<Token> &tokens)
    {
        if (tokens.empty())
            return false;

        size_t i = 0;
        bool negate = false;

        if (tokens[i].type == TokenType::exclamation) // !
        {
            negate = true;
            i++;
        }

        if (i < tokens.size() && tokens[i].type == TokenType::identifier && tokens[i].value.value() == "defined")
        {
            i++;                                                        // skip "defined"
            if (i < tokens.size() && tokens[i].type == TokenType::less) // '('
                i++;
            if (i < tokens.size() && tokens[i].type == TokenType::identifier)
            {
                std::string macroName = tokens[i].value.value();
                i++;
                if (i < tokens.size() && tokens[i].type == TokenType::greater) // ')'
                    i++;
                bool isDefined = m_definitions.find(macroName) != m_definitions.end();
                return negate ? !isDefined : isDefined;
            }
        }
        else if (i < tokens.size() && tokens[i].type == TokenType::int_literal)
        {
            int value = std::stoi(tokens[i].value.value());
            return negate ? (value == 0) : (value != 0);
        }

        // fallback: treat undefined as false
        return false;
    }

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