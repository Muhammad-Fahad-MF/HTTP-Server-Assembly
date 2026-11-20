; ==============================================================================
; utils/string.inc
; Unified collection of string, memory, and conversion procedures
; Cleaned version: test data and test main PROC removed; 2-3 line documentation added.
; ==============================================================================

.code

; ------------------------------------------------------------------------------
; 1. StrLen: Computes the length of a null-terminated string (excluding null byte).
; Parameters: [ebp+8]=pStr
; Returns: EAX = length in bytes
; ------------------------------------------------------------------------------
StrLen PROC
    push ebp
    mov ebp, esp
    push esi

    mov esi, [ebp+8]
    xor eax, eax

L1:
    cmp BYTE PTR [esi], 0
    je Done1
    inc eax
    inc esi
    jmp L1

Done1:
    pop esi
    pop ebp
    ret 4
StrLen ENDP


; ------------------------------------------------------------------------------
; 2. StrCopy: Copies null-terminated string from pSrc to pDest including null byte.
; Parameters: [ebp+8]=pDest, [ebp+12]=pSrc
; Returns: EAX = pointer to pDest
; ------------------------------------------------------------------------------
StrCopy PROC
    push ebp
    mov ebp, esp
    push esi
    push edi

    mov edi, [ebp+8]
    mov esi, [ebp+12]

L2:
    mov al, [esi]
    mov [edi], al
    inc esi
    inc edi
    cmp al, 0
    jne L2

    mov eax, [ebp+8]
    pop edi
    pop esi
    pop ebp
    ret 8
StrCopy ENDP


; ------------------------------------------------------------------------------
; 3. StrCompare: Case-sensitive lexical comparison of two null-terminated strings.
; Parameters: [ebp+8]=pStr1, [ebp+12]=pStr2
; Returns: EAX = 0 if identical, -1 if pStr1 < pStr2, 1 if pStr1 > pStr2
; ------------------------------------------------------------------------------
StrCompare PROC
    push ebp
    mov ebp, esp
    push esi
    push edi
    push ebx

    mov esi, [ebp+8]
    mov edi, [ebp+12]

L3:
    mov al, [esi]
    mov bl, [edi]

    cmp al, bl
    jb Less3
    ja Greater3

    cmp al, 0
    je Equal3

    inc esi
    inc edi
    jmp L3

Less3:
    mov eax, -1
    jmp Done3

Greater3:
    mov eax, 1
    jmp Done3

Equal3:
    xor eax, eax

Done3:
    pop ebx
    pop edi
    pop esi
    pop ebp
    ret 8
StrCompare ENDP


; ------------------------------------------------------------------------------
; 4. MemCopy: Copies count bytes from source to destination buffer.
; Parameters: [ebp+8]=pDest, [ebp+12]=pSrc, [ebp+16]=count
; Returns: EAX = pointer to destination buffer
; ------------------------------------------------------------------------------
MemCopy PROC
    push ebp
    mov ebp, esp
    push esi
    push edi
    push ecx

    mov edi, [ebp+8]
    mov esi, [ebp+12]
    mov ecx, [ebp+16]

    cld
    rep movsb

    mov eax, [ebp+8]
    pop ecx
    pop edi
    pop esi
    pop ebp
    ret 12
MemCopy ENDP


; ------------------------------------------------------------------------------
; 5. MemMove: Safely copies count bytes handling overlapping memory regions.
; Parameters: [ebp+8]=pDest, [ebp+12]=pSrc, [ebp+16]=count
; Returns: EAX = pointer to destination buffer
; ------------------------------------------------------------------------------
MemMove PROC
    push ebp
    mov ebp, esp
    push esi
    push edi
    push ecx

    mov edi, [ebp+8]
    mov esi, [ebp+12]
    mov ecx, [ebp+16]

    cmp ecx, 0
    je Done5

    cmp edi, esi
    je Done5
    jb Forward5

    lea eax, [esi+ecx]
    cmp edi, eax
    jae Forward5

    add esi, ecx
    add edi, ecx
    dec esi
    dec edi

    std
    rep movsb
    cld
    jmp Done5

Forward5:
    cld
    rep movsb

Done5:
    mov eax, [ebp+8]
    cld
    pop ecx
    pop edi
    pop esi
    pop ebp
    ret 12
MemMove ENDP


; ------------------------------------------------------------------------------
; 6. MemSet: Fills count bytes in destination buffer with specified byte value.
; Parameters: [ebp+8]=pDest, [ebp+12]=val (byte), [ebp+16]=count
; Returns: EAX = pointer to destination buffer
; ------------------------------------------------------------------------------
MemSet PROC
    push ebp
    mov ebp, esp
    push edi
    push ecx

    mov edi, [ebp+8]
    mov eax, [ebp+12]
    mov ecx, [ebp+16]

    cld
    rep stosb

    mov eax, [ebp+8]
    pop ecx
    pop edi
    pop ebp
    ret 12
MemSet ENDP


; ------------------------------------------------------------------------------
; 7. MemCompare: Compares count bytes between two memory blocks.
; Parameters: [ebp+8]=pBuf1, [ebp+12]=pBuf2, [ebp+16]=count
; Returns: EAX = 0 if identical, -1 if pBuf1 < pBuf2, 1 if pBuf1 > pBuf2
; ------------------------------------------------------------------------------
MemCompare PROC
    push ebp
    mov ebp, esp
    push esi
    push edi
    push ebx

    mov esi, [ebp+8]
    mov edi, [ebp+12]
    mov ecx, [ebp+16]

L7:
    cmp ecx, 0
    je Equal7

    mov al, [esi]
    mov bl, [edi]

    cmp al, bl
    jb Less7
    ja Greater7

    inc esi
    inc edi
    dec ecx
    jmp L7

Less7:
    mov eax, -1
    jmp Done7

Greater7:
    mov eax, 1
    jmp Done7

Equal7:
    xor eax, eax

Done7:
    pop ebx
    pop edi
    pop esi
    pop ebp
    ret 12
MemCompare ENDP


; ------------------------------------------------------------------------------
; 8. StrFindChar: Locates the first occurrence of targetChar in a string.
; Parameters: [ebp+8]=pStr, [ebp+12]=targetChar (byte)
; Returns: EAX = pointer to matching character, or 0 if not found
; ------------------------------------------------------------------------------
StrFindChar PROC
    push ebp
    mov ebp, esp
    push esi

    mov esi, [ebp+8]
    mov edx, [ebp+12]

L8:
    mov al, [esi]
    cmp al, dl
    je Found8

    cmp al, 0
    je NotFound8

    inc esi
    jmp L8

Found8:
    mov eax, esi
    jmp Done8

NotFound8:
    xor eax, eax

Done8:
    pop esi
    pop ebp
    ret 8
StrFindChar ENDP


; ------------------------------------------------------------------------------
; 9. StrFindSubstring: Locates the first occurrence of substring pSub in pStr.
; Parameters: [ebp+8]=pStr, [ebp+12]=pSub
; Returns: EAX = pointer to beginning of substring in pStr, or 0 if not found
; ------------------------------------------------------------------------------
StrFindSubstring PROC
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi

    mov esi, [ebp+8]
    mov edi, [ebp+12]

    cmp BYTE PTR [edi], 0
    je Found9

Outer9:
    cmp BYTE PTR [esi], 0
    je NotFound9

    mov ebx, esi
    mov edx, edi

Inner9:
    mov al, [edx]
    cmp al, 0
    je Found9

    cmp al, [ebx]
    jne Next9

    inc ebx
    inc edx
    jmp Inner9

Next9:
    inc esi
    jmp Outer9

Found9:
    mov eax, esi
    jmp Done9

NotFound9:
    xor eax, eax

Done9:
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret 8
StrFindSubstring ENDP


; ------------------------------------------------------------------------------
; 10. StrCaseCompare: Case-insensitive lexical comparison of two strings.
; Parameters: [ebp+8]=pStr1, [ebp+12]=pStr2
; Returns: EAX = 0 if identical, -1 if pStr1 < pStr2, 1 if pStr1 > pStr2
; ------------------------------------------------------------------------------
StrCaseCompare PROC
    push ebp
    mov ebp, esp
    push esi
    push edi
    push ebx

    mov esi, [ebp+8]
    mov edi, [ebp+12]

L10:
    mov al, [esi]
    mov bl, [edi]

    cmp al, 'A'
    jb CheckB10
    cmp al, 'Z'
    ja CheckB10
    add al, 32

CheckB10:
    cmp bl, 'A'
    jb Compare10
    cmp bl, 'Z'
    ja Compare10
    add bl, 32

Compare10:
    cmp al, bl
    jb Less10
    ja Greater10

    cmp al, 0
    je Equal10

    inc esi
    inc edi
    jmp L10

Less10:
    mov eax, -1
    jmp Done10

Greater10:
    mov eax, 1
    jmp Done10

Equal10:
    xor eax, eax

Done10:
    pop ebx
    pop edi
    pop esi
    pop ebp
    ret 8
StrCaseCompare ENDP


; ------------------------------------------------------------------------------
; 11. StrTrim: Strips leading and trailing whitespace characters in-place.
; Parameters: [ebp+8]=pStr
; Returns: EAX = pointer to trimmed string
; ------------------------------------------------------------------------------
StrTrim PROC
    push ebp
    mov ebp, esp
    push esi
    push edi

    mov esi, [ebp+8]
    mov edi, esi

SkipStart11:
    mov al, [esi]
    cmp al, ' '
    je Advance11
    cmp al, 9
    je Advance11
    cmp al, 13
    je Advance11
    cmp al, 10
    je Advance11
    jmp Copy11

Advance11:
    inc esi
    jmp SkipStart11

Copy11:
    mov al, [esi]
    mov [edi], al
    cmp al, 0
    je RemoveEnd11
    inc esi
    inc edi
    jmp Copy11

RemoveEnd11:
    cmp edi, [ebp+8]
    je Finish11

    dec edi
    mov al, [edi]
    cmp al, ' '
    je Remove11
    cmp al, 9
    je Remove11
    cmp al, 13
    je Remove11
    cmp al, 10
    je Remove11
    inc edi
    jmp Finish11

Remove11:
    mov BYTE PTR [edi], 0
    jmp RemoveEnd11

Finish11:
    mov eax, [ebp+8]
    pop edi
    pop esi
    pop ebp
    ret 4
StrTrim ENDP


; ------------------------------------------------------------------------------
; 12. FindCRLF: Searches for the first occurrence of CRLF (13, 10).
; Parameters: [ebp+8]=pStr
; Returns: EAX = pointer to byte 13 of sequence, or 0 if not found
; ------------------------------------------------------------------------------
FindCRLF PROC
    push ebp
    mov ebp, esp
    push esi

    mov esi, [ebp+8]

L12:
    cmp BYTE PTR [esi], 0
    je NotFound12
    cmp BYTE PTR [esi], 13
    jne Next12
    cmp BYTE PTR [esi+1], 10
    je Found12

Next12:
    inc esi
    jmp L12

Found12:
    mov eax, esi
    jmp Done12

NotFound12:
    xor eax, eax

Done12:
    pop esi
    pop ebp
    ret 4
FindCRLF ENDP


; ------------------------------------------------------------------------------
; 13. FindCRLFCRLF: Searches for double CRLF (13, 10, 13, 10) separating headers.
; Parameters: [ebp+8]=pStr
; Returns: EAX = pointer to first byte of 4-byte sequence, or 0 if not found
; ------------------------------------------------------------------------------
FindCRLFCRLF PROC
    push ebp
    mov ebp, esp
    push esi

    mov esi, [ebp+8]

L13:
    cmp BYTE PTR [esi], 0
    je NotFound13
    cmp BYTE PTR [esi], 13
    jne Next13
    cmp BYTE PTR [esi+1], 10
    jne Next13
    cmp BYTE PTR [esi+2], 13
    jne Next13
    cmp BYTE PTR [esi+3], 10
    je Found13

Next13:
    inc esi
    jmp L13

Found13:
    mov eax, esi
    jmp Done13

NotFound13:
    xor eax, eax

Done13:
    pop esi
    pop ebp
    ret 4
FindCRLFCRLF ENDP


; ------------------------------------------------------------------------------
; 14. ParseRequestLine: Extracts Method and URI from an HTTP request line.
; Parameters: [ebp+8]=pReqLine
; Returns: EAX = pointer to Method, EDX = pointer to URI
; ------------------------------------------------------------------------------
ParseRequestLine PROC
    push ebp
    mov ebp, esp
    push esi
    push edi

    mov esi, [ebp+8]

L14:
    cmp BYTE PTR [esi], ' '
    je FirstSpace14
    cmp BYTE PTR [esi], 0
    je Invalid14
    inc esi
    jmp L14

FirstSpace14:
    mov BYTE PTR [esi], 0
    lea edi, [esi+1]
    mov esi, edi

L14_2:
    cmp BYTE PTR [esi], ' '
    je SecondSpace14
    cmp BYTE PTR [esi], 0
    je Invalid14
    inc esi
    jmp L14_2

SecondSpace14:
    mov BYTE PTR [esi], 0
    mov eax, [ebp+8]
    mov edx, edi
    jmp Done14

Invalid14:
    xor eax, eax
    xor edx, edx

Done14:
    pop edi
    pop esi
    pop ebp
    ret 4
ParseRequestLine ENDP


; ------------------------------------------------------------------------------
; 15. FindHeader: Searches for a specific HTTP header line by header name.
; Parameters: [ebp+8]=pHeaders, [ebp+12]=pHeaderName
; Returns: EAX = pointer to start of header line, or 0 if not found
; ------------------------------------------------------------------------------
FindHeader PROC
    push ebp
    mov ebp, esp
    push esi
    push edi
    push ebx

    mov esi, [ebp+8]
    mov edi, [ebp+12]

NextLine15:
    cmp BYTE PTR [esi], 0
    je NotFound15
    cmp BYTE PTR [esi], 13
    je NotFound15

    mov ebx, esi
    mov edx, edi

Compare15:
    mov al, [edx]
    cmp al, 0
    je CheckColon15
    cmp al, [ebx]
    jne SkipLine15
    inc ebx
    inc edx
    jmp Compare15

CheckColon15:
    cmp BYTE PTR [ebx], ':'
    je Found15

SkipLine15:
    cmp BYTE PTR [esi], 0
    je NotFound15
    cmp BYTE PTR [esi], 13
    je NextLineStart15
    inc esi
    jmp SkipLine15

NextLineStart15:
    cmp BYTE PTR [esi+1], 10
    jne NotFound15
    add esi, 2
    jmp NextLine15

Found15:
    mov eax, esi
    jmp Done15

NotFound15:
    xor eax, eax

Done15:
    pop ebx
    pop edi
    pop esi
    pop ebp
    ret 8
FindHeader ENDP


; ------------------------------------------------------------------------------
; 16. GetHeaderValue: Extracts pointer to value of a specified HTTP header.
; Parameters: [ebp+8]=pHeaders, [ebp+12]=pHeaderName
; Returns: EAX = pointer to header value string (skipping colon and whitespace)
; ------------------------------------------------------------------------------
GetHeaderValue PROC
    push ebp
    mov ebp, esp
    push esi

    push DWORD PTR [ebp+12]
    push DWORD PTR [ebp+8]
    call FindHeader

    cmp eax, 0
    je NotFound16

    mov esi, eax

L16:
    cmp BYTE PTR [esi], ':'
    je SkipSpaces16
    cmp BYTE PTR [esi], 0
    je NotFound16
    inc esi
    jmp L16

SkipSpaces16:
    inc esi

L16_2:
    cmp BYTE PTR [esi], ' '
    je Advance16
    cmp BYTE PTR [esi], 9
    je Advance16
    jmp Found16

Advance16:
    inc esi
    jmp L16_2

Found16:
    mov eax, esi
    jmp Done16

NotFound16:
    xor eax, eax

Done16:
    pop esi
    pop ebp
    ret 8
GetHeaderValue ENDP


; ------------------------------------------------------------------------------
; 17. ParseContentLength: Parses numeric value of Content-Length header.
; Parameters: [ebp+8]=pHeaders
; Returns: EAX = integer content length, or 0 if not present
; ------------------------------------------------------------------------------
.data
contentName_str BYTE "Content-Length", 0
.code

ParseContentLength PROC
    push ebp
    mov ebp, esp
    push esi
    push ebx

    push OFFSET contentName_str
    push DWORD PTR [ebp+8]
    call GetHeaderValue

    cmp eax, 0
    je NotFound17

    mov esi, eax
    xor eax, eax

L17:
    mov bl, [esi]
    cmp bl, '0'
    jb Done17
    cmp bl, '9'
    ja Done17

    sub bl, '0'
    imul eax, 10
    movzx ebx, bl
    add eax, ebx

    inc esi
    jmp L17

NotFound17:
    xor eax, eax

Done17:
    pop ebx
    pop esi
    pop ebp
    ret 4
ParseContentLength ENDP


; ------------------------------------------------------------------------------
; 18. StrToUInt: Parses an unsigned 32-bit integer from an ASCII string.
; Parameters: [ebp+8]=pStr
; Returns: EAX = parsed unsigned integer
; ------------------------------------------------------------------------------
StrToUInt PROC
    push ebp
    mov ebp, esp
    push esi
    push ebx

    mov esi, [ebp+8]
    xor eax, eax

L18:
    mov bl, [esi]
    cmp bl, 0
    je Done18
    cmp bl, '0'
    jb Invalid18
    cmp bl, '9'
    ja Invalid18

    sub bl, '0'
    imul eax, 10
    movzx ebx, bl
    add eax, ebx

    inc esi
    jmp L18

Invalid18:
    xor eax, eax

Done18:
    pop ebx
    pop esi
    pop ebp
    ret 4
StrToUInt ENDP


; ------------------------------------------------------------------------------
; 19. UIntToStr: Converts an unsigned integer to a null-terminated ASCII string.
; Parameters: [ebp+8]=val, [ebp+12]=pDest
; Returns: EAX = pointer to destination buffer
; ------------------------------------------------------------------------------
UIntToStr PROC
    push ebp
    mov ebp, esp
    push ebx
    push ecx
    push edi

    mov eax, [ebp+8]
    mov edi, [ebp+12]

    mov ebx, 10
    xor ecx, ecx

    cmp eax, 0
    jne Divide19

    mov BYTE PTR [edi], '0'
    mov BYTE PTR [edi+1], 0
    mov eax, [ebp+12]
    jmp Done19

Divide19:
    xor edx, edx
    div ebx
    push edx
    inc ecx
    cmp eax, 0
    jne Divide19

Write19:
    pop edx
    add dl, '0'
    mov [edi], dl
    inc edi
    loop Write19

    mov BYTE PTR [edi], 0
    mov eax, [ebp+12]

Done19:
    pop edi
    pop ecx
    pop ebx
    pop ebp
    ret 8
UIntToStr ENDP
