#include "headers.h"
#include <ctype.h>

int main(){
    printf("Configuring local address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *bindAddress;
    getaddrinfo(0, "8080", &hints, &bindAddress);

    printf("Creating socket...\n");
    SOCKET socketListen;
    socketListen = socket(bindAddress->ai_family, bindAddress->ai_socktype, bindAddress->ai_protocol);
    if (!ISVALIDSOCKET(socketListen)) {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    printf("Binding socket to local address...\n");
    if (bind(socketListen, bindAddress->ai_addr, bindAddress->ai_addrlen)) {
        fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }
    freeaddrinfo(bindAddress);

    printf("Listening...\n");
    if(listen(socketListen, 10) < 0) {
        fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    fd_set master;
    FD_ZERO(&master);
    FD_SET(socketListen, &master);
    SOCKET maxSocket = socketListen;

    printf("Waiting for connections...\n");
    while(1) {
        fd_set reads;
        reads = master;
        if (select(maxSocket+1, &reads, 0, 0, 0) < 0) {
            fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
            return 1;
        }

        for (SOCKET i = 1; i <= maxSocket; i++) {
            if (FD_ISSET(i, &reads)) {
                if (i == socketListen) {
                    struct sockaddr_storage clientAddress;
                    socklen_t clientLen = sizeof(clientAddress);
                    SOCKET socketClient = accept(socketListen, (struct sockaddr*) &clientAddress, &clientLen);
                    if (!ISVALIDSOCKET(socketClient)) {
                        fprintf(stderr, "accept() failed. (%d)\n", GETSOCKETERRNO());
                        return 1;
                    }
                    FD_SET(socketClient, &master);
                    if(maxSocket < socketClient) {
                        maxSocket = socketClient;
                    }
                    char addressBuffer[100];
                    getnameinfo((struct sockaddr*)&clientAddress, clientLen, addressBuffer, 
                    sizeof(addressBuffer), 0, 0, AI_NUMERICHOST);
                    printf("New connection from %s\n", addressBuffer);
                } else {
                    char read[1024];
                    int bytesReceived = recv(i, read, 1024, 0);
                    if (bytesReceived < 1) {
                        FD_CLR(i, &master);
                        CLOSESOCKET(i);
                        continue;
                    }

                    for (int j = 0; j < bytesReceived; j++) {
                        read[j] = toupper(read[j]);
                    }
                    send(i, read, bytesReceived, 0);
                }
            }
        }
    }
    printf("Closing listening socket...\n");
    CLOSESOCKET(socketListen);

    printf("Finished.\n");
    
    return 0;
}
