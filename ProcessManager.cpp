#include "ProcessManager.h"
#include <tlhelp32.h>
#include <iostream>

ProcessManager::ProcessManager(const std::wstring& processName)
    : m_processName(processName), m_pid(0), m_processHandle(NULL) {
}

ProcessManager::~ProcessManager() {
    CloseHandleIfNeeded();
}

DWORD ProcessManager::FindProcessIdByName(const std::wstring& processName) {
    PROCESSENTRY32W entry;
    entry.dwSize = sizeof(PROCESSENTRY32W);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        std::wcerr << L"Failed to create process snapshot.\n";
        return 0;
    }

    if (Process32FirstW(snapshot, &entry)) {
        do {
            if (_wcsicmp(entry.szExeFile, processName.c_str()) == 0) {
                CloseHandle(snapshot);
                return entry.th32ProcessID;
            }
        } while (Process32NextW(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return 0;
}

bool ProcessManager::AttachToProcess() {
    CloseHandleIfNeeded();

    m_pid = FindProcessIdByName(m_processName);
    if (m_pid == 0) {
        std::wcerr << L"Process " << m_processName << L" not found.\n";
        return false;
    }

    m_processHandle = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, m_pid);
    if (m_processHandle == NULL) {
        std::wcerr << L"Failed to open process handle. Error: " << GetLastError() << L"\n";
        m_pid = 0;
        return false;
    }

    std::wcout << L"Attached to process " << m_processName << L" with PID " << m_pid << L"\n";
    return true;
}

bool ProcessManager::IsAttached() const {
    return m_processHandle != NULL && m_pid != 0;
}

HANDLE ProcessManager::GetProcessHandle() const {
    return m_processHandle;
}

DWORD ProcessManager::GetProcessId() const {
    return m_pid;
}

void ProcessManager::CloseHandleIfNeeded() {
    if (m_processHandle) {
        CloseHandle(m_processHandle);
        m_processHandle = NULL;
        m_pid = 0;
    }
}

bool ProcessManager::Refresh() {
    // Try to find process again and reopen handle
    return AttachToProcess();
}