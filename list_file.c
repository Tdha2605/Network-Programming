#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 9000
#define BUFFER_SIZE 1024

void *thread_proc(void *socket_desc);
void send_file_list(int sock);
void send_file(int sock, char *file_name);

int main()
{
    int server_fd, new_socket, *new_sock;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    // {
    //     perror("setsockopt");
    //     exit(EXIT_FAILURE);
    // }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        printf("Dang cho ket noi tren cong 9000...\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            continue;
        }

        pthread_t thread_id;
        new_sock = malloc(1);
        *new_sock = new_socket;

        if (pthread_create(&thread_id, NULL, thread_proc, (void *)new_sock) != 0)
        {
            perror("could not create thread");
            free(new_sock);
        }
    }

    return 0;
}

void *thread_proc(void *socket_desc)
{
    int sock = *(int *)socket_desc;
    free(socket_desc);

    send_file_list(sock);

    char buffer[BUFFER_SIZE] = {0};
    int read_size;

    // Get the file name from the client
    if ((read_size = recv(sock, buffer, BUFFER_SIZE, 0)) > 0)
    {
        buffer[read_size] = '\0';
        send_file(sock, buffer);
    }

    close(sock);
    return 0;
}

void send_file_list(int sock)
{
    DIR *d;
    struct dirent *dir;
    d = opendir(".");
    if (d)
    {
        int count = 0;
        char files_list[BUFFER_SIZE] = "";
        char temp_list[BUFFER_SIZE] = "OK "; // Temporary list to hold files
        char count_str[10];
        char *end_of_message = "\r\n\r\n";

        while ((dir = readdir(d)) != NULL)
        {
            if (dir->d_type == DT_REG)
            { // If the entry is a regular file
                strcat(files_list, dir->d_name);
                strcat(files_list, "\r\n");
                count++;
            }
        }
        sprintf(count_str, "%d", count);
        strcat(temp_list, count_str);
        strcat(temp_list, "\r\n");
        strcat(temp_list, files_list);
        strcat(temp_list, end_of_message);
        send(sock, temp_list, strlen(temp_list), 0);
        closedir(d);
    }
    else
    {
        char *error_msg = "ERROR No files to download \r\n";
        send(sock, error_msg, strlen(error_msg), 0);
    }
}

void send_file(int sock, char *file_name)
{
    FILE *file;
    char buffer[BUFFER_SIZE];
    int bytes_read;

    file = fopen(file_name, "rb");

    if (file == NULL)
    {
        char *error_msg = "ERROR File does not exist \r\n";
        send(sock, error_msg, strlen(error_msg), 0);
    }
    else
    {
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        rewind(file);

        sprintf(buffer, "OK %ld\r\n", file_size);
        send(sock, buffer, strlen(buffer), 0);

        while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0)
        {
            send(sock, buffer, bytes_read, 0);
        }
        fclose(file);
    }

    close(sock);
}
