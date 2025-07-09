#pragma once

#ifdef _WIN32
    #define PLATFORM_WINDOWS
    #include <windows.h>
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <iphlpapi.h>
    #include <tchar.h>
    #include <conio.h>
    #include <tlhelp32.h>
    
    #define SOCKET_TYPE SOCKET
    #define INVALID_SOCKET_VALUE INVALID_SOCKET
    #define SOCKET_ERROR_VALUE SOCKET_ERROR
    #define CLOSE_SOCKET closesocket
    
    typedef TCHAR char_t;
    #define _T(x) _T(x)
    #define lprintf(fmt, ...) _tprintf(_T(fmt), ##__VA_ARGS__)
    
#elif defined(__linux__)
    #define PLATFORM_LINUX
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <ifaddrs.h>
    #include <net/if.h>
    #include <unistd.h>
    #include <dlfcn.h>
    #include <sys/ptrace.h>
    #include <sys/wait.h>
    #include <errno.h>
    #include <string.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <fcntl.h>
    #include <termios.h>
    
    #define SOCKET_TYPE int
    #define INVALID_SOCKET_VALUE -1
    #define SOCKET_ERROR_VALUE -1
    #define CLOSE_SOCKET close
    
    typedef char char_t;
    #define _T(x) x
    #define lprintf(fmt, ...) printf(fmt, ##__VA_ARGS__)
    
    // Windows compatibility macros for Linux
    #define WSAGetLastError() errno
    #define WSAStartup(ver, data) 0
    #define WSACleanup() 0
    
    typedef struct {
        unsigned short wVersion;
        unsigned short wHighVersion;
    } WSADATA;
    
    // socklen_t is already defined on Linux
    // Additional socket constants
    #ifndef MAKEWORD
    #define MAKEWORD(a, b) ((unsigned short)(((unsigned char)(a)) | ((unsigned short)((unsigned char)(b))) << 8))
    #endif
    
#endif

// Common includes
#include <iostream>
#include <string>
#include <cstring>
#include <algorithm>
#include <vector>
#include <fstream>

// Common utility functions
inline int getch() {
#ifdef PLATFORM_WINDOWS
    return _getch();
#else
    struct termios oldattr, newattr;
    int ch;
    tcgetattr(STDIN_FILENO, &oldattr);
    newattr = oldattr;
    newattr.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
    return ch;
#endif
}
