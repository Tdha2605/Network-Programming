#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <sys/stat.h>
#include <netdb.h>

#define BUFFER_SIZE 4096
#define PORT 80

void send_response(int client_socket, const char *header, const char *body, int body_length)
{
    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response), "%sContent-Length: %d\r\n\r\n", header, body_length);
    send(client_socket, response, strlen(response), 0);
    send(client_socket, body, body_length, 0);
}

void send_directory_listing(int client_socket, const char *path)
{
    DIR *dir;
    struct dirent *entry;
    char body[BUFFER_SIZE];
    int body_length = 0;

    snprintf(body + body_length, sizeof(body) - body_length, "<html><body><h1>Directory listing for %s</h1><ul>", path);
    body_length = strlen(body);

    dir = opendir(path);
    if (dir == NULL)
    {
        perror("opendir");
        const char *error_message = "<html><body><h1>500 Internal Server Error</h1></body></html>";
        send_response(client_socket, "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html\r\n", error_message, strlen(error_message));
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        char entry_path[BUFFER_SIZE];
        snprintf(entry_path, sizeof(entry_path), "%s/%s", path, entry->d_name);
        struct stat entry_stat;
        stat(entry_path, &entry_stat);

        if (S_ISDIR(entry_stat.st_mode))
        {
            snprintf(body + body_length, sizeof(body) - body_length, "<li><a href=\"%s/\">%s/</a></li>", entry->d_name, entry->d_name);
        }
        else
        {
            snprintf(body + body_length, sizeof(body) - body_length, "<li><a href=\"%s\">%s</a></li>", entry->d_name, entry->d_name);
        }
        body_length = strlen(body);
    }

    closedir(dir);

    snprintf(body + body_length, sizeof(body) - body_length, "</ul></body></html>");
    body_length = strlen(body);

    send_response(client_socket, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n", body, body_length);
}

void send_file(int client_socket, const char *path)
{
    FILE *file = fopen(path, "rb");
    if (file == NULL)
    {
        perror("fopen");
        const char *error_message = "<html><body><h1>404 Not Found</h1></body></html>";
        send_response(client_socket, "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n", error_message, strlen(error_message));
        return;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *file_content = malloc(file_size);
    fread(file_content, 1, file_size, file);
    fclose(file);

    const char *content_type = "application/octet-stream";
    if (strstr(path, ".txt"))
    {
        content_type = "text";
    }
    else if (strstr(path, ".jpg") || strstr(path, ".jpeg"))
    {
        content_type = "image/jpeg";
    }
    else if (strstr(path, ".png"))
    {
        content_type = "image/png";
    }
    else if (strstr(path, ".c"))
    {
        content_type = "client/server application";
    }
    else if (strstr(path, ".mp4"))
    {
        content_type = "audio/mpeg";
    }

    char header[BUFFER_SIZE];
    snprintf(header, sizeof(header), "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n", content_type);

    send_response(client_socket, header, file_content, file_size);
    free(file_content);
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
        if (strcmp(path, "/") == 0)
        {
            strcpy(path, ".");
        }
        else
        {
            memmove(path, path + 1, strlen(path)); // Remove leading '/'
        }

        struct stat path_stat;
        stat(path, &path_stat);

        if (S_ISDIR(path_stat.st_mode))
        {
            send_directory_listing(client_socket, path);
        }
        else if (S_ISREG(path_stat.st_mode))
        {
            send_file(client_socket, path);
        }
        else
        {
            const char *error_message = "<html><body><h1>404 Not Found</h1></body></html>";
            send_response(client_socket, "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n", error_message, strlen(error_message));
        }
    }
    else
    {
        const char *error_message = "<html><body><h1>405 Method Not Allowed</h1></body></html>";
        send_response(client_socket, "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: text/html\r\n", error_message, strlen(error_message));
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

    if (listen(server_socket, 5) < 0)
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
