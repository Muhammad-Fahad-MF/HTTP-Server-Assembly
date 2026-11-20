; ==============================================================================
; main.asm
; Entry point for the COAL MASM x86 HTTP 1.0 Web Server
; Built with Irvine32 console support and Winsock 2 networking
; ==============================================================================

INCLUDE Irvine32.inc
INCLUDE includes/constants.inc
INCLUDE includes/structures.inc
INCLUDE includes/prototypes.inc

; Utility routines
INCLUDE utils/strings.inc
INCLUDE utils/memory.inc
INCLUDE utils/conversions.inc

; Networking routines
INCLUDE winsock/socket_utils.inc

; HTTP protocol routines
INCLUDE http/headers.inc
INCLUDE http/request_parser.inc
INCLUDE http/response.inc

.data
    ; Server State
    g_serverSocket      DWORD ?
    g_clientSocket      DWORD ?
    g_rawRecvBuffer     BYTE RECV_BUFFER_SIZE DUP(0)
    g_currentRequest    HttpRequest <>

    ; Method literals
    szMethodGet         BYTE "GET", 0
    szMethodPost        BYTE "POST", 0

    ; Console Banners and Messages
    msgBanner1          BYTE "==========================================================", 13, 10, 0
    msgBanner2          BYTE "           COAL MASM x86 HTTP 1.0 WEB SERVER              ", 13, 10, 0
    msgBanner3          BYTE "==========================================================", 13, 10, 0
    msgListening        BYTE "[SERVER] Server listening on http://localhost:8080 ...", 13, 10, 0
    msgPressExit        BYTE "[SERVER] Ready to accept connections. Press Ctrl+C to stop.", 13, 10, 13, 10, 0

    msgClientAccepted   BYTE "[CONN] Accepted incoming client connection.", 13, 10, 0
    msgEmptyReq         BYTE "[WARN] Empty request received or client disconnected.", 13, 10, 0
    msgMalformedReq     BYTE "[WARN] Failed to parse HTTP request line.", 13, 10, 0

    msgReqMethod        BYTE "[REQ] Method: ", 0
    msgReqPath          BYTE " | URI: ", 0
    msgReqDone          BYTE "[DONE] Response transmitted, connection closed.", 13, 10, 13, 10, 0

    msgPostReceived     BYTE "[POST] Received POST payload (Content-Length: ", 0
    msgPostBytes        BYTE " bytes):", 13, 10, 0
    msgPostPayload      BYTE "       Payload: ", 0

    msgInitFail         BYTE "[ERROR] Failed to initialize Winsock subsystem.", 13, 10, 0
    msgBindFail         BYTE "[ERROR] Failed to create or bind server socket to port 8080.", 13, 10, 0

    ; POST Response Payload
    szJsonPostResponse  BYTE '{"status":"success","message":"POST payload received by COAL MASM Server"}', 13, 10, 0

.code

; ------------------------------------------------------------------------------
; Endpoint_Post: Handles HTTP POST submissions by logging payload and responding.
; Parameters: [ebp+8]=clientSocket, [ebp+12]=pRequest (HttpRequest ptr)
; Returns: EAX = bytes sent
; ------------------------------------------------------------------------------
Endpoint_Post PROC
    push ebp
    mov ebp, esp
    push ebx
    push esi

    mov esi, [ebp+12]                  ; ESI = HttpRequest ptr

    ; Print POST receipt message in yellow
    mov eax, COLOR_INFO
    call SetTextColor

    mov edx, OFFSET msgPostReceived
    call WriteString

    mov eax, [(HttpRequest PTR [esi]).contentLength]
    call WriteDec

    mov edx, OFFSET msgPostBytes
    call WriteString

    ; Print the actual body payload
    mov edx, OFFSET msgPostPayload
    call WriteString

    lea edx, [(HttpRequest PTR [esi]).body]
    call WriteString
    call Crlf

    ; Restore console color
    mov eax, COLOR_DEFAULT
    call SetTextColor

    ; Calculate JSON body length
    push OFFSET szJsonPostResponse
    call StrLen

    ; Send 200 OK with JSON response
    push eax                           ; bodyLen
    push OFFSET szJsonPostResponse     ; pBody
    push OFFSET szTypeJson             ; "application/json"
    push OFFSET szStatus200            ; "200 OK"
    push DWORD PTR [ebp+8]             ; clientSocket
    call SendHttpResponse

    pop esi
    pop ebx
    pop ebp
    ret 8
Endpoint_Post ENDP


; ------------------------------------------------------------------------------
; RouteRequest: Inspects HTTP method and routes to appropriate handler/file PROC.
; Parameters: [ebp+8]=clientSocket, [ebp+12]=pRequest (HttpRequest ptr)
; Returns: EAX = 1 on success, 0 on error
; ------------------------------------------------------------------------------
RouteRequest PROC
    push ebp
    mov ebp, esp
    push ebx
    push esi

    mov esi, [ebp+12]                  ; ESI = HttpRequest ptr

    ; 1. Check if method is POST
    push OFFSET szMethodPost
    lea eax, [(HttpRequest PTR [esi]).method]
    push eax
    call StrCompare
    cmp eax, 0
    je HandlePostRoute

    ; 2. Check if method is GET
    push OFFSET szMethodGet
    lea eax, [(HttpRequest PTR [esi]).method]
    push eax
    call StrCompare
    cmp eax, 0
    je HandleGetRoute

    ; 3. Unsupported method -> Return 404
    push DWORD PTR [ebp+8]
    call Send404Response
    xor eax, eax
    jmp RouteDone

HandlePostRoute:
    push esi
    push DWORD PTR [ebp+8]
    call Endpoint_Post
    mov eax, 1
    jmp RouteDone

HandleGetRoute:
    lea eax, [(HttpRequest PTR [esi]).uri]
    push eax
    push DWORD PTR [ebp+8]
    call ServeDiskFile
    jmp RouteDone

RouteDone:
    pop esi
    pop ebx
    pop ebp
    ret 8
RouteRequest ENDP


; ------------------------------------------------------------------------------
; main: Application entry point initializing Winsock and running the server loop.
; ------------------------------------------------------------------------------
main PROC
    ; 1. Display colored banner via Irvine32
    mov eax, COLOR_BANNER
    call SetTextColor

    mov edx, OFFSET msgBanner1
    call WriteString
    mov edx, OFFSET msgBanner2
    call WriteString
    mov edx, OFFSET msgBanner3
    call WriteString

    ; 2. Initialize Winsock 2
    call InitWinsock
    cmp eax, 0
    jne HandleInitFailure

    ; 3. Create listening socket on port 8080
    push SERVER_PORT
    call CreateServerSocket
    cmp eax, INVALID_SOCKET_VAL
    je HandleBindFailure
    mov g_serverSocket, eax

    ; 4. Display server running status in green
    mov eax, COLOR_SUCCESS
    call SetTextColor
    mov edx, OFFSET msgListening
    call WriteString
    mov edx, OFFSET msgPressExit
    call WriteString

    mov eax, COLOR_DEFAULT
    call SetTextColor

; ------------------------------------------------------------------------------
; Main Server Loop (Sequential Accept-Receive-Parse-Respond-Close)
; ------------------------------------------------------------------------------
ServerAcceptLoop:
    ; A. Accept incoming TCP connection
    push g_serverSocket
    call AcceptClient
    cmp eax, INVALID_SOCKET_VAL
    je ServerAcceptLoop                ; Retry if interrupted or transient error

    mov g_clientSocket, eax

    ; Log connection
    mov eax, COLOR_BANNER
    call SetTextColor
    mov edx, OFFSET msgClientAccepted
    call WriteString
    mov eax, COLOR_DEFAULT
    call SetTextColor

    ; B. Clear raw receive buffer
    push RECV_BUFFER_SIZE
    push 0
    push OFFSET g_rawRecvBuffer
    call MemSet

    ; C. Receive HTTP request bytes
    push RECV_BUFFER_SIZE
    push OFFSET g_rawRecvBuffer
    push g_clientSocket
    call RecvRequest

    cmp eax, 0
    jle HandleEmptyClientRequest

    ; D. Parse HTTP request into g_currentRequest struct
    push OFFSET g_currentRequest
    push OFFSET g_rawRecvBuffer
    call ParseHttpRequest

    cmp eax, 0
    je HandleMalformedClientRequest

    ; E. Log Request details in yellow/cyan
    mov eax, COLOR_INFO
    call SetTextColor
    mov edx, OFFSET msgReqMethod
    call WriteString
    mov edx, OFFSET g_currentRequest.method
    call WriteString

    mov edx, OFFSET msgReqPath
    call WriteString
    mov edx, OFFSET g_currentRequest.uri
    call WriteString
    call Crlf
    mov eax, COLOR_DEFAULT
    call SetTextColor

    ; F. Dispatch request to router
    push OFFSET g_currentRequest
    push g_clientSocket
    call RouteRequest

    jmp CloseClientConnection

HandleEmptyClientRequest:
    mov eax, COLOR_ERROR
    call SetTextColor
    mov edx, OFFSET msgEmptyReq
    call WriteString
    mov eax, COLOR_DEFAULT
    call SetTextColor
    jmp CloseClientConnection

HandleMalformedClientRequest:
    mov eax, COLOR_ERROR
    call SetTextColor
    mov edx, OFFSET msgMalformedReq
    call WriteString
    mov eax, COLOR_DEFAULT
    call SetTextColor

    push g_clientSocket
    call Send404Response

CloseClientConnection:
    ; G. Close client socket cleanly
    push g_clientSocket
    call CloseClientSocket

    ; H. Log completion
    mov eax, COLOR_BANNER
    call SetTextColor
    mov edx, OFFSET msgReqDone
    call WriteString
    mov eax, COLOR_DEFAULT
    call SetTextColor

    ; Loop back for next request
    jmp ServerAcceptLoop

; ------------------------------------------------------------------------------
; Error Handlers
; ------------------------------------------------------------------------------
HandleInitFailure:
    mov eax, COLOR_ERROR
    call SetTextColor
    mov edx, OFFSET msgInitFail
    call WriteString
    mov eax, COLOR_DEFAULT
    call SetTextColor
    exit

HandleBindFailure:
    mov eax, COLOR_ERROR
    call SetTextColor
    mov edx, OFFSET msgBindFail
    call WriteString
    mov eax, COLOR_DEFAULT
    call SetTextColor
    call CleanupWinsock
    exit

main ENDP

END main
