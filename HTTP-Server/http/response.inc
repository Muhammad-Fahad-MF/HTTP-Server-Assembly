; ==============================================================================
; http/response.inc
; Construction and transmission of HTTP responses and static file serving
; ==============================================================================

.data
    szHttp10_Prefix     BYTE "HTTP/1.0 ", 0
    szHdr_ContentType   BYTE 13, 10, "Content-Type: ", 0
    szHdr_ContentLen    BYTE 13, 10, "Content-Length: ", 0
    szHdr_Connection    BYTE 13, 10, "Connection: close", 13, 10, 13, 10, 0

    szStatus200         BYTE "200 OK", 0
    szStatus400         BYTE "400 Bad Request", 0
    szStatus404         BYTE "404 Not Found", 0
    szStatus405         BYTE "405 Method Not Allowed", 0

    szTypeHtml          BYTE "text/html", 0
    szTypeCss           BYTE "text/css", 0
    szTypeJs            BYTE "application/javascript", 0
    szTypeJson          BYTE "application/json", 0
    szTypePlain         BYTE "text/plain", 0

    szExtHtml           BYTE ".html", 0
    szExtCss            BYTE ".css", 0
    szExtJs             BYTE ".js", 0
    szExtJson           BYTE ".json", 0

    szPublicPrefix      BYTE "public", 0
    szDefaultIndex      BYTE "public/index.html", 0
    szHttpServerPrefix  BYTE "HTTP-Server/", 0
    szDotDotPrefix      BYTE "../", 0
    szAboutUri          BYTE "/about.html", 0

    szFallbackIndex     BYTE "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>COAL x86 HTTP Server</title>"
                        BYTE "<style>body{font-family:Segoe UI,sans-serif;background:#1e1e2f;color:#fff;padding:40px;text-align:center;}"
                        BYTE ".box{max-width:600px;margin:0 auto;background:#2a2a40;padding:30px;border-radius:12px;}"
                        BYTE "h1{color:#4ecdc4;}a{color:#4ecdc4;font-weight:bold;margin:0 10px;text-decoration:none;}</style></head>"
                        BYTE "<body><div class='box'><h1>COAL HTTP 1.0 Server</h1>"
                        BYTE "<p>32-bit x86 Assembly Server running on Windows with Irvine32 & Winsock 2!</p>"
                        BYTE "<hr style='border-color:#444;margin:20px 0;'>"
                        BYTE "<a href='/'>Home</a> | <a href='/about.html'>About Project</a>"
                        BYTE "</div></body></html>", 0

    szFallbackAbout     BYTE "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>About - COAL HTTP Server</title>"
                        BYTE "<style>body{font-family:Segoe UI,sans-serif;background:#18191a;color:#eee;padding:40px;text-align:center;}"
                        BYTE ".box{max-width:600px;margin:0 auto;background:#242526;padding:30px;border-radius:12px;border:1px solid #333;}"
                        BYTE "h1{color:#58a6ff;}ul{text-align:left;line-height:1.8;}a{color:#58a6ff;text-decoration:none;}</style></head>"
                        BYTE "<body><div class='box'><h1>About the Project</h1>"
                        BYTE "<p>Modular HTTP/1.0 Web Server in MASM x86 Assembly.</p>"
                        BYTE "<ul><li><strong>Language:</strong> x86 Assembly (MASM)</li>"
                        BYTE "<li><strong>Networking:</strong> Winsock 2 (ws2_32.lib)</li>"
                        BYTE "<li><strong>Terminal:</strong> Irvine32 Library</li>"
                        BYTE "<li><strong>Supported Methods:</strong> GET and POST</li></ul>"
                        BYTE "<p><a href='/'>&larr; Back to Home</a></p></div></body></html>", 0

    szBody404           BYTE "<!DOCTYPE html><html><head><title>404 Not Found</title></head>", 13, 10
                        BYTE "<body style='font-family:sans-serif; text-align:center; padding-top:50px;'>", 13, 10
                        BYTE "<h1 style='color:#e74c3c;'>404 - File Not Found</h1>", 13, 10
                        BYTE "<p>The requested file does not exist on this COAL x86 HTTP server.</p>", 13, 10
                        BYTE "<hr/><small>MASM HTTP Server &copy; COAL Project</small></body></html>", 0

    szBody400           BYTE "<!DOCTYPE html><html><head><title>400 Bad Request</title></head>", 13, 10
                        BYTE "<body style='font-family:sans-serif; text-align:center; padding-top:50px;'>", 13, 10
                        BYTE "<h1 style='color:#e67e22;'>400 - Bad Request</h1>", 13, 10
                        BYTE "<p>The request could not be parsed by this COAL HTTP server.</p>", 13, 10
                        BYTE "<hr/><small>MASM HTTP Server &copy; COAL Project</small></body></html>", 0

    szBody405           BYTE "<!DOCTYPE html><html><head><title>405 Method Not Allowed</title></head>", 13, 10
                        BYTE "<body style='font-family:sans-serif; text-align:center; padding-top:50px;'>", 13, 10
                        BYTE "<h1 style='color:#e67e22;'>405 - Method Not Allowed</h1>", 13, 10
                        BYTE "<p>The requested HTTP method is not supported by this server.</p>", 13, 10
                        BYTE "<hr/><small>MASM HTTP Server &copy; COAL Project</small></body></html>", 0

    g_headerBuffer      BYTE 1024 DUP(0)
    g_lenStrBuffer      BYTE 32 DUP(0)
    g_diskPath          BYTE MAX_PATH_LEN DUP(0)
    g_altPath           BYTE MAX_PATH_LEN DUP(0)
    g_fileBuffer        BYTE FILE_BUFFER_SIZE DUP(0)

.code

; ------------------------------------------------------------------------------
; SendHttpResponse: Formats HTTP 1.0 headers and sends both header and payload.
; Parameters: [ebp+8]=clientSocket, [ebp+12]=pStatusCode, [ebp+16]=pContentType,
;             [ebp+20]=pBody, [ebp+24]=bodyLen
; Returns: EAX = total bytes sent
; ------------------------------------------------------------------------------
SendHttpResponse PROC
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi

    ; 1. Clear header buffer
    push 1024
    push 0
    push OFFSET g_headerBuffer
    call MemSet

    ; 2. Build header: "HTTP/1.0 " + StatusCode + "\r\nContent-Type: " + ContentType
    push OFFSET szHttp10_Prefix
    push OFFSET g_headerBuffer
    call StrCopy

    ; Append status code
    push [ebp+12]
    push OFFSET g_headerBuffer
    call AppendString

    ; Append Content-Type header
    push OFFSET szHdr_ContentType
    push OFFSET g_headerBuffer
    call AppendString

    push [ebp+16]
    push OFFSET g_headerBuffer
    call AppendString

    ; Append Content-Length header
    push OFFSET szHdr_ContentLen
    push OFFSET g_headerBuffer
    call AppendString

    ; Convert bodyLen to string
    push OFFSET g_lenStrBuffer
    push DWORD PTR [ebp+24]
    call UIntToStr

    push OFFSET g_lenStrBuffer
    push OFFSET g_headerBuffer
    call AppendString

    ; Append Connection: close and terminal double CRLF
    push OFFSET szHdr_Connection
    push OFFSET g_headerBuffer
    call AppendString

    ; 3. Measure header length
    push OFFSET g_headerBuffer
    call StrLen
    mov ebx, eax                       ; EBX = header byte length

    ; 4. Send HTTP headers
    push ebx
    push OFFSET g_headerBuffer
    push DWORD PTR [ebp+8]
    call SendResponseData

    ; 5. Send body payload if present
    mov ecx, [ebp+24]
    cmp ecx, 0
    jle SendHttp_Done

    cmp DWORD PTR [ebp+20], 0
    je SendHttp_Done

    push ecx                           ; bodyLen
    push DWORD PTR [ebp+20]            ; pBody
    push DWORD PTR [ebp+8]             ; clientSocket
    call SendResponseData

SendHttp_Done:
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret 20
SendHttpResponse ENDP


; ------------------------------------------------------------------------------
; AppendString: Helper procedure appending source string to null-terminated dest.
; Parameters: [ebp+8]=pDest, [ebp+12]=pSrc
; Returns: EAX = pointer to pDest
; ------------------------------------------------------------------------------
AppendString PROC
    push ebp
    mov ebp, esp
    push esi
    push edi

    mov edi, [ebp+8]
    mov esi, [ebp+12]

FindDestEnd:
    cmp BYTE PTR [edi], 0
    je CopySrcChars
    inc edi
    jmp FindDestEnd

CopySrcChars:
    mov al, [esi]
    mov [edi], al
    cmp al, 0
    je AppendDone
    inc esi
    inc edi
    jmp CopySrcChars

AppendDone:
    mov eax, [ebp+8]
    pop edi
    pop esi
    pop ebp
    ret 8
AppendString ENDP


; ------------------------------------------------------------------------------
; Send404Response: Sends a standard HTTP 404 Not Found error page.
; Parameters: [ebp+8]=clientSocket
; Returns: EAX = total bytes sent
; ------------------------------------------------------------------------------
Send404Response PROC
    push ebp
    mov ebp, esp

    push OFFSET szBody404
    call StrLen

    push eax                           ; bodyLen
    push OFFSET szBody404              ; pBody
    push OFFSET szTypeHtml             ; Content-Type
    push OFFSET szStatus404            ; Status 404 Not Found
    push DWORD PTR [ebp+8]             ; clientSocket
    call SendHttpResponse

    pop ebp
    ret 4
Send404Response ENDP


; ------------------------------------------------------------------------------
; Send400Response: Sends an HTTP 400 Bad Request error page for malformed input.
; Parameters: [ebp+8]=clientSocket
; Returns: EAX = total bytes sent
; ------------------------------------------------------------------------------
Send400Response PROC
    push ebp
    mov ebp, esp

    push OFFSET szBody400
    call StrLen

    push eax                           ; bodyLen
    push OFFSET szBody400              ; pBody
    push OFFSET szTypeHtml             ; Content-Type
    push OFFSET szStatus400            ; Status 400 Bad Request
    push DWORD PTR [ebp+8]             ; clientSocket
    call SendHttpResponse

    pop ebp
    ret 4
Send400Response ENDP


; ------------------------------------------------------------------------------
; Send405Response: Sends an HTTP 405 Method Not Allowed error page.
; Parameters: [ebp+8]=clientSocket
; Returns: EAX = total bytes sent
; ------------------------------------------------------------------------------
Send405Response PROC
    push ebp
    mov ebp, esp

    push OFFSET szBody405
    call StrLen

    push eax                           ; bodyLen
    push OFFSET szBody405              ; pBody
    push OFFSET szTypeHtml             ; Content-Type
    push OFFSET szStatus405            ; Status 405 Method Not Allowed
    push DWORD PTR [ebp+8]             ; clientSocket
    call SendHttpResponse

    pop ebp
    ret 4
Send405Response ENDP


; ------------------------------------------------------------------------------
; ServeDiskFile: Reads a file from public/ and transmits it with HTTP 200/404.
; Parameters: [ebp+8]=clientSocket, [ebp+12]=pUri
; Returns: EAX = 1 if file served, 0 if 404 returned
; ------------------------------------------------------------------------------
ServeDiskFile PROC
    push ebp
    mov ebp, esp
    push ebx
    push esi
    push edi

    mov esi, [ebp+12]                  ; ESI = URI string

    ; 1. If URI is "/" or empty, serve default "public/index.html"
    cmp BYTE PTR [esi], 0
    je ServeDefaultIndex
    cmp BYTE PTR [esi], '/'
    jne BuildPublicPath
    cmp BYTE PTR [esi+1], 0
    je ServeDefaultIndex

BuildPublicPath:
    ; Construct disk path: "public" + uri (e.g. "public/about.html")
    push OFFSET szPublicPrefix
    push OFFSET g_diskPath
    call StrCopy

    ; If uri doesn't start with '/', add '/'
    cmp BYTE PTR [esi], '/'
    je AppendUriDirectly

    push OFFSET g_diskPath
    call StrLen
    mov edi, OFFSET g_diskPath
    add edi, eax
    mov BYTE PTR [edi], '/'
    mov BYTE PTR [edi+1], 0

AppendUriDirectly:
    push esi
    push OFFSET g_diskPath
    call AppendString
    jmp OpenDiskFile

ServeDefaultIndex:
    push OFFSET szDefaultIndex
    push OFFSET g_diskPath
    call StrCopy

OpenDiskFile:
    ; 2. Try primary path via Irvine32 OpenInputFile
    mov edx, OFFSET g_diskPath
    call OpenInputFile
    cmp eax, -1
    jne FileOpenedOk

    ; Try alternative path 1: "HTTP-Server/" + g_diskPath
    push OFFSET g_diskPath
    push OFFSET g_altPath
    call StrCopy

    push OFFSET szHttpServerPrefix
    push OFFSET g_diskPath
    call StrCopy

    push OFFSET g_altPath
    push OFFSET g_diskPath
    call AppendString

    mov edx, OFFSET g_diskPath
    call OpenInputFile
    cmp eax, -1
    jne FileOpenedOk

    ; Try alternative path 2: "../" + g_altPath
    push OFFSET szDotDotPrefix
    push OFFSET g_diskPath
    call StrCopy

    push OFFSET g_altPath
    push OFFSET g_diskPath
    call AppendString

    mov edx, OFFSET g_diskPath
    call OpenInputFile
    cmp eax, -1
    jne FileOpenedOk

    ; If file could not be found on disk, check built-in in-memory fallback
    cmp BYTE PTR [esi], 0
    je ServeMemoryIndex
    cmp BYTE PTR [esi], '/'
    jne CheckAboutMemory
    cmp BYTE PTR [esi+1], 0
    je ServeMemoryIndex

CheckAboutMemory:
    push OFFSET szAboutUri
    push esi
    call StrCompare
    cmp eax, 0
    je ServeMemoryAbout

    jmp FileNotFound

FileOpenedOk:
    mov ebx, eax                       ; EBX = file handle

    ; 3. Read file contents into g_fileBuffer
    mov eax, ebx
    mov edx, OFFSET g_fileBuffer
    mov ecx, (FILE_BUFFER_SIZE - 1)
    call ReadFromFile

    mov esi, eax                       ; ESI = actual bytes read

    ; 4. Close file handle cleanly
    mov eax, ebx
    call CloseFile

    ; 5. Determine Content-Type by file extension
    mov edi, OFFSET szTypeHtml         ; default to HTML

    push OFFSET szExtCss
    push OFFSET g_diskPath
    call StrFindSubstring
    cmp eax, 0
    je CheckJsExt
    mov edi, OFFSET szTypeCss
    jmp TransmitFileResponse

CheckJsExt:
    push OFFSET szExtJs
    push OFFSET g_diskPath
    call StrFindSubstring
    cmp eax, 0
    je CheckJsonExt
    mov edi, OFFSET szTypeJs
    jmp TransmitFileResponse

CheckJsonExt:
    push OFFSET szExtJson
    push OFFSET g_diskPath
    call StrFindSubstring
    cmp eax, 0
    je TransmitFileResponse
    mov edi, OFFSET szTypeJson

TransmitFileResponse:
    ; 6. Send HTTP 200 OK with file contents
    push esi                           ; bodyLen
    push OFFSET g_fileBuffer           ; pBody
    push edi                           ; pContentType
    push OFFSET szStatus200            ; "200 OK"
    push DWORD PTR [ebp+8]             ; clientSocket
    call SendHttpResponse

    mov eax, 1                         ; Success
    jmp ServeFile_Done

ServeMemoryIndex:
    push OFFSET szFallbackIndex
    call StrLen

    push eax                           ; bodyLen
    push OFFSET szFallbackIndex        ; pBody
    push OFFSET szTypeHtml             ; Content-Type: text/html
    push OFFSET szStatus200            ; "200 OK"
    push DWORD PTR [ebp+8]             ; clientSocket
    call SendHttpResponse

    mov eax, 1
    jmp ServeFile_Done

ServeMemoryAbout:
    push OFFSET szFallbackAbout
    call StrLen

    push eax                           ; bodyLen
    push OFFSET szFallbackAbout        ; pBody
    push OFFSET szTypeHtml             ; Content-Type: text/html
    push OFFSET szStatus200            ; "200 OK"
    push DWORD PTR [ebp+8]             ; clientSocket
    call SendHttpResponse

    mov eax, 1
    jmp ServeFile_Done

FileNotFound:
    push DWORD PTR [ebp+8]
    call Send404Response
    xor eax, eax                       ; File not found

ServeFile_Done:
    pop edi
    pop esi
    pop ebx
    pop ebp
    ret 8
ServeDiskFile ENDP
