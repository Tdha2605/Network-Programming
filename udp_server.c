#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        fprintf(stderr, "Nhap sai cu phap\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    int localPort = atoi(argv[3]);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Khong tao duoc socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in rev_addr;
    memset(&rev_addr, 0, sizeof(rev_addr));
    rev_addr.sin_family = AF_INET;
    rev_addr.sin_addr.s_addr = INADDR_ANY;
    rev_addr.sin_port = htons(localPort);

    if (bind(sockfd, (const struct sockaddr *)&rev_addr, sizeof(rev_addr)) < 0)
    {
        perror("Khong the blind");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in send_addr;
    memset(&send_addr, 0, sizeof(send_addr));
    send_addr.sin_family = AF_INET;
    send_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &send_addr.sin_addr);

    fd_set readfds;
    char buffer[BUF_SIZE];
    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        FD_SET(STDIN_FILENO, &readfds);

        int activity = select(sockfd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0)
        {
            perror("select error");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(STDIN_FILENO, &readfds))
        {
            fgets(buffer, BUF_SIZE, stdin);
            sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&send_addr, sizeof(send_addr));
        }

        if (FD_ISSET(sockfd, &readfds))
        {
            int len = recvfrom(sockfd, buffer, BUF_SIZE, 0, NULL, NULL);
            if (len < 0)
            {
                perror("Khong the nhan du lieu");
                exit(EXIT_FAILURE);
            }
            buffer[len] = '\0';
            printf("Server nhan: %s", buffer);
        }
    }

    close(sockfd);
    return 0;
}
