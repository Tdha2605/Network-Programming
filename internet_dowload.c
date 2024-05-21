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
    const char *filename = "dowload.mp4";
    FILE *file;
    int bytes_received;

    // Lấy thông tin server
    server = gethostbyname("lebavui.github.io");
    if (server == NULL)
    {
        fprintf(stderr, "Error, no such host\n");
        exit(1);
    }

    // Tạo socket
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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

    // Tạo HTTP GET request
    snprintf(request, sizeof(request),
             "GET /network_programming/videos/ecard.mp4 HTTP/1.1\r\n"
             "Host: lebavui.github.io\r\n"
             "Connection: close\r\n\r\n");

    // Gửi request
    if (send(sockfd, request, strlen(request), 0) < 0)
    {
        perror("Error sending to socket");
        exit(1);
    }

    // Mở file để ghi dữ liệu
    file = fopen(filename, "wb");
    if (file == NULL)
    {
        perror("Error opening file");
        close(sockfd);
        exit(1);
    }

    // Đọc response và ghi vào file
    while ((bytes_received = recv(sockfd, response, sizeof(response) - 1, 0)) > 0)
    {
        fwrite(response, 1, bytes_received, file);
    }

    // Đóng file và socket
    fclose(file);
    close(sockfd);
    return 0;
}
