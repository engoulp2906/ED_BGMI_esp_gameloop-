#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <string>
#include "ProcessManager.h"
#include "MemoryReader.h"
#include "Overlay.h"

// Your offsets as constants
constexpr uintptr_t OFFSET_GWORLD = 0x0A095EBC;
constexpr uintptr_t OFFSET_GWORLD_ADDITIONAL = 0x3C;

constexpr uintptr_t OFFSET_PERSISTENTLEVEL = 0x20;
constexpr uintptr_t OFFSET_ACTORS_ARRAY = 0xA0;
constexpr uintptr_t OFFSET_ACTORS_COUNT = 0xA8;

constexpr uintptr_t OFFSET_ROOTCOMPONENT = 0x158;
constexpr uintptr_t OFFSET_RELATIVELOCATION = 0x120;
constexpr uintptr_t OFFSET_TEAMID = 0x6D8;
constexpr uintptr_t OFFSET_ISDEAD = 0xA5C;

constexpr uintptr_t OFFSET_VIEWMATRIX = 0x09E373CC;

struct FVector {
    float X, Y, Z;
};

uintptr_t GetModuleBaseAddress(DWORD processId, const std::wstring& moduleName) {
    uintptr_t moduleBaseAddress = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        std::wcerr << L"Failed to create module snapshot.\n";
        return 0;
    }

    MODULEENTRY32W moduleEntry;
    moduleEntry.dwSize = sizeof(MODULEENTRY32W);

    if (Module32FirstW(hSnapshot, &moduleEntry)) {
        do {
            if (_wcsicmp(moduleEntry.szModule, moduleName.c_str()) == 0) {
                moduleBaseAddress = reinterpret_cast<uintptr_t>(moduleEntry.modBaseAddr);
                break;
            }
        } while (Module32NextW(hSnapshot, &moduleEntry));
    }

    CloseHandle(hSnapshot);
    return moduleBaseAddress;
}

uintptr_t ReadGWorld(MemoryReader& memReader, uintptr_t ue4Base) {
    uintptr_t gWorldPtr = 0;
    if (!memReader.ReadMemory(ue4Base + OFFSET_GWORLD, &gWorldPtr, sizeof(uintptr_t))) return 0;

    uintptr_t gWorld = 0;
    if (!memReader.ReadMemory(gWorldPtr + OFFSET_GWORLD_ADDITIONAL, &gWorld, sizeof(uintptr_t))) return 0;

    return gWorld;
}

uintptr_t ReadPersistentLevel(MemoryReader& memReader, uintptr_t gWorld) {
    uintptr_t persistentLevel = 0;
    if (!memReader.ReadMemory(gWorld + OFFSET_PERSISTENTLEVEL, &persistentLevel, sizeof(uintptr_t))) return 0;
    return persistentLevel;
}

bool ReadActors(MemoryReader& memReader, uintptr_t persistentLevel, uintptr_t& actorsArray, int& actorsCount) {
    if (!memReader.ReadMemory(persistentLevel + OFFSET_ACTORS_ARRAY, &actorsArray, sizeof(uintptr_t))) return false;
    if (!memReader.ReadMemory(persistentLevel + OFFSET_ACTORS_COUNT, &actorsCount, sizeof(int))) return false;
    return true;
}

bool ReadActorInfo(MemoryReader& memReader, uintptr_t actor, FVector& pos, int& teamId, bool& isDead) {
    uintptr_t rootComponent = 0;
    if (!memReader.ReadMemory(actor + OFFSET_ROOTCOMPONENT, &rootComponent, sizeof(uintptr_t))) return false;

    if (!memReader.ReadMemory(rootComponent + OFFSET_RELATIVELOCATION, &pos, sizeof(FVector))) return false;
    if (!memReader.ReadMemory(actor + OFFSET_TEAMID, &teamId, sizeof(int))) return false;

    BYTE deadFlag = 0;
    if (!memReader.ReadMemory(actor + OFFSET_ISDEAD, &deadFlag, sizeof(BYTE))) return false;
    isDead = (deadFlag != 0);

    return true;
}

bool ReadViewMatrix(MemoryReader& memReader, uintptr_t ue4Base, float viewMatrix[16]) {
    uintptr_t viewMatrixAddr = ue4Base + OFFSET_VIEWMATRIX;
    return memReader.ReadMemory(viewMatrixAddr, viewMatrix, sizeof(float) * 16);
}

bool WorldToScreen(const FVector& worldPos, int screenWidth, int screenHeight, float viewMatrix[16], int& screenX, int& screenY) {
    float clipX = worldPos.X * viewMatrix[0] + worldPos.Y * viewMatrix[4] + worldPos.Z * viewMatrix[8] + viewMatrix[12];
    float clipY = worldPos.X * viewMatrix[1] + worldPos.Y * viewMatrix[5] + worldPos.Z * viewMatrix[9] + viewMatrix[13];
    float clipW = worldPos.X * viewMatrix[3] + worldPos.Y * viewMatrix[7] + worldPos.Z * viewMatrix[11] + viewMatrix[15];

    if (clipW < 0.1f) return false;

    float ndcX = clipX / clipW;
    float ndcY = clipY / clipW;

    screenX = static_cast<int>((screenWidth / 2) * (1 + ndcX));
    screenY = static_cast<int>((screenHeight / 2) * (1 - ndcY));

    return true;
}

int main() {
    ProcessManager procManager(L"AndroidEmulatorEx.exe");
    if (!procManager.AttachToProcess()) {
        std::cerr << "Failed to attach to Gameloop process.\n";
        return 1;
    }

    MemoryReader memReader(procManager.GetProcessHandle());

    // Dynamically get UE4 base address (replace module name with your actual UE4 module name)
    uintptr_t ue4Base = GetModuleBaseAddress(procManager.GetProcessId(), L"9daaa9381e928e43.nvph");
    if (!ue4Base) {
        std::cerr << "Failed to find UE4 base address.\n";
        return 1;
    }
    std::cout << "UE4 base address: 0x" << std::hex << ue4Base << std::dec << "\n";

    uintptr_t gWorld = ReadGWorld(memReader, ue4Base);
    if (!gWorld) {
        std::cerr << "Failed to read GWorld pointer.\n";
        return 1;
    }

    uintptr_t persistentLevel = ReadPersistentLevel(memReader, gWorld);
    if (!persistentLevel) {
        std::cerr << "Failed to read PersistentLevel pointer.\n";
        return 1;
    }

    uintptr_t actorsArray = 0;
    int actorsCount = 0;
    if (!ReadActors(memReader, persistentLevel, actorsArray, actorsCount)) {
        std::cerr << "Failed to read actors array.\n";
        return 1;
    }

    HWND gameloopWnd = FindWindow(NULL, L"Gameloop");
    if (!gameloopWnd) {
        std::cerr << "Failed to find Gameloop window.\n";
        return 1;
    }

    RECT rect;
    GetClientRect(gameloopWnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    Overlay overlay(L"ESP Overlay", gameloopWnd);
    if (!overlay.CreateOverlayWindow(width, height)) {
        std::cerr << "Failed to create overlay window.\n";
        return 1;
    }

    float viewMatrix[16] = {0};

    while (true) {
        if (!ReadViewMatrix(memReader, ue4Base, viewMatrix)) {
            Sleep(10);
            continue;
        }

        for (int i = 0; i < actorsCount; i++) {
            uintptr_t actor = 0;
            if (!memReader.ReadMemory(actorsArray + i * sizeof(uintptr_t), &actor, sizeof(uintptr_t))) continue;
            if (actor == 0) continue;

            FVector pos;
            int teamId = 0;
            bool isDead = false;

            if (!ReadActorInfo(memReader, actor, pos, teamId, isDead)) continue;
            if (isDead) continue;

            // TODO: Add your team filtering here to show only enemies

            int screenX, screenY;
            if (WorldToScreen(pos, width, height, viewMatrix, screenX, screenY)) {
                overlay.RenderDot(screenX, screenY);
            }
        }

        Sleep(16); // ~60 FPS
    }

    return 0;
}