#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Sai tham so \n");
        return 1;
    }
    // Khai bao socket
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    // Khai bao dia chi cua server
    int portno = atoi(argv[2]);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(portno);
    // Ket noi den server
    int res = connect(client, (struct sockaddr *)&addr, sizeof(addr));
    if (res == -1)
    {
        printf("Khong ket noi duoc den server!");
        return 1;
    }

    // Nhan tin nhan tu server
    char buf[256];
    res = recv(client, buf, sizeof(buf), 0);
    if (res <= 0)
    {
        printf("Connection closed\n");
        return 1;
    }
    printf("Phản hồi từ server: %s\n", buf);

    // Gui tin nhan den server

    while (1)
    {
        char msg[258];
        printf("Nhập dữ liệu: ");
        fgets(msg, sizeof(msg), stdin);

        send(client, msg, strlen(msg), 0);

        if (strncmp(msg, "exit", 4) == 0)
            break;
    }
    // Ket thuc, dong socket
    close(client);
    return 0;
}