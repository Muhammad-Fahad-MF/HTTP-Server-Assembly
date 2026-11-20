; ==============================================================================
; http/headers.inc
; Scanning and extraction of HTTP request headers
; ==============================================================================

.data
    hdr_ContentLength BYTE "Content-Length", 0

.code

; ------------------------------------------------------------------------------
; FindHeader: Searches an HTTP header block for a specific header by name.
; Parameters: [ebp+8]=pHeaders, [ebp+12]=pHeaderName
; Returns: EAX = pointer to beginning of matching header line, or 0 if absent
; ------------------------------------------------------------------------------
FindHeader PROC
    push ebp
    mov ebp, esp
    push esi
    push edi
    push ebx

    mov esi, [ebp+8]
    mov edi, [ebp+12]

ScanNextHeaderLine:
    cmp BYTE PTR [esi], 0
    je HeaderNotFound
    cmp BYTE PTR [esi], 13             ; Reached end of headers (\r\n)
    je HeaderNotFound

    mov ebx, esi
    mov edx, edi

CompareHeaderName:
    mov al, [edx]
    cmp al, 0
    je CheckHeaderColon

    cmp al, [ebx]
    jne SkipToNextHeaderLine

    inc ebx
    inc edx
    jmp CompareHeaderName

CheckHeaderColon:
    cmp BYTE PTR [ebx], ':'
    je HeaderFound

SkipToNextHeaderLine:
    cmp BYTE PTR [esi], 0
    je HeaderNotFound
    cmp BYTE PTR [esi], 13
    je AdvanceOverCRLF
    inc esi
    jmp SkipToNextHeaderLine

AdvanceOverCRLF:
    cmp BYTE PTR [esi+1], 10
    jne HeaderNotFound
    add esi, 2
    jmp ScanNextHeaderLine

HeaderFound:
    mov eax, esi
    jmp FindHeaderDone

HeaderNotFound:
    xor eax, eax

FindHeaderDone:
    pop ebx
    pop edi
    pop esi
    pop ebp
    ret 8
FindHeader ENDP


; ------------------------------------------------------------------------------
; GetHeaderValue: Extracts pointer to value of header, past colon and whitespace.
; Parameters: [ebp+8]=pHeaders, [ebp+12]=pHeaderName
; Returns: EAX = pointer to value string within buffer, or 0 if header not found
; ------------------------------------------------------------------------------
GetHeaderValue PROC
    push ebp
    mov ebp, esp
    push esi

    push DWORD PTR [ebp+12]
    push DWORD PTR [ebp+8]
    call FindHeader

    cmp eax, 0
    je GetHeaderValue_NotFound

    mov esi, eax

ScanColon:
    cmp BYTE PTR [esi], ':'
    je FoundColon
    cmp BYTE PTR [esi], 0
    je GetHeaderValue_NotFound
    inc esi
    jmp ScanColon

FoundColon:
    inc esi

SkipHeaderSpaces:
    cmp BYTE PTR [esi], ' '
    je NextHeaderSpace
    cmp BYTE PTR [esi], 9              ; Tab
    je NextHeaderSpace
    jmp GetHeaderValue_Found

NextHeaderSpace:
    inc esi
    jmp SkipHeaderSpaces

GetHeaderValue_Found:
    mov eax, esi
    jmp GetHeaderValue_Done

GetHeaderValue_NotFound:
    xor eax, eax

GetHeaderValue_Done:
    pop esi
    pop ebp
    ret 8
GetHeaderValue ENDP


; ------------------------------------------------------------------------------
; ParseContentLength: Extracts numeric value of Content-Length header.
; Parameters: [ebp+8]=pHeaders
; Returns: EAX = 32-bit integer length, or 0 if missing/invalid
; ------------------------------------------------------------------------------
ParseContentLength PROC
    push ebp
    mov ebp, esp
    push esi
    push ebx

    push OFFSET hdr_ContentLength
    push DWORD PTR [ebp+8]
    call GetHeaderValue

    cmp eax, 0
    je ContentLength_NotFound

    mov esi, eax
    xor eax, eax

ParseCL_Loop:
    mov bl, [esi]
    cmp bl, '0'
    jb ContentLength_Done
    cmp bl, '9'
    ja ContentLength_Done

    sub bl, '0'
    imul eax, 10
    movzx ebx, bl
    add eax, ebx

    inc esi
    jmp ParseCL_Loop

ContentLength_NotFound:
    xor eax, eax

ContentLength_Done:
    pop ebx
    pop esi
    pop ebp
    ret 4
ParseContentLength ENDP
