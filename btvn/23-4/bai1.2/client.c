#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
int main()
{
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(9000);

    if (connect(client, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("connect() failed");
        return 1;
    }

    // Truyen nhan du lieu
    FILE *f;
    f = fopen("streaming.txt", "r");
    // Định kích thước của file
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    rewind(f);
    // Đọc toàn bộ nội dung của file
    char *buf = (char *)malloc(file_size + 1); // Cấp phát đệm đọc file
    if (buf == NULL)
    {
        perror("Lỗi cấp phát bộ nhớ");
        fclose(f);
        return 1;
    }

    fread(buf, 1, file_size, f);
    buf[file_size] = '\0'; // Thêm kí tự kết thúc chuỗi
    printf("Nội dung file:\n%s", buf);
    fclose(f);
    send(client, buf, strlen(buf), 0);

    // Ket thuc, dong socket
    close(client);

    return 0;
}