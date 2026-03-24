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
};

int main() {
    SetConsoleTitleW(L"Combat Master Dumper - UnknownCheats Sigs");
    std::cout << "[*] Waiting for CombatMaster.exe..." << std::endl;

    DWORD pid = 0;
    while (!pid) {
        pid = GetProcessIdByName(L"CombatMaster.exe");
        Sleep(1000);
    }

    std::cout << "[+] Found CombatMaster.exe (PID: " << pid << ")" << std::endl;

    uintptr_t modSize = 0;
    uintptr_t modBase = GetModuleBaseAddress(pid, L"GameAssembly.dll", modSize);
    
    if (!modBase) {
        std::cerr << "[-] Failed to find GameAssembly.dll. Is the game fully loaded?" << std::endl;
        system("pause");
        return 1;
    }

    std::cout << "[+] GameAssembly.dll Base: 0x" << std::hex << modBase << " Size: 0x" << modSize << std::endl;

    HANDLE hProc = OpenProcess(PROCESS_VM_READ, FALSE, pid);
    if (!hProc) {
        std::cerr << "[-] Failed to open process. Try running as Admin." << std::endl;
        system("pause");
        return 1;
    }

    std::cout << "[*] Dumping GameAssembly.dll to memory..." << std::endl;
    std::vector<uint8_t> dump(modSize);
    SIZE_T bytesRead;
    if (!ReadProcessMemory(hProc, (LPCVOID)modBase, dump.data(), modSize, &bytesRead)) {
        std::cerr << "[-] ReadProcessMemory failed." << std::endl;
        CloseHandle(hProc);
        system("pause");
        return 1;
    }

    std::vector<SigData> sigs = {
        {"get_transform", "40 ? 48 83 EC ? 48 8B ? ? ? ? ? 48 8B ? 48 85 ? 75 ? 48 8D ? ? ? ? ? E8 ? ? ? ? 48 89 ? ? ? ? ? 48 8B ? 48 83 C4 ? 5B 48 FF ? CC CC CC CC CC CC CC CC CC CC CC CC CC 48 89 ? ? ? 57 48 83 EC ? 48 8B ? ? ? ? ? 48 8B ? 48 8B ? 48 85 ? 75 ? 48 8D ? ? ? ? ? E8 ? ? ? ? 48 89 ? ? ? ? ? 48 8B ? FF D0 48 8B"},
        {"get_transform_position", "48 89 ? ? ? 57 48 83 EC ? 33 C0 48 8B ? 48 89 ? 48 8B ? 89 41 ? 48 8B ? ? ? ? ? 48 85 ? 75 ? 48 8D ? ? ? ? ? E8 ? ? ? ? 48 89 ? ? ? ? ? 48 8B ? 48 8B ? FF D0 48 8B ? 48 8B ? ? ? 48 83 C4 ? 5F C3 CC CC CC 48 89 ? ? ? 57 48 83 EC ? 33 C0 0F 57"},
        {"get_teamid", "40 ? 48 83 EC ? 80 3D A3 26 CD 02"},
        {"get_camera", "48 83 EC ? 48 8B ? ? ? ? ? 48 85 ? 75 ? 48 8D ? ? ? ? ? E8 ? ? ? ? 48 89 ? ? ? ? ? 48 83 C4 ? 48 FF ? CC CC CC CC CC CC 40 ? 48 83 EC ? 48 8B ? ? ? ? ? 48 8B ? 48 85 ? 75 ? 48 8D ? ? ? ? ? E8 ? ? ? ? 48 89 ? ? ? ? ? 48 8B ? 48 83 C4 ? 5B 48 FF ? CC CC CC CC CC CC CC CC CC CC CC CC CC 48 89 ? ? ? 57 48 83 EC ? 48 8B ? ? ? ? ? 48 8B ? 48 8B ? 48 85 ? 75 ? 48 8D ? ? ? ? ? E8 ? ? ? ? 48 89 ? ? ? ? ? 48 8B ? 48 8B ? 48 8B ? ? ? 48 83 C4 ? 5F 48 FF ? CC CC CC CC CC CC CC CC CC CC CC CC CC CC 48 89 ? ? ? 57"}
    };

    std::cout << "\n[+] Scanning for Signatures...\n" << std::endl;
    std::ofstream outFile("Dumped_RVAs.txt");

    for (const auto& sig : sigs) {
        uintptr_t result = FindPattern(dump, sig.Pattern);
        if (result) {
            std::cout << "constexpr uintptr_t " << sig.Name << " = 0x" << std::uppercase << std::hex << result << ";" << std::endl;
            outFile << "constexpr uintptr_t " << sig.Name << " = 0x" << std::uppercase << std::hex << result << ";" << std::endl;
        } else {
            std::cout << "// FAILED TO FIND: " << sig.Name << std::endl;
            outFile << "// FAILED TO FIND: " << sig.Name << std::endl;
        }
    }

    outFile.close();
    CloseHandle(hProc);
    
    std::cout << "\n[+] Dump saved to Dumped_RVAs.txt" << std::endl;
    system("pause");
    return 0;
}
