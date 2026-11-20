; ==============================================================================
; includes/structures.inc
; Data structures for Winsock networking and HTTP request representation
; ==============================================================================

IFNDEF STRUCTURES_INC
STRUCTURES_INC EQU 1

; Winsock 2 Initialization Structure (400 bytes on 32-bit Windows)
WSADATA STRUCT
    wVersion        WORD ?
    wHighVersion    WORD ?
    szDescription   BYTE 257 DUP(?)
    szSystemStatus  BYTE 129 DUP(?)
    iMaxSockets     WORD ?
    iMaxUdpDg       WORD ?
    lpVendorInfo    DWORD ?
WSADATA ENDS

; Socket Internet Address Structure (16 bytes)
sockaddr_in STRUCT
    sin_family      WORD 0
    sin_port        WORD 0
    sin_addr        DWORD 0
    sin_zero        BYTE 8 DUP(0)
sockaddr_in ENDS

; Parsed HTTP 1.0 Request Structure
HttpRequest STRUCT
    method          BYTE MAX_METHOD_LEN DUP(0)     ; e.g. "GET" or "POST"
    uri             BYTE MAX_PATH_LEN DUP(0)       ; e.g. "/index.html"
    version         BYTE MAX_VERSION_LEN DUP(0)    ; e.g. "HTTP/1.0"
    contentLength   DWORD 0                        ; Value from Content-Length header
    body            BYTE MAX_BODY_LEN DUP(0)       ; Payload for POST requests
    bodyLen         DWORD 0                        ; Actual received body length
HttpRequest ENDS

ENDIF
