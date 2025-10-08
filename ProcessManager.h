
#pragma once
#include <windows.h>
#include <string>

class ProcessManager {
public:
    ProcessManager(const std::wstring& processName);
    ~ProcessManager();

    // Finds the process by name and opens a handle
    bool AttachToProcess();

    // Returns true if currently attached
    bool IsAttached() const;

    // Returns the opened process handle
    HANDLE GetProcessHandle() const;

    // Returns the PID of the attached process
    DWORD GetProcessId() const;

    // Refreshes the process handle (e.g., if process restarted)
    bool Refresh();

private:
    std::wstring m_processName;
    DWORD m_pid;
    HANDLE m_processHandle;

    DWORD FindProcessIdByName(const std::wstring& processName);
    void CloseHandleIfNeeded();
};