#include "headers.h"

int main() {
    printf("Configuring remote address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;

    struct addrinfo *peerAddress;
    if (getaddrinfo("127.0.0.1", "8080", &hints, &peerAddress)) {
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    printf("Remote address is: ");
    char addrBuffer[100];
    char serviceBuffer[100];
    getnameinfo(peerAddress->ai_addr, peerAddress->ai_addrlen, addrBuffer, sizeof(addrBuffer),
        serviceBuffer, sizeof(serviceBuffer), NI_NUMERICHOST | NI_NUMERICSERV);
    printf("%s %s\n", addrBuffer, serviceBuffer);

    printf("Creating socket...\n");
    SOCKET socketPeer;
    socketPeer = socket(peerAddress->ai_family, peerAddress->ai_socktype, peerAddress->ai_protocol);
    if (!ISVALIDSOCKET(socketPeer)) {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    const char* message = "Hey!, UDP here :)";
    printf("Sending: %s\n", message);
    int bytesSent = sendto(socketPeer, message, strlen(message), 0, 
        peerAddress->ai_addr, peerAddress->ai_addrlen);
    printf("Sent %d bytes.\n", bytesSent);
    
    freeaddrinfo(peerAddress);
    CLOSESOCKET(socketPeer);

    printf("Finished.\n");

    return 0;
}