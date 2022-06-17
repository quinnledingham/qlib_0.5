//#include <windows.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

internal int
timeout(int sock)
{
    // Waits for a recieve on sock. if none it returns 0.
    fd_set fds;
    int n;
    struct timeval tv;
    
    FD_ZERO(&fds);
    FD_SET(sock, &fds);
    
    tv.tv_sec = 0;
    tv.tv_usec = 100000;
    
    n = select(sock, &fds, NULL, NULL, &tv);
    if (n == 0)
    {
        fprintf(stderr, "Timeout.. (UDP)\n");
        return 0;
    }
    else if (n < 0)
    {
        fprintf(stderr, "timeout(): select() call failed!\n");
        return 1;
    }
    
    return 1;
}

global_variable sockaddr_in Addr;
global_variable SOCKET Socket;

internal int
recvPlatform(int sock, char* buffer, int bufferSize, int flags)
{
    return recv(sock, buffer, bufferSize, flags);
}

internal int
recvfromPlatform(int sock, char* buffer, int bufferSize, int flags, struct addrinfo *info)
{
    //return recvfrom(sock, buffer, bufferSize, flags, info->ai_addr, reinterpret_cast<int*>(&info->ai_addrlen));
    return recvfrom(Socket, buffer, bufferSize, flags, (SOCKADDR *) &Addr, reinterpret_cast<int*>(sizeof(Addr)));
}

internal int
sendPlatform(int sock, char* buffer, int bytesToSend, int flags)
{
    return send(sock, buffer, bytesToSend, 0);
}

internal int
sendtoPlatform(int sock, char* buffer, int bytesToSend, int flags, struct addrinfo *info)
{
    //return sendto(sock, buffer, bytesToSend, flags, info->ai_addr, static_cast<int>(info->ai_addrlen));
    return sendto(Socket, buffer, bytesToSend, flags, (sockaddr *) &Addr, sizeof(Addr));
}

internal addrinfo*
getHost(const char *ip, const char* port, int type)
{
    // Initialize Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
    }
    
    addrinfo hints, *server_info;
    server_info = (addrinfo*)malloc(sizeof(addrinfo));
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    
    if (type == TCP) {
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
    }
    else if (type == UDP) {
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_protocol = IPPROTO_UDP;
    }
    
    hints.ai_flags = AI_PASSIVE;
    getaddrinfo(ip, port, &hints, &server_info);
    
    return server_info;
}

internal addrinfo*
addressInit(const char* port, int type)
{
    // Initialize Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
    }
    
    addrinfo hints, *server_info;
    server_info = (addrinfo*)malloc(sizeof(addrinfo));
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_INET;
    
    if (type == TCP) {
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
    }
    else if (type == UDP) {
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_protocol = IPPROTO_UDP;
    }
    
    hints.ai_flags = AI_PASSIVE;
    getaddrinfo(NULL, port, &hints, &server_info);
    
    return server_info;
} 

internal int socketq(struct addrinfo server_info)
{
    int sock = (int)socket(server_info.ai_family, server_info.ai_socktype, server_info.ai_protocol);
    if (sock < 0)
    {
        fprintf(stderr, "socketM(): socket() called failed!\n");
        exit(1);
    }
    return sock;
}

internal void
connectq(int sock, struct addrinfo server_info)
{
    if (connect(sock, server_info.ai_addr, (int)server_info.ai_addrlen) == -1)
    {
        fprintf(stderr, "connectM(): connect() call failed!\n");
    }
}

internal void
bindq(int sock, struct addrinfo server_info)
{
    if (bind(sock, server_info.ai_addr, (int)server_info.ai_addrlen) == -1)
    {
        fprintf(stderr, "%d %d\n", errno, WSAGetLastError());
        fprintf(stderr, "bindq(): bind() call failed!\n");
    }
}

internal void 
listenq(int sock)
{
    if (listen(sock, 5) == -1)
    {
        fprintf(stderr, "listenq(): listen() call failed!\n");
        exit(1);
    }
}

internal int 
acceptq(int sock, struct addrinfo server_info)
{
    int newsock;
    if ((newsock = (int)accept(sock, server_info.ai_addr,
                               reinterpret_cast<int*>(&server_info.ai_addrlen))) == 1)
    {
        fprintf(stderr, "acceptq(): accept() call failed!\n");
        exit(1);
    }
    return newsock;
}
