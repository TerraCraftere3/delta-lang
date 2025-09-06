#pragma once

#include "Nodes.h"

namespace Delta
{
    class Assembler
    {
    public:
        Assembler(NodeExit root);
        std::string generate();

    private:
        const NodeExit m_root;
    };
}