#include "../common/platform.h"
#include "../common/network_manager.h"

static bool verbose = false;

void print_usage() {
    lprintf("Usage: forcebindip [options] <program> [args...]\n");
    lprintf("Options:\n");
    lprintf("  -v               : Enable verbose output\n");
    lprintf("  -o <logfile>     : Save logs to the specified file\n");
    lprintf("  -l               : List available network interfaces\n");
    lprintf("Injection Methods (choose one):\n");
    lprintf("  -4 <IPv4>        : Use specified IPv4 address for binding\n");
    lprintf("  -6 <IPv6>        : Use specified IPv6 address for binding\n");
#ifdef PLATFORM_WINDOWS
    lprintf("  -i <GUID>        : Use the IP address associated with the specified interface GUID\n");
#elif defined(PLATFORM_LINUX)
    lprintf("  -i <interface>   : Use the specified network interface (e.g., eth0, wlan0)\n");
#endif
    lprintf("Binding Options:\n");
    lprintf("  -t               : Bind TCP sockets (default if no binding specified)\n");
    lprintf("  -u               : Bind UDP sockets (default if no binding specified)\n");
    lprintf("  -p <port>        : Bind to the specified port (requires -t or -u)\n");
    lprintf("Note: If neither -t nor -u is specified, both TCP and UDP sockets will be bound by default.\n");
}

void list_interfaces() {
    auto interfaces = NetworkManager::GetNetworkInterfaces();
    
    if (interfaces.empty()) {
        lprintf("No network interfaces found.\n");
        return;
    }
    
    lprintf("\nAvailable Network Interfaces:\n");
    lprintf("========================================\n");
    
    for (const auto& iface : interfaces) {
        lprintf("\nInterface Name: %s\n", iface.name.c_str());
#ifdef PLATFORM_WINDOWS
        lprintf("Interface GUID: %s\n", iface.id.c_str());
#elif defined(PLATFORM_LINUX)
        lprintf("Interface ID: %s\n", iface.id.c_str());
#endif
        lprintf("Description: %s\n", iface.description.c_str());
        lprintf("Status: %s\n", iface.is_up ? "Up" : "Down");
        
        if (!iface.ipv4.empty()) {
            lprintf("IPv4 Address: %s\n", iface.ipv4.c_str());
        }
        if (!iface.ipv6.empty()) {
            lprintf("IPv6 Address: %s\n", iface.ipv6.c_str());
        }
    }
}

#ifdef PLATFORM_LINUX
bool inject_library(pid_t pid, const std::string& lib_path) {
    // For Linux, we use LD_PRELOAD environment variable
    // This is handled by the calling process
    return true;
}

int execute_with_preload(const std::vector<std::string>& args, const std::string& lib_path) {
    // Prepare environment with LD_PRELOAD
    std::string ld_preload = lib_path;
    
    // Check if LD_PRELOAD already exists
    const char* existing_preload = getenv("LD_PRELOAD");
    if (existing_preload && strlen(existing_preload) > 0) {
        ld_preload = std::string(existing_preload) + ":" + lib_path;
    }
    
    // Set LD_PRELOAD environment
    if (setenv("LD_PRELOAD", ld_preload.c_str(), 1) != 0) {
        lprintf("Failed to set LD_PRELOAD environment variable: %s\n", strerror(errno));
        return 1;
    }
    
    // Prepare argv for execvp
    std::vector<const char*> argv;
    for (const auto& arg : args) {
        argv.push_back(arg.c_str());
    }
    argv.push_back(nullptr);
    
    // Execute the program
    execvp(argv[0], const_cast<char* const*>(argv.data()));
    
    // If we reach here, execvp failed
    lprintf("Failed to execute program: %s\n", strerror(errno));
    return 1;
}
#endif

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }
    
    bool specify_binding = false;
    bool bind_tcp = false;
    bool bind_udp = false;
    int bind_port = 0;
    
    std::string injection_type;
    int program_index = -1;
    std::string preferred_ip;
    std::string preferred_interface;
    std::string log_file;
    
    // Parse command line arguments
    for (int i = 1; i < argc; ) {
        std::string arg = argv[i];
        
        if (arg == "-v") {
            verbose = true;
            if (verbose) {
                lprintf("Verbose mode enabled\n");
            }
            i++;
        }
        else if (arg == "-l") {
            list_interfaces();
            return 0;
        }
        else if (arg == "-4") {
            if (!injection_type.empty()) {
                lprintf("Error: Multiple injection methods specified\n");
                return 1;
            }
            if (i + 1 >= argc || argv[i + 1][0] == '-') {
                lprintf("Error: -4 requires an IPv4 address\n");
                return 1;
            }
            preferred_ip = "IPv4:" + std::string(argv[i + 1]);
            injection_type = "ipv4";
            if (verbose) {
                lprintf("Set preferred IP to IPv4: %s\n", argv[i + 1]);
            }
            i += 2;
        }
        else if (arg == "-6") {
            if (!injection_type.empty()) {
                lprintf("Error: Multiple injection methods specified\n");
                return 1;
            }
            if (i + 1 >= argc || argv[i + 1][0] == '-') {
                lprintf("Error: -6 requires an IPv6 address\n");
                return 1;
            }
            preferred_ip = "IPv6:" + std::string(argv[i + 1]);
            injection_type = "ipv6";
            if (verbose) {
                lprintf("Set preferred IP to IPv6: %s\n", argv[i + 1]);
            }
            i += 2;
        }
        else if (arg == "-i") {
            if (!injection_type.empty()) {
                lprintf("Error: Multiple injection methods specified\n");
                return 1;
            }
            if (i + 1 >= argc || argv[i + 1][0] == '-') {
#ifdef PLATFORM_WINDOWS
                lprintf("Error: -i requires a GUID\n");
#elif defined(PLATFORM_LINUX)
                lprintf("Error: -i requires an interface name\n");
#endif
                return 1;
            }
            
            std::string interface_id = argv[i + 1];
            std::string ipv4, ipv6;
            
            if (!NetworkManager::GetInterfaceIP(interface_id, ipv4, ipv6)) {
#ifdef PLATFORM_WINDOWS
                lprintf("Error: Invalid GUID or no valid IP found\n");
#elif defined(PLATFORM_LINUX)
                lprintf("Error: Invalid interface name or no valid IP found\n");
#endif
                return 1;
            }
            
            preferred_interface = interface_id;
            
            // Prioritize IPv6 over IPv4
            if (!ipv6.empty()) {
                preferred_ip = "IPv6:" + ipv6;
            } else if (!ipv4.empty()) {
                preferred_ip = "IPv4:" + ipv4;
            } else {
                lprintf("Error: No valid IP found for interface\n");
                return 1;
            }
            
            injection_type = "interface";
            if (verbose) {
                lprintf("Resolved preferred IP from interface: %s\n", preferred_ip.c_str());
            }
            i += 2;
        }
        else if (arg == "-t") {
            bind_tcp = true;
            specify_binding = true;
            i++;
        }
        else if (arg == "-u") {
            bind_udp = true;
            specify_binding = true;
            i++;
        }
        else if (arg == "-p") {
            if (i + 1 >= argc || argv[i + 1][0] == '-') {
                lprintf("Error: -p requires a port number\n");
                return 1;
            }
            bind_port = std::atoi(argv[i + 1]);
            if (bind_port <= 0 || bind_port > 65535) {
                lprintf("Error: Invalid port number %s\n", argv[i + 1]);
                return 1;
            }
            i += 2;
        }
        else if (arg == "-o") {
            if (i + 1 >= argc || argv[i + 1][0] == '-') {
                lprintf("Error: -o requires a log file path\n");
                return 1;
            }
            log_file = argv[i + 1];
            i += 2;
        }
        else if (arg[0] != '-') {
            program_index = i;
            break;
        }
        else {
            lprintf("Unknown option: %s\n", arg.c_str());
            return 1;
        }
    }
    
    if (program_index == -1) {
        lprintf("Error: No program specified\n");
        return 1;
    }
    
    if (bind_port != 0 && !specify_binding) {
        lprintf("Error: -p must be used with -t or -u\n");
        return 1;
    }
    
    if (!specify_binding) {
        bind_tcp = true;
        bind_udp = true;
    }
    
    // Set environment variables for the hook library
#ifdef PLATFORM_WINDOWS
    if (verbose) {
        SetEnvironmentVariable(_T("VERBOSE"), _T("1"));
    }
    SetEnvironmentVariable(_T("BIND_TCP"), bind_tcp ? _T("1") : _T("0"));
    SetEnvironmentVariable(_T("BIND_UDP"), bind_udp ? _T("1") : _T("0"));
    
    if (bind_port != 0) {
        char port_str[6];
        sprintf(port_str, "%d", bind_port);
        SetEnvironmentVariable(_T("BIND_PORT"), _T(port_str));
    }
    
    if (!preferred_ip.empty()) {
        SetEnvironmentVariable(_T("PREFERRED_IP"), _T(preferred_ip.c_str()));
    }
    
    if (!log_file.empty()) {
        SetEnvironmentVariable(_T("LOG_FILE"), _T(log_file.c_str()));
    }
    
    // TODO: Windows injection implementation
    lprintf("Windows injection not implemented yet\n");
    return 1;
    
#elif defined(PLATFORM_LINUX)
    if (verbose) {
        setenv("VERBOSE", "1", 1);
    }
    setenv("BIND_TCP", bind_tcp ? "1" : "0", 1);
    setenv("BIND_UDP", bind_udp ? "1" : "0", 1);
    
    if (bind_port != 0) {
        char port_str[6];
        sprintf(port_str, "%d", bind_port);
        setenv("BIND_PORT", port_str, 1);
    }
    
    if (!preferred_ip.empty()) {
        setenv("PREFERRED_IP", preferred_ip.c_str(), 1);
    }
    
    if (!preferred_interface.empty()) {
        setenv("PREFERRED_INTERFACE", preferred_interface.c_str(), 1);
    }
    
    if (!log_file.empty()) {
        setenv("LOG_FILE", log_file.c_str(), 1);
    }
    
    // Prepare arguments for execution
    std::vector<std::string> exec_args;
    for (int i = program_index; i < argc; i++) {
        exec_args.push_back(argv[i]);
    }
    
    if (verbose) {
        lprintf("Executing: ");
        for (const auto& arg : exec_args) {
            lprintf("%s ", arg.c_str());
        }
        lprintf("\n");
    }
    
    // Find the hook library
    std::string lib_path = "./libforcebindip.so";
    
    // Check if library exists
    if (access(lib_path.c_str(), F_OK) == -1) {
        lprintf("Error: Hook library not found at %s\n", lib_path.c_str());
        return 1;
    }
    
    if (verbose) {
        lprintf("Using hook library: %s\n", lib_path.c_str());
    }
    
    // Execute with LD_PRELOAD
    return execute_with_preload(exec_args, lib_path);
#endif
    
    if (injection_type == "ipv4") {
        lprintf("Program executed successfully with IPv4 binding\n");
    } else if (injection_type == "ipv6") {
        lprintf("Program executed successfully with IPv6 binding\n");
    } else if (injection_type == "interface") {
        lprintf("Program executed successfully with interface binding: %s\n", preferred_ip.c_str());
    } else {
        lprintf("Program executed successfully with default settings\n");
    }
    
    return 0;
}
