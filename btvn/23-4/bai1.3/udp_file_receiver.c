#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Sai tham so \n");
        return 1;
    }
    int receiver = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    int portno = atoi(argv[1]);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(portno);

    bind(receiver, (struct sockaddr *)&addr, sizeof(addr));

    char filename[64];
    char content[256];
    char buf[256];
    while (1)
    {
        int ret = recvfrom(receiver, buf, sizeof(buf), 0, NULL, NULL);
        if (ret == -1)
        {
            printf("recvfrom() failed\n");
            return 1;
        }
        if (ret < sizeof(buf))
        {
            buf[ret] = 0;
            int pos = 0;
            // nhan file
            strcpy(filename, buf);
            pos += strlen(filename) + 1;
            printf("Filename: %s\n", filename);
            strcpy(content, buf + pos);
            printf("Content: %s\n", content);
            pos += sizeof(content);
            FILE *output;
            output = fopen(filename, "w");
            fwrite(content, sizeof(char), strlen(content), output);
            fclose(output);
        }
    }
}
