#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9000
#define BUFFER_SIZE 1024

int main()
{
    int sock;
    struct sockaddr_in server;
    char message[BUFFER_SIZE], server_request[66], server_reply[32];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        printf("Could not create socket");
        return -1;
    }
    printf("Socket created\n");

    server.sin_addr.s_addr = inet_addr(SERVER_IP);
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }

    printf("Connected to server\n");

    if (recv(sock, server_request, BUFFER_SIZE, 0) < 0)
    {
        puts("recv failed");
        return 1;
    }
    printf("%s", server_request);

    printf("Enter username and password: ");
    fgets(message, BUFFER_SIZE, stdin);

    if (send(sock, message, strlen(message), 0) < 0)
    {
        puts("Send failed");
        return 1;
    }

    if (recv(sock, server_reply, BUFFER_SIZE, 0) < 0)
    {
        puts("recv failed");
        return 1;
    }
    printf("Server reply: %s\n", server_reply);

    close(sock);
    return 0;
}
