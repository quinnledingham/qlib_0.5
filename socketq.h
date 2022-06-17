/*
UDP doesn't work on windows. It does not like how the ips are being worked out.
*/

#ifndef SOCKETQ_H
#define SOCKETQ_H

#define CLIENT 0
#define SERVER 1

#define HEADER_BUFFER_SIZE 20

struct Server
{
    struct addrinfo *info;
    int sock;
    //int nextSockClient = 0;
    //int sockClient[5];
    int protocol;
    
    void create(const char* port, int protocol);
    int waitForConnection();
    void disconnect();
    int recvq(int i, char* buffer,int bufferSize);
    void sendq(int i, char* buffer, int bufferSize);
};


struct Client
{
    struct addrinfo *info;
    int sock;
    int protocol;
    
    void create(const char* ip, const char* port, int protocol);
    int recvq(char* buffer, int bufferSize);
    void sendq(char* buffer, int bufferSize);
    void disconnect();
};

#endif //SOCKETQ_H