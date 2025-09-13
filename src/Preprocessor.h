#pragma once

#include <vector>
#include "Tokens.h"
#include "Nodes.h"
#include "Arena.h"

namespace Delta
{
    struct PreprocessorResult
    {
        std::vector<Token> tokens;
        std::unordered_map<std::string, std::vector<Token>> macros;
    };

    class Preprocessor
    {
    public:
        Preprocessor(std::vector<Token> tokens);
        PreprocessorResult process(std::vector<std::string> includeDirs);

    private:
        std::optional<Token> peek(int count) const;
        Token consume();
        Token try_consume(TokenType type, const std::string &c, int line, int row = 0);
        std::optional<Token> try_consume(TokenType type);

    private:
        std::unordered_map<std::string, std::vector<Token>> m_definitions;
        const std::vector<Token> m_tokens;
        std::vector<Token> m_output;
        size_t m_position = 0;
    };
}