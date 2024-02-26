// Chuỗi ký tự sau là phản hồi của lệnh PASV trong giao thức FTP, hãy xác định giá trị địa chỉ IP và cổng.
// 227 Entering Passive Mode (213,229,112,130,216,4)

#include <stdio.h>
#include <string.h>

int main()
{
    char PASV_response[49];
    gets(PASV_response);

    char *token = strtok(PASV_response, "(");

    int count = 0;

    printf("IP address is: ");
    while (token != NULL && (count++ < 4))
    {
        token = strtok(NULL, ",");
        printf("%s.", token);
    }

    token = strtok(NULL, ",");
    int p1 = atoi(token);

     token = strtok(NULL, ",");
    int p2 = atoi(token);


    printf("\nPort Number is: %d\n", p1 * 256 + p2);
    return 0;
}