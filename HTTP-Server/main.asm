INCLUDE Irvine32.inc

.data
    prompt BYTE "Irvine32 setup successful on low-spec laptop!", 0

.code
main PROC
    mov edx, OFFSET prompt
    call WriteString
    call CrLf
    exit
main ENDP
END main