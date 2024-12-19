#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)

int main() {
    printf("Configuring local address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6; // AF_UNSPEC : IPv4 or IPv6
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

    int option = 0;
    if (setsockopt(socketListen, IPPROTO_IPV6, IPV6_V6ONLY, (void*)&option, sizeof(option))) {
        fprintf(stderr, "setsockopt() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    printf("Binding socket to local address...\n");
    if (bind(socketListen, bindAddress->ai_addr, bindAddress->ai_addrlen)) { //bind() returns 0 on success and non-zero on failure.
        fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }
    freeaddrinfo(bindAddress);

    printf("Listening...\n");
    if (listen(socketListen, 10)) { //The second argument to listen(),10, tells listen() how many connections it is allowed to queue up.
        fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
    }

    printf("Waiting for connection...\n");
    struct sockaddr_storage clientAddress;
    socklen_t clientLen = sizeof(clientAddress);
    SOCKET socketClient = accept(socketListen, (struct sockaddr*) &clientAddress, &clientLen);
    if (!ISVALIDSOCKET(socketClient)) {
        fprintf(stderr, "accept() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    printf("Client is connected...\n");
    char addressBuffer[100];
    getnameinfo((struct sockaddr*)&clientAddress,
            clientLen, addressBuffer, sizeof(addressBuffer), 0, 0, NI_NUMERICHOST);
    printf("%s\n", addressBuffer);

    printf("Reading request...\n");
    char request[1024];
    int bytesReceived = recv(socketClient, request, 1024, 0);
    printf("Received %d bytes.\n", bytesReceived);
    // printf("%.*s", bytesReceived, request);

    printf("Sending response...\n");
    const char *response = 
        "HTTP/1.1 200 OK\r\n"
        "Connection: close\r\n"
        "Content-Type: text/plain\r\n\r\n"
        "Local time is: ";
    int bytesSent = send(socketClient, response, strlen(response), 0);
    printf("Sent %d of %d bytes.\n", bytesSent, (int)strlen(response));

    time_t currentTime;
    time(&currentTime);

    char *timeMsg = ctime(&currentTime);
    bytesSent = send(socketClient, timeMsg, strlen(timeMsg), 0);
    printf("Sent %d of %d bytes.\n", bytesSent, (int)strlen(timeMsg));

    printf("Closing connection...\n");
    CLOSESOCKET(socketClient);

    printf("Closing listening socket...\n");
    CLOSESOCKET(socketListen);

    printf("Finished.\n");

    return 0;
}
