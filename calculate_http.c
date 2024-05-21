#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 4096
#define PORT 80

void handle_request(int client_socket)
{
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

    if (bytes_received < 0)
    {
        perror("Error receiving data");
        close(client_socket);
        return;
    }

    buffer[bytes_received] = '\0';
    printf("Request:\n%s\n", buffer);

    if (strstr(buffer, "GET /calculate?") != NULL)
    {
        char *query = strstr(buffer, "GET /calculate?") + strlen("GET /calculate?");
        char *end_of_query = strstr(query, " ");
        if (end_of_query)
        {
            *end_of_query = '\0';
        }

        int a = 0, b = 0;
        char op = 0;
        sscanf(query, "a=%d&b=%d&op=%c", &a, &b, &op);

        int result = 0;
        switch (op)
        {
        case '+':
            result = a + b;
            break;
        case '-':
            result = a - b;
            break;
        case '*':
            result = a * b;
            break;
        case '/':
            result = (b != 0) ? a / b : 0;
            break;
        }

        char response[BUFFER_SIZE];
        snprintf(response, sizeof(response),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/html\r\n"
                 "Connection: close\r\n\r\n"
                 "<html><body>"
                 "<h1>Calculation Result</h1>"
                 "<p>%d %c %d = %d</p>"
                 "</body></html>",
                 a, op, b, result);

        send(client_socket, response, strlen(response), 0);
    }
    else if (strstr(buffer, "POST /calculate") != NULL)
    {
        char *body = strstr(buffer, "\r\n\r\n") + 4;
        int a = 0, b = 0;
        char op = 0;
        sscanf(body, "a=%d&b=%d&op=%c", &a, &b, &op);

        int result = 0;
        switch (op)
        {
        case '+':
            result = a + b;
            break;
        case '-':
            result = a - b;
            break;
        case '*':
            result = a * b;
            break;
        case '/':
            result = (b != 0) ? a / b : 0;
            break;
        }

        char response[BUFFER_SIZE];
        snprintf(response, sizeof(response),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/html\r\n"
                 "Connection: close\r\n\r\n"
                 "<html><body>"
                 "<h1>Calculation Result</h1>"
                 "<p>%d %c %d = %d</p>"
                 "</body></html>",
                 a, op, b, result);

        send(client_socket, response, strlen(response), 0);
    }
    else
    {
        const char *response =
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n\r\n"
            "<html><body><h1>404 Not Found</h1></body></html>";
        send(client_socket, response, strlen(response), 0);
    }

    close(client_socket);
}

int main()
{
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("Error creating socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error binding socket");
        close(server_socket);
        exit(1);
    }

    if (listen(server_socket, 10) < 0)
    {
        perror("Error listening on socket");
        close(server_socket);
        exit(1);
    }

    printf("HTTP server is running on port %d\n", PORT);

    while (1)
    {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0)
        {
            perror("Error accepting connection");
            close(server_socket);
            exit(1);
        }

        handle_request(client_socket);
    }

    close(server_socket);
    return 0;
}
