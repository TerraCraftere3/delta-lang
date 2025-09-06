#pragma once

#include "Tokens.h"

namespace Delta
{
    struct NodeExpression
    {
        Token int_literal;
    };

    struct NodeExit
    {
        NodeExpression expr;
    };
}