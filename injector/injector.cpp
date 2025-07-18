#define WIN32_LEAN_AND_MEAN

#include "helper.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#include <tlhelp32.h>
#include <string>
#include <cstring>
#include <algorithm>
#include <cwctype>
#include <string>
#include <conio.h>
#include <iostream>

static bool verbose = false;

bool is_link_local_ipv6(const IN6_ADDR &addr)
{
    return (addr.u.Byte[0] == 0xFE && (addr.u.Byte[1] & 0xC0) == 0x80);
}

bool get_guid_ip()
{
    ULONG outBufLen = 0;
    GetAdaptersAddresses(AF_INET, 0, NULL, NULL, &outBufLen);
    PIP_ADAPTER_ADDRESSES pAddresses = (PIP_ADAPTER_ADDRESSES)malloc(outBufLen);

    if (GetAdaptersAddresses(AF_INET, 0, NULL, pAddresses, &outBufLen) == NO_ERROR)
    {
        PIP_ADAPTER_ADDRESSES pCurrent = pAddresses;
        while (pCurrent)
        {
            if (pCurrent->OperStatus == IfOperStatusUp)
            {
                std::wcout << L"\nInterface Name: " << pCurrent->FriendlyName << std::endl;
                std::wcout << L"Interface GUID: " << pCurrent->AdapterName << std::endl;
                std::wcout << L"Description: " << pCurrent->Description << std::endl;
                std::cout << "Status: Up" << std::endl;

                PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pCurrent->FirstUnicastAddress;
                while (pUnicast)
                {
                    sockaddr_in *addr = (sockaddr_in *)pUnicast->Address.lpSockaddr;
                    char currentIp[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &addr->sin_addr, currentIp, sizeof(currentIp));
                    std::cout << "IP Address: " << currentIp << std::endl;
                    pUnicast = pUnicast->Next;
                }
            }
            pCurrent = pCurrent->Next;
        }
    }

    free(pAddresses);
    return true;
}

bool get_preferred_ip_from_guid(const std::basic_string<TCHAR> &guid_str, std::basic_string<TCHAR> &preferred_ip)
{
    ULONG outBufLen = 15000;
    PIP_ADAPTER_ADDRESSES pAddresses = (IP_ADAPTER_ADDRESSES *)malloc(outBufLen);
    if (!pAddresses)
    {
        _tprintf(_T("Memory allocation failed for adapter addresses\n"));
        return false;
    }

    DWORD dwRetVal = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, pAddresses, &outBufLen);
    if (dwRetVal == ERROR_BUFFER_OVERFLOW)
    {
        free(pAddresses);
        pAddresses = (IP_ADAPTER_ADDRESSES *)malloc(outBufLen);
        if (!pAddresses)
            return false;
        dwRetVal = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, pAddresses, &outBufLen);
    }

    if (dwRetVal != NO_ERROR)
    {
        _tprintf(_T("GetAdaptersAddresses failed with error: %lu\n"), dwRetVal);
        free(pAddresses);
        return false;
    }

    bool found = false;
    std::basic_string<TCHAR> ipv6_addr;
    std::basic_string<TCHAR> ipv4_addr;

    for (PIP_ADAPTER_ADDRESSES pAdapter = pAddresses; pAdapter; pAdapter = pAdapter->Next)
    {
        int len = MultiByteToWideChar(CP_ACP, 0, pAdapter->AdapterName, -1, NULL, 0);
        std::wstring adapterName(len, 0);
        MultiByteToWideChar(CP_ACP, 0, pAdapter->AdapterName, -1, &adapterName[0], len);
        adapterName.resize(len - 1);

        std::transform(adapterName.begin(), adapterName.end(), adapterName.begin(), ::towupper);

        if (adapterName.size() >= 2 && adapterName.front() == L'{' && adapterName.back() == L'}')
        {
            adapterName = adapterName.substr(1, adapterName.size() - 2);
        }

        if (adapterName == guid_str)
        {
            found = true;

            for (PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pAdapter->FirstUnicastAddress; pUnicast; pUnicast = pUnicast->Next)
            {
                if (pUnicast->Address.lpSockaddr->sa_family == AF_INET6)
                {
                    sockaddr_in6 *sa_in6 = (sockaddr_in6 *)pUnicast->Address.lpSockaddr;
                    if (!is_link_local_ipv6(sa_in6->sin6_addr))
                    {
                        WCHAR ip_str[INET6_ADDRSTRLEN];
                        InetNtopW(AF_INET6, &sa_in6->sin6_addr, ip_str, INET6_ADDRSTRLEN);
                        ipv6_addr = ip_str;
                        break;
                    }
                }
                else if (pUnicast->Address.lpSockaddr->sa_family == AF_INET)
                {
                    sockaddr_in *sa_in = (sockaddr_in *)pUnicast->Address.lpSockaddr;
                    WCHAR ip_str[INET_ADDRSTRLEN];
                    InetNtopW(AF_INET, &sa_in->sin_addr, ip_str, INET_ADDRSTRLEN);
                    ipv4_addr = ip_str;
                }
            }
            break;
        }
    }
    free(pAddresses);

    if (!found)
    {
        _tprintf(_T("No adapter found for GUID: %s\n"), guid_str.c_str());
        return false;
    }

    if (!ipv6_addr.empty())
    {
        preferred_ip = _T("IPv6:") + ipv6_addr;
    }
    else if (!ipv4_addr.empty())
    {
        preferred_ip = _T("IPv4:") + ipv4_addr;
    }
    else
    {
        _tprintf(_T("No valid IP found for GUID: %s\n"), guid_str.c_str());
        return false;
    }
    return true;
}

bool inject_dll(HANDLE hProcess, const TCHAR *dllPath)
{
    SIZE_T pathSize = (_tcslen(dllPath) + 1) * sizeof(TCHAR);
    LPVOID remoteMem = VirtualAllocEx(hProcess, NULL, pathSize, MEM_COMMIT, PAGE_READWRITE);
    if (!remoteMem)
    {
        _tprintf(_T("Failed to allocate memory in target process.\n"));
        return false;
    }

    if (!WriteProcessMemory(hProcess, remoteMem, dllPath, pathSize, NULL))
    {
        _tprintf(_T("Failed to write DLL path to target process.\n"));
        VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
        return false;
    }

    HMODULE kernel32 = GetModuleHandle(_T("kernel32"));
    if (!kernel32)
    {
        _tprintf(_T("Failed to get kernel32 module.\n"));
        VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
        return false;
    }
    FARPROC loadLib = GetProcAddress(kernel32, "LoadLibraryW");
    if (!loadLib)
    {
        _tprintf(_T("Failed to get LoadLibrary address.\n"));
        VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
        return false;
    }

    HANDLE thread = CreateRemoteThread(hProcess, NULL, 0,
                                       (LPTHREAD_START_ROUTINE)loadLib, remoteMem, 0, NULL);
    if (!thread)
    {
        _tprintf(_T("Failed to create remote thread.\n"));
        VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
        return false;
    }

    WaitForSingleObject(thread, INFINITE);
    DWORD exitCode = 0;
    GetExitCodeThread(thread, &exitCode);
    CloseHandle(thread);
    VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);

    return exitCode != 0;
}

int _tmain(int argc, TCHAR *argv[])
{
    static bool verbose = false;

    if (argc < 2)
    {
        _tprintf(_T("Usage: injector.exe [options] <program> [args...]\n"));
        _tprintf(_T("Options:\n"));
        _tprintf(_T("  -v               : Enable verbose output in terminal for injector and debug output from DLL\n"));
        _tprintf(_T("  -o <logfile>     : Save DLL logs to the specified file (does not affect terminal output)\n"));
        _tprintf(_T("  -l               : List available network interfaces with GUIDs and IP addresses\n"));
        _tprintf(_T("Injection Methods (choose one):\n"));
        _tprintf(_T("  -4 <IPv4>        : Use specified IPv4 address for binding\n"));
        _tprintf(_T("  -6 <IPv6>        : Use specified IPv6 address for binding\n"));
        _tprintf(_T("  -i <GUID>        : Use the IP address associated with the specified interface GUID\n"));
        _tprintf(_T("Binding Options:\n"));
        _tprintf(_T("  -t               : Bind TCP sockets (default if no binding specified)\n"));
        _tprintf(_T("  -u               : Bind UDP sockets (default if no binding specified)\n"));
        _tprintf(_T("  -p <port>        : Bind to the specified port (requires -t or -u)\n"));
        _tprintf(_T("Note: If neither -t nor -u is specified, both TCP and UDP sockets will be bound by default.\n"));
        return 1;
    }

    bool specify_binding = false;
    bool bind_tcp = false;
    bool bind_udp = false;
    int bind_port = 0;

    std::string injection_type;
    int program_index = -1;
    std::basic_string<TCHAR> preferred_ip;

    for (int i = 1; i < argc;)
    {
        if (_tcscmp(argv[i], _T("-v")) == 0)
        {
            verbose = true;
            SetEnvironmentVariable(_T("VERBOSE"), _T("1"));
            if (verbose)
                _tprintf(_T("Verbose mode enabled\n"));
            i++;
        }
        else if (_tcscmp(argv[i], _T("-l")) == 0)
        {
            get_guid_ip();
            return 1;
        }
        else if (_tcscmp(argv[i], _T("-4")) == 0)
        {
            if (!injection_type.empty())
            {
                _tprintf(_T("Error: Multiple injection methods specified\n"));
                return 1;
            }
            if (i + 1 >= argc || argv[i + 1][0] == _T('-'))
            {
                _tprintf(_T("Error: -4 requires an IPv4 address\n"));
                return 1;
            }
            preferred_ip = _T("IPv4:") + std::basic_string<TCHAR>(argv[i + 1]);
            injection_type = "ipv4";
            if (verbose)
                _tprintf(_T("Set PREFERRED_IP to IPv4: %s\n"), argv[i + 1]);
            i += 2;
        }
        else if (_tcscmp(argv[i], _T("-6")) == 0)
        {
            if (!injection_type.empty())
            {
                _tprintf(_T("Error: Multiple injection methods specified\n"));
                return 1;
            }
            if (i + 1 >= argc || argv[i + 1][0] == _T('-'))
            {
                _tprintf(_T("Error: -6 requires an IPv6 address\n"));
                return 1;
            }
            preferred_ip = _T("IPv6:") + std::basic_string<TCHAR>(argv[i + 1]);
            injection_type = "ipv6";
            if (verbose)
                _tprintf(_T("Set PREFERRED_IP to IPv6: %s\n"), argv[i + 1]);
            i += 2;
        }
        else if (_tcscmp(argv[i], _T("-i")) == 0)
        {
            if (!injection_type.empty())
            {
                _tprintf(_T("Error: Multiple injection methods specified\n"));
                return 1;
            }
            if (i + 1 >= argc || argv[i + 1][0] == _T('-'))
            {
                _tprintf(_T("Error: -i requires a GUID\n"));
                return 1;
            }
            std::basic_string<TCHAR> guid = argv[i + 1];

            if (guid.front() == _T('{') && guid.back() == _T('}'))
            {
                guid = guid.substr(1, guid.length() - 2);
            }
            if (!get_preferred_ip_from_guid(guid, preferred_ip))
            {
                _tprintf(_T("Error: Invalid GUID or no valid IP found\n"));
                return 1;
            }
            injection_type = "interface";
            if (verbose)
                _tprintf(_T("Resolved PREFERRED_IP from GUID: %s\n"), preferred_ip.c_str());
            i += 2;
        }

        else if (_tcscmp(argv[i], _T("-t")) == 0)
        {
            bind_tcp = true;
            specify_binding = true;
            i++;
        }
        else if (_tcscmp(argv[i], _T("-u")) == 0)
        {
            bind_udp = true;
            specify_binding = true;
            i++;
        }
        else if (_tcscmp(argv[i], _T("-p")) == 0)
        {
            if (i + 1 >= argc || argv[i + 1][0] == _T('-'))
            {
                _tprintf(_T("Error: -p requires a port number\n"));
                return 1;
            }
            bind_port = _ttoi(argv[i + 1]);
            if (bind_port <= 0 || bind_port > 65535)
            {
                _tprintf(_T("Error: Invalid port number %s\n"), argv[i + 1]);
                return 1;
            }
            i += 2;
        }
        else if (_tcscmp(argv[i], _T("-o")) == 0)
        {
            if (i + 1 >= argc || argv[i + 1][0] == _T('-'))
            {
                _tprintf(_T("Error: -o requires a log file path\n"));
                return 1;
            }
            SetEnvironmentVariable(_T("LOG_FILE"), argv[i + 1]);
            i += 2;
        }
        else if (argv[i][0] != _T('-'))
        {
            program_index = i;
            break;
        }
        else
        {
            _tprintf(_T("Unknown option: %s\n"), argv[i]);
            return 1;
        }
    }

    if (program_index == -1)
    {
        _tprintf(_T("Error: No program specified\n"));
        return 1;
    }

    if (bind_port != 0 && !specify_binding)
    {
        _tprintf(_T("Error: -p must be used with -t or -u\n"));
        return 1;
    }

    if (!specify_binding)
    {

        SetEnvironmentVariable(_T("BIND_TCP"), _T("1"));
        SetEnvironmentVariable(_T("BIND_UDP"), _T("1"));
    }
    else
    {
        SetEnvironmentVariable(_T("BIND_TCP"), bind_tcp ? _T("1") : _T("0"));
        SetEnvironmentVariable(_T("BIND_UDP"), bind_udp ? _T("1") : _T("0"));
    }

    if (bind_port != 0)
    {
        TCHAR port_str[6];
        _itot_s(bind_port, port_str, 10);
        SetEnvironmentVariable(_T("BIND_PORT"), port_str);
    }

    if (!preferred_ip.empty())
    {
        SetEnvironmentVariable(_T("PREFERRED_IP"), preferred_ip.c_str());
    }

    std::basic_string<TCHAR> cmdline;
    for (int i = program_index; i < argc; i++)
    {
        if (i > program_index)
            cmdline += _T(" ");
        cmdline += argv[i];
    }
    if (verbose)
        _tprintf(_T("Command line: %s\n"), cmdline.c_str());

    TCHAR dllPath[MAX_PATH];
    GetModuleFileName(NULL, dllPath, MAX_PATH);
    TCHAR *dirEnd = _tcsrchr(dllPath, _T('\\'));
    if (dirEnd)
    {
        *dirEnd = _T('\0');
        _tcscat_s(dllPath, MAX_PATH, _T("\\forcebindipdllx86.dll"));
        //_tcscat_s(dllPath, MAX_PATH, _T("\\forcebindipdllx64.dll"));
        if (verbose)
            _tprintf(_T("DLL path: %s\n"), dllPath);
    }
    else
    {
        _tprintf(_T("Failed to determine DLL path\n"));
        return 1;
    }

    STARTUPINFO si = {sizeof(si)};
    PROCESS_INFORMATION pi;
    if (!CreateProcess(NULL, &cmdline[0], NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi))
    {
        _tprintf(_T("Failed to create process: %s\n"), cmdline.c_str());
        return 1;
    }
    if (verbose)
        _tprintf(_T("Process created with PID: %lu\n"), pi.dwProcessId);

    if (!inject_dll(pi.hProcess, dllPath))
    {
        _tprintf(_T("DLL injection failed\n"));
        TerminateProcess(pi.hProcess, 1);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return 1;
    }
    if (verbose)
        _tprintf(_T("DLL injected into PID: %lu\n"), pi.dwProcessId);

    ResumeThread(pi.hThread);
    if (verbose)
        _tprintf(_T("Thread resumed\n"));
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (injection_type == "ipv4")
    {
        _tprintf(_T("DLL injected successfully with IPv4: %s\n"), argv[2]);
    }
    else if (injection_type == "ipv6")
    {
        _tprintf(_T("DLL injected successfully with IPv6: %s\n"), argv[2]);
    }
    else if (injection_type == "interface")
    {
        _tprintf(_T("DLL injected successfully with interface IP: %s\n"), preferred_ip.c_str());
    }
    else
    {
        _tprintf(_T("DLL injected successfully with default settings\n"));
    }

    return 0;
}
/* TODO here

-t Bind all TCP sockets.[DONE]
-u Bind all UDP sockets.[DONE]
-p Specify a custom port to bind to the socket.[DONE]
-l Provide a list of GUIDs with IP addresses [DONE].

when using `-p`, you must also specify either `-t` or `-u` to define the socket type (TCP or UDP)
or the injector will automatically detect the port's socket type


long Term TODO

We need to add a mechanism for certain applications, like FDM in my case now,
to hook into the injector and allow it to exit cleanly. This mechanism should perform the following steps:
inject the necessary data.
validate the injection.
exit the injector.

This would enforce the target application to use the specified IP or GUID interface.
However, there’s an issue: if we force the target application to use a particular IP or interface,
and something happens (e.g., the IP or interface goes down and the status shows as "Down"), we need to handle this situation gracefully.
To address this, we plan to introduce an option like -k for a kill switch. When the IP or interface goes down,
this will force the application to stop using any other IPs or interfaces.

*/
