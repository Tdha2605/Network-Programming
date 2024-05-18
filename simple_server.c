#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

int main()
{

    // Tạo socket
    int server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == -1) 
    {
        perror("SERVER_SOCKET FAILED()");
        return 1;
    }

    // Mô tả cấu trúc sockaddrin bên gửi
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(9000);

    int bind_state = bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (bind_state == -1)
    {
        perror("BIND SOCKET FAILED()");
        return 1;
    }

    int listen_state = listen(server_socket, 5);
    if (listen_state == -1)
    {
        perror("LISTEN FAILED()");
        return 1;
    }
    printf("SERVER IS LISTENING ON %d", server_addr.sin_port);

    char buf[1024];

    while (1)
    {
        int accecpt_client = accept(server_socket, NULL, NULL);
        int receive_state = recv(server_socket, buf, sizeof(buf), 0);
        if (receive_state < 0)
        {
            close(server_socket);
            continue;
        }

        if (receive_state < sizeof(buf))
        {
            buf[receive_state] = 0;
            printf("%s\n", buf);

            char from_server[] = "Bye Client!!!";
            send(accecpt_client, from_server, sizeof(from_server), 0);
        }

        close(server_socket);
    }
    return 0;
}