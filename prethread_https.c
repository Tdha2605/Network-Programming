#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>

void *client_proc(void *arg)
{
    int client = *(int *)arg;

    char buf[256];

    // Receive data from the client
    int ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0)
    {
        close(client);
        return NULL;
    }
    buf[ret] = 0;
    printf("Received from %d: %s\n", client, buf);

    // Send a response back to the client
    char *msg = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Xin chao cac ban</h1></body></html>";
    send(client, msg, strlen(msg), 0);

    close(client);
    return NULL;
}

int main()
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket failed: ");
        return 1;
    }

    // khai bao dia chi server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    // gan socket voi dia chi
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind failed: ");
        return 1;
    }

    // chuyen sang che do cho ket noi
    if (listen(listener, 5))
    {
        perror("listen failed: ");
        return 1;
    }

    printf("Server is listening on port 9000\n");

    while (1)
    {
        int client = accept(listener, NULL, NULL);
        printf("Client %d has connected...\n", client);

        pthread_t tid;

        if (pthread_create(&tid, NULL, client_proc, &client))
        {
            perror("Could not create thread");
            return 1;
        }

        pthread_detach(tid);
    }

    return 0;
}