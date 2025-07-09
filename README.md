# ForceBindIP - Network Interface Binding Tool

## What It Does
ForceBindIP forces any application to use a specific network interface or IP address for all network connections. Instead of letting programs choose which network adapter to use, this tool intercepts network calls and redirects them through your preferred interface.

## Why Use It?
- **VPN Routing**: Force specific apps through VPN while others use regular connection
- **Multi-Network Setup**: Choose which network adapter apps should use (WiFi, Ethernet, etc.)
- **Testing**: Test applications with different network configurations
- **Security**: Ensure sensitive apps on## Contributing
1. Fork the repository
2. Create feature branch: `git checkout -b feature/my-feature`
3. Test on both platforms
4. Submit pull request

## Quick Reference

### Installation
```bash
# Quick install (Debian/Ubuntu)
sudo dpkg -i forcebindip_1.0.0_amd64.deb

# Or build from source
make clean && make
```

### Common Commands
```bash
forcebindip -l                                    # List interfaces
forcebindip -4 <IP> <command>                     # Bind to IPv4
forcebindip -6 <IP> <command>                     # Bind to IPv6
forcebindip -i <interface> <command>              # Bind to interface
forcebindip -v -4 <IP> <command>                  # Verbose mode
```

### Testing
```bash
make test-usage        # Test all 27 usage scenarios
make test-deb          # Test package installation
make deb              # Build Debian package
```

### Support
- 📖 **Documentation**: README.md (English), README_FA.md (Persian)
- 🧪 **Testing**: `./test-all-usage.sh` for comprehensive testing
- 📦 **Package**: Pre-built .deb available
- 🔧 **Build**: Use `make help` for all options

## License
Based on original ForceBindIP project license.

---

**Ready to control your network traffic?**  
Start with `forcebindip -l` to see your interfaces, then force your apps to use exactly the network path you want! network interfaces
- **Bandwidth Management**: Route heavy downloads through specific connections

## Features
-  **Cross-Platform**: Works on Windows (DLL injection) and Linux (LD_PRELOAD)
-  **IPv4 & IPv6**: Full support for both IP versions
-  **Interface Binding**: Bind by network interface name or IP address
-  **Protocol Support**: TCP and UDP socket binding
-  **Real-time**: No app restart needed - works with running applications
-  **Verbose Logging**: Detailed connection information for debugging
-  **Debian Package**: Ready-to-install .deb package for Ubuntu/Debian
-  **Comprehensive Testing**: Complete test suite for all usage scenarios

## Quick Start

### Linux
```bash
# Build from source
make clean && make

# Or install pre-built package
sudo dpkg -i forcebindip_1.0.0_amd64.deb

# List available network interfaces
./forcebindip -l

# Force curl to use specific IP
./forcebindip -4 192.168.1.100 curl http://httpbin.org/ip

# Force app through VPN interface
./forcebindip -i tun0 firefox

# Test with sample program
./forcebindip -v -4 192.168.1.3 ./test_ipv4

# Run comprehensive tests
make test-usage
```

### Windows
```cmd
# List network interfaces
injector.exe -l

# Force app to use specific IP
injector.exe -4 192.168.1.1 program.exe

# Force app through interface GUID
injector.exe -i 11111111-2222-3333-4444-555555555555 program.exe
```

## Command Options

### Linux
```bash
./forcebindip [options] <command> [args...]

-4 <ip>      # Use specific IPv4 address
-6 <ip>      # Use specific IPv6 address  
-i <iface>   # Use network interface (eth0, wlo1, tun0, etc.)
-v           # Verbose output (show connection details)
-l           # List all available network interfaces
-h           # Show help
```

### Windows
```cmd
injector.exe [options] <program> [args...]

-4 <ip>      # Use specific IPv4 address
-6 <ip>      # Use specific IPv6 address
-i <GUID>    # Use network interface by GUID
-v           # Verbose output
-l           # List network interfaces with GUIDs
-t           # Bind TCP sockets only
-u           # Bind UDP sockets only
-p <port>    # Bind to specific port
```

## Practical Examples

### Force Specific Applications
```bash
# Linux Examples
./forcebindip -i eth0 wget http://example.com          # Download via Ethernet
./forcebindip -i wlo1 curl http://api.service.com      # API call via WiFi
./forcebindip -i tun0 ssh user@server.com             # SSH via VPN
./forcebindip -4 10.0.0.5 firefox                     # Browser via specific IP

# Windows Examples
injector.exe -i {GUID} chrome.exe                     # Chrome via specific adapter
injector.exe -4 192.168.1.100 uTorrent.exe           # Torrent via specific IP
injector.exe -i {VPN-GUID} discord.exe                # Discord via VPN
```

### Testing & Verification
```bash
# Check your real IP before
curl http://httpbin.org/ip

# Force through different interface
./forcebindip -4 192.168.1.100 curl http://httpbin.org/ip

# Compare the IPs - they should be different!
```

### VPN Use Cases
```bash
# Force sensitive apps through VPN only
./forcebindip -i tun0 telegram-desktop
./forcebindip -i tun0 signal-desktop
./forcebindip -i tun0 firefox --private-window

# Keep regular apps on normal connection
./forcebindip -i wlo1 spotify
./forcebindip -i eth0 steam
```

## How It Works

### Linux (LD_PRELOAD Method)
1. Intercepts socket system calls using `LD_PRELOAD`
2. When app creates a socket, ForceBindIP binds it to your chosen interface
3. All network traffic from that app goes through the specified interface
4. Works with: `socket()`, `bind()`, `connect()`, `sendto()`, `getsockname()`

### Windows (DLL Injection Method)
1. Injects a DLL into the target process
2. Hooks WinSock API functions using MinHook library
3. Redirects network calls to use specified interface/IP
4. Works with: `socket`, `bind`, `connect`, `WSASendTo`, `getsockname`

## Supported Applications
- **Web Browsers**: Firefox, Chrome, Edge
- **Communication**: Discord, Telegram, Signal, WhatsApp
- **Development**: curl, wget, ssh, git, npm, pip
- **Entertainment**: Steam, Spotify, VLC
- **File Sharing**: BitTorrent clients, FTP clients
- **Custom Apps**: Any app using standard socket APIs

## Installation

### Linux - Option 1: Install Pre-built Package (Recommended)
```bash
# Download and install the .deb package
sudo dpkg -i forcebindip_1.0.0_amd64.deb

# Fix any missing dependencies
sudo apt-get install -f

# Verify installation
forcebindip -l
```

### Linux - Option 2: Build from Source
```bash
# Clone and build
git clone <repository-url>
cd FBI
make clean && make

# Generated files:
# - forcebindip: Main executable
# - libforcebindip.so: Hook library
# - test_ipv4, test_ipv6: Test programs

# Optional: Install system-wide
sudo make install
```

### Windows
```bash
# Use Visual Studio 2022
# Open solution files in dll/ and injector/ directories
# Build with MinHook library dependency
```

## Troubleshooting

### Common Issues

**"Invalid argument" error:**
```bash
# ❌ Don't use loopback for external connections
./forcebindip -4 127.0.0.1 curl http://google.com

# ✅ Use actual interface IP
./forcebindip -4 192.168.1.3 curl http://google.com
```

**Permission denied:**
```bash
# Some apps need elevated privileges
sudo ./forcebindip -i eth0 ping google.com
```

**Library not found:**
```bash
# Make sure library exists
ls -la libforcebindip.so

# Rebuild if needed
make clean && make
```

### Debugging
```bash
# Enable verbose mode to see what's happening
./forcebindip -v -4 192.168.1.3 curl http://httpbin.org/ip

# Check available interfaces
./forcebindip -l

# Test connectivity manually
ping -I 192.168.1.3 google.com
```

## Testing & Verification

### Quick Test
```bash
# Test basic functionality
forcebindip -l                                    # List interfaces
forcebindip -4 192.168.1.100 curl httpbin.org/ip # Test IP binding
```

### Comprehensive Testing
```bash
# Run all 27 usage scenarios (recommended)
make test-usage
# or directly:
./test-all-usage.sh

# Build and test Debian package
make test-deb

# Build Debian package only
make deb
```

### Test Results Overview
The comprehensive test suite validates:
- ✅ **Basic Commands** (help, interface listing)
- ✅ **IPv4 Binding** (multiple IP addresses)
- ✅ **IPv6 Binding** (when available)
- ✅ **Interface Binding** (network adapters)
- ✅ **Real Applications** (curl, wget, ping, ssh, netcat)
- ✅ **Error Handling** (invalid inputs)
- ✅ **Performance** (stress testing)

### Manual Testing Examples
```bash
# Test IPv4 binding with verbose output
./forcebindip -v -4 192.168.1.3 ./test_ipv4

# Test interface binding
./forcebindip -v -i eth0 ./test_ipv4

# Test with real applications
./forcebindip -4 192.168.1.100 curl -s httpbin.org/ip
./forcebindip -i tun0 wget http://example.com
./forcebindip -v -4 10.0.0.5 ping -c 3 google.com
```

## Package Management

### Debian Package (.deb)
```bash
# Build package
./build-deb.sh

# Install package
sudo dpkg -i forcebindip_1.0.0_amd64.deb

# Remove package
sudo dpkg -r forcebindip

# Package information
dpkg-deb --info forcebindip_1.0.0_amd64.deb
```

## Development

### Build System
```bash
# Available make targets
make help

# Key targets:
make all               # Build everything
make deb              # Create Debian package
make test-usage       # Test all usage scenarios
make install          # Install system-wide
make clean            # Clean build files
```

## Files Structure
```
FBI/
├── build/                     # Build output directory
├── common/                    # Cross-platform headers
├── dll/                       # Windows DLL source code
├── forcebindip                # Main Linux executable
├── forcebindip_1.0.0_amd64.deb # Pre-built Debian package
├── injector/                  # Windows injector source
├── injector_crossplatform/    # Cross-platform injector
├── libforcebindip_simple.c    # Linux hook library source (C)
├── libforcebindip.so          # Linux hook library (compiled)
├── Makefile                   # Linux build system
├── README_FA.md               # Persian documentation
├── README.md                  # English documentation
├── test_crossplatform/        # Cross-platform test programs
├── test_ipv4                  # IPv4 test executable
├── test_ipv6                  # IPv6 test executable
├── testv4/                    # Original IPv4 test source
├── testv6/                    # Original IPv6 test source
├── build-deb.sh               # Debian package builder
├── test-all-usage.sh          # Comprehensive usage tester
└── debian/                    # Debian package control files
```

## Detailed Usage Guide

### Installation Details

#### Linux Build Requirements
```bash
# Ensure you have required packages
sudo apt-get update
sudo apt-get install build-essential gcc g++ make

# Clone and build
git clone <repository-url>
cd FBI
make clean && make

# Generated files:
# - forcebindip: Main injector executable
# - libforcebindip.so: Hook library for LD_PRELOAD
# - test_ipv4, test_ipv6: Test programs
```

#### Windows Build Requirements
- Windows operating system with Winsock support
- Visual Studio 2022
- MinHook library for hooking Winsock functions

### Complete Command Reference

#### All Linux Options
```bash
./forcebindip [options] <command> [args...]

-4 <ip>        # Bind to specific IPv4 address
-6 <ip>        # Bind to specific IPv6 address  
-i <interface> # Bind to network interface
-p <port>      # Bind to specific port (Linux only)
-v             # Enable verbose output
-l             # List available network interfaces
-h, --help     # Show help message
```

#### All Windows Options
```cmd
injector.exe [options] <program> [args...]

-4 <IPv4>      # Bind to specified IPv4 address
-6 <IPv6>      # Bind to specified IPv6 address
-i <GUID>      # Bind to network interface by GUID
-v             # Enable verbose output
-o <file>      # Save DLL logs to specified file
-l             # List available network interfaces with GUIDs
-t             # Bind TCP sockets (default if no binding specified)
-u             # Bind UDP sockets (default if no binding specified)
-p <port>      # Bind to specified port (requires -t or -u)
```

### Comprehensive Examples

#### Basic Interface Operations
```bash
# List all available network interfaces
./forcebindip -l

# Example output:
# Available Network Interfaces:
# ========================================
# Interface Name: wlo1
# Interface ID: wlo1
# Description: Linux Network Interface
# Status: Up
# IPv4 Address: 192.168.1.3
```

#### IPv4 Binding Examples
```bash
# Force curl to use specific IP address
./forcebindip -v -4 192.168.1.100 curl http://httpbin.org/ip

# Force wget to use specific IP
./forcebindip -4 10.0.0.5 wget http://example.com

# Test with sample program
./forcebindip -v -4 192.168.1.3 ./test_ipv4
```

#### Interface Binding Examples
```bash
# Force connection through specific interface
./forcebindip -v -i wlo1 curl http://httpbin.org/ip

# Use ethernet interface
./forcebindip -i eth0 wget http://example.com

# Test with interface binding
./forcebindip -v -i wlo1 ./test_ipv4
```

#### IPv6 Support Examples
```bash
# Bind to IPv6 address (when available)
./forcebindip -6 2001:db8::1 curl -6 http://httpbin.org/ip

# Use IPv6 interface
./forcebindip -i eth0 curl -6 http://example.com
```

#### Advanced Usage Examples
```bash
# Verbose mode for debugging
./forcebindip -v -4 192.168.1.3 ssh user@server.com

# Force specific application through VPN interface
./forcebindip -i tun0 firefox

# Test UDP binding
./forcebindip -v -4 192.168.1.3 dig @8.8.8.8 google.com
```

### Technical Implementation Details

#### Linux Implementation (LD_PRELOAD)
- Uses **LD_PRELOAD** technique to intercept socket functions
- Hooks: `socket()`, `bind()`, `connect()`, `sendto()`, `getsockname()`
- **SO_BINDTODEVICE** for interface binding
- Network interface discovery via `getifaddrs()`

#### Windows Implementation (DLL Injection)
- Uses **DLL injection** with MinHook library
- Hooks WinSock functions: `socket()`, `bind()`, `connect()`, `WSASendTo()`
- Interface binding via IP address resolution

### Supported Applications
The tool works with any dynamically linked application that uses standard socket functions:
- **Web browsers**: Firefox, Chrome (with some limitations)
- **Command-line tools**: curl, wget, ssh, nc, dig
- **Network utilities**: ping, traceroute, nmap
- **Custom applications**: Any program using standard socket APIs

### Complete Troubleshooting Guide

#### Common Issues and Solutions

**1. "Invalid argument" Error**
This usually occurs when trying to bind loopback address (127.0.0.1) to external connections:
```bash
# ❌ This may fail:
./forcebindip -4 127.0.0.1 curl http://google.com

# ✅ Use actual interface IP:
./forcebindip -4 192.168.1.3 curl http://google.com
```

**2. Permission Denied**
Some applications may require elevated privileges:
```bash
sudo ./forcebindip -i eth0 ping google.com
```

**3. IPv6 Link-Local Addresses**
Link-local IPv6 addresses require scope specification:
```bash
# Add %interface to link-local addresses
./forcebindip -6 fe80::1%eth0 curl -6 http://example.com
```

**4. Library Not Found**
Ensure the hook library is built and accessible:
```bash
# Check if library exists
ls -la libforcebindip.so

# Rebuild if necessary
make clean && make
```

### Debugging Procedures

#### Enable Verbose Mode
```bash
./forcebindip -v -4 192.168.1.3 ./test_ipv4
```

#### Check Network Configuration
```bash
# List interfaces
ip addr show

# Check routing
ip route show

# Test connectivity
ping -I 192.168.1.3 google.com
```

#### Manual Testing
```bash
# Test without ForceBindIP
./test_ipv4

# Test with ForceBindIP
./forcebindip -v -4 192.168.1.3 ./test_ipv4
```

### Sample Output Examples

#### Successful IPv4 Binding
```
$ ./forcebindip -v -4 192.168.1.3 ./test_ipv4
Verbose mode enabled
Set preferred IP to IPv4: 192.168.1.3
Executing: ./test_ipv4 
Using hook library: ./libforcebindip.so
ForceBindIP Linux hooks initializing...
Preferred IPv4: 192.168.1.3
Preferred Interface: 
Bind TCP: yes
Bind UDP: yes
Linux hooks initialized successfully
Socket 3 created: domain=2, type=1, protocol=0
Bound to IPv4: 192.168.1.3:0
TCP socket 3 connected from: 192.168.1.3:42019
Connected from local IP: 192.168.1.3
```

#### Interface Listing Output
```
$ ./forcebindip -l
Available Network Interfaces:
========================================

Interface Name: wlo1
Interface ID: wlo1
Description: Linux Network Interface
Status: Up
IPv4 Address: 192.168.1.3

Interface Name: eth0
Interface ID: eth0
Description: Linux Network Interface
Status: Up
IPv4 Address: 10.0.0.5
```

### Development Information

#### Building from Source
```bash
# Clean build
make clean

# Build all components
make

# Build specific targets
make libforcebindip.so
make forcebindip
make test_ipv4
```

#### Testing Procedures
```bash
# Test IPv4 binding
./forcebindip -v -4 192.168.1.3 ./test_ipv4

# Test interface binding
./forcebindip -v -i wlo1 ./test_ipv4

# Test with real applications
./forcebindip -v -4 192.168.1.3 curl http://httpbin.org/ip
```

#### Code Structure Details
```
FBI/
├── build/                           # Build output directory
├── common/                          # Shared headers
│   ├── platform.h                   # Platform abstraction
│   ├── network_manager.h            # Network interface management
│   └── network_manager.cpp
├── dll/                             # Windows DLL source code
├── forcebindip                      # Main Linux executable
├── forcebindip_1.0.0_amd64.deb      # Pre-built Debian package
├── injector/                        # Windows injector source
├── injector_crossplatform/          # Cross-platform injector
│   └── injector.cpp
├── libforcebindip_simple.c          # Linux hook library source (C)
├── libforcebindip.so                # Linux hook library (compiled)
├── Makefile                         # Linux build system
├── README_FA.md                     # Persian documentation
├── README.md                        # English documentation
├── test_crossplatform/              # Cross-platform test programs
│   ├── test_ipv4.cpp
│   └── test_ipv6.cpp
├── test_ipv4                        # IPv4 test executable
├── test_ipv6                        # IPv6 test executable
├── testv4/                          # Original IPv4 test source
│   └── testv4.cpp
├── testv6/                          # Original IPv6 test source
│   └── testv6.cpp
├── build-deb.sh                     # Debian package builder script
├── test-all-usage.sh                # Comprehensive usage tester
└── debian/                          # Debian package control files
    ├── control                      # Package metadata
    ├── postinst                     # Post-installation script
    ├── prerm                        # Pre-removal script
    ├── copyright                    # License information
    ├── changelog                    # Version history
    └── forcebindip.1                # Manual page
```

## Contributing
1. Fork the repository
2. Create feature branch: `git checkout -b feature/my-feature`
3. Test on both platforms
4. Submit pull request

## License
Based on original ForceBindIP project license.

---

**Ready to control your network traffic?**  
Start with `./forcebindip -l` to see your interfaces, then force your apps to use exactly the network path you want!
