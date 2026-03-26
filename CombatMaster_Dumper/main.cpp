#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>

// Helper to convert IDA style signature to bytes and mask
bool ParseSignature(const std::string& signature, std::vector<uint8_t>& bytes, std::string& mask) {
    std::istringstream iss(signature);
    std::string byteStr;
    while (iss >> byteStr) {
        if (byteStr == "?" || byteStr == "??") {
            bytes.push_back(0);
            mask += "?";
        } else {
            bytes.push_back((uint8_t)std::stoul(byteStr, nullptr, 16));
            mask += "x";
        }
    }
    return !bytes.empty();
}

// Find Pattern in memory dump
uintptr_t FindPattern(const std::vector<uint8_t>& data, const std::string& signature) {
    std::vector<uint8_t> bytes;
    std::string mask;
    if (!ParseSignature(signature, bytes, mask)) return 0;

    for (size_t i = 0; i < data.size() - bytes.size(); ++i) {
        bool found = true;
        for (size_t j = 0; j < bytes.size(); ++j) {
            if (mask[j] == 'x' && data[i + j] != bytes[j]) {
                found = false;
                break;
            }
        }
        if (found) return i;
    }
    return 0;
}

DWORD GetProcessIdByName(const std::wstring& name) {
    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (Process32FirstW(hSnapshot, &pe32)) {
        do {
            if (name == pe32.szExeFile) {
                CloseHandle(hSnapshot);
                return pe32.th32ProcessID;
            }
        } while (Process32NextW(hSnapshot, &pe32));
    }
    CloseHandle(hSnapshot);
    return 0;
}

uintptr_t GetModuleBaseAddress(DWORD procId, const std::wstring& modName, uintptr_t& outSize) {
    uintptr_t modBaseAddr = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
    if (hSnap != INVALID_HANDLE_VALUE) {
        MODULEENTRY32W modEntry;
        modEntry.dwSize = sizeof(modEntry);
        if (Module32FirstW(hSnap, &modEntry)) {
            do {
                if (modName == modEntry.szModule) {
                    modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
                    outSize = modEntry.modBaseSize;
                    break;
                }
            } while (Module32NextW(hSnap, &modEntry));
        }
    }
    CloseHandle(hSnap);
    return modBaseAddr;
}

struct SigData {
    std::string Name;
    std::string Pattern;
    int Extra; // Offset for RIP relative or similar
};

int main() {
    SetConsoleTitleW(L"Bloodstrike Dumper (Messiah Engine) - Nexus");
    std::cout << "[*] Searching for Bloodstrike process..." << std::endl;

    DWORD pid = GetProcessIdByName(L"Bloodstrike.exe");
    if (!pid) pid = GetProcessIdByName(L"Bloodstrike_PC.exe");
    
    while (!pid) {
        std::cout << "[!] Waiting for Bloodstrike.exe / Bloodstrike_PC.exe..." << std::endl;
        pid = GetProcessIdByName(L"Bloodstrike.exe");
        if (!pid) pid = GetProcessIdByName(L"Bloodstrike_PC.exe");
        Sleep(1000);
    }

    std::cout << "[+] Found process (PID: " << pid << ")" << std::endl;

    uintptr_t modSize = 0;
    // Messiah Engine usually uses Project.dll or the main exe
    uintptr_t modBase = GetModuleBaseAddress(pid, L"Project.dll", modSize);
    std::wstring modNameTarget = L"Project.dll";
    
    if (!modBase) {
        modBase = GetModuleBaseAddress(pid, L"GameAssembly.dll", modSize);
        modNameTarget = L"GameAssembly.dll";
    }

    if (!modBase) {
        std::cerr << "[-] Failed to find Project.dll or GameAssembly.dll." << std::endl;
        system("pause");
        return 1;
    }

    std::wcout << L"[+] Target Module: " << modNameTarget << L" Base: 0x" << std::hex << modBase << L" Size: 0x" << modSize << std::endl;

    HANDLE hProc = OpenProcess(PROCESS_VM_READ, FALSE, pid);
    if (!hProc) {
        std::cerr << "[-] Failed to open process. Try running as Admin." << std::endl;
        system("pause");
        return 1;
    }

    std::cout << "[*] Dumping module to memory..." << std::endl;
    std::vector<uint8_t> dump(modSize);
    SIZE_T bytesRead;
    if (!ReadProcessMemory(hProc, (LPCVOID)modBase, dump.data(), modSize, &bytesRead)) {
        std::cerr << "[-] ReadProcessMemory failed." << std::endl;
        CloseHandle(hProc);
        system("pause");
        return 1;
    }

    std::vector<SigData> sigs = {
        // Messiah Engine / Bloodstrike Signatures
        {"adrObjects", "48 8D 2D ? ? ? ? 48 2B FD", 0x38},
        {"adrD3D11Device", "48 8B 0D ? ? ? ? 48 8B D9 48 89 4C 24 ? 45 33 ED 45 8D 65 FF", 0},
        {"get_camera", "48 83 EC ? 48 8B ? ? ? ? ? 48 85 ? 75 ? 48 8D ? ? ? ? ? E8", 0},
        {"get_localplayer", "48 8B 05 ? ? ? ? 48 85 C0 74 ? 48 8B 40 ? 48 85 C0 74 ? 48 8B 80", 0}
    };

    std::cout << "\n[+] Scanning for Signatures...\n" << std::endl;
    std::ofstream outFile("Bloodstrike_Offsets.txt");

    for (const auto& sig : sigs) {
        uintptr_t result = FindPattern(dump, sig.Pattern);
        if (result) {
            // Calculate actual RVA for RIP relative if needed (skipped for now as placeholder)
            std::cout << "constexpr uintptr_t " << sig.Name << " = 0x" << std::uppercase << std::hex << result << "; // + extra: " << sig.Extra << std::endl;
            outFile << "constexpr uintptr_t " << sig.Name << " = 0x" << std::uppercase << std::hex << result << ";" << std::endl;
        } else {
            std::cout << "// FAILED TO FIND: " << sig.Name << std::endl;
            outFile << "// FAILED TO FIND: " << sig.Name << std::endl;
        }
    }

    outFile.close();
    CloseHandle(hProc);
    
    std::cout << "\n[+] Offsets saved to Bloodstrike_Offsets.txt" << std::endl;
    system("pause");
    return 0;
}
