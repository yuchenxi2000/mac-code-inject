#include <iostream>
#include <cmath>
#include "patch.hpp"
using namespace std;

double msin(double x) {
    return sin(x);
}

__attribute__((constructor))
void patch_app(void) {
    printf("Patch: I got in!\n");
    memory.AttachSelf();

    printf("load address: %llx\n", memory.LoadAddress());
    
    // patch: replace function cos with msin (sin)
    {
        vm_address_t func_addr = 0x100010e50;
        Patch patch(memory, memory.ToLogicalAddress(func_addr));
        auto& as = patch.get_assembler();
        
        as.call((uint64_t)msin);
        patch.patch();
    }
    
    printf("Patch: I left.\n");
}
