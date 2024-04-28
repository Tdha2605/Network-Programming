#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_CHILDREN 8

void handle_client(int client) {
    char buf[1024];
    int ret = recv(client, buf, sizeof(buf), 0);
    if (ret > 0) {
        if (ret < sizeof(buf)) buf[ret] = '\0';
        printf("Received: %s\n", buf);

        char msg[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Hello World</h1></body></html>";
        send(client, msg, strlen(msg), 0);
    }
    close(client);
}

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1) {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind() failed");
        return 1;
    }

    if (listen(listener, 5) == -1) {
        perror("listen() failed");
        return 1;
    }

    for (int i = 0; i < MAX_CHILDREN; i++) {
        int pid = fork();
        if (pid == 0) { 
            while (1) {
                int client = accept(listener, NULL, NULL);
                if (client == -1) {
                    perror("accept() failed");
                    continue;
                }
                handle_client(client);
            }
            exit(0);  
        } else if (pid < 0) {
            perror("fork() failed");
            return 1;
        }
    }

    
    int status = 0;
    while (wait(&status) > 0);

    close(listener);

    return 0;
}
