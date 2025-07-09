#include "../common/platform.h"

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET_TYPE sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET_VALUE) {
        std::cerr << "Socket creation failed" << std::endl;
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(80);
    inet_pton(AF_INET, "142.250.190.78", &serverAddr.sin_addr);

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR_VALUE) {
        std::cerr << "Connect failed" << std::endl;
        CLOSE_SOCKET(sock);
        WSACleanup();
        return 1;
    }

    sockaddr_in localAddr;
    socklen_t addrLen = sizeof(localAddr);
    getsockname(sock, (sockaddr*)&localAddr, &addrLen);

    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &localAddr.sin_addr, ipStr, sizeof(ipStr));
    std::cout << "Connected from local IP: " << ipStr << std::endl;

    CLOSE_SOCKET(sock);
    WSACleanup();

    std::cout << "Press any key to exit..." << std::endl;
    getch();

    return 0;
}
