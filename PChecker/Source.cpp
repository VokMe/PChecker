#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>
#include <string>
#include <urlmon.h>
#include <wininet.h>
#include <Windows.h>
#include <limits>
#include <fstream>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "wininet.lib")

void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void resetColor() {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
}

std::string GetLocalIP() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return "Error initializing Winsock";
    }

    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        WSACleanup();
        return "Error getting hostname";
    }

    struct hostent* host = gethostbyname(hostname);
    if (host == nullptr) {
        WSACleanup();
        return "Error getting host info";
    }

    std::string ip = inet_ntoa(*(struct in_addr*)*host->h_addr_list);
    WSACleanup();
    return ip;
}

std::string GetPublicIP() {
    HINTERNET net = InternetOpen(L"IP_CHECK", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!net) return "Error opening internet connection";

    HINTERNET conn = InternetOpenUrl(net, L"http://api.ipify.org", NULL, 0, INTERNET_FLAG_RELOAD, 0); // or http://checkip.dyndns.org/
    if (!conn) {
        InternetCloseHandle(net);
        return "Error connecting to IP service";
    }

    char buffer[256] = "";
    DWORD read;
    InternetReadFile(conn, buffer, sizeof(buffer) - 1, &read);
    buffer[read] = '\0';

    InternetCloseHandle(conn);
    InternetCloseHandle(net);
    return std::string(buffer);
}

bool CheckPort(const char* ip, int port) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return false;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        WSACleanup();
        return false;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    int result = connect(sock, (sockaddr*)&addr, sizeof(addr));
    closesocket(sock);
    WSACleanup();

    return result != SOCKET_ERROR;
}

std::string ResolveDNS(const std::string& domain) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return "Error initializing Winsock";
    }

    struct addrinfo hints;
    struct addrinfo* result;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int status = getaddrinfo(domain.c_str(), NULL, &hints, &result);
    if (status != 0) {
        WSACleanup();
        return "Error resolving DNS";
    }

    char ipstr[INET_ADDRSTRLEN];
    void* addr;
    struct sockaddr_in* ipv4 = (struct sockaddr_in*)result->ai_addr;
    addr = &(ipv4->sin_addr);
    inet_ntop(AF_INET, addr, ipstr, INET_ADDRSTRLEN);

    freeaddrinfo(result);
    WSACleanup();
    return std::string(ipstr);
}

void ScanPortRange(const char* ip, int startPort, int endPort, const std::string& filename) {
    std::ofstream file(filename, std::ios::app);
    file << "\nScan Results for " << ip << " (Ports " << startPort << "-" << endPort << "):\n";
    file << "----------------------------------------\n";
    
    for (int port = startPort; port <= endPort; port++) {
        setColor(14);
        std::cout << "\rScanning port " << port << "..." << std::flush;
        bool isOpen = CheckPort(ip, port);
        if (isOpen) {
            file << "Port " << port << ": OPEN\n";
            setColor(10);
            std::cout << "\rPort " << port << ": OPEN              \n";
        }
    }
    file.close();
    setColor(7);
    std::cout << "\nScan complete! Results saved to " << filename << std::endl;
}

int main() {
    SetConsoleTitle(L"IP/Port Checker");
    int choice;
    do {
        setColor(11);
        std::cout << "\n*------------------------*\n";
        std::cout << "    IP/Port Checker\n";
        std::cout << "*------------------------*\n\n";
        
        setColor(14);
        std::cout << "1. ";
        setColor(7);
        std::cout << "Show Local IP\n";
        
        setColor(14);
        std::cout << "2. ";
        setColor(7);
        std::cout << "Show Public IP\n";
        
        setColor(14);
        std::cout << "3. ";
        setColor(7);
        std::cout << "Check Port (using Public IP)\n";
        
        setColor(14);
        std::cout << "4. ";
        setColor(7);
        std::cout << "Scan Port Range\n";
        
        setColor(14);
        std::cout << "5. ";
        setColor(7);
        std::cout << "Resolve Domain Name\n";
        
        setColor(14);
        std::cout << "6. ";
        setColor(7);
        std::cout << "Exit\n\n";
        
        setColor(10);
        std::cout << "Enter your choice: ";
        setColor(7);
        
        if (scanf_s("%d", &choice) != 1) {
            while (getchar() != '\n');
            choice = -1;
            system("cls");
            continue;
        }
        system("cls");

        switch (choice) {
        case 1: {
            setColor(11);
            std::cout << "\n---------------------------\n";
            std::cout << " IP Checker - Local IP Info\n";
            std::cout << "---------------------------\n\n";
            setColor(10);
            std::cout << "Local IP: ";
            setColor(7);
            std::cout << GetLocalIP() << std::endl;
            break;
        }
        case 2: {
            setColor(11);
            std::cout << "\n----------------------------\n";
            std::cout << " IP Checker - Public IP Info\n";
            std::cout << "----------------------------\n\n";
            setColor(10);
            std::cout << "Public IP: ";
            setColor(7);
            std::cout << GetPublicIP() << std::endl;
            break;
        }
        case 3: {
            setColor(11);
            std::cout << "\n-------------------------\n";
            std::cout << " IP Checker - Port Test\n";
            std::cout << "-------------------------\n\n";
            
            int port;
            std::string publicIP = GetPublicIP();
            setColor(10);
            std::cout << "Using Public IP: ";
            setColor(7);
            std::cout << publicIP << std::endl;
            
            setColor(10);
            std::cout << "Enter port number (1-65535): ";
            setColor(7);
            
            if (scanf_s("%d", &port) != 1 || port < 1 || port > 65535) {
                while (getchar() != '\n');
                setColor(12);
                std::cout << "\nInvalid port number! Port must be between 1 and 65535.\n";
                setColor(7);
                system("pause");
                system("cls");
                break;
            }
            
            setColor(11);
            std::cout << "\n-------------------------\n";
            std::cout << " IP Checker - Test Results\n";
            std::cout << "-------------------------\n\n";
            
            setColor(14);
            std::cout << "Testing port " << port << " on " << publicIP << "...\n\n";
            bool isOpen = CheckPort(publicIP.c_str(), port);
            
            setColor(10);
            std::cout << "Status: ";
            if (isOpen) {
                setColor(10);
                std::cout << "OPEN";
            } else {
                setColor(12);
                std::cout << "CLOSED";
            } 
            std::cout << std::endl;
            break;
        }
        case 4: {
            setColor(11);
            std::cout << "\n-------------------------\n";
            std::cout << " IP Checker - Port Range\n";
            std::cout << "-------------------------\n\n";
            
            int startPort, endPort;
            std::string publicIP = GetPublicIP();
            setColor(10);
            std::cout << "Using Public IP: " << publicIP << "\n\n";
            std::cout << "Enter start port (1-65535): ";
            setColor(7);
            
            if (scanf_s("%d", &startPort) != 1 || startPort < 1 || startPort > 65535) {
                while (getchar() != '\n');
                setColor(12);
                std::cout << "\nInvalid port number!\n";
                system("pause");
                system("cls");
                break;
            }
            
            setColor(10);
            std::cout << "Enter end port (1-65535): ";
            setColor(7);
            
            if (scanf_s("%d", &endPort) != 1 || endPort < startPort || endPort > 65535) {
                while (getchar() != '\n');
                setColor(12);
                std::cout << "\nInvalid port range!\n";
                system("pause");
                system("cls");
                break;
            }
            
            std::string filename = "PortResults.txt";
            ScanPortRange(publicIP.c_str(), startPort, endPort, filename);
            system("pause");
            break;
        }
        
        case 5: {
            setColor(11);
            std::cout << "\n---------------------------\n";
            std::cout << " IP Checker - DNS Resolver\n";
            std::cout << "---------------------------\n\n";
            
            char domain[256];
            setColor(10);
            std::cout << "Enter domain name (e.g., google.com): ";
            setColor(7);
            while (getchar() != '\n');
            scanf_s("%255s", domain, (unsigned)sizeof(domain));
            
            std::string ip = ResolveDNS(domain);
            setColor(10);
            std::cout << "\nDomain: ";
            setColor(7);
            std::cout << domain << "\n";
            setColor(10);
            std::cout << "IP Address: ";
            setColor(7);
            std::cout << ip << "\n";
            break;
        }
        
        case 6: {
            break;
        }
        default: {
            setColor(12);
            std::cout << "\n--------------\n";
            std::cout << " Invalid Input\n";
            std::cout << "--------------\n\n";
            setColor(7);
        }
        }
    } while (choice != 6);

    resetColor();
    return 0;
}
