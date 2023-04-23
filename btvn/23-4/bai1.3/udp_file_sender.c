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
    if (argc != 4)
    {
        printf("Sai tham so \n");
        return 1;
    }
    int sender = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    int portno = atoi(argv[3]);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[2]);
    addr.sin_port = htons(portno);

    // Truyen nhan du lieu
    char buf[512];
    char content[256];
    while (1)
    {
        printf("Name file's sender: %s\n", argv[1]);
        strcpy(buf, argv[1]);
        int pos = strlen(argv[1]);
        buf[pos] = 0;
        pos++;
        printf("Content file: ");
        fgets(content, sizeof(content), stdin);
        content[strlen(content) - 1] = 0;
        strcpy(buf + pos, content);
        pos += strlen(content);
        int ret = sendto(sender, buf, pos, 0, (struct sockaddr *)&addr, sizeof(addr));
        printf("%d bytes sent.\n", ret);
    }
}