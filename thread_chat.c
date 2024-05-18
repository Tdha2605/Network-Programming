#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>

#define PORT 9000
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 1024

int client_sockets[MAX_CLIENTS];
int num_clients = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void *thread_proc (void *arg)
{
    int sock = *((int *)arg);
    char buffer[BUFFER_SIZE];
    char client_name[100];
    int name_set = 0;

    // Loop to get the client name
    while (!name_set)
    {
        int read_size = recv(sock, buffer, BUFFER_SIZE, 0);
        if (read_size > 0)
        {
            buffer[read_size] = '\0';
            if (sscanf(buffer, "client_id: %s", client_name) == 1)
            {
                name_set = 1;
            }
            else
            {
                char *msg = "Invalid format. Use 'client_id: client_name'\n";
                send(sock, msg, strlen(msg), 0);
            }
        }
    }

    while (1)
    {
        int read_size = recv(sock, buffer, BUFFER_SIZE, 0);
        if (read_size > 0)
        {
            buffer[read_size] = '\0';
            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < num_clients; i++)
            {
                if (client_sockets[i] != sock)
                {
                    char message[1024];
                    printf(message, "%s: %s", client_name, buffer);
                    send(client_sockets[i], message, strlen(message), 0);
                }
            }
            pthread_mutex_unlock(&clients_mutex);
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
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < num_clients; i++)
    {
        if (client_sockets[i] == sock)
        {
            for (int j = i; j < num_clients - 1; j++)
            {
                client_sockets[j] = client_sockets[j + 1];
            }
            num_clients--;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    return 0;
}

int main()
{
    int server_fd, new_socket;
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

    printf("Server is listening on port %d...\n", PORT);

    while (1)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            continue;
        }

        pthread_mutex_lock(&clients_mutex);
        client_sockets[num_clients++] = new_socket;
        pthread_mutex_unlock(&clients_mutex);

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, thread_proc, (void *)&new_socket) != 0)
        {
            perror("Could not create thread");
            return 1;
        }
    }

    return 0;
}
