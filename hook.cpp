#include "hook.hpp"
void HookFunction(Memory& memory, const void * origin, const void * replace) {
    Patch patch(memory, memory.ToLogicalAddress((unsigned long)origin));
    auto& as = patch.get_assembler();
    as.jmp((unsigned long)replace);
    patch.patch();
}
