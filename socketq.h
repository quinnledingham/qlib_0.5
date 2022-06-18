/*
UDP doesn't work on windows. It does not like how the ips are being worked out.
*/

#ifndef SOCKETQ_H
#define SOCKETQ_H

#define CLIENT 0
#define SERVER 1

#define TCP 0
#define UDP 1

#define HEADER_SIZE 20

#include "strinq.h"

struct server
{
    struct addrinfo *Info;
    int Sock;
    int Protocol;
};

struct client
{
    struct addrinfo *Info;
    int Sock;
    int Protocol;
};

#if defined(_WIN32)

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

internal int
Win32Timeout(int Sock)
{
    // Waits for a recieve on sock. if none it returns 0.
    fd_set fds;
    int n;
    struct timeval tv;
    
    FD_ZERO(&fds);
    FD_SET(Sock, &fds);
    
    tv.tv_sec = 0;
    tv.tv_usec = 100000;
    
    n = select(Sock, &fds, NULL, NULL, &tv);
    if (n == 0)
        return 1; // Timeout
    else if (n < 0) {
        PrintqDebug(S() + "Timeout(): select() call failed!\n");
        return 0; // Timeout Failed
    }
    
    return 0; //No Timeout
}
inline int Timeout(int Sock) { return Win32Timeout(Sock); }

inline int Win32Recv(int Sock,  char* Buffer, int BufferSize, int Flags) { return recv(Sock, Buffer, BufferSize, Flags); }
inline int Recv(int Sock, char* Buffer, int BufferSize, int Flags) { return Win32Recv(Sock, Buffer, BufferSize, Flags); }

inline int Win32RecvFrom(int Sock, char* Buffer, int BufferSize, int Flags, addrinfo *Info) { return recvfrom(Sock, Buffer, BufferSize, Flags, Info->ai_addr, reinterpret_cast<int*>(&Info->ai_addrlen)); }
inline int RecvFrom(int Sock, char* Buffer, int BufferSize, int Flags, addrinfo *Info) { return Win32RecvFrom(Sock, Buffer, BufferSize, Flags, Info); }

inline int Win32Send(int Sock, const char* Buffer, int BytesToSend, int Flags) { return send(Sock, Buffer, BytesToSend, 0); }
inline int Send(int Sock, const char* Buffer, int BytesToSend, int Flags) { return Win32Send(Sock, Buffer, BytesToSend, 0); }

inline int Win32SendTo(int Sock, const char* Buffer, int BytesToSend, int Flags, addrinfo *Info) { return sendto(Sock, Buffer, BytesToSend, Flags, Info->ai_addr, static_cast<int>(Info->ai_addrlen)); }
inline int SendTo(int Sock, const char* Buffer, int BytesToSend, int Flags, addrinfo *Info) { return Win32SendTo(Sock, Buffer, BytesToSend, Flags, Info); }

internal void
Win32CreateClient(client *Client, const char *IP, const char *Port, int Protocol)
{
    Client->Protocol = Protocol;
    
    // GetHost
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
    
    if (Client->Protocol == TCP) {
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
    }
    else if (Client->Protocol == UDP) {
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_protocol = IPPROTO_UDP;
    }
    
    hints.ai_flags = AI_PASSIVE;
    getaddrinfo(IP, Port, &hints, &server_info);
    
    Client->Info = server_info;
    // End of GetHost
    
    // Socket, Connect
    Client->Sock = (int)socket(Client->Info->ai_family, Client->Info->ai_socktype, Client->Info->ai_protocol);
    if (Client->Sock >= 0) {
        if (Client->Protocol == TCP) {
            int ConnectResult = connect(Client->Sock, Client->Info->ai_addr, (int)Client->Info->ai_addrlen);
            if (ConnectResult >= 0) {
                // Client Created;
            } 
            else {
                PrintqDebug(S() + "CreateClient(): connect() call failed! errno: " + errno +", WSA: " + WSAGetLastError() + "\n");
            }
        } 
    }
    else {
        PrintqDebug(S() + "CreateClient(): socket() called failed! errno: " + errno +", WSA: " + WSAGetLastError() + "\n");
    } 
}
inline void SocketqInit(client *Client, const char *IP, const char *Port, int Protocol) { Win32CreateClient(Client, IP, Port, Protocol); };

internal void
Win32CreateServer(server *Server, const char *Port, int Protocol)
{
    Server->Protocol = Protocol;
    
    // AddressInit
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
    
    if (Server->Protocol == TCP) {
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
    }
    else if (Server->Protocol == UDP) {
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_protocol = IPPROTO_UDP;
    }
    
    hints.ai_flags = AI_PASSIVE;
    getaddrinfo(NULL, Port, &hints, &server_info);
    
    Server->Info = server_info;
    // End of AddressInit
    
    // Socket, Bind, Listen
    Server->Sock = (int)socket(Server->Info->ai_family, Server->Info->ai_socktype, Server->Info->ai_protocol);
    if (Server->Sock >= 0) {
        int BindResult = bind(Server->Sock, Server->Info->ai_addr, (int)Server->Info->ai_addrlen);
        if (BindResult >= 0) {
            if (Server->Protocol == TCP) {
                int ListenResult = listen(Server->Sock, 5);
                if (ListenResult >= 0) {
                    // Server Created;
                } 
                else {
                    PrintqDebug(S() + "CreateServer(): listen() call failed! errno: " + errno +", WSA: " + WSAGetLastError() + "\n");
                }
            } 
        } 
        else {
            PrintqDebug(S() + "CreateServer(): bind() call failed! errno: " + errno +", WSA: " + WSAGetLastError() + "\n");
        }
    }
    else {
        PrintqDebug(S() + "CreateServer(): socket() called failed! errno: " + errno +", WSA: " + WSAGetLastError() + "\n");
    } 
}
inline void SocketqInit(server *Server, const char *Port, int Protocol) { Win32CreateServer(Server, Port, Protocol); }

inline int SocketqWaitForConnection(server *Server) 
{
    int newsock = (int)accept(Server->Sock, Server->Info->ai_addr, reinterpret_cast<int*>(&Server->Info->ai_addrlen));
    if (newsock == 1) {
        PrintqDebug(S() + "WaitForConnection(): accept() called failed! errno: " + errno +", WSA: " + WSAGetLastError() + "\n");
    }
    return newsock;
}

#elif __linux__

#endif

internal int
RecvBuffer(int Sock, struct addrinfo *Info, int Protocol, int Type, char* Buffer, int BufferSize)
{
    char *Cursor = Buffer;
    int BytesRecd = 0;
    int BytesRecdTotal = 0;
    int RecvBufferSize = 0;
    
    while(1) {
        if (Protocol == TCP) {
            if (Timeout(Sock)) {
                PrintqDebug(S() + "Timeout! RecvBuffer (TCP).\n");
                return BytesRecdTotal;
            }
            
            PrintqDebug(S() + "Waiting for message (TCP).\n");
            BytesRecd = Recv(Sock, Cursor, BufferSize, 0);
            if (BytesRecd < 0) {
                PrintqDebug(S() + "RecvBuffer(): recv() called failed! errno: " + errno +", WSA: " + WSAGetLastError() + "\n");
                return BytesRecdTotal;
            }
            else if (BytesRecd == 0) {
                PrintqDebug(S() + "RecvBuffer(): recv() no bytes. errno: " + errno +", WSA: " + WSAGetLastError() + "\n");
                return BytesRecdTotal;
            }
            
            // Put HEADER_BUFFER_SIZE into Buffer and then copy bytes received
            if (BytesRecdTotal == 0) {
                char Temp[HEADER_SIZE];
                memcpy(Temp, Buffer, HEADER_SIZE);
                RecvBufferSize = atoi(Temp);
                BytesRecd = BytesRecd - HEADER_SIZE;
                Cursor = Cursor + HEADER_SIZE;
                memcpy(Buffer, Cursor, BytesRecd);
            }
            else {
                memcpy(Buffer, Cursor, BytesRecd);
            }
            
            BytesRecdTotal += BytesRecd;
            Cursor += BytesRecd;
            
            if (BytesRecdTotal == RecvBufferSize)
                break;
        }
        else if (Protocol == UDP) {
            if (Type == CLIENT) {
                if (Timeout(Sock)) {
                    PrintqDebug(S() + "Timeout! RecvBuffer (UDP).\n");
                    return 0;
                }
            }
            PrintqDebug(S() + "Waiting for message (UDP).\n");
            BytesRecd = RecvFrom(Sock, Buffer, BufferSize, 0, Info);
            if (BytesRecd < 0) {
                PrintqDebug(S() + "RecvBuffer(): recvfrom() called failed! errno: " + errno + ", WSA: " + WSAGetLastError() + "\n");
                return 0;
            }
            BytesRecdTotal = BytesRecd;
            break;
        }
    }
    
    return BytesRecdTotal;
}
inline int SocketqRecv(client *Client, char *Buffer, int BufferSize) { return RecvBuffer(Client->Sock, Client->Info, Client->Protocol, CLIENT, Buffer, BufferSize); }
inline int SocketqRecv(server *Server, char *Buffer, int BufferSize)
{
    if (Server->Protocol != UDP) {
        PrintqDebug("UDP Recv looks like Recv(server, const char*, int)\n");
        return 0;
    }
    return RecvBuffer(Server->Sock, Server->Info, Server->Protocol, SERVER, Buffer, BufferSize);
}
inline int SocketqRecv(server *Server, int ClientSock, char *Buffer, int BufferSize)
{
    if (Server->Protocol != TCP) {
        PrintqDebug("TCP Recv looks like Recv(server, int, const char*, int)\n");
        return 0;
    }
    return RecvBuffer(ClientSock, Server->Info, Server->Protocol, SERVER, Buffer, BufferSize);
}

internal int
SendBuffer(int Sock, struct addrinfo *Info, int Protocol, const char *Buffer, int BufferSize)
{
    char Header[HEADER_SIZE];
    sprintf(Header, "%d", BufferSize);
    
    char* HeaderBuffer = (char*)malloc(sizeof(char) * (HEADER_SIZE + BufferSize));
    memset(HeaderBuffer, 0, sizeof(char) * (HEADER_SIZE + BufferSize));
    
    char* Cursor = HeaderBuffer;
    memcpy(Cursor, Header, HEADER_SIZE);
    Cursor = Cursor + HEADER_SIZE;
    memcpy(Cursor, Buffer, BufferSize);
    
    Cursor = HeaderBuffer;
    
    int BytesToSend = HEADER_SIZE + BufferSize;
    int BytesSent = 0;
    
    while (BytesToSend > 0) {
        if (Protocol == TCP) {
            PrintqDebug(S() + "Sending message (TCP).\n");
            BytesSent = Send(Sock, Cursor, BytesToSend, 0);
            if (BytesSent < 0) {
                PrintqDebug(S() + "SendBuffer(): send() called failed! errno: " + errno +", WSA: " + WSAGetLastError() + "\n");
                free(HeaderBuffer);
                return 0;
            }
        }
        else if (Protocol == UDP) {
            PrintqDebug(S() + "Sending message (UDP).\n");
            BytesSent = SendTo(Sock, Cursor, BytesToSend, 0, Info);
            if (BytesSent < 0) {
                PrintqDebug(S() + "SendBuffer(): sendto() called failed! errno: " + errno +", WSA: " + WSAGetLastError() + "\n");
                free(HeaderBuffer);
                return 0;
            }
        }
        
        Cursor = Cursor + BytesSent;
        BytesToSend = BytesToSend - BytesSent;
    }
    
    free(HeaderBuffer);
    return BytesSent;
}
inline void SocketqSend(client *Client, const char *Buffer, int BufferSize) { SendBuffer(Client->Sock, Client->Info, Client->Protocol, Buffer, BufferSize); }
inline void SocketqSend(server *Server, const char *Buffer, int BufferSize) 
{
    if (Server->Protocol != UDP) {
        PrintqDebug("UDP Send looks like Send(server, const char*, int)\n");
        return;
    }
    SendBuffer(Server->Sock, Server->Info, Server->Protocol, Buffer, BufferSize);
} 
inline void SocketqSend(server *Server, int ClientSock, const char* Buffer, int BufferSize)
{
    if (Server->Protocol != TCP) {
        PrintqDebug("TCP Send looks like Send(server, int, const char*, int)\n");
        return;
    }
    SendBuffer(ClientSock, Server->Info, Server->Protocol, Buffer, BufferSize);
}

#endif //SOCKETQ_H