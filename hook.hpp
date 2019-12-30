#ifndef hook_hpp
#define hook_hpp

#include "patch.hpp"
// hook library function
#define DYLD_INTERPOSE(_replacement,_replacee) __attribute__((used)) static struct{ const void* replacement; const void* replacee; } _interpose_##_replacee __attribute__ ((section ("__DATA,__interpose"))) = { (const void*)(unsigned long)&_replacement, (const void*)(unsigned long)&_replacee };

// hook internal function
void HookFunction(Memory& memory, const void * origin, const void * replace);

#endif /* hook_hpp */
