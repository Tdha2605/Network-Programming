#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>

#define PORT 9000
#define BUFFER_SIZE 1024

void format_time(char *format, char *buffer)
{
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    if (strcmp(format, "dd/mm/yyyy") == 0)
    {
        strftime(buffer, BUFFER_SIZE, "%d/%m/%Y", timeinfo);
    }
    else if (strcmp(format, "dd/mm/yy") == 0)
    {
        strftime(buffer, BUFFER_SIZE, "%d/%m/%y", timeinfo);
    }
    else if (strcmp(format, "mm/dd/yyyy") == 0)
    {
        strftime(buffer, BUFFER_SIZE, "%m/%d/%Y", timeinfo);
    }
    else if (strcmp(format, "mm/dd/yy") == 0)
    {
        strftime(buffer, BUFFER_SIZE, "%m/%d/%y", timeinfo);
    }
    else
    {
        strcpy(buffer, "Invalid format");
    }
}

void *thread_proc(void *arg)
{
    int sock = *((int *)arg);
    free(arg);
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    while (1)
    {
        int read_size = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (read_size > 0)
        {
            buffer[read_size] = '\0';
            char command[10], format[20];
            if (sscanf(buffer, "GET_TIME %[^\n]", format) == 1)
            {
                format_time(format, response);
                send(sock, response, strlen(response), 0);
            }
            else
            {
                char *error_msg = "Invalid command. Use 'GET_TIME [format]'\n";
                send(sock, error_msg, strlen(error_msg), 0);
            }
        }
        else if (read_size == 0)
        {
            break;
        }
        else
        {
            perror("recv failed");
            break;
        }
    }

    close(sock);
    return 0;
}

int main()
{
    int server_fd, *new_sock;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Time Server is listening on port %d...\n", PORT);

    while (1)
    {
        new_sock = malloc(sizeof(int));
        if ((*new_sock = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            free(new_sock);
            continue;
        }

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, thread_proc, (void *)new_sock) != 0)
        {
            perror("Could not create thread");
            free(new_sock);
        }
    }

    return 0;
}
