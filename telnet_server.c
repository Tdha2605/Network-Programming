#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define PORT 9000

int check_credentials(const char *user, const char *pass)
{
    FILE *file = fopen("username_password.txt", "r");
    if (!file)
    {
        perror("Failed to open the user database");
        return 0;
    }

    char db_user[100], db_pass[100];
    while (fscanf(file, "%s %s", db_user, db_pass) != EOF)
    {

        char *pos;
        if ((pos = strchr(db_pass, '\n')) != NULL)
        {
            *pos = '\0';
        }

        if (strcmp(user, db_user) == 0 && strcmp(pass, db_pass) == 0)
        {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

int main()
{
    int sockfd, newsockfd, client_socket[MAX_CLIENTS], max_sd, sd, activity, valread, addrlen;
    struct sockaddr_in address;
    char buffer[BUFFER_SIZE];

    memset(client_socket, 0, sizeof(client_socket));

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", PORT);

    if (listen(sockfd, 3) < 0)
    {
        perror("Listen");
        exit(EXIT_FAILURE);
    }

    addrlen = sizeof(address);
    puts("Waiting for connections ...");

    while (1)
    {
        fd_set readfds;

        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        max_sd = sockfd;

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            sd = client_socket[i];
            if (sd > 0)
                FD_SET(sd, &readfds);
            if (sd > max_sd)
                max_sd = sd;
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(sockfd, &readfds))
        {
            if ((newsockfd = accept(sockfd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
            {
                perror("Accept");
                exit(EXIT_FAILURE);
            }

            char *message = "Please login with your username and password separated by space.\n";
            send(newsockfd, message, strlen(message), 0);

            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (client_socket[i] == 0)
                {
                    client_socket[i] = newsockfd;
                    printf("Adding to list of sockets as %d\n", i);
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            sd = client_socket[i];

            if (FD_ISSET(sd, &readfds))
            {

                if ((valread = read(sd, buffer, BUFFER_SIZE)) == 0)
                {

                    getpeername(sd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
                    printf("Host disconnected, ip %s, port %d \n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                    close(sd);
                    client_socket[i] = 0;
                }
                else
                {

                    buffer[valread] = '\0';

                    char *user = strtok(buffer, " ");
                    char *pass = strtok(NULL, " ");
                    printf("%s %s\n", user, pass);

                    if (user != NULL && pass != NULL && check_credentials(user, pass))
                    {
                        send(sd, "Login successful.\n", 18, 0);
                    }
                    else
                    {
                        send(sd, "Login failed. Please try again.\n", 32, 0);
                        close(sd);
                        client_socket[i] = 0;
                    }
                }
            }
        }
    }

    return 0;
}
