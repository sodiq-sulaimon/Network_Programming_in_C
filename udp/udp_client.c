#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "usage: udp_client hostname port\n");
        return 1;
    }

    printf("Configuring remote address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    struct addrinfo *peerAddress;
    if (getaddrinfo(argv[1], argv[2], &hints, &peerAddress)) {
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    printf("Remote address is: \n");
    char addressBuffer[100];
    char serviceBuffer[100];

    getnameinfo(peerAddress->ai_addr, peerAddress->ai_addrlen, addressBuffer, sizeof(addressBuffer),
            serviceBuffer, sizeof(serviceBuffer), NI_NUMERICHOST);
    printf("%s, %s\n", addressBuffer, serviceBuffer);

    printf("Creating socket: \n");
    SOCKET socketPeer;
    socketPeer = socket(peerAddress->ai_family, peerAddress->ai_socktype, peerAddress->ai_protocol);
    if(!ISVALIDSOCKET(socketPeer)) {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }
    
    printf("Connecting: \n");
    if(connect(socketPeer, peerAddress->ai_addr, peerAddress->ai_addrlen)) {
        fprintf(stderr, "connect() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }
    freeaddrinfo(peerAddress);

    printf("Connected.\n");
    printf("To send data, enter text followed by enter.\n");

    while (1) {
        fd_set reads;
        FD_ZERO(&reads);
        FD_SET(socketPeer, &reads);
        FD_SET(0, &reads);


        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;

        if (select(socketPeer+1, &reads, 0, 0, &timeout) < 0) {
            fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
            return 1;
        }

        // checking for udp data
        if (FD_ISSET(socketPeer, &reads)) {
            char read[4096];
            int bytesReceived = recv(socketPeer, read, 4096, 0);
            if (bytesReceived < 1) {
                printf("Connection closed by peer.\n");
                break;
            }
            printf("Received (%d bytes): %.*s", bytesReceived, bytesReceived, read);

        }

        // checking for terminal data
        if (FD_ISSET(0, &reads)) {
            char read[4096];
            if (!fgets(read, 4096, stdin)) break;
            printf("Sending: %s", read);
            int bytesSent = send(socketPeer, read, strlen(read), 0);
            printf("Sent %d bytes.\n", bytesSent);
        }
    }
    printf("Closing socket...\n");
    CLOSESOCKET(socketPeer);

    printf("Finished.\n");

    return 0;
}