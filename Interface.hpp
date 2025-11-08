// Memory/Interface.hpp
#pragma once
#include "../driver/Memory.hpp"

namespace I {
    template <typename T>
    inline T Read(uint64_t address) {
        return read<T>(address);
    }

    template <typename T>
    inline void Write(uint64_t address, const T& value) {
        write<T>(address, value);
    }

    inline void ReadBytes(uint64_t address, unsigned char* buffer, size_t size) {
        mem::read_physical((PVOID)address, buffer, (DWORD)size);
    }

    inline void WriteBytes(uint64_t address, const unsigned char* buffer, size_t size) {
        mem::write_physical((PVOID)address, (PVOID)buffer, (DWORD)size);
    }
}

// Global CR3 variable (defined in render.cpp)
extern uint64_t cr3;