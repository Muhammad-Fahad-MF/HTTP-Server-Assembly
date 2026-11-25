# COAL HTTP/1.0 Server — Test Commands (curl)

This document contains the exact `curl` commands corresponding to the test cases documented in **Table 3 (Section 6.1)** of the Final Project Report.

Ensure the server is running on `http://localhost:8080` before executing these commands.

---

### Test Case 1: Default Root GET Request (`200 OK`)
Requests the default homepage (`/`), which maps to `public/index.html`.
```bash
curl -i http://localhost:8080/
```
**Expected Output:**
- **Status:** `HTTP/1.0 200 OK`
- **Headers:** `Content-Type: text/html`, `Content-Length: 1815`, `Connection: close`
- **Body:** HTML content of `index.html`.

---

### Test Case 2: Direct Known Resource GET Request (`200 OK`)
Directly requests `index.html` by filename.
```bash
curl -i http://localhost:8080/index.html
```
**Expected Output:**
- **Status:** `HTTP/1.0 200 OK`
- **Headers:** `Content-Type: text/html`, `Content-Length: 1815`, `Connection: close`
- **Body:** HTML content of `index.html`.

---

### Test Case 3: Secondary Static Page GET Request (`200 OK`)
Requests the secondary project description page (`about.html`).
```bash
curl -i http://localhost:8080/about.html
```
**Expected Output:**
- **Status:** `HTTP/1.0 200 OK`
- **Headers:** `Content-Type: text/html`, `Content-Length: 1355`, `Connection: close`
- **Body:** HTML content of `about.html`.

---

### Test Case 4: Non-Existent Resource GET Request (`404 Not Found`)
Requests a path that does not exist on disk or in fallbacks.
```bash
curl -i http://localhost:8080/missing
```
**Expected Output:**
- **Status:** `HTTP/1.0 404 Not Found`
- **Headers:** `Content-Type: text/html`, `Connection: close`
- **Body:** Styled `404 - File Not Found` HTML page.

---

### Test Case 5: Unsupported HTTP Method (`405 Method Not Allowed`)
Sends an unsupported HTTP verb (`PUT`) to verify method filtering.
```bash
curl -i -X PUT http://localhost:8080/api/echo
```
**Expected Output:**
- **Status:** `HTTP/1.0 405 Method Not Allowed`
- **Headers:** `Content-Type: text/html`, `Connection: close`
- **Body:** Styled `405 - Method Not Allowed` HTML page.

---

### Test Case 6: POST Data Submission with Payload (`200 OK`)
Submits a bounded form body to verify `Content-Length` header parsing and payload extraction.
```bash
curl -i -X POST http://localhost:8080/api/echo -H "Content-Type: application/x-www-form-urlencoded" -d "student=Fahad&course=COAL"
```
**Expected Output:**
- **Status:** `HTTP/1.0 200 OK`
- **Headers:** `Content-Type: application/json`, `Connection: close`
- **Body:** `{"status":"success","message":"POST payload received by COAL MASM Server"}`
- **Server Console:** Displays payload bytes in bright yellow.

---

### Test Case 7: Malformed Request Line (`400 Bad Request`)
Transmits a malformed byte stream missing the required space-delimited HTTP tokens.

Using PowerShell TCP stream:
```powershell
powershell -Command "$client = New-Object System.Net.Sockets.TcpClient('127.0.0.1', 8080); $stream = $client.GetStream(); $bytes = [System.Text.Encoding]::ASCII.GetBytes('MALFORMED_REQUEST_NO_SPACES`r`n`r`n'); $stream.Write($bytes, 0, $bytes.Length); $reader = New-Object System.IO.StreamReader($stream); $reader.ReadToEnd(); $client.Close()"
```

Or using Python one-liner:
```bash
python -c "import socket; s = socket.socket(); s.connect(('127.0.0.1', 8080)); s.sendall(b'INVALID_SYNTAX\r\n\r\n'); print(s.recv(1024).decode()); s.close()"
```
**Expected Output:**
- **Status:** `HTTP/1.0 400 Bad Request`
- **Headers:** `Content-Type: text/html`, `Connection: close`
- **Body:** Styled `400 - Bad Request` HTML page.
