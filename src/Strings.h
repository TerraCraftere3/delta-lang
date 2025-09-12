#pragma once
#include <string>

namespace Delta
{
    std::string unescape(const std::string &input);

    std::string escape(const std::string &input);
}