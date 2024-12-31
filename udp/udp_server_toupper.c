#include "headers.h"
#include <ctype.h>

int main() {
    printf("Configuring local address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *bindAddress;
    getaddrinfo(0, "5000", &hints, &bindAddress);

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

        if (FD_ISSET(socketListen, &reads)) {
            struct sockaddr_storage clientAddress;
            socklen_t clientLen = sizeof(clientAddress);
            
            char read[1024];
            int bytesReceived = recvfrom(socketListen, read, 1024, 0, (struct sockaddr*)&clientAddress, &clientLen);
            if (bytesReceived < 1) {
                fprintf(stderr, "Connection closed. (%d)\n", GETSOCKETERRNO());
                return 1;
            }
            printf("Received (%d bytes): %.*s\n", bytesReceived, bytesReceived, read);

            for (int j = 0; j < bytesReceived; j++) {
                read[j] = toupper(read[j]);
            }
            sendto(socketListen, read, bytesReceived, 0, (struct sockaddr*)&clientAddress, clientLen);

            printf("Remote address is: ");
            char addrBuffer[100];
            char serviceBuffer[100];
            getnameinfo((struct sockaddr*) &clientAddress, clientLen, addrBuffer, sizeof(addrBuffer), 
                serviceBuffer, sizeof(serviceBuffer), NI_NUMERICHOST | NI_NUMERICSERV);
            printf("%s %s\n", addrBuffer, serviceBuffer);
        }
    }
    
    printf("Closing listening socket...\n");
    CLOSESOCKET(socketListen);

    printf("Finished.\n");
    
    return 0;
}
