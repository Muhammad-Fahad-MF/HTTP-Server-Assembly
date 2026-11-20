; ==============================================================================
; utils/memory.inc
; Low-level block memory operations (MemCopy, MemMove, MemSet, MemCompare)
; ==============================================================================

.code

; ------------------------------------------------------------------------------
; MemCopy: Copies count bytes from source to destination buffer (forward copy).
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
; MemMove: Safely copies count bytes handling overlapping source/dest blocks.
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
    je DoneMemMove

    cmp edi, esi
    je DoneMemMove
    jb ForwardMemMove

    lea eax, [esi+ecx]
    cmp edi, eax
    jae ForwardMemMove

    ; Overlap detected, copy backwards
    add esi, ecx
    add edi, ecx
    dec esi
    dec edi

    std
    rep movsb
    cld
    jmp DoneMemMove

ForwardMemMove:
    cld
    rep movsb

DoneMemMove:
    mov eax, [ebp+8]
    cld
    pop ecx
    pop edi
    pop esi
    pop ebp
    ret 12
MemMove ENDP


; ------------------------------------------------------------------------------
; MemSet: Fills a memory buffer with a specified byte value for count bytes.
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
; MemCompare: Compares count bytes between two memory blocks.
; Parameters: [ebp+8]=pBuf1, [ebp+12]=pBuf2, [ebp+16]=count
; Returns: EAX = 0 if equal, -1 if pBuf1 < pBuf2, 1 if pBuf1 > pBuf2
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

CompareMemLoop:
    cmp ecx, 0
    je EqualMem

    mov al, [esi]
    mov bl, [edi]

    cmp al, bl
    jb LessMem
    ja GreaterMem

    inc esi
    inc edi
    dec ecx
    jmp CompareMemLoop

LessMem:
    mov eax, -1
    jmp DoneMemCompare

GreaterMem:
    mov eax, 1
    jmp DoneMemCompare

EqualMem:
    xor eax, eax

DoneMemCompare:
    pop ebx
    pop edi
    pop esi
    pop ebp
    ret 12
MemCompare ENDP
