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
    int client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_socket == -1)
    {
        perror("CLIENT_SOCKET FAILED()");
        return 1;
    }

    // Mô tả cấu trúc sockaddrin bên nhận
    struct sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = inet_addr("127.0.0.2");
    client_addr.sin_port = htons(9000);

    // int bind_state = bind(client_socket, (struct sockaddr *)&client_addr, sizeof(client_addr));
    // if (bind_state == -1)
    // {
    //     perror("BIND SOCKET FAILED()");
    //     return 1;
    // }

    int connect_state = connect(client_socket, (struct sockaddr *)&client_addr, sizeof(client_addr));
    if (connect_state == -1)
    {
        printf("CAN NOT CONNECT TO SERVER()");
        return 1;
    }

    char *msg = "HELLO SERVER !!!\n";
    send(client_socket, msg, strlen(msg), 0);

    char buf[2048];
    int ret = recv(client_socket, buf, sizeof(buf), 0);
    buf[ret] = 0;
    printf("Data receive : %s", buf);
    close(client_socket);
    return 0;
}