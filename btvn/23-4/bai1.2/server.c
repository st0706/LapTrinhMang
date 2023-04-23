#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <string.h>

int count(char *s1, char *s2)
{
    int res = 0;
    char *pos = s2;
    while ((pos = strstr(pos, s1)) != NULL)
    {
        res++;
        pos += strlen(s1);
    }
    return res;
}

int main()
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed");
        return 1;
    }

    if (listen(listener, 5))
    {
        perror("listen() failed");
        return 1;
    }

    struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);

    int client = accept(listener, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (client == -1)
    {
        perror("accept() failed");
        return 1;
    }
    printf("New client connected: %d\n", client);

    // Truyen nhan du lieu
    char buf[256];
    int res;
    int ret = recv(client, buf, sizeof(buf), 0);

    buf[ret] = 0;
    char des[] = "0123456789";
    printf("Số lần xuất hiện xâu đã cho: %d\n", count(des, buf));
    close(client);
    close(listener);

    return 0;
}