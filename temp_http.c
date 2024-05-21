#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

// Hàm để tạo kết nối TCP tới máy chủ
int create_connection(const char *hostname, int port)
{
    int sockfd;
    struct hostent *host;
    struct sockaddr_in server_addr;

    host = gethostbyname(hostname);
    if (host == NULL)
    {
        fprintf(stderr, "Error: No such host.\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error opening socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    memcpy(&server_addr.sin_addr.s_addr, host->h_addr, host->h_length);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error connecting");
        exit(1);
    }

    return sockfd;
}

// Hàm để gửi yêu cầu HTTP
void send_request(int sockfd, const char *hostname, const char *path)
{
    char buffer[1024];

    snprintf(buffer, sizeof(buffer), "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, hostname);
    send(sockfd, buffer, strlen(buffer), 0);
}

// Hàm để đọc phản hồi HTTP
char *read_response(int sockfd)
{
    char buffer[4096];
    char *response = NULL;
    int total_length = 0;
    int received_length;

    while ((received_length = recv(sockfd, buffer, sizeof(buffer), 0)) > 0)
    {
        response = realloc(response, total_length + received_length + 1);
        memcpy(response + total_length, buffer, received_length);
        total_length += received_length;
    }
    if (response)
    {
        response[total_length] = '\0';
    }

    return response;
}

// Hàm để lấy giá trị temp_c từ JSON
void parse_and_print_temperature(const char *response)
{
    const char *temp_str = "\"temp_c\":";
    char *temp_ptr = strstr(response, temp_str);

    if (temp_ptr)
    {
        temp_ptr += strlen(temp_str);
        float temp_c;
        sscanf(temp_ptr, "%f", &temp_c);
        printf("Nhiệt độ hiện tại: %.2f°C\n", temp_c);
    }
    else
    {
        printf("Không tìm thấy giá trị temp_c trong dữ liệu trả về.\n");
    }
}

int main()
{
    const char *hostname = "api.weatherapi.com";
    const char *path = "/v1/current.json?key=48bab0abac324847925230945232306&q=Hanoi&aqi=no";
    int port = 80;

    // Tạo kết nối TCP
    int sockfd = create_connection(hostname, port);

    // Gửi yêu cầu HTTP
    send_request(sockfd, hostname, path);

    // Đọc phản hồi HTTP
    char *response = read_response(sockfd);

    if (response)
    {
        // In phản hồi (chỉ để kiểm tra)
        // printf("%s\n", response);

        // Phân tích và in nhiệt độ
        parse_and_print_temperature(response);

        free(response);
    }
    else
    {
        printf("Không nhận được phản hồi từ máy chủ.\n");
    }

    close(sockfd);
    return 0;
}
