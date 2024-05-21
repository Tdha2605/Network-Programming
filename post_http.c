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
    const char *post_data = "key1=value1&key2=value2";
    int bytes_received;

    // Lấy thông tin server
    server = gethostbyname("httpbin.org");
    if (server == NULL)
    {
        fprintf(stderr, "Error, no such host\n");
        exit(1);
    }

    // Tạo socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error creating socket");
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
        perror("Error connecting");
        exit(1);
    }

    // Tạo HTTP POST request
    snprintf(request, sizeof(request),
             "POST /post HTTP/1.1\r\n"
             "Host: httpbin.org\r\n"
             "Content-Type: application/x-www-form-urlencoded\r\n"
             "Content-Length: %lu\r\n"
             "Connection: close\r\n\r\n"
             "%s",
             strlen(post_data), post_data);

    // Gửi request
    if (send(sockfd, request, strlen(request), 0) < 0)
    {
        perror("Error sending to socket");
        exit(1);
    }

    // Đọc response
    while ((bytes_received = recv(sockfd, response, sizeof(response) - 1, 0)) > 0)
    {
        response[bytes_received] = '\0'; // Đảm bảo response được null-terminated
        printf("%s", response);
    }

    close(sockfd);
    return 0;
}
