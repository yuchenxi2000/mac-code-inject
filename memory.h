#ifndef MEMORY_H
#define MEMORY_H

#include <vector>
#include <array>
#include <sys/sysctl.h>
#include <mach/mach.h>

#pragma GCC visibility push(hidden)

typedef struct MemoryRegion {
    mach_vm_address_t address;
    mach_vm_size_t size;
    vm_region_basic_info_data_64_t info;
} MemoryRegion_t;

#if (!defined __GNUC__ || __GNUC__ < 2 || __GNUC_MINOR__ < (defined __cplusplus ? 6 : 4))
#define __MACH_CHECK_FUNCTION (reinterpret_cast<__const char *>(0))
#else
#define __MACH_CHECK_FUNCTION __PRETTY_FUNCTION__
#endif

#define MACH_CHECK_ERROR(ret) \
mach_check_error (ret, __FILE__, __LINE__, __MACH_CHECK_FUNCTION);

void mach_check_error(kern_return_t ret, const char *file, unsigned int line, const char *func);

/*
 read / write memory
 int Memory::Attach(pid_t pid); attach to other process, root privilege is required
 int Memory::AttachSelf(); attach to self (dll inject)
 */
class Memory {
public:
    Memory();
    
    int AttachSelf();
    
    int Attach(pid_t pid);
    
    void Detach();
    
    mach_vm_address_t LoadAddress();
    
    // 地址随机加载，反汇编得到地址换算实际地址
    mach_vm_address_t ToLogicalAddress(mach_vm_address_t addr);
    
    kern_return_t Read(vm_address_t address, size_t size, void *buffer);
    
    kern_return_t Write(vm_address_t address, size_t size, void *buffer);
    
    kern_return_t Protect(vm_address_t address, size_t size, vm_prot_t protection, vm_prot_t *backup = nullptr);
    
    vm_size_t Allocate(size_t size, vm_prot_t prot = VM_PROT_ALL, vm_address_t BaseAddr = 0);
    
    kern_return_t Free(vm_address_t address, size_t size);
    
    std::string ReadString(mach_vm_address_t address);
    
    template<typename T>
    T Read(mach_vm_address_t dwAddress) {
        T result;
        Read(dwAddress, sizeof(T), &result);
        return result;
    }
    
    template<typename T>
    T ReadMemory(std::initializer_list<mach_vm_address_t> addr) {
        T result = T();
        mach_vm_address_t offset = 0;
        for (auto it = addr.begin(); it != addr.end(); it++)
            if (it != addr.end() - 1)
                Read(offset + *it, sizeof(mach_vm_address_t), &offset);
            else
                Read(offset + *it, sizeof(result), &result);
        return result;
    }
    
    template<typename T, size_t size>
    std::array<T, size> ReadMemory(std::initializer_list<mach_vm_address_t> addr) {
        std::array<T, size> result = {T()};
        
        T buff[size] = {0};
        mach_vm_address_t offset = 0;
        for (auto it = addr.begin(); it != addr.end(); it++)
            if (it != addr.end() - 1)
                Read(offset + *it, sizeof(mach_vm_address_t), &offset);
            else
                Read(offset + *it, sizeof(buff), &buff);
        for (size_t i = 0; i < size; i++)
            result[i] = buff[i];
        return result;
    }
    
    template<class T>
    kern_return_t Write(const T &data, mach_vm_address_t dwAddress) {
        return Write(dwAddress, sizeof(T), (void *) &data);
    }
    
    template<typename T>
    void WriteMemory(T value, std::initializer_list<mach_vm_address_t> addr) {
        mach_vm_address_t offset = 0;
        for (auto it = addr.begin(); it != addr.end(); it++)
            if (it != addr.end() - 1)
                Read(offset + *it, sizeof(mach_vm_address_t), &offset);
            else
                Write(offset + *it, sizeof(value), &value);
    }
    
    template<typename T, size_t size>
    void WriteMemory(std::array<T, size> value, std::initializer_list<mach_vm_address_t> addr) {
        T buff[size];
        for (size_t i = 0; i < size; i++)
            buff[i] = value[i];
        mach_vm_address_t offset = 0;
        for (auto it = addr.begin(); it != addr.end(); it++)
            if (it != addr.end() - 1)
                Read(offset + *it, sizeof(mach_vm_address_t), &offset);
            else
                Write(offset + *it, sizeof(buff), &buff);
    }

private:
    int Open();
    
    int Open(pid_t pid);
    
    void Close();
    
    void QueryRegions();
    
public:
    mach_port_t pmach_port;
    std::vector<MemoryRegion_t> segments;
    mach_vm_address_t load_addr;
};

#pragma GCC visibility pop

#endif //MEMORY_H
