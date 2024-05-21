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

    // Kiểm tra loại yêu cầu và trả về nội dung thích hợp
    if (strstr(buffer, "GET / ") != NULL)
    {
        const char *response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n\r\n"
            "<html><body><h1>Welcome to HTTP Server</h1>"
            "<p>This is a simple HTTP server in C.</p>"
            "<img src=\"/image\" alt=\"Image\"><br>"
            "<audio controls><source src=\"/audio\" type=\"audio/mpeg\">Your browser does not support the audio element.</audio></body></html>";
        send(client_socket, response, strlen(response), 0);
    }
    else if (strstr(buffer, "GET /image ") != NULL)
    {
        FILE *image_file = fopen("image.jpg", "rb");
        if (image_file == NULL)
        {
            perror("Error opening image file");
            close(client_socket);
            return;
        }

        fseek(image_file, 0, SEEK_END);
        long image_size = ftell(image_file);
        fseek(image_file, 0, SEEK_SET);

        char *image_data = malloc(image_size);
        fread(image_data, 1, image_size, image_file);
        fclose(image_file);

        char header[BUFFER_SIZE];
        snprintf(header, sizeof(header),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: image/jpeg\r\n"
                 "Content-Length: %ld\r\n"
                 "Connection: close\r\n\r\n",
                 image_size);

        send(client_socket, header, strlen(header), 0);
        send(client_socket, image_data, image_size, 0);

        free(image_data);
    }
    else if (strstr(buffer, "GET /audio ") != NULL)
    {
        FILE *audio_file = fopen("dowload.mp4", "rb");
        if (audio_file == NULL)
        {
            perror("Error opening audio file");
            close(client_socket);
            return;
        }

        fseek(audio_file, 0, SEEK_END);
        long audio_size = ftell(audio_file);
        fseek(audio_file, 0, SEEK_SET);

        char *audio_data = malloc(audio_size);
        fread(audio_data, 1, audio_size, audio_file);
        fclose(audio_file);

        char header[BUFFER_SIZE];
        snprintf(header, sizeof(header),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: audio/mpeg\r\n"
                 "Content-Length: %ld\r\n"
                 "Connection: close\r\n\r\n",
                 audio_size);

        send(client_socket, header, strlen(header), 0);
        send(client_socket, audio_data, audio_size, 0);

        free(audio_data);
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
