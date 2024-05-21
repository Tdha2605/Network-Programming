#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8000
#define BUFFER_SIZE 1024

void handle_client_request(int client_socket)
{
    char buffer[BUFFER_SIZE];
    int read_size;
    char *method, *path, *version, *query_string;
    double a = 0, b = 0, result = 0;
    char cmd[10] = {0};
    char response[BUFFER_SIZE];

    // Đọc request từ client
    read_size = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (read_size < 0)
    {
        perror("recv failed");
        return;
    }
    buffer[read_size] = '\0';

    // Phân tích dòng đầu tiên của HTTP request
    method = strtok(buffer, " ");
    path = strtok(NULL, " ");
    version = strtok(NULL, "\r\n");

    // Kiểm tra phương thức GET hoặc POST
    if (strcmp(method, "GET") == 0)
    {
        query_string = strchr(path, '?');
        if (query_string)
        {
            query_string++; // Bỏ qua ký tự '?'
            sscanf(query_string, "a=%lf&b=%lf&cmd=%s", &a, &b, cmd);
        }
    }
    else if (strcmp(method, "POST") == 0)
    {
        // Tìm chuỗi chứa dữ liệu POST
        query_string = strstr(buffer, "\r\n\r\n");
        if (query_string)
        {
            query_string += 4; // Bỏ qua ký tự "\r\n\r\n"
            sscanf(query_string, "a=%lf&b=%lf&cmd=%s", &a, &b, cmd);
        }
    }

    // Thực hiện phép tính
    if (strcmp(cmd, "add") == 0)
    {
        result = a + b;
    }
    else if (strcmp(cmd, "sub") == 0)
    {
        result = a - b;
    }
    else if (strcmp(cmd, "mul") == 0)
    {
        result = a * b;
    }
    else if (strcmp(cmd, "div") == 0)
    {
        if (b != 0)
        {
            result = a / b;
        }
        else
        {
            snprintf(response, sizeof(response), "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<div>Division by zero is not allowed.</div>");
            send(client_socket, response, strlen(response), 0);
            close(client_socket);
            return;
        }
    }
    else
    {
        snprintf(response, sizeof(response), "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<div>Invalid command.</div>");
        send(client_socket, response, strlen(response), 0);
        close(client_socket);
        return;
    }

    // Tạo response HTML
    snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<div>Kết quả: %.2lf</div>", result);

    // Gửi response về client
    send(client_socket, response, strlen(response), 0);

    // Đóng kết nối với client
    close(client_socket);
}

int main()
{
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Tạo socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Thiết lập địa chỉ server
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Gắn socket với địa chỉ server
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Lắng nghe kết nối
    if (listen(server_socket, 5) < 0)
    {
        perror("listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", PORT);

    while (1)
    {
        // Chấp nhận kết nối từ client
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0)
        {
            perror("accept failed");
            close(server_socket);
            exit(EXIT_FAILURE);
        }

        // Xử lý request từ client
        handle_client_request(client_socket);
    }

    // Đóng socket server
    close(server_socket);

    return 0;
}
