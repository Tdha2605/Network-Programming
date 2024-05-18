#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>

int main()
{
    int client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Mô tả địa chỉ server
    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(9000);

    int result = connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (result == -1)
    {
        printf("CAN NOT CONNECT TO SERVER\n");
        return 1;
    }

    struct pollfd fds[2];

    // Mô tả các sự kiện từ bàn phím
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;

    // Mô tả các sự kiện từ socket 
    fds[1].fd = client_socket;
    fds[1].events = POLLIN;

    char buf[256];

    while(1)
    {
        int poll_result = poll(fds, 2, -1);

        // Nếu sự kiện là có dữ liệu từ bàn phím
        if (fds[0].revents & POLLIN)
        {
            fgets(buf, sizeof(buf), stdin);
            send(client_socket, buf, strlen(buf), 0);

            // Nếu exit thì kết thúc
            if (strncmp(buf, "exit", 4) == 0) break;
        }

        // Nếu sự liệN từ socket
        if (fds[1].revents & POLLIN)
        {
            poll_result = recv(client_socket, buf, sizeof(buf), 0);
            if (poll_result <= 0) break;

            buf[poll_result] = 0;
            printf("RECEIVED: %s\n", buf);
        }
    }
    close(client_socket);
}