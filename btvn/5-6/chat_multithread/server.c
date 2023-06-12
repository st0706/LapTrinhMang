#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>

void *client_thread(void *);
int num_users = 0;
int users[64];

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

    while (1)
    {
        int client = accept(listener, NULL, NULL);
        if (client == -1)
        {
            perror("accept() failed");
            continue;
            ;
        }
        printf("New client connected: %d\n", client);

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, client_thread, &client);
        pthread_detach(thread_id);
    }

    close(listener);

    return 0;
}

void *client_thread(void *param)
{
    char buf[1024], id[20], name[20];
    int client = *(int *)param;
    int s = send(client, "Vui lòng nhập tên của bạn (đúng định dạng client_id:name):", strlen("Vui lòng nhập tên của bạn (đúng định dạng client_id:name):"), 0);
    if (s <= 0)
    {
        close(client);
        pthread_exit(NULL);
    }
    bool visited = false;
    while (1)
    {
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
            break;

        buf[ret] = 0;
        if (!visited)
        {
            if (strchr(buf, ':') == NULL)
            {
                send(client, "Vui lòng nhập tên của bạn (đúng định dạng client_id:name):", strlen("Vui lòng nhập tên của bạn (đúng định dạng client_id:name):"), 0);
            }
            else
            {
                char *token = strtok(buf, ":");
                strcpy(id, token);
                token = strtok(NULL, ":");
                token[strlen(token) - 1] = 0;
                strcpy(name, token);
                if (strcmp(id, "client_id") != 0 || strlen(name) < 3)
                {
                    send(client, "Vui lòng nhập tên của bạn (đúng định dạng client_id:name):", strlen("Vui lòng nhập tên của bạn (đúng định dạng client_id:name):"), 0);
                }
                else
                {
                    printf("%s đã kết nối tới Server chat.\n", name);
                    visited = true;
                    users[num_users++] = client;
                }
            }
        }
        else
        {
            // memset(buf, 0, sizeof(buf));
            // int ret = recv(client, buf, sizeof(buf), 0);
            // if (ret <= 0)
            // {
            //     // Client đã ngắt kết nối
            //     break;
            // }
            buf[ret] = 0;
            time_t now = time(NULL);
            struct tm *tm_info = localtime(&now);
            char time_str[50];
            strftime(time_str, 50, "%Y/%m/%d %I:%M:%S%p", tm_info);
            char full_msg[500];
            int msg_len = snprintf(full_msg, sizeof(full_msg), "%s %s:%s", time_str, name, buf);
            full_msg[strlen(full_msg) - 1] = 0;
            for (int i = 0; i < num_users; i++)
            {
                if (users[i] != client)
                {
                    send(users[i], full_msg, strlen(full_msg), 0);
                }
            }
            visited = true;
        }
    }

    close(client);
}