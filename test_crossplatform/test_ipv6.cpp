#include "../common/platform.h"

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET_TYPE sock = socket(AF_INET6, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET_VALUE) {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in6 serverAddr = {0};
    serverAddr.sin6_family = AF_INET6;
    serverAddr.sin6_port = htons(80);
    inet_pton(AF_INET6, "2001:4860:4860::8888", &serverAddr.sin6_addr);

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR_VALUE) {
        std::cerr << "Connect failed: " << WSAGetLastError() << std::endl;
        CLOSE_SOCKET(sock);
        WSACleanup();
        return 1;
    }

    sockaddr_in6 localAddr = {0};
    socklen_t addrLen = sizeof(localAddr);
    getsockname(sock, (sockaddr*)&localAddr, &addrLen);

    char ipStr[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &localAddr.sin6_addr, ipStr, sizeof(ipStr));
    std::cout << "Connected from local IP: " << ipStr << std::endl;

    CLOSE_SOCKET(sock);
    WSACleanup();

    std::cout << "Press any key to exit..." << std::endl;
    getch();

    return 0;
}
