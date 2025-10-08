
#pragma once
#include <windows.h>
#include <vector>

class MemoryReader {
public:
    MemoryReader(HANDLE processHandle);
    ~MemoryReader();

    // Read arbitrary data from process memory
    bool ReadMemory(uintptr_t address, void* buffer, SIZE_T size) const;

    // Read a pointer chain with offsets: base + offsets[0] -> ptr1 + offsets[1] -> ptr2 ...
    bool ReadPointerChain(uintptr_t baseAddress, const std::vector<uintptr_t>& offsets, uintptr_t& outAddress) const;

private:
    HANDLE m_processHandle;
};