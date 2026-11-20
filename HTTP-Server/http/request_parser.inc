; ==============================================================================
; http/request_parser.inc
; Parsing of incoming raw HTTP request line, headers, and body into HttpRequest
; ==============================================================================

.data
    szHttp10 BYTE "HTTP/1.0", 0

.code

; ------------------------------------------------------------------------------
; ParseRequestLine: Tokenizes the first line of an HTTP request into Method and URI.
; Parameters: [ebp+8]=pReqLine
; Returns: EAX = pointer to Method string, EDX = pointer to URI string
; ------------------------------------------------------------------------------
ParseRequestLine PROC
    push ebp
    mov ebp, esp
    push esi
    push edi

    mov esi, [ebp+8]

FindMethodEnd:
    cmp BYTE PTR [esi], ' '
    je SplitMethod
    cmp BYTE PTR [esi], 0
    je ParseReqLine_Invalid
    inc esi
    jmp FindMethodEnd

SplitMethod:
    mov BYTE PTR [esi], 0              ; Null-terminate method
    lea edi, [esi+1]                   ; Start of URI
    mov esi, edi

FindUriEnd:
    cmp BYTE PTR [esi], ' '
    je SplitUri
    cmp BYTE PTR [esi], 0
    je ParseReqLine_Invalid
    inc esi
    jmp FindUriEnd

SplitUri:
    mov BYTE PTR [esi], 0              ; Null-terminate URI
    mov eax, [ebp+8]                   ; Return Method in EAX
    mov edx, edi                       ; Return URI in EDX
    jmp ParseReqLine_Done

ParseReqLine_Invalid:
    xor eax, eax
    xor edx, edx

ParseReqLine_Done:
    pop edi
    pop esi
    pop ebp
    ret 4
ParseRequestLine ENDP


; ------------------------------------------------------------------------------
; ParseHttpRequest: Master parser populating an HttpRequest struct from raw bytes.
; Parameters: [ebp+8]=pRawBuffer, [ebp+12]=pRequestStruct
; Returns: EAX = 1 on success, 0 on malformed request
; ------------------------------------------------------------------------------
ParseHttpRequest PROC
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi

    mov esi, [ebp+8]                   ; ESI = raw request buffer
    mov edi, [ebp+12]                  ; EDI = HttpRequest struct pointer

    ; 1. Zero out entire HttpRequest struct
    push SIZEOF HttpRequest
    push 0
    push edi
    call MemSet

    ; 2. Locate double CRLF (delimiter between headers and body)
    push esi
    call FindCRLFCRLF
    mov ebx, eax                       ; EBX = start of \r\n\r\n (or 0)

    ; 3. Parse request line (Method and URI)
    push esi
    call ParseRequestLine
    cmp eax, 0
    je ParseHttp_Fail
    cmp edx, 0
    je ParseHttp_Fail

    ; Copy Method into request struct
    push eax
    lea ecx, [(HttpRequest PTR [edi]).method]
    push ecx
    call StrCopy

    ; Copy URI into request struct
    push edx
    lea ecx, [(HttpRequest PTR [edi]).uri]
    push ecx
    call StrCopy

    ; Set default version
    push OFFSET szHttp10
    lea ecx, [(HttpRequest PTR [edi]).version]
    push ecx
    call StrCopy

    ; 4. Check for Content-Length (needed for POST requests)
    cmp ebx, 0
    je NoBodyPresent

    ; Parse Content-Length header from the header section
    push esi
    call ParseContentLength
    mov [(HttpRequest PTR [edi]).contentLength], eax

    ; 5. Extract body payload if body exists
    cmp eax, 0
    jle NoBodyPresent

    ; Body starts after \r\n\r\n (EBX + 4)
    lea edx, [ebx+4]

    ; Determine copy length: min(contentLength, MAX_BODY_LEN - 1)
    mov ecx, eax
    cmp ecx, (MAX_BODY_LEN - 1)
    jbe DoCopyBody
    mov ecx, (MAX_BODY_LEN - 1)

DoCopyBody:
    mov [(HttpRequest PTR [edi]).bodyLen], ecx

    push ecx                           ; count
    push edx                           ; pSrc (start of body)
    lea eax, [(HttpRequest PTR [edi]).body]
    push eax                           ; pDest
    call MemCopy

    ; Ensure body is null-terminated
    lea eax, [(HttpRequest PTR [edi]).body]
    mov ecx, [(HttpRequest PTR [edi]).bodyLen]
    mov BYTE PTR [eax+ecx], 0

NoBodyPresent:
    mov eax, 1                         ; Success
    jmp ParseHttp_Done

ParseHttp_Fail:
    xor eax, eax                       ; Failure

ParseHttp_Done:
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret 8
ParseHttpRequest ENDP
