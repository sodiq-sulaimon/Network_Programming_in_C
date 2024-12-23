#include "headers.h"
#include <ctype.h>
#include <stdlib.h>

int main(){
    printf("Configuring local address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *bindAddress;
    getaddrinfo(0, "3000", &hints, &bindAddress);

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

    printf("Waiting for connections...\n");
    while(1) {
        struct sockaddr_storage clientAddress;
        socklen_t clientLen = sizeof(clientAddress);
        SOCKET socketClient = accept(socketListen, (struct sockaddr*) &clientAddress, &clientLen);
        if (!ISVALIDSOCKET(socketClient)) {
            fprintf(stderr, "accept() failed. (%d)\n", GETSOCKETERRNO());
            return 1;
        }

        char addressBuffer[100];
        getnameinfo((struct sockaddr*)&clientAddress, clientLen, addressBuffer, 
                sizeof(addressBuffer), 0, 0, AI_NUMERICHOST);
        printf("New connection from %s\n", addressBuffer);

        int pid = fork();

        if (pid == 0) { //child process
            CLOSESOCKET(socketListen);
            while(1) {
                char read[1024];
                int bytesReceived = recv(socketClient, read, 1024, 0);
                if (bytesReceived < 1) {
                    CLOSESOCKET(socketClient);
                    exit(0);
                }
                for (int j = 0; j < bytesReceived; j++) {
                    read[j] = toupper(read[j]);
                }
                send(socketClient, read, bytesReceived, 0);
            }
        }

    }
    printf("Closing listening socket...\n");
    CLOSESOCKET(socketListen);

    printf("Finished.\n");
    
    return 0;
}
