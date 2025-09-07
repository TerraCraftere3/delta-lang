#include "Error.h"

#include "Log.h"

namespace Delta
{

    void Error::throwExpected(const std::string &c, int line, int col)
    {
        LOG_ERROR("Expected {} on line {}", c, line);
        exit(EXIT_FAILURE);
    }
}