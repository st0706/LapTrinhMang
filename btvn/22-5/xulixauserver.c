#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/select.h>
#include <stdbool.h>
#include <ctype.h>

void sanitize_input(char *str)
{
    int i, j;
    bool prev_space = true;

    for (i = 0, j = 0; str[i] != '\0'; i++)
    {
        if (str[i] == ' ')
        {
            if (!prev_space)
            {
                str[j++] = str[i];
                prev_space = true;
            }
        }
        else
        {
            if (prev_space)
            {
                str[j++] = toupper(str[i]);
                prev_space = false;
            }
            else
            {
                str[j++] = tolower(str[i]);
            }
        }
    }

    str[j] = '\0';
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

    fd_set fdread;

    int clients[64];
    char nameClients[64][50];
    int num_clients = 0;
    bool visited[64] = {false};

    char buf[256];

    while (1)
    {
        // Xóa tất cả socket trong tập fdread
        FD_ZERO(&fdread);

        // Thêm socket listener vào tập fdread
        FD_SET(listener, &fdread);
        int maxdp = listener + 1;

        // Thêm các socket client vào tập fdread
        for (int i = 0; i < num_clients; i++)
        {
            FD_SET(clients[i], &fdread);
            if (maxdp < clients[i] + 1)
                maxdp = clients[i] + 1;
        }

        // Chờ đến khi sự kiện xảy ra
        int ret = select(maxdp, &fdread, NULL, NULL, NULL);

        if (ret < 0)
        {
            perror("select() failed");
            return 1;
        }

        // Kiểm tra sự kiện có yêu cầu kết nối
        if (FD_ISSET(listener, &fdread))
        {

            int client = accept(listener, NULL, NULL);

            clients[num_clients++] = client;
            char msg[64];
            sprintf(msg, "Xin chào. Hiện có %d clients đang kết nối:", num_clients);
            int s = send(client, msg, strlen(msg), 0);
        }

        // Kiểm tra sự kiện có dữ liệu truyền đến socket client
        for (int i = 0; i < num_clients; i++)
        {
            if (FD_ISSET(clients[i], &fdread))
            {
                ret = recv(clients[i], buf, sizeof(buf), 0);
                if (ret <= 0)
                {
                    // TODO: Client đã ngắt kết nối, xóa client ra khỏi mảng
                    continue;
                }
                buf[ret] = 0;
                if (strcmp(buf, "exit\n") == 0)
                {
                    send(clients[i], "Tạm biệt\n", strlen("Tạm biệt\n"), 0);
                    close(clients[i]);
                    clients[i] = 0;
                }
                else
                {
                    char editmsg[1024];
                    sanitize_input(buf);
                    strcpy(editmsg, buf);
                    send(clients[i], editmsg, strlen(editmsg), 0);
                }
            }
        }
    }

    close(listener);

    return 0;
}