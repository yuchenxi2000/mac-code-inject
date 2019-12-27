#include "patch.hpp"
Memory memory;

bool MakePatch(PatchFunc func, uintptr_t& patch_addr, size_t& length) {
    uintptr_t code_length = func(patch_addr, length);
    patch_addr += code_length;
    length -= code_length;
    return true;
}
