#include "network_manager.h"
#include <map>

#ifdef PLATFORM_WINDOWS
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")

bool NetworkManager::IsLinkLocalIPv6(const IN6_ADDR& addr) {
    return (addr.u.Byte[0] == 0xFE && (addr.u.Byte[1] & 0xC0) == 0x80);
}

std::vector<NetworkInterface> NetworkManager::GetNetworkInterfaces() {
    std::vector<NetworkInterface> interfaces;
    
    ULONG outBufLen = 15000;
    PIP_ADAPTER_ADDRESSES pAddresses = (PIP_ADAPTER_ADDRESSES)malloc(outBufLen);
    if (!pAddresses) return interfaces;
    
    DWORD dwRetVal = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, pAddresses, &outBufLen);
    if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
        free(pAddresses);
        pAddresses = (PIP_ADAPTER_ADDRESSES)malloc(outBufLen);
        if (!pAddresses) return interfaces;
        dwRetVal = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, pAddresses, &outBufLen);
    }
    
    if (dwRetVal == NO_ERROR) {
        for (PIP_ADAPTER_ADDRESSES pAdapter = pAddresses; pAdapter; pAdapter = pAdapter->Next) {
            if (pAdapter->OperStatus == IfOperStatusUp) {
                NetworkInterface iface;
                
                // Convert wide string to string
                size_t convertedChars = 0;
                char friendlyName[256];
                wcstombs_s(&convertedChars, friendlyName, sizeof(friendlyName), pAdapter->FriendlyName, _TRUNCATE);
                iface.name = friendlyName;
                
                char description[256];
                wcstombs_s(&convertedChars, description, sizeof(description), pAdapter->Description, _TRUNCATE);
                iface.description = description;
                
                iface.id = pAdapter->AdapterName;
                iface.is_up = true;
                
                // Get IP addresses
                for (PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pAdapter->FirstUnicastAddress; pUnicast; pUnicast = pUnicast->Next) {
                    if (pUnicast->Address.lpSockaddr->sa_family == AF_INET) {
                        sockaddr_in* sa_in = (sockaddr_in*)pUnicast->Address.lpSockaddr;
                        char ip_str[INET_ADDRSTRLEN];
                        inet_ntop(AF_INET, &sa_in->sin_addr, ip_str, sizeof(ip_str));
                        iface.ipv4 = ip_str;
                    } else if (pUnicast->Address.lpSockaddr->sa_family == AF_INET6) {
                        sockaddr_in6* sa_in6 = (sockaddr_in6*)pUnicast->Address.lpSockaddr;
                        if (!IsLinkLocalIPv6(sa_in6->sin6_addr)) {
                            char ip_str[INET6_ADDRSTRLEN];
                            inet_ntop(AF_INET6, &sa_in6->sin6_addr, ip_str, sizeof(ip_str));
                            iface.ipv6 = ip_str;
                        }
                    }
                }
                
                interfaces.push_back(iface);
            }
        }
    }
    
    free(pAddresses);
    return interfaces;
}

bool NetworkManager::GetInterfaceIP(const std::string& interface_id, std::string& ipv4, std::string& ipv6) {
    auto interfaces = GetNetworkInterfaces();
    for (const auto& iface : interfaces) {
        std::string clean_id = interface_id;
        if (clean_id.front() == '{' && clean_id.back() == '}') {
            clean_id = clean_id.substr(1, clean_id.length() - 2);
        }
        
        if (iface.id == clean_id) {
            ipv4 = iface.ipv4;
            ipv6 = iface.ipv6;
            return true;
        }
    }
    return false;
}

#elif defined(PLATFORM_LINUX)

bool NetworkManager::IsLinkLocalIPv6(const struct in6_addr& addr) {
    return (addr.s6_addr[0] == 0xFE && (addr.s6_addr[1] & 0xC0) == 0x80);
}

std::vector<NetworkInterface> NetworkManager::GetNetworkInterfaces() {
    std::vector<NetworkInterface> interfaces;
    struct ifaddrs *ifaddr, *ifa;
    
    if (getifaddrs(&ifaddr) == -1) {
        return interfaces;
    }
    
    std::map<std::string, NetworkInterface> iface_map;
    
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;
        
        std::string ifname = ifa->ifa_name;
        
        // Skip loopback interface
        if (ifname == "lo") continue;
        
        NetworkInterface& iface = iface_map[ifname];
        if (iface.name.empty()) {
            iface.name = ifname;
            iface.id = ifname;
            iface.description = "Linux Network Interface";
            iface.is_up = (ifa->ifa_flags & IFF_UP) && (ifa->ifa_flags & IFF_RUNNING);
        }
        
        if (ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in* sa_in = (struct sockaddr_in*)ifa->ifa_addr;
            char ip_str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &sa_in->sin_addr, ip_str, sizeof(ip_str));
            iface.ipv4 = ip_str;
        } else if (ifa->ifa_addr->sa_family == AF_INET6) {
            struct sockaddr_in6* sa_in6 = (struct sockaddr_in6*)ifa->ifa_addr;
            if (!IsLinkLocalIPv6(sa_in6->sin6_addr)) {
                char ip_str[INET6_ADDRSTRLEN];
                inet_ntop(AF_INET6, &sa_in6->sin6_addr, ip_str, sizeof(ip_str));
                iface.ipv6 = ip_str;
            }
        }
    }
    
    freeifaddrs(ifaddr);
    
    for (const auto& pair : iface_map) {
        if (pair.second.is_up) {
            interfaces.push_back(pair.second);
        }
    }
    
    return interfaces;
}

bool NetworkManager::GetInterfaceIP(const std::string& interface_name, std::string& ipv4, std::string& ipv6) {
    auto interfaces = GetNetworkInterfaces();
    for (const auto& iface : interfaces) {
        if (iface.id == interface_name || iface.name == interface_name) {
            ipv4 = iface.ipv4;
            ipv6 = iface.ipv6;
            return true;
        }
    }
    return false;
}

bool NetworkManager::BindToInterface(int socket_fd, const std::string& interface_name) {
    if (setsockopt(socket_fd, SOL_SOCKET, SO_BINDTODEVICE, 
                   interface_name.c_str(), interface_name.length()) < 0) {
        return false;
    }
    return true;
}

#endif

bool NetworkManager::IsValidIP(const std::string& ip, bool is_ipv6) {
    if (is_ipv6) {
        struct in6_addr addr;
        return inet_pton(AF_INET6, ip.c_str(), &addr) == 1;
    } else {
        struct in_addr addr;
        return inet_pton(AF_INET, ip.c_str(), &addr) == 1;
    }
}
