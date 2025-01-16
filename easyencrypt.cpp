#include <windows.h>
#include <wincrypt.h>
#include <cstdio>
#pragma comment (lib, "crypt32.lib")

// this is not a ransomware!

void encrypt(LPCWSTR filename){
    BYTE temp[1024];
    DWORD bytesRead, bytesWritten;

    HANDLE originalFile = CreateFileW(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    HANDLE encryptedFile = CreateFileW(L"encrypted.lol", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    HCRYPTPROV csp;
    CryptAcquireContextW(&csp, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
    HCRYPTKEY key;
    CryptGenKey(csp, CALG_RC4, CRYPT_EXPORTABLE, &key);
    while (ReadFile(originalFile, temp, sizeof(temp), &bytesRead, NULL) && bytesRead){
        CryptEncrypt(key, 0, bytesRead < sizeof(temp), 0, temp, &bytesRead, sizeof(temp)); // litle bug here but who cares
        WriteFile(encryptedFile, temp, bytesRead, &bytesWritten, NULL);
    }
    CryptDestroyKey(key);
    CryptReleaseContext(csp, 0);
    CloseHandle(originalFile);
    CloseHandle(encryptedFile);
}

int main(){
    encrypt(L"encryptme.txt");
    return 0;
}