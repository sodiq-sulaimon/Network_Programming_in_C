#include "headers_http.h"

void parseURL(char *url, char **hostname, char **port, char **path) {
    printf("URL: %s\n", url);

    char *p;
    p = strstr(url, "://");

    char *protocol = 0; // 0 means no protocol has been found.
    if (p) {
        protocol = url;
        *p = 0; // terminates the protocol portion of the string by replacing the ':' with a null terminator (\0)
        p += 3; // Moves the pointer p past the "://", so p now points to the next part of the URL
    } else {
        p = url;
    }

    if (protocol) {
        if (strcmp(protocol, "http")) {
            fprintf(stderr, "Unknown protocol '%s'. Only 'http' is supported.\n", protocol);
            exit(1);
        }
    }
    *hostname = p;
    while (*p && *p != ':' && *p != '/' && *p != '#') p++;

    *port = "80";
    if (*p == ':') {
        *p++ = 0; // set ':' to 0 to null-terminate.
        *port = p;
    }
    while (*p && *p != '/' && *p != '#') p++;

    *path = p;
    if (*p == '/') {
        *path = p + 1; // omit the leading '/'
    }
    *p = 0;

    while (*p && *p != '#') p++;
    if (*p == '#') *p = 0;

    printf("hostname: %s\n", *hostname);
    printf("port: %s\n", *port);
    printf("path: %s\n", *path);
}

void sendRequest(SOCKET s, char *hostname, char *port, char *path) {
    char buffer[2048];

    sprintf(buffer, "GET /%s HTTP/1.1\r\n", path);
    sprintf(buffer + strlen(buffer), "Host: %s:%s\r\n", hostname, port);
    sprintf(buffer + strlen(buffer), "Connection: close\r\n");
    sprintf(buffer + strlen(buffer), "User-Agent: honpwc web_get 1.0\r\n");
    sprintf(buffer + strlen(buffer), "\r\n");

    send(s, buffer, strlen(buffer), 0);
    printf("Sent Headers:\n%s", buffer);
}



int main() {

    return 0;
}