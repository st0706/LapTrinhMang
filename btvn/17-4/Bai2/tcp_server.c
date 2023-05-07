#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
int main(int argc, char *argv[])
{
    // tao socket
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }
    // Khai bao dia chi cua server
    int portno = atoi(argv[1]);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(portno);

    // tao bind
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed");
        return 1;
    }
    // lang nghe
    if (listen(listener, 5))
    {
        perror("listen() failed");
        return 1;
    }

    // Chap nhan ket noi
    struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(addr);

    int client = accept(listener, (struct sockaddr *)&clientAddr, &clientAddrLen);
    printf("Client IP: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

    // gui file
    FILE *welcome;
    welcome = fopen(argv[2], "r");
    // Định kích thước của file
    fseek(welcome, 0, SEEK_END);
    long file_size = ftell(welcome);
    rewind(welcome);
    // Đọc toàn bộ nội dung của file
    char *buf1 = (char *)malloc(file_size + 1); // Cấp phát đệm đọc file
    if (buf1 == NULL)
    {
        perror("Lỗi cấp phát bộ nhớ");
        fclose(welcome);
        return 1;
    }

    fread(buf1, 1, file_size, welcome);
    buf1[file_size] = '\0'; // Thêm kí tự kết thúc chuỗi
    printf("Nội dung file:\n%s", buf1);
    fclose(welcome);
    send(client, buf1, strlen(buf1), 0);

    // nhan file
    char buf2[256];
    FILE *output;
    output = fopen(argv[3], "w");
    while (1)
    {
        int ret = recv(client, buf2, sizeof(buf2), 0);
        if (ret <= 0)
        {
            printf("Ket noi bi dong.\n");
            break;
        }
        if (ret < sizeof(buf2))
            buf2[ret] = 0;

        fwrite(buf2, sizeof(char), ret, output);
    }
    fclose(output);
    close(client);
    close(listener);
}