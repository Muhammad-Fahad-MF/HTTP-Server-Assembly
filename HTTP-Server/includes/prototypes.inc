; ==============================================================================
; includes/prototypes.inc
; External API prototypes for Windows Winsock 2 (ws2_32.lib)
; ==============================================================================

IFNDEF PROTOTYPES_INC
PROTOTYPES_INC EQU 1

; Winsock 2 import library
INCLUDELIB ws2_32.lib

; Winsock 2 External C/Windows API declarations
WSAStartup          PROTO STDCALL :DWORD, :DWORD
WSACleanup          PROTO STDCALL
socket              PROTO STDCALL :DWORD, :DWORD, :DWORD
bind                PROTO STDCALL :DWORD, :DWORD, :DWORD
listen              PROTO STDCALL :DWORD, :DWORD
accept              PROTO STDCALL :DWORD, :DWORD, :DWORD
recv                PROTO STDCALL :DWORD, :DWORD, :DWORD, :DWORD
send                PROTO STDCALL :DWORD, :DWORD, :DWORD, :DWORD
closesocket         PROTO STDCALL :DWORD
htons               PROTO STDCALL :WORD
WSAGetLastError     PROTO STDCALL

ENDIF
