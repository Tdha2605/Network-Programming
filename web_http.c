#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 4096
#define PORT 80
#define USER_FILE "users.txt"

typedef struct
{
    char username[50];
    char password[50];
    char email[50];
    char name[50];
} User;

void send_response(int client_socket, const char *header, const char *body, int body_length)
{
    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response), "%sContent-Length: %d\r\n\r\n", header, body_length);
    send(client_socket, response, strlen(response), 0);
    send(client_socket, body, body_length, 0);
}

void handle_register(int client_socket, char *body)
{
    User user;
    sscanf(body, "username=%49[^&]&password=%49[^&]&email=%49[^&]&name=%49[^&]", user.username, user.password, user.email, user.name);

    FILE *file = fopen(USER_FILE, "a");
    if (file == NULL)
    {
        perror("Error opening file");
        const char *response = "<html><body><h1>500 Internal Server Error</h1></body></html>";
        send_response(client_socket, "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html\r\n", response, strlen(response));
        return;
    }

    fprintf(file, "%s,%s,%s,%s\n", user.username, user.password, user.email, user.name);
    fclose(file);

    const char *response = "<html><body><h1>Registration Successful</h1></body></html>";
    send_response(client_socket, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n", response, strlen(response));
}

void handle_login(int client_socket, char *body)
{
    char username[50], password[50];
    sscanf(body, "username=%49[^&]&password=%49[^&]", username, password);

    FILE *file = fopen(USER_FILE, "r");
    if (file == NULL)
    {
        perror("Error opening file");
        const char *response = "<html><body><h1>500 Internal Server Error</h1></body></html>";
        send_response(client_socket, "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html\r\n", response, strlen(response));
        return;
    }

    char line[BUFFER_SIZE];
    while (fgets(line, sizeof(line), file))
    {
        User user;
        sscanf(line, "%49[^,],%49[^,],%49[^,],%49[^\n]", user.username, user.password, user.email, user.name);
        if (strcmp(user.username, username) == 0 && strcmp(user.password, password) == 0)
        {
            const char *response = "<html><body><h1>Login Successful</h1></body></html>";
            send_response(client_socket, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n", response, strlen(response));
            fclose(file);
            return;
        }
    }

    fclose(file);
    const char *response = "<html><body><h1>Login Failed</h1></body></html>";
    send_response(client_socket, "HTTP/1.1 401 Unauthorized\r\nContent-Type: text/html\r\n", response, strlen(response));
}

void handle_list_users(int client_socket)
{
    FILE *file = fopen(USER_FILE, "r");
    if (file == NULL)
    {
        perror("Error opening file");
        const char *response = "<html><body><h1>500 Internal Server Error</h1></body></html>";
        send_response(client_socket, "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html\r\n", response, strlen(response));
        return;
    }

    char body[BUFFER_SIZE];
    int body_length = snprintf(body, sizeof(body), "<html><body><h1>User List</h1><ul>");

    char line[BUFFER_SIZE];
    while (fgets(line, sizeof(line), file))
    {
        User user;
        sscanf(line, "%49[^,],%49[^,],%49[^,],%49[^\n]", user.username, user.password, user.email, user.name);
        body_length += snprintf(body + body_length, sizeof(body) - body_length, "<li>%s (%s, %s)</li>", user.username, user.email, user.name);
    }

    fclose(file);

    body_length += snprintf(body + body_length, sizeof(body) - body_length, "</ul></body></html>");
    send_response(client_socket, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n", body, body_length);
}

void handle_request(int client_socket)
{
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

    if (bytes_received < 0)
    {
        perror("recv");
        close(client_socket);
        return;
    }

    buffer[bytes_received] = '\0';

    char method[16], path[256];
    sscanf(buffer, "%s %s", method, path);

    if (strcmp(method, "GET") == 0)
    {
        if (strcmp(path, "/users") == 0)
        {
            handle_list_users(client_socket);
        }
        else
        {
            const char *response = "<html><body><h1>404 Not Found</h1></body></html>";
            send_response(client_socket, "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n", response, strlen(response));
        }
    }
    else if (strcmp(method, "POST") == 0)
    {
        char *body = strstr(buffer, "\r\n\r\n") + 4;
        if (strcmp(path, "/register") == 0)
        {
            handle_register(client_socket, body);
        }
        else if (strcmp(path, "/login") == 0)
        {
            handle_login(client_socket, body);
        }
        else
        {
            const char *response = "<html><body><h1>404 Not Found</h1></body></html>";
            send_response(client_socket, "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n", response, strlen(response));
        }
    }
    else
    {
        const char *response = "<html><body><h1>405 Method Not Allowed</h1></body></html>";
        send_response(client_socket, "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: text/html\r\n", response, strlen(response));
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
        perror("socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        close(server_socket);
        exit(1);
    }

    if (listen(server_socket, 10) < 0)
    {
        perror("listen");
        close(server_socket);
        exit(1);
    }

    printf("HTTP server is running on port %d\n", PORT);

    while (1)
    {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0)
        {
            perror("accept");
            close(server_socket);
            exit(1);
        }

        handle_request(client_socket);
    }

    close(server_socket);
    return 0;
}
