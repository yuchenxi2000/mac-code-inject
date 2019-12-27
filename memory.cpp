#include "memory.h"
#include <mach/mach_vm.h>
#include <cassert>
#include <cerrno>
#include <string>
#include <err.h>
#include <unistd.h>

void mach_check_error(kern_return_t ret, const char *file, unsigned int line, const char *func) {
    if (ret == KERN_SUCCESS) {
        return;
    }
    if (func == nullptr) {
        func = "[UNKNOWN]";
    }

#ifndef NDEBUG
    errx(1, "fatal Mach error on line %u of \"%s\" : %s\n", line, file, mach_error_string(ret));
#endif

}

Memory::Memory() {
    pmach_port = 0;
}

int Memory::AttachSelf() {
    Close();
    Open();
    QueryRegions();
    return 1;
}

int Memory::Attach(pid_t pid) {
    Close();
    int s = Open(pid);
    if (s)
        QueryRegions();
    return s;
}

void Memory::Detach() {
    Close();
}

kern_return_t Memory::Read(vm_address_t address, size_t size, void *buffer) {
    assert(size != 0 || address != 0);
    
    vm_size_t data_cnt = size;
    
    kern_return_t kret = vm_read_overwrite(pmach_port, address, size, (vm_address_t) buffer, &data_cnt);
    
    if (kret != KERN_SUCCESS)
        MACH_CHECK_ERROR(kret)
    
    return kret;
}

kern_return_t Memory::Write(vm_address_t address, size_t size, void *buffer) {
    if (address == 0 || size == 0)
        return KERN_INVALID_ARGUMENT;
    
    kern_return_t kret;
    if ((kret = vm_write(pmach_port, address, vm_offset_t(buffer), mach_msg_type_number_t(size))) != KERN_SUCCESS) {
        vm_prot_t backup = 0;
        
        for (auto it = segments.begin(); it != segments.end(); it++) {
            if (it->address <= address && address < (it->address + it->size)) {
                backup = it->info.protection;
                break;
            }
        }
        vm_protect(pmach_port, address, size, 0, backup | VM_PROT_WRITE | VM_PROT_READ);
        kret = vm_write(pmach_port, address, vm_offset_t(buffer), mach_msg_type_number_t(size));
        vm_protect(pmach_port, address, size, 0, backup);
        if (kret != KERN_SUCCESS) {
            MACH_CHECK_ERROR(kret)
            return kret;
        }
    }
    return kret;
}

kern_return_t Memory::Protect(vm_address_t address, size_t size, vm_prot_t protection, vm_prot_t *backup) {
    if (backup != nullptr) {
        for (auto it = segments.begin(); it != segments.end(); ++it) {
            if (it->address <= address && address <= (it->address + it->size)) {
                *backup = it->info.protection;
                break;
            }
        }
    }
    
    kern_return_t kret = vm_protect(pmach_port, vm_address_t(address), size, 0, protection);
    if (kret != KERN_SUCCESS)
        MACH_CHECK_ERROR(kret)
    
    return kret;
}

vm_address_t Memory::Allocate(size_t size, vm_prot_t prot, vm_address_t BaseAddr) {
    kern_return_t kret = KERN_SUCCESS;
    boolean_t anywhere;
    unsigned char *address;
    address = (unsigned char *) &BaseAddr;
    
    if (size == 0)
        printf("Warning -- size to allocate is zero.\n");
    
    if (*address == 0)
        anywhere = TRUE;
    else
        anywhere = FALSE;
    
    kret = vm_allocate(pmach_port, (vm_address_t *) &address, size, anywhere);
    if (kret)
        MACH_CHECK_ERROR(kret)
    
    kret = Protect((vm_address_t) address, size, prot);
    if (kret)
        MACH_CHECK_ERROR(kret)
    
    return (vm_address_t) address;
}

kern_return_t Memory::Free(vm_address_t address, size_t size) {
    kern_return_t kret = vm_deallocate(pmach_port, vm_address_t(address), size);
    if (kret)
        MACH_CHECK_ERROR(kret)
    return kret;
}

#define kMaxStringLength 8192

std::string Memory::ReadString(mach_vm_address_t address) {
    std::string result;
    for (size_t i = 0; i < kMaxStringLength; i++) {
        char c = Read<unsigned char>(address + i);
        if (c)
            result.append(&c, sizeof(unsigned char));
        else
            break;
    }
    return result;
}

int Memory::Open() {
    pmach_port = mach_task_self();
    return 1;
}

int Memory::Open(pid_t pid) {
    kern_return_t kret = task_for_pid(mach_task_self(), pid, &pmach_port);
    if (kret != KERN_SUCCESS) {
        printf("task_for_pid() error, try running as sudo!\n");
        MACH_CHECK_ERROR(kret)
        return 0;
    }
    return 1;
}

void Memory::Close() {
    pmach_port = 0;
}

mach_vm_address_t Memory::LoadAddress() {
    return load_addr;
}

mach_vm_address_t Memory::ToLogicalAddress(mach_vm_address_t addr) {
    return addr + load_addr - 0x100000000;
}

void Memory::QueryRegions() {
    mach_vm_address_t address = 0x0;
    mach_vm_size_t size;
    vm_region_basic_info_data_64_t info;
    mach_msg_type_number_t infoCount = VM_REGION_BASIC_INFO_COUNT_64;
    mach_port_t objectName = MACH_PORT_NULL;
    
    while (mach_vm_region(pmach_port, &address, &size, VM_REGION_BASIC_INFO_64, (vm_region_info_t) &info,
                          &infoCount, &objectName) == 0) {
        MemoryRegion_t region;
        region.address = address;
        region.size = size;
        region.info = info;
        segments.push_back(region);
        address += size;
    }
    
    load_addr = segments[0].address;
}
