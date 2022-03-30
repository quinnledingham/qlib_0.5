#ifndef SOCKETQ_H
#define SOCKETQ_H

#define TCP 0
#define UDP 1

#define CLIENT 0
#define SERVER 1

#define HEADER_BUFFER_SIZE 10

struct Server
{
    struct addrinfo *info;
    int sock;
    int sockClient;
    int protocol;
    
    void create(char* port, int protocol);
    void waitForConnection();
    void recvq(char* buffer,int bufferSize);
    void sendq(char* buffer, int bufferSize);
};


struct Client
{
    struct addrinfo *info;
    int sock;
    int protocol;
    
    void create(char* ip, char* port, int protocol);
    void recvq(char* buffer, int bufferSize);
    void sendq(char* buffer, int bufferSize);
};

#endif //SOCKETQ_H
