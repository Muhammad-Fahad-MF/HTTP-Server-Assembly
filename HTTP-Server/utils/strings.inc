; ==============================================================================
; utils/strings.inc
; Null-terminated ASCII string operations and delimiter search routines
; ==============================================================================

.code

; ------------------------------------------------------------------------------
; StrLen: Computes the length of a null-terminated string (excluding null byte).
; Parameters: [ebp+8]=pStr
; Returns: EAX = length in bytes
; ------------------------------------------------------------------------------
StrLen PROC
    push ebp
    mov ebp, esp
    push esi

    mov esi, [ebp+8]
    xor eax, eax

StrLenLoop:
    cmp BYTE PTR [esi], 0
    je StrLenDone
    inc eax
    inc esi
    jmp StrLenLoop

StrLenDone:
    pop esi
    pop ebp
    ret 4
StrLen ENDP


; ------------------------------------------------------------------------------
; StrCopy: Copies null-terminated string from pSrc to pDest including null byte.
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

StrCopyLoop:
    mov al, [esi]
    mov [edi], al
    inc esi
    inc edi
    cmp al, 0
    jne StrCopyLoop

    mov eax, [ebp+8]
    pop edi
    pop esi
    pop ebp
    ret 8
StrCopy ENDP


; ------------------------------------------------------------------------------
; StrCompare: Case-sensitive lexical comparison of two null-terminated strings.
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

StrCmpLoop:
    mov al, [esi]
    mov bl, [edi]

    cmp al, bl
    jb StrCmpLess
    ja StrCmpGreater

    cmp al, 0
    je StrCmpEqual

    inc esi
    inc edi
    jmp StrCmpLoop

StrCmpLess:
    mov eax, -1
    jmp StrCmpDone

StrCmpGreater:
    mov eax, 1
    jmp StrCmpDone

StrCmpEqual:
    xor eax, eax

StrCmpDone:
    pop ebx
    pop edi
    pop esi
    pop ebp
    ret 8
StrCompare ENDP


; ------------------------------------------------------------------------------
; StrCaseCompare: Case-insensitive lexical comparison of two strings.
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

CaseCmpLoop:
    mov al, [esi]
    mov bl, [edi]

    ; Normalize AL to lowercase
    cmp al, 'A'
    jb CheckCaseB
    cmp al, 'Z'
    ja CheckCaseB
    add al, 32

CheckCaseB:
    ; Normalize BL to lowercase
    cmp bl, 'A'
    jb DoCaseCmp
    cmp bl, 'Z'
    ja DoCaseCmp
    add bl, 32

DoCaseCmp:
    cmp al, bl
    jb CaseCmpLess
    ja CaseCmpGreater

    cmp al, 0
    je CaseCmpEqual

    inc esi
    inc edi
    jmp CaseCmpLoop

CaseCmpLess:
    mov eax, -1
    jmp CaseCmpDone

CaseCmpGreater:
    mov eax, 1
    jmp CaseCmpDone

CaseCmpEqual:
    xor eax, eax

CaseCmpDone:
    pop ebx
    pop edi
    pop esi
    pop ebp
    ret 8
StrCaseCompare ENDP


; ------------------------------------------------------------------------------
; StrFindChar: Searches for the first occurrence of targetChar in a string.
; Parameters: [ebp+8]=pStr, [ebp+12]=targetChar (byte)
; Returns: EAX = pointer to matching character, or 0 (NULL) if not found
; ------------------------------------------------------------------------------
StrFindChar PROC
    push ebp
    mov ebp, esp
    push esi

    mov esi, [ebp+8]
    mov edx, [ebp+12]

FindCharLoop:
    mov al, [esi]
    cmp al, dl
    je CharFound

    cmp al, 0
    je CharNotFound

    inc esi
    jmp FindCharLoop

CharFound:
    mov eax, esi
    jmp FindCharDone

CharNotFound:
    xor eax, eax

FindCharDone:
    pop esi
    pop ebp
    ret 8
StrFindChar ENDP


; ------------------------------------------------------------------------------
; StrFindSubstring: Searches for the first occurrence of pSub inside pStr.
; Parameters: [ebp+8]=pStr, [ebp+12]=pSub
; Returns: EAX = pointer to start of substring in pStr, or 0 if not found
; ------------------------------------------------------------------------------
StrFindSubstring PROC
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi

    mov esi, [ebp+8]
    mov edi, [ebp+12]

    ; If substring is empty, match immediately
    cmp BYTE PTR [edi], 0
    je SubFound

SubOuter:
    cmp BYTE PTR [esi], 0
    je SubNotFound

    mov ebx, esi
    mov edx, edi

SubInner:
    mov al, [edx]
    cmp al, 0
    je SubFound

    cmp al, [ebx]
    jne SubNext

    inc ebx
    inc edx
    jmp SubInner

SubNext:
    inc esi
    jmp SubOuter

SubFound:
    mov eax, esi
    jmp SubDone

SubNotFound:
    xor eax, eax

SubDone:
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret 8
StrFindSubstring ENDP


; ------------------------------------------------------------------------------
; StrTrim: Strips leading and trailing whitespace (' ', tab, CR, LF) in-place.
; Parameters: [ebp+8]=pStr
; Returns: EAX = pointer to the trimmed string
; ------------------------------------------------------------------------------
StrTrim PROC
    push ebp
    mov ebp, esp
    push esi
    push edi

    mov esi, [ebp+8]
    mov edi, esi

SkipLeadingWS:
    mov al, [esi]
    cmp al, ' '
    je NextLeadingWS
    cmp al, 9
    je NextLeadingWS
    cmp al, 13
    je NextLeadingWS
    cmp al, 10
    je NextLeadingWS
    jmp CopyTrimmed

NextLeadingWS:
    inc esi
    jmp SkipLeadingWS

CopyTrimmed:
    mov al, [esi]
    mov [edi], al
    cmp al, 0
    je RemoveTrailingWS
    inc esi
    inc edi
    jmp CopyTrimmed

RemoveTrailingWS:
    cmp edi, [ebp+8]
    je TrimDone

    dec edi
    mov al, [edi]
    cmp al, ' '
    je CutTrailingWS
    cmp al, 9
    je CutTrailingWS
    cmp al, 13
    je CutTrailingWS
    cmp al, 10
    je CutTrailingWS
    inc edi
    jmp TrimDone

CutTrailingWS:
    mov BYTE PTR [edi], 0
    jmp RemoveTrailingWS

TrimDone:
    mov eax, [ebp+8]
    pop edi
    pop esi
    pop ebp
    ret 4
StrTrim ENDP


; ------------------------------------------------------------------------------
; FindCRLF: Searches for the first CRLF sequence (13, 10) in a string.
; Parameters: [ebp+8]=pStr
; Returns: EAX = pointer to byte 13 of sequence, or 0 if not found
; ------------------------------------------------------------------------------
FindCRLF PROC
    push ebp
    mov ebp, esp
    push esi

    mov esi, [ebp+8]

FindCRLF_Loop:
    cmp BYTE PTR [esi], 0
    je CRLF_NotFound

    cmp BYTE PTR [esi], 13
    jne NextCRLF_Char

    cmp BYTE PTR [esi+1], 10
    je CRLF_Found

NextCRLF_Char:
    inc esi
    jmp FindCRLF_Loop

CRLF_Found:
    mov eax, esi
    jmp CRLF_Done

CRLF_NotFound:
    xor eax, eax

CRLF_Done:
    pop esi
    pop ebp
    ret 4
FindCRLF ENDP


; ------------------------------------------------------------------------------
; FindCRLFCRLF: Searches for the double CRLF sequence separating headers from body.
; Parameters: [ebp+8]=pStr
; Returns: EAX = pointer to start of 4-byte delimiter, or 0 if not found
; ------------------------------------------------------------------------------
FindCRLFCRLF PROC
    push ebp
    mov ebp, esp
    push esi

    mov esi, [ebp+8]

Find2CRLF_Loop:
    cmp BYTE PTR [esi], 0
    je CRLF2_NotFound

    cmp BYTE PTR [esi], 13
    jne Next2CRLF_Char
    cmp BYTE PTR [esi+1], 10
    jne Next2CRLF_Char
    cmp BYTE PTR [esi+2], 13
    jne Next2CRLF_Char
    cmp BYTE PTR [esi+3], 10
    je CRLF2_Found

Next2CRLF_Char:
    inc esi
    jmp Find2CRLF_Loop

CRLF2_Found:
    mov eax, esi
    jmp CRLF2_Done

CRLF2_NotFound:
    xor eax, eax

CRLF2_Done:
    pop esi
    pop ebp
    ret 4
FindCRLFCRLF ENDP
