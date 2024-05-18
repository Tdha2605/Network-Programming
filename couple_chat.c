#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 9000
#define BUFSIZE 1024

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int clients[2];
int num_clients = 0;

void *thread_proc(void *arg)
{
    int sock = *((int *)arg);
    pthread_t thread_id = pthread_self();
    char id_message[BUFSIZE];
    sprintf(id_message, "Your thread ID: %lu\n", (unsigned long)thread_id);
    write(sock, id_message, strlen(id_message));

    int paired_sock = (clients[0] == sock) ? clients[1] : clients[0];
    sprintf(id_message, "Paired with thread ID: %lu\n", (unsigned long)(sock == clients[0] ? pthread_self() : pthread_self()));
    write(paired_sock, id_message, strlen(id_message));

    char buffer[BUFSIZE];
    while (1)
    {
        bzero(buffer, BUFSIZE);
        int n = read(sock, buffer, BUFSIZE);
        if (n <= 0)
        {
            close(sock);
            close(paired_sock);
            break;
        }
        write(paired_sock, buffer, strlen(buffer));
    }

    return NULL;
}

int main()
{
    int sockfd, newsockfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen = sizeof(cli_addr);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("ERROR opening socket");
        exit(1);
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("ERROR on binding");
        exit(1);
    }

    listen(sockfd, 5);
    while (1)
    {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0)
        {
            perror("ERROR on accept");
            continue;
        }

        pthread_mutex_lock(&lock);
        clients[num_clients++] = newsockfd;
        if (num_clients == 2)
        {
            pthread_t t1, t2;
            pthread_create(&t1, NULL, thread_proc, &clients[0]);
            pthread_create(&t2, NULL, thread_proc, &clients[1]);
            num_clients = 0;
        }
        pthread_mutex_unlock(&lock);
    }

    close(sockfd);
    return 0;
}
