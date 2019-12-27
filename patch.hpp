#ifndef patch_hpp
#define patch_hpp

#include <iostream>
#include <cassert>
#include <asmjit/asmjit.h>
#include "memory.h"

#pragma GCC visibility push(hidden)

/*
 write code into given address
 use asmjit for machine code generation
 example:
 
 Patch patch(memory, patch_addr);
 auto& as = *patch.as;
 
 as.mov(eax, dword_ptr(ebp, 0x8));
 as.mov(dword_ptr(esp), eax);
 as.call(0x117ec);
 as.jmp(0x2a2a4);
 
 patch.patch();
 */
class Patch {
    uintptr_t addr;
    asmjit::CodeHolder code;
    Memory& memory;
public:
    Patch(Memory& memory_writer, uintptr_t addr) : memory(memory_writer) {
        this->addr = addr;
        asmjit::CodeInfo codeinfo(asmjit::ArchInfo::kIdX64);
        codeinfo.setBaseAddress(addr);
        code.init(codeinfo);
        as = new asmjit::x86::Assembler(&code);
    }
    ~Patch() {
        delete as;
    }
    asmjit::x86::Assembler* as;
    asmjit::x86::Assembler& get_assembler() {
        return *as;
    }
    void patch() {
        asmjit::CodeBuffer& buffer = code.textSection()->buffer();
        uint8_t* data = buffer.data();
        size_t length = buffer.size();
        memory.Write(addr, length, data);
    }
    size_t code_length() {
        return code.textSection()->buffer().size();
    }
};

extern Memory memory;

// 在patch-addr开始的length长度内存写一段代码，返回写的字节数
typedef size_t PatchFunc(uintptr_t patch_addr, size_t length);

bool MakePatch(PatchFunc func, uintptr_t& patch_addr, size_t& length);

#pragma GCC visibility pop

#endif /* patch_hpp */
