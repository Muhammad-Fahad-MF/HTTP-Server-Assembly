; ==============================================================================
; winsock/socket_utils.inc
; Winsock 2 network wrapper procedures for TCP server socket lifecycle
; Uses standard MASM INVOKE with ws2_32.lib for robust STDCALL parameter passing
; ==============================================================================

.data
    g_wsadata   BYTE 400 DUP(0)        ; 400-byte buffer for Winsock 2 WSADATA
    g_serverSin sockaddr_in <>         ; 16-byte sockaddr_in

.code

; ------------------------------------------------------------------------------
; InitWinsock: Initializes the Winsock 2 library subsystem (version 2.2).
; Parameters: None
; Returns: EAX = 0 on success, non-zero error code on failure
; ------------------------------------------------------------------------------
InitWinsock PROC
    INVOKE WSAStartup, 0202h, ADDR g_wsadata
    ret
InitWinsock ENDP


; ------------------------------------------------------------------------------
; CreateServerSocket: Creates, binds, and configures a TCP listening socket.
; Parameters: [ebp+8]=port (DWORD/WORD)
; Returns: EAX = valid listening socket handle, or INVALID_SOCKET_VAL (-1) on error
; ------------------------------------------------------------------------------
CreateServerSocket PROC
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi

    ; 1. Create TCP streaming socket
    INVOKE socket, AF_INET, SOCK_STREAM, IPPROTO_TCP
    cmp eax, INVALID_SOCKET_VAL
    je CreateSocketFail

    mov ebx, eax                       ; EBX = socket handle

    ; 2. Configure sockaddr_in struct
    mov WORD PTR [g_serverSin.sin_family], AF_INET

    ; Convert port to network byte order (htons)
    movzx eax, WORD PTR [ebp+8]
    INVOKE htons, ax
    mov WORD PTR [g_serverSin.sin_port], ax

    mov DWORD PTR [g_serverSin.sin_addr], 0    ; INADDR_ANY (0.0.0.0)

    ; Clear sin_zero bytes
    lea edi, [g_serverSin.sin_zero]
    xor al, al
    mov ecx, 8
    cld
    rep stosb

    ; 3. Bind socket to local address
    INVOKE bind, ebx, ADDR g_serverSin, 16
    cmp eax, SOCKET_ERROR_VAL
    je BindError

    ; 4. Put socket in listening mode
    INVOKE listen, ebx, SOMAXCONN_VAL
    cmp eax, SOCKET_ERROR_VAL
    je ListenError

    mov eax, ebx                       ; Return valid server socket
    jmp CreateSocketDone

BindError:
ListenError:
    INVOKE closesocket, ebx
CreateSocketFail:
    mov eax, INVALID_SOCKET_VAL

CreateSocketDone:
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret 4
CreateServerSocket ENDP


; ------------------------------------------------------------------------------
; AcceptClient: Blocks waiting for an incoming TCP client connection.
; Parameters: [ebp+8]=listenSocket (DWORD)
; Returns: EAX = connected client socket handle, or INVALID_SOCKET_VAL on error
; ------------------------------------------------------------------------------
AcceptClient PROC
    push ebp
    mov ebp, esp

    INVOKE accept, DWORD PTR [ebp+8], 0, 0

    pop ebp
    ret 4
AcceptClient ENDP


; ------------------------------------------------------------------------------
; RecvRequest: Reads incoming HTTP request bytes into buffer and null-terminates.
; Parameters: [ebp+8]=clientSocket, [ebp+12]=pBuffer, [ebp+16]=bufSize
; Returns: EAX = bytes received (0=client closed, <0=error)
; ------------------------------------------------------------------------------
RecvRequest PROC
    push ebp
    mov ebp, esp
    push esi

    mov ecx, [ebp+16]
    dec ecx                            ; Reserve space for null terminator

    INVOKE recv, DWORD PTR [ebp+8], DWORD PTR [ebp+12], ecx, 0

    cmp eax, 0
    jle RecvDone

    ; Null-terminate the received buffer
    mov esi, [ebp+12]
    mov BYTE PTR [esi+eax], 0

RecvDone:
    pop esi
    pop ebp
    ret 12
RecvRequest ENDP


; ------------------------------------------------------------------------------
; SendResponseData: Transmits a buffer of bytes over a connected TCP client socket.
; Parameters: [ebp+8]=clientSocket, [ebp+12]=pBuffer, [ebp+16]=bufLen
; Returns: EAX = total bytes sent, or SOCKET_ERROR_VAL on failure
; ------------------------------------------------------------------------------
SendResponseData PROC
    push ebp
    mov ebp, esp

    INVOKE send, DWORD PTR [ebp+8], DWORD PTR [ebp+12], DWORD PTR [ebp+16], 0

    pop ebp
    ret 12
SendResponseData ENDP


; ------------------------------------------------------------------------------
; CloseClientSocket: Closes an open socket descriptor cleanly.
; Parameters: [ebp+8]=s (socket handle)
; Returns: EAX = 0 on success, or SOCKET_ERROR_VAL on error
; ------------------------------------------------------------------------------
CloseClientSocket PROC
    push ebp
    mov ebp, esp

    INVOKE closesocket, DWORD PTR [ebp+8]

    pop ebp
    ret 4
CloseClientSocket ENDP


; ------------------------------------------------------------------------------
; CleanupWinsock: Terminates Winsock 2 library use.
; Parameters: None
; Returns: EAX = 0 on success
; ------------------------------------------------------------------------------
CleanupWinsock PROC
    INVOKE WSACleanup
    ret
CleanupWinsock ENDP
