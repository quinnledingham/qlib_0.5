#include <sys/socket.h> 
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <netdb.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "linux_socketq.h"

#include "socketq.h"

internal int
timeout(int sock)
{
    // Waits for a recieve on sock. if none it returns 0.
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;
    int t = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv));
    if (t < 0) {
        perror("Error\n");
    }
    fprintf(stderr, "\ntimeout: %d\n", t);
    return t;
}

internal int
recvPlatform(int sock, char* buffer, int bufferSize, int flags)
{
    int bytes = recv(sock, buffer, bufferSize, flags);
    return bytes;
}

internal int
recvfromPlatform(int sock, char* buffer, int bufferSize, int flags, struct addrinfo *info)
{
    int bytes = recvfrom(sock, buffer, bufferSize, flags,
                         info->ai_addr,
                         &info->ai_addrlen);
    return bytes;
}

internal int
sendPlatform(int sock, char* buffer, int bytesToSend, int flags)
{
    return send(sock, buffer, bytesToSend, 0);
}

internal int
sendtoPlatform(int sock, char* buffer, int bytesToSend, int flags, struct addrinfo *info)
{
    return sendto(sock, buffer, bytesToSend, 0,
                  info->ai_addr, info->ai_addrlen);
}

internal addrinfo* 
getHost(char* ip, char* port, int type)
{
    struct addrinfo hints, *server_info;
    server_info = (struct addrinfo*)malloc(sizeof(struct addrinfo));
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    
    if (type == TCP)
        hints.ai_socktype = SOCK_STREAM;
    else if (type == UDP)
        hints.ai_socktype = SOCK_DGRAM;
    
    getaddrinfo(ip, port, &hints, &server_info);
    
    return server_info;
}

internal addrinfo*
addressInit(char* port, int type)
{
    struct addrinfo hints, *server_info;
    server_info = (struct addrinfo*)malloc(sizeof(struct addrinfo));
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    
    if (type == TCP)
        hints.ai_socktype = SOCK_STREAM;
    else if (type == UDP)
        hints.ai_socktype = SOCK_DGRAM;
    
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    getaddrinfo(NULL, port, &hints, &server_info);
    return server_info;
}

internal int socketq(struct addrinfo server_info)
{
    int sock = socket(server_info.ai_family, 
                      server_info.ai_socktype,
                      server_info.ai_protocol);
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
    if (connect(sock, server_info.ai_addr, server_info.ai_addrlen) == -1)
    {
        fprintf(stderr, "connectM(): connect() call failed!\n");
    }
}

internal void
bindq(int sock, struct addrinfo server_info)
{
    if (bind(sock, server_info.ai_addr, server_info.ai_addrlen) == -1)
    {
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
    if ((newsock = accept(sock, server_info.ai_addr,
                          &server_info.ai_addrlen)) == 1)
    {
        fprintf(stderr, "acceptq(): accept() call failed!\n");
        exit(1);
    }
    return newsock;
}