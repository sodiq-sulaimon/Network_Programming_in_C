#include "headers.h"

int main() {
    printf("Configuring local address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family = AF_INET;
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

    while(1) {
        struct sockaddr_storage clientAddress;
        socklen_t clientLen = sizeof(clientAddress);
        char read[1024];
        int byteReceived = recvfrom(socketListen, read, 1024, 0, (struct sockaddr*) &clientAddress, &clientLen);
        printf("Received (%d bytes): %.*s\n", byteReceived, byteReceived, read);
        printf("Remote address is: ");
        char addrBuffer[100];
        char serviceBuffer[100];
        getnameinfo((struct sockaddr*) &clientAddress, clientLen, addrBuffer, sizeof(addrBuffer), 
            serviceBuffer, sizeof(serviceBuffer), NI_NUMERICHOST | NI_NUMERICSERV);
        printf("%s %s\n", addrBuffer, serviceBuffer);
    }

    CLOSESOCKET(socketListen);

    printf("Finished.\n");

    return 0;
}