#include <windows.h>
#include <tlhelp32.h>
#include <iostream>

unsigned char bytes[] = { 0x29 };
LPCWSTR executable = L""; // Path to the executable

DWORD FindProcess(const std::wstring& processName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to create snapshot: " << GetLastError() << std::endl;
        return 0;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hSnapshot, &pe32)) {
        std::cerr << "Failed to retrieve the first process: " << GetLastError() << std::endl;
        CloseHandle(hSnapshot);
        return 0;
    }

    do {
        if (processName == pe32.szExeFile) {
            std::wcout << L"Found process: " << pe32.szExeFile << L" with PID: " << pe32.th32ProcessID << std::endl;
            CloseHandle(hSnapshot);
            return pe32.th32ProcessID;
        }
    } while (Process32Next(hSnapshot, &pe32));

    std::wcout << L"Process " << processName << L" not found." << std::endl;
    CloseHandle(hSnapshot);
    return 0;
}

int main() {
    std::wstring processName = L"wesnoth.exe";
    DWORD pid = FindProcess(processName);
    if (!pid) {
        std::cerr << "Process not found or an error occurred." << std::endl;
        return 1;
    }
    else {
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
        if (hProcess == NULL) {
            std::cerr << "Failed to open process: " << GetLastError() << std::endl;
            return 1;
        }
        std::cout << "Process opened successfully." << std::endl;

        HANDLE module_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
        if (module_snapshot == INVALID_HANDLE_VALUE) {
            std::cerr << "Failed to create module snapshot: " << GetLastError() << std::endl;
            CloseHandle(hProcess);
            return 1;
        }

        MODULEENTRY32 module_entry;
        module_entry.dwSize = sizeof(MODULEENTRY32);

        if (!Module32First(module_snapshot, &module_entry)) {
            std::cerr << "Failed to retrieve the first module: " << GetLastError() << std::endl;
            CloseHandle(module_snapshot);
            CloseHandle(hProcess);
            return 1;
        }

        do {
            if (wcsstr(module_entry.szModule, executable) != NULL) {
                std::wcout << L"Found module: " << module_entry.szModule << L" at address: " << module_entry.modBaseAddr << std::endl;
            }
        } while (Module32Next(module_snapshot, &module_entry));

        unsigned char* buffer = new unsigned char[module_entry.modBaseSize];
        SIZE_T bytesRead = 0;

        if (!ReadProcessMemory(hProcess, module_entry.modBaseAddr, buffer, module_entry.modBaseSize, &bytesRead)) {
            std::cerr << "Failed to read process memory: " << GetLastError() << std::endl;
            delete[] buffer;
            CloseHandle(module_snapshot);
            CloseHandle(hProcess);
            return 1;
        }

        CloseHandle(module_snapshot);
        CloseHandle(hProcess);

        for (unsigned int i = 0; i < module_entry.modBaseSize - sizeof(bytes); i++) {
            for (int j = 0; j < sizeof(bytes); j++) {
                if (bytes[j] != buffer[i + j]) {
                    break;
                }

                if (j + 1 == sizeof(bytes)) {
                    printf("%x\n", i + (DWORD)module_entry.modBaseAddr);
                }
            }
        }
		std::cout << "Scan complete." << std::endl;
        delete[] buffer;

    }
    return 0;
}