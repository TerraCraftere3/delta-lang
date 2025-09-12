#include "Strings.h"

std::string Delta::unescape(const std::string &input)
{
    std::string result;
    result.reserve(input.size());
    for (size_t i = 0; i < input.size(); ++i)
    {
        if (input[i] == '\\' && i + 1 < input.size())
        {
            char next = input[i + 1];
            switch (next)
            {
            case 'n':
                result.push_back('\n');
                break;
            case 't':
                result.push_back('\t');
                break;
            case 'r':
                result.push_back('\r');
                break;
            case '\\':
                result.push_back('\\');
                break;
            case '"':
                result.push_back('"');
                break;
            case '\'':
                result.push_back('\'');
                break;
            default:
                result.push_back('\\');
                result.push_back(next);
                break;
            }
            ++i; // skip the next char since it's consumed
        }
        else
        {
            result.push_back(input[i]);
        }
    }
    return result;
}

std::string Delta::escape(const std::string &input)
{
    std::string result;
    result.reserve(input.size());
    for (char c : input)
    {
        switch (c)
        {
        case '\n':
            result += "\\n";
            break;
        case '\t':
            result += "\\t";
            break;
        case '\r':
            result += "\\r";
            break;
        case '\\':
            result += "\\\\";
            break;
        case '"':
            result += "\\\"";
            break;
        case '\'':
            result += "\\\'";
            break;
        default:
            result.push_back(c);
            break;
        }
    }
    return result;
}
