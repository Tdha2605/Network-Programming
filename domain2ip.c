#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>

int main(int argc, char *argv[])
{

    // Kiểm tra tham số có được nhập vào không
    if (argc != 2)
    {
        printf("Parameter is Invalid !\n");
        return 1;
    }

    // Khai báo con trỏ kết quả
    struct addrinfo *IP_address, *pointer_IP_Address;

    int is_IP_Address = getaddrinfo(argv[1], "http", NULL, &IP_address);

    if (is_IP_Address != 0 || IP_address == NULL)
    {
        printf("Failed to get IP addIP_addresss.\n");
        return 1;
    }

    // Duyệt danh sách kết quả và in ra địa chỉ IP
    pointer_IP_Address = IP_address;

    while (pointer_IP_Address != NULL)
    {
        if (pointer_IP_Address->ai_family == AF_INET) // IPv4
        {
            printf("IPv4\n");
            struct sockaddr_in addr;
            memcpy(&addr, pointer_IP_Address->ai_addr, pointer_IP_Address->ai_addrlen);
            printf("IP: %s\n", inet_ntoa(addr.sin_addr));
        }
        else if (pointer_IP_Address->ai_family == AF_INET6) // IPv6
        {
            printf("IPv6\n");
            char buf[64];
            struct sockaddr_in6 addr6;
            memcpy(&addr6, pointer_IP_Address->ai_addr, pointer_IP_Address->ai_addrlen);
            printf("IP: %s\n", inet_ntop(pointer_IP_Address->ai_family, &addr6.sin6_addr, buf, sizeof(buf)));
        }
        pointer_IP_Address = pointer_IP_Address->ai_next;
    }

  
    freeaddrinfo(IP_address);

    return 0;
}