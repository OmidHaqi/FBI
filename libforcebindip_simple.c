#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdarg.h>

// Global configuration
static char g_preferred_ipv4[INET_ADDRSTRLEN] = {0};
static char g_preferred_ipv6[INET6_ADDRSTRLEN] = {0};
static char g_preferred_interface[64] = {0};
static int g_bind_tcp = 0;
static int g_bind_udp = 0;
static int g_bind_port = 0;
static int g_verbose = 0;
static FILE *g_log_file = NULL;

// Original function pointers
static int (*original_socket)(int domain, int type, int protocol) = NULL;
static int (*original_bind)(int sockfd, const struct sockaddr *addr, socklen_t addrlen) = NULL;
static int (*original_connect)(int sockfd, const struct sockaddr *addr, socklen_t addrlen) = NULL;
static ssize_t (*original_sendto)(int sockfd, const void *buf, size_t len, int flags,
                                  const struct sockaddr *dest_addr, socklen_t addrlen) = NULL;
static int (*original_getsockname)(int sockfd, struct sockaddr *addr, socklen_t *addrlen) = NULL;

// Helper function to log messages
static void log_message(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    if (g_log_file)
    {
        vfprintf(g_log_file, format, args);
        fflush(g_log_file);
    }

    if (g_verbose)
    {
        vprintf(format, args);
        fflush(stdout);
    }

    va_end(args);
}

// Helper function to bind socket to preferred address
static int enforce_binding(int sockfd, int family)
{
    struct sockaddr_storage bind_addr;
    memset(&bind_addr, 0, sizeof(bind_addr));
    socklen_t bind_len;

    if (family == AF_INET && g_preferred_ipv4[0] != 0)
    {
        struct sockaddr_in *sin = (struct sockaddr_in *)&bind_addr;
        sin->sin_family = AF_INET;
        inet_pton(AF_INET, g_preferred_ipv4, &sin->sin_addr);
        sin->sin_port = (g_bind_port != 0) ? htons(g_bind_port) : 0;
        bind_len = sizeof(struct sockaddr_in);
    }
    else if (family == AF_INET6 && g_preferred_ipv6[0] != 0)
    {
        struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&bind_addr;
        sin6->sin6_family = AF_INET6;
        inet_pton(AF_INET6, g_preferred_ipv6, &sin6->sin6_addr);
        sin6->sin6_port = (g_bind_port != 0) ? htons(g_bind_port) : 0;
        bind_len = sizeof(struct sockaddr_in6);
    }
    else
    {
        if (g_verbose)
        {
            log_message("No preferred address for family %d\n", family);
        }
        return 1;
    }

    // Bind to interface if specified
    if (g_preferred_interface[0] != 0)
    {
        if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE,
                       g_preferred_interface, strlen(g_preferred_interface)) < 0)
        {
            if (g_verbose)
            {
                log_message("Failed to bind to interface %s: %s\n",
                            g_preferred_interface, strerror(errno));
            }
        }
    }

    int bind_result = original_bind(sockfd, (struct sockaddr *)&bind_addr, bind_len);
    if (bind_result != 0)
    {
        if (g_verbose)
        {
            log_message("Binding failed: %s\n", strerror(errno));
        }
        return 0;
    }

    if (g_verbose)
    {
        char ip_str[INET6_ADDRSTRLEN];
        if (family == AF_INET)
        {
            struct sockaddr_in *sin = (struct sockaddr_in *)&bind_addr;
            inet_ntop(AF_INET, &sin->sin_addr, ip_str, sizeof(ip_str));
            log_message("Bound to IPv4: %s:%d\n", ip_str, ntohs(sin->sin_port));
        }
        else if (family == AF_INET6)
        {
            struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&bind_addr;
            inet_ntop(AF_INET6, &sin6->sin6_addr, ip_str, sizeof(ip_str));
            log_message("Bound to IPv6: %s:%d\n", ip_str, ntohs(sin6->sin6_port));
        }
    }

    return 1;
}

// Hook implementations
int socket(int domain, int type, int protocol)
{
    if (!original_socket)
    {
        original_socket = (int (*)(int, int, int))dlsym(RTLD_NEXT, "socket");
    }

    int sockfd = original_socket(domain, type, protocol);

    if (g_verbose)
    {
        log_message("Socket %d created: domain=%d, type=%d, protocol=%d\n",
                    sockfd, domain, type, protocol);
    }

    return sockfd;
}

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    if (!original_bind)
    {
        original_bind = (int (*)(int, const struct sockaddr *, socklen_t))dlsym(RTLD_NEXT, "bind");
    }

    struct sockaddr_storage bind_addr;
    memcpy(&bind_addr, addr, addrlen);
    int override = 0;

    if (bind_addr.ss_family == AF_INET && g_preferred_ipv4[0] != 0)
    {
        struct sockaddr_in *sin = (struct sockaddr_in *)&bind_addr;
        if (sin->sin_addr.s_addr == INADDR_ANY)
        {
            inet_pton(AF_INET, g_preferred_ipv4, &sin->sin_addr);
            override = 1;
            if (g_verbose)
            {
                log_message("Overriding bind from INADDR_ANY to preferred IPv4: %s\n",
                            g_preferred_ipv4);
            }
        }
    }
    else if (bind_addr.ss_family == AF_INET6 && g_preferred_ipv6[0] != 0)
    {
        struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&bind_addr;
        if (IN6_IS_ADDR_UNSPECIFIED(&sin6->sin6_addr))
        {
            inet_pton(AF_INET6, g_preferred_ipv6, &sin6->sin6_addr);
            override = 1;
            if (g_verbose)
            {
                log_message("Overriding bind from unspecified to preferred IPv6: %s\n",
                            g_preferred_ipv6);
            }
        }
    }

    // Bind to interface if specified
    if (g_preferred_interface[0] != 0)
    {
        setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE,
                   g_preferred_interface, strlen(g_preferred_interface));
    }

    int result = original_bind(sockfd, (struct sockaddr *)&bind_addr, addrlen);

    if (override && result == 0)
    {
        log_message("Successfully bound to preferred IP\n");
    }

    return result;
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    if (!original_connect)
    {
        original_connect = (int (*)(int, const struct sockaddr *, socklen_t))dlsym(RTLD_NEXT, "connect");
    }

    // Check socket type
    int sock_type;
    socklen_t optlen = sizeof(sock_type);
    if (getsockopt(sockfd, SOL_SOCKET, SO_TYPE, &sock_type, &optlen) == -1)
    {
        if (g_verbose)
        {
            log_message("Failed to get socket type: %s\n", strerror(errno));
        }
        return -1;
    }

    if (g_bind_tcp && sock_type == SOCK_STREAM)
    {
        // Check if socket is already bound
        struct sockaddr_storage local_addr;
        socklen_t addr_len = sizeof(local_addr);
        int bound = (getsockname(sockfd, (struct sockaddr *)&local_addr, &addr_len) == 0);

        if (!bound || (local_addr.ss_family == AF_INET && ((struct sockaddr_in *)&local_addr)->sin_addr.s_addr == INADDR_ANY) ||
            (local_addr.ss_family == AF_INET6 &&
             IN6_IS_ADDR_UNSPECIFIED(&((struct sockaddr_in6 *)&local_addr)->sin6_addr)))
        {

            if (!enforce_binding(sockfd, addr->sa_family))
            {
                if (g_verbose)
                {
                    log_message("Failed to enforce binding for TCP socket %d\n", sockfd);
                }
                return -1;
            }
        }
    }

    int result = original_connect(sockfd, addr, addrlen);

    if (g_verbose && result != 0)
    {
        log_message("Connect failed: %s\n", strerror(errno));
    }

    if (result == 0 && g_verbose)
    {
        struct sockaddr_storage local_addr;
        socklen_t addr_len = sizeof(local_addr);
        if (getsockname(sockfd, (struct sockaddr *)&local_addr, &addr_len) == 0)
        {
            char ip_str[INET6_ADDRSTRLEN];
            if (local_addr.ss_family == AF_INET)
            {
                struct sockaddr_in *sin = (struct sockaddr_in *)&local_addr;
                inet_ntop(AF_INET, &sin->sin_addr, ip_str, sizeof(ip_str));
                log_message("TCP socket %d connected from: %s:%d\n",
                            sockfd, ip_str, ntohs(sin->sin_port));
            }
            else if (local_addr.ss_family == AF_INET6)
            {
                struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&local_addr;
                inet_ntop(AF_INET6, &sin6->sin6_addr, ip_str, sizeof(ip_str));
                log_message("TCP socket %d connected from: %s:%d\n",
                            sockfd, ip_str, ntohs(sin6->sin6_port));
            }
        }
    }

    return result;
}

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen)
{
    if (!original_sendto)
    {
        original_sendto = (ssize_t (*)(int, const void *, size_t, int,
                                       const struct sockaddr *, socklen_t))dlsym(RTLD_NEXT, "sendto");
    }

    // Check socket type
    int sock_type;
    socklen_t optlen = sizeof(sock_type);
    if (getsockopt(sockfd, SOL_SOCKET, SO_TYPE, &sock_type, &optlen) == 0 &&
        sock_type == SOCK_DGRAM)
    {

        if (g_bind_udp)
        {
            struct sockaddr_storage addr;
            socklen_t addr_len = sizeof(addr);

            // Check if socket is already bound
            if (getsockname(sockfd, (struct sockaddr *)&addr, &addr_len) == -1 && errno == EINVAL)
            {
                if (!enforce_binding(sockfd, dest_addr->sa_family))
                {
                    if (g_verbose)
                    {
                        log_message("Failed to enforce binding for UDP socket %d\n", sockfd);
                    }
                    return -1;
                }
            }
        }
    }

    return original_sendto(sockfd, buf, len, flags, dest_addr, addrlen);
}

int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    if (!original_getsockname)
    {
        original_getsockname = (int (*)(int, struct sockaddr *, socklen_t *))dlsym(RTLD_NEXT, "getsockname");
    }

    return original_getsockname(sockfd, addr, addrlen);
}

// Constructor function to initialize hooks when library is loaded
__attribute__((constructor)) static void init_hooks()
{
    // Parse environment variables
    const char *verbose_env = getenv("VERBOSE");
    if (verbose_env && strcmp(verbose_env, "1") == 0)
    {
        g_verbose = 1;
    }

    const char *preferred_ip = getenv("PREFERRED_IP");
    if (preferred_ip)
    {
        if (strncmp(preferred_ip, "IPv4:", 5) == 0)
        {
            strncpy(g_preferred_ipv4, preferred_ip + 5, sizeof(g_preferred_ipv4) - 1);
        }
        else if (strncmp(preferred_ip, "IPv6:", 5) == 0)
        {
            strncpy(g_preferred_ipv6, preferred_ip + 5, sizeof(g_preferred_ipv6) - 1);
        }
    }

    const char *bind_tcp = getenv("BIND_TCP");
    if (bind_tcp && strcmp(bind_tcp, "1") == 0)
    {
        g_bind_tcp = 1;
    }

    const char *bind_udp = getenv("BIND_UDP");
    if (bind_udp && strcmp(bind_udp, "1") == 0)
    {
        g_bind_udp = 1;
    }

    const char *bind_port = getenv("BIND_PORT");
    if (bind_port)
    {
        g_bind_port = atoi(bind_port);
    }

    const char *log_file = getenv("LOG_FILE");
    if (log_file)
    {
        g_log_file = fopen(log_file, "a");
        if (g_log_file)
        {
            log_message("Logging started for process %d\n", getpid());
        }
    }

    const char *interface_env = getenv("PREFERRED_INTERFACE");
    if (interface_env)
    {
        strncpy(g_preferred_interface, interface_env, sizeof(g_preferred_interface) - 1);
    }

    // Get original function pointers
    original_socket = (int (*)(int, int, int))dlsym(RTLD_NEXT, "socket");
    original_bind = (int (*)(int, const struct sockaddr *, socklen_t))dlsym(RTLD_NEXT, "bind");
    original_connect = (int (*)(int, const struct sockaddr *, socklen_t))dlsym(RTLD_NEXT, "connect");
    original_sendto = (ssize_t (*)(int, const void *, size_t, int,
                                   const struct sockaddr *, socklen_t))dlsym(RTLD_NEXT, "sendto");
    original_getsockname = (int (*)(int, struct sockaddr *, socklen_t *))dlsym(RTLD_NEXT, "getsockname");

    if (g_verbose)
    {
        log_message("ForceBindIP Linux hooks initializing...\n");
        log_message("Preferred IPv4: %s\n", g_preferred_ipv4);
        log_message("Preferred IPv6: %s\n", g_preferred_ipv6);
        log_message("Preferred Interface: %s\n", g_preferred_interface);
        log_message("Bind TCP: %s\n", g_bind_tcp ? "yes" : "no");
        log_message("Bind UDP: %s\n", g_bind_udp ? "yes" : "no");
        log_message("Bind Port: %d\n", g_bind_port);
        log_message("Linux hooks initialized successfully\n");
    }
}

// Destructor function to cleanup when library is unloaded
__attribute__((destructor)) static void cleanup_hooks()
{
    if (g_log_file)
    {
        fclose(g_log_file);
        g_log_file = NULL;
    }
}
