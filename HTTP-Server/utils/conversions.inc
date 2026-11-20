; ==============================================================================
; utils/conversions.inc
; Numeric conversion utilities between strings and unsigned 32-bit integers
; ==============================================================================

.code

; ------------------------------------------------------------------------------
; StrToUInt: Parses an unsigned decimal integer from an ASCII string.
; Parameters: [ebp+8]=pStr
; Returns: EAX = 32-bit unsigned integer value
; ------------------------------------------------------------------------------
StrToUInt PROC
    push ebp
    mov ebp, esp
    push esi
    push ebx

    mov esi, [ebp+8]
    xor eax, eax

ParseDigitLoop:
    mov bl, [esi]
    cmp bl, 0
    je StrToUInt_Done

    cmp bl, '0'
    jb StrToUInt_Invalid
    cmp bl, '9'
    ja StrToUInt_Invalid

    sub bl, '0'
    imul eax, 10
    movzx ebx, bl
    add eax, ebx

    inc esi
    jmp ParseDigitLoop

StrToUInt_Invalid:
    xor eax, eax

StrToUInt_Done:
    pop ebx
    pop esi
    pop ebp
    ret 4
StrToUInt ENDP


; ------------------------------------------------------------------------------
; UIntToStr: Converts an unsigned 32-bit integer to a null-terminated ASCII string.
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

    ; Special case for zero
    cmp eax, 0
    jne UIntDivideLoop

    mov BYTE PTR [edi], '0'
    mov BYTE PTR [edi+1], 0
    mov eax, [ebp+12]
    jmp UIntToStr_Done

UIntDivideLoop:
    xor edx, edx
    div ebx

    push edx
    inc ecx

    cmp eax, 0
    jne UIntDivideLoop

UIntWriteDigits:
    pop edx
    add dl, '0'
    mov [edi], dl
    inc edi
    loop UIntWriteDigits

    mov BYTE PTR [edi], 0
    mov eax, [ebp+12]

UIntToStr_Done:
    pop edi
    pop ecx
    pop ebx
    pop ebp
    ret 8
UIntToStr ENDP
