# mac-code-inject
Mac OS code inject

## Usage

* memory.h:

read / write memory

int Memory::Attach(pid_t pid); attach to other process, root privilege is required

int Memory::AttachSelf(); attach to self (dll inject)

* patch.hpp:

write code into given address

use asmjit (https://github.com/asmjit/asmjit) for machine code generation

* hook.hpp:

hook a library function or internal function

example:

``` c++
Patch patch(memory, patch_addr);
auto& as = *patch.as;

as.mov(eax, dword_ptr(ebp, 0x8));
as.mov(dword_ptr(esp), eax);
as.call(0x117ec);
as.jmp(0x2a2a4);

patch.patch();
```

## Example

test program: test.cpp

1. build a dylib use main.cpp, memory.cpp, patch.cpp

2. compile test.cpp to test as our executable to inject

3. run add_library.py to add a LC_LOAD_DYLIB command to the Mach-O file test (we use LIEF library to modify the Mach-O file)

> LIEF: pip install lief, https://github.com/lief-project/LIEF

4. run test, and you can see our dylib is successfully injected.

