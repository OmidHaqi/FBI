#pragma once

#include "platform.h"
#include <vector>
#include <string>

struct NetworkInterface {
    std::string name;
    std::string id;  // GUID on Windows, interface name on Linux
    std::string ipv4;
    std::string ipv6;
    bool is_up;
    std::string description;
};

class NetworkManager {
public:
    static std::vector<NetworkInterface> GetNetworkInterfaces();
    static bool GetInterfaceIP(const std::string& interface_id, std::string& ipv4, std::string& ipv6);
    static bool IsValidIP(const std::string& ip, bool is_ipv6 = false);
    
#ifdef PLATFORM_LINUX
    static bool BindToInterface(int socket_fd, const std::string& interface_name);
#endif

private:
#ifdef PLATFORM_WINDOWS
    static bool IsLinkLocalIPv6(const IN6_ADDR& addr);
#elif defined(PLATFORM_LINUX)  
    static bool IsLinkLocalIPv6(const struct in6_addr& addr);
#endif
};
