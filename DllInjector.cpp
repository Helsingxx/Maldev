#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <iostream>

const char* dllPath = ""; // "C:\\path\\to\\dll.dll"
const TCHAR* processName = _T(""); // "process.exe"

DWORD FindProcess(const TCHAR* processName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to take snapshot of processes." << std::endl;
        return false;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hSnapshot, &pe32)) {
        std::cerr << "Failed to retrieve information about the first process." << std::endl;
        CloseHandle(hSnapshot);
        return false;
    }

    do {
        if (_tcscmp(pe32.szExeFile, processName) == 0) {
            std::wcout << "Found process: " << pe32.szExeFile << " with PID: " << pe32.th32ProcessID << std::endl;
            CloseHandle(hSnapshot);
            return pe32.th32ProcessID;
        }
    } while (Process32Next(hSnapshot, &pe32));

    std::cerr << "Process not found." << std::endl;
    CloseHandle(hSnapshot);
    return false;
}

int main() {
    DWORD processID = FindProcess(processName);
    if (!processID) {
        std::cerr << "Failed to find process." << std::endl;
        return 1;
    }
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, true, processID);
    if (hProcess == NULL) {
        DWORD error = GetLastError();
        std::cerr << "Failed to open process. Error code: " << error << std::endl;
        return 1;
    }
    LPVOID pRemoteMemory = VirtualAllocEx(hProcess, NULL, strlen(dllPath), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (pRemoteMemory == NULL) {
        std::cerr << "Failed to allocate memory in the remote process." << std::endl;
        CloseHandle(hProcess);
        return 1;
    }
    std::cout << "Memory allocated at address: " << pRemoteMemory << std::endl;
    if (!WriteProcessMemory(hProcess, pRemoteMemory, dllPath, strlen(dllPath), NULL)) {
        std::cerr << "Failed to write to process memory." << std::endl;
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }
    HMODULE kernel32 = GetModuleHandle(_T("kernel32.dll"));
    if (kernel32 == NULL) {
        std::cerr << "Failed to get handle of kernel32.dll." << std::endl;
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }
    HANDLE thread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)GetProcAddress(kernel32, "LoadLibraryA"), pRemoteMemory, 0, NULL);
    if (thread == NULL) {
        std::cerr << "Failed to create remote thread." << std::endl;
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }
    WaitForSingleObject(thread, INFINITE);
    DWORD exitCode;
    if (!GetExitCodeThread(thread, &exitCode)) {
        std::cerr << "Failed to get exit code of the thread. Error: " << GetLastError() << std::endl;
        CloseHandle(thread);
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }
    std::cout << "Remote thread exited with code: " << exitCode << std::endl;
    CloseHandle(thread);
    VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
    CloseHandle(hProcess);
    return 0;
}