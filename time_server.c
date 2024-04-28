#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_CHILDREN 8
#define BUFFER_SIZE 1024

void handle_client(int client)
{
    char buf[BUFFER_SIZE];
    char response[100];
    time_t now;
    struct tm *timeinfo;
    char *format;

    while (1)
    {
        int len = recv(client, buf, BUFFER_SIZE - 1, 0);
        if (len <= 0)
        {
            break;
        }

        buf[len] = '\0';

        if (strncmp(buf, "GET_TIME", 8) == 0)
        {
            char *cmd_format = strtok(buf + 9, " \r\n");
            if (cmd_format)
            {
                now = time(NULL);
                timeinfo = localtime(&now);

                if (strcmp(cmd_format, "dd/mm/yyyy") == 0)
                {
                    format = "%d/%m/%Y";
                }
                else if (strcmp(cmd_format, "dd/mm/yy") == 0)
                {
                    format = "%d/%m/%y";
                }
                else if (strcmp(cmd_format, "mm/dd/yyyy") == 0)
                {
                    format = "%m/%d/%Y";
                }
                else if (strcmp(cmd_format, "mm/dd/yy") == 0)
                {
                    format = "%m/%d/%y";
                }
                else
                {
                    strcpy(response, "Khong ton tai dinh dang\r\n");
                    send(client, response, strlen(response), 0);
                    continue;
                }

                strftime(response, sizeof(response), format, timeinfo);
                strcat(response, "\r\n");
                send(client, response, strlen(response), 0);
            }
            else
            {
                strcpy(response, "Khong ton tai dinh dang\r\n");
                send(client, response, strlen(response), 0);
            }
        }
        else
        {
            strcpy(response, "Khong ton tai dinh dang\r\n");
            send(client, response, strlen(response), 0);
        }
    }
    close(client);
}

int main()
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8888);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("bind() failed");
        return 1;
    }

    if (listen(listener, 5) == -1)
    {
        perror("listen() failed");
        return 1;
    }

    for (int i = 0; i < MAX_CHILDREN; i++)
    {
        int pid = fork();
        if (pid == 0)
        {
            while (1)
            {
                int client = accept(listener, NULL, NULL);
                if (client == -1)
                {
                    perror("accept() failed");
                    continue;
                }
                handle_client(client);
            }
            exit(0);
        }
        else if (pid < 0)
        {
            perror("fork() failed");
            return 1;
        }
    }

    int status = 0;
    while (wait(&status) > 0)
        ;

    close(listener);

    return 0;
}
