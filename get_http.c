#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUFFER_SIZE 4096

int main()
{
    int sockfd;
    struct hostent *server;
    struct sockaddr_in serv_addr;
    char request[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    int ret;

    // Lấy thông tin server
    server = gethostbyname("httpbin.org");
    if (server == NULL)
    {
        fprintf(stderr, "Khong ton tai may chu\n");
        exit(1);
    }

    // Tạo socket
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
        perror("socket failed");
        exit(1);
    }

    // Thiết lập địa chỉ server
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(80);

    // Kết nối đến server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("connect");
        exit(1);
    }

    // Tạo HTTP GET request
    snprintf(request, sizeof(request),
             "GET /get HTTP/1.1\r\n"
             "Host: httpbin.org\r\n"
             "Connection: close\r\n\r\n");

    // Gửi request
    if (send(sockfd, request, strlen(request), 0) < 0)
    {
        perror("send");
        exit(1);
    }

    // Đọc response
    while ((ret = recv(sockfd, response, sizeof(response) - 1, 0)) > 0)
    {
        response[ret] = '\0';
        printf("%s", response);
    }

    close(sockfd);
    return 0;
}