#include "MemoryReader.h"
#include <iostream>

MemoryReader::MemoryReader(HANDLE processHandle)
    : m_processHandle(processHandle) {
}

MemoryReader::~MemoryReader() {
    // No need to close handle here; ProcessManager owns it
}

bool MemoryReader::ReadMemory(uintptr_t address, void* buffer, SIZE_T size) const {
    SIZE_T bytesRead;
    BOOL result = ReadProcessMemory(m_processHandle, (LPCVOID)address, buffer, size, &bytesRead);
    if (!result || bytesRead != size) {
        // Optional: print error or silently fail
        return false;
    }
    return true;
}

bool MemoryReader::ReadPointerChain(uintptr_t baseAddress, const std::vector<uintptr_t>& offsets, uintptr_t& outAddress) const {
    uintptr_t address = baseAddress;
    for (size_t i = 0; i < offsets.size(); ++i) {
        if (!ReadMemory(address, &address, sizeof(address))) {
            return false;
        }
        address += offsets[i];
    }
    outAddress = address;
    return true;
}