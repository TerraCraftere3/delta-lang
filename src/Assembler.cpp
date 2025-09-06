#include "Assembler.h"

#include "sstream"

namespace Delta
{
    Assembler::Assembler(NodeExit root) : m_root(root) {}
    std::string Assembler::generate()
    {
        std::stringstream output;
        output << "global _start\n";                                            // ASM: Text Section
        output << "extern ExitProcess\n";                                       // ASM: WinAPI ExitProcess
        output << "\n";                                                         // ASM:
        output << "section .text\n";                                            // ASM: Entry Point definition
        output << "_start:\n";                                                  // ASM: Start Label
        output << "\tsub rsp, 40\n";                                            // ASM: Align Stack
        output << "\tmov ecx, " + m_root.expr.int_literal.value.value() + "\n"; // ASM: Align Stack
        output << "\tcall ExitProcess\n";                                       // ASM: Align Stack
        return output.str();
    };
} // namespace Delta