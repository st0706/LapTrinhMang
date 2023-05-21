#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <poll.h>
#include <stdbool.h>
#include <time.h>

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

    struct pollfd fds[64];
    int nfds = 1;

    fds[0].fd = listener;
    fds[0].events = POLLIN;

    char buf[256];
    bool visited[64] = {false};
    char nameClients[64][50];

    while (1)
    {
        int ret = poll(fds, nfds, -1);
        if (ret < 0)
        {
            perror("poll() failed");
            break;
        }

        if (fds[0].revents & POLLIN)
        {
            int client = accept(listener, NULL, NULL);
            if (nfds == 64)
            {
                // Tu choi ket noi
                close(client);
            }
            else
            {
                fds[nfds].fd = client;
                fds[nfds].events = POLLIN;
                nfds++;

                printf("Có kết nối mới: %d\n", client);
                int s = send(client, "Vui lòng nhập tên của bạn (đúng định dạng client_id:name):", strlen("Vui lòng nhập tên của bạn (đúng định dạng client_id:name):"), 0);
            }
        }

        for (int i = 1; i < nfds; i++)
            if (fds[i].revents & POLLIN)
            {
                ret = recv(fds[i].fd, buf, sizeof(buf), 0);
                if (ret <= 0)
                {
                    close(fds[i].fd);
                    // Xoa khoi mang
                    if (i < nfds - 1)
                        fds[i] = fds[nfds - 1];
                    nfds--;
                    i--;
                }
                else
                {
                    buf[ret] = 0;
                    if (visited[i] == false)
                    {
                        if (strchr(buf, ':') == NULL)
                        {
                            send(fds[i].fd, "Vui lòng nhập tên của bạn (đúng định dạng client_id:name):", strlen("Vui lòng nhập tên của bạn (đúng định dạng client_id:name):"), 0);
                        }
                        else
                        {
                            char id[20];
                            char name[20];
                            char *token = strtok(buf, ":");
                            strcpy(id, token);
                            token = strtok(NULL, ":");
                            token[strlen(token) - 1] = 0;
                            strcpy(name, token);
                            if (strcmp(id, "client_id") != 0 || strlen(name) < 3)
                            {
                                send(fds[i].fd, "Vui lòng nhập tên của bạn (đúng định dạng client_id:name):", strlen("Vui lòng nhập tên của bạn (đúng định dạng client_id:name):"), 0);
                            }
                            else
                            {
                                strcpy(nameClients[i], name);
                                printf("%s đã kết nối tới Server chat.\n", name);
                                visited[i] = true;
                            }
                        }
                    }
                    else
                    {
                        buf[ret] = 0;
                        time_t now = time(NULL);
                        struct tm *tm_info = localtime(&now);
                        char time_str[50];
                        strftime(time_str, 50, "%Y/%m/%d %I:%M:%S%p", tm_info);
                        char full_msg[500];
                        int msg_len = snprintf(full_msg, sizeof(full_msg), "%s %s:%s", time_str, nameClients[i], buf);
                        full_msg[strlen(full_msg) - 1] = 0;
                        for (int j = 1; j < nfds; j++)
                        {
                            if (j != i)
                            {
                                send(fds[j].fd, full_msg, strlen(full_msg), 0);
                            }
                        }
                    }
                }
            }
    }

    close(listener);

    return 0;
}