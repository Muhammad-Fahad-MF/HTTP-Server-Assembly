; ==============================================================================
; includes/constants.inc
; Global constants for HTTP Server (Winsock, buffers, ports, and console colors)
; ==============================================================================

IFNDEF CONSTANTS_INC
CONSTANTS_INC EQU 1

; Network Configuration
SERVER_PORT         EQU 8080
SOMAXCONN_VAL       EQU 5

; Winsock Constants
AF_INET             EQU 2
SOCK_STREAM         EQU 1
IPPROTO_TCP         EQU 6
INVALID_SOCKET_VAL  EQU -1
SOCKET_ERROR_VAL    EQU -1

; Buffer Sizes
RECV_BUFFER_SIZE    EQU 4096
SEND_BUFFER_SIZE    EQU 8192
FILE_BUFFER_SIZE    EQU 4096
MAX_PATH_LEN        EQU 128
MAX_METHOD_LEN      EQU 16
MAX_VERSION_LEN     EQU 16
MAX_BODY_LEN        EQU 1024

; Irvine Console Color Codes
COLOR_DEFAULT       EQU 7    ; Light gray on black
COLOR_BANNER        EQU 11   ; Light cyan
COLOR_SUCCESS       EQU 10   ; Light green
COLOR_INFO          EQU 14   ; Yellow
COLOR_ERROR         EQU 12   ; Light red
COLOR_DEBUG         EQU 13   ; Light magenta

ENDIF
