#include "socketq.h"


#if defined(_WIN32)

#include "win32_socketq.cpp"

#elif __linux__

#include "linux_socketq.cpp"

#endif

internal int
recvBuffer(int sock, struct addrinfo *info, int protocol, int type, char* buffer, int bufferSize)
{
    char* cursor = buffer;
    
    int bytesRecd;
    int bytesRecdTotal = 0;
    
    int recvBufferSize = 0;
    
    while(1)
    {
        if (protocol == TCP)
        {
            printf("Waiting for message (TCP).\n");
            bytesRecd = recvPlatform(sock, cursor, bufferSize, 0);
            
            if (bytesRecd < 0)
            {
                fprintf(stderr, "%d\n", errno);
                fprintf(stderr, "recvBuffer(): recv() call failed!\n");
                exit(1);
            }
            
            if (bytesRecdTotal == 0)
            {
                char temp[HEADER_BUFFER_SIZE];
                memcpy(temp, buffer, HEADER_BUFFER_SIZE);
                recvBufferSize = atoi(temp);
                bytesRecd = bytesRecd - HEADER_BUFFER_SIZE;
                
                cursor = cursor + HEADER_BUFFER_SIZE;
                memcpy(buffer, cursor, bytesRecd);
            }
            
            
            bytesRecdTotal = bytesRecdTotal + bytesRecd;
            cursor = cursor + bytesRecd;
            
            if (bytesRecdTotal == recvBufferSize)
            {
                break;
            }
        }
        else if (protocol == UDP)
        {
            if (type == CLIENT)
            {
                timeout(sock);
            }
            
            printf("Waiting for message (UDP).");
            bytesRecd = recvfromPlatform(sock, buffer, bufferSize, 0, info);
            if (bytesRecd < 0)
            {
                fprintf(stderr, "recvBuffer(): recvfrom() call failed!\n");
                return 0;
            }
            
            bytesRecdTotal = bytesRecd;
            break;
        }
    }
    
    return bytesRecdTotal;
}

internal int
sendBuffer(int sock, struct addrinfo *info, int protocol, char* buffer, int bufferSize)
{
    char header[HEADER_BUFFER_SIZE];
    sprintf(header, "%d", bufferSize);
    
    char* header_buffer = (char*)malloc(sizeof(char) * (HEADER_BUFFER_SIZE + bufferSize));
    memset(header_buffer, 0, sizeof(char) * (HEADER_BUFFER_SIZE + bufferSize));
    
    char* cursor = header_buffer;
    memcpy(cursor, header, HEADER_BUFFER_SIZE);
    cursor = cursor + HEADER_BUFFER_SIZE;
    memcpy(cursor, buffer, bufferSize);
    
    cursor = header_buffer;
    
    int bytesToSend = HEADER_BUFFER_SIZE + bufferSize;
    int bytesSent = 0;
    
    while (bytesToSend > 0)
    {
        if (protocol == TCP)
        {
            printf("Sending message (TCP).\n");
            bytesSent = sendPlatform(sock, cursor, bytesToSend, 0);
            if (bytesSent < 0)
            {
                fprintf(stderr, "%d\n", errno);
                fprintf(stderr, "sendBuffer(): send() call failed\n");
                exit(1);
            }
        }
        else if (protocol == UDP)
        {
            printf("Sending message (UDP).");
            bytesSent = sendtoPlatform(sock, cursor, bytesToSend, 0, info);
            if (bytesSent < 0)
            {
                fprintf(stderr, "sendBuffer(): sendto() call failed!\n");
                exit(1);
            }
        }
        
        cursor = cursor + bytesSent;
        bytesToSend = bytesToSend - bytesSent;
    }
    
    return bytesSent;
}

void
Server::create(const char* por, int proto)
{
    protocol = proto;
    info = addressInit(por, protocol);
    sock = socketq(*info);
    bindq(sock, *info);
    
    if (protocol == TCP)
        listenq(sock);
}

void
Server::waitForConnection()
{
    sockClient[nextSockClient++] = acceptq(sock, *info);
}

void
Server::recvq(int i, char* buffer, int bufferSize)
{
    if (protocol == TCP)
        recvBuffer(sockClient[i], info, protocol, SERVER, buffer, bufferSize);
    else if (protocol == UDP)
        recvBuffer(sock, info, protocol, SERVER, buffer, bufferSize);
}

void
Server::sendq(int i, char* buffer, int bufferSize)
{
    if (protocol == TCP)
        sendBuffer(sockClient[i], info, protocol, buffer, bufferSize);
    else if (protocol == UDP)
        sendBuffer(sock, info, protocol, buffer, bufferSize);
}

void
Client::create(const char* ip, const char* por, int proto)
{
    protocol = proto;
    info = getHost(ip, por, proto);
    sock = socketq(*info);
    
    if (protocol == TCP)
    {
        connectq(sock, *info);
    }
}

void
Client::recvq(char* buffer, int bufferSize)
{
    recvBuffer(sock, info, protocol, CLIENT, buffer, bufferSize);
}

void
Client::sendq(char* buffer, int bufferSize)
{
    sendBuffer(sock, info, protocol, buffer, bufferSize);
}