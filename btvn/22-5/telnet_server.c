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
#define BUFFER_SIZE 1024

int authenticate(char *username, char *password)
{
    FILE *db;
    char line[100];
    char *user, *pass;

    db = fopen("logindb.txt", "r");
    if (db == NULL)
    {
        perror("Failed to open database file");
        return -1;
    }

    while (fgets(line, sizeof(line), db))
    {
        user = strtok(line, " ");
        pass = strtok(NULL, "\n");
        if (user != NULL && pass != NULL && strcmp(username, user) == 0 && strcmp(password, pass) == 0)
        {
            fclose(db);
            return 1; // Authentication successful
        }
    }

    fclose(db);
    return 0; // Authentication failed
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
    char username[64][50], Password[64][50];
    bool visited[64] = {false};
    char buf[1024];
    char *request = "Vui lòng nhập tài khoản và mật khẩu của bạn(đúng dạng username password): ";

    while (1)
    {
        FD_ZERO(&fdread);
        FD_SET(listener, &fdread);
        int maxdp = listener + 1;

        for (int i = 0; i < num_clients; i++)
        {
            FD_SET(clients[i], &fdread);
            if (maxdp < clients[i] + 1)
                maxdp = clients[i] + 1;
        }

        int ret = select(maxdp, &fdread, NULL, NULL, NULL);
        if (ret < 0)
        {
            perror("select() failed");
            return 1;
        }

        if (FD_ISSET(listener, &fdread))
        {
            int client = accept(listener, NULL, NULL);
            int s = send(client, request, strlen(request), 0);
            clients[num_clients++] = client;
        }

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
                if (visited[i] == false)
                {
                    if (strchr(buf, ' ') == NULL)
                    {
                        send(clients[i], request, strlen(request), 0);
                    }
                    else
                    {
                        char pass[20];
                        char name[20];
                        sscanf(buf, "%s %s", name, pass);
                        if (authenticate(name, pass))
                        {
                            strcpy(username[i], name);
                            strcpy(Password[i], pass);

                            printf("%s đã kết nối tới Server chat.\n", name);
                            visited[i] = true;
                            send(clients[i], "Kết nốt thành công, vui lòng nhập lệnh: ", strlen("Kết nốt thành công, vui lòng nhập lệnh: "), 0);
                        }
                        else
                        {
                            send(clients[i], "Tài khoản hoặc mật khẩu không đúng! Vui lòng nhập lại: ", strlen("Tài khoản hoặc mật khẩu không đúng! Vui lòng nhập lại: "), 0);
                        }
                    }
                }
                else
                {
                    // Thực hiện lệnh từ client
                    buf[ret] = 0;
                    char command[3000];
                    snprintf(command, 3000, "%.900s > out.txt", buf);
                    printf("%s", command);
                    system(command);

                    // Đọc kết quả từ file out.txt
                    FILE *fp = fopen("out.txt", "r");

                    if (fp != NULL)
                    {
                        char result_buffer[BUFFER_SIZE];
                        memset(result_buffer, 0, BUFFER_SIZE);
                        fread(result_buffer, sizeof(char), BUFFER_SIZE - 1, fp);
                        send(clients[i], result_buffer, strlen(result_buffer), 0);
                        fseek(fp, 0, SEEK_END);
                        long size = ftell(fp);
                        if (size == 0)
                        {
                            char *error_message = "Error executing command\n";
                            send(clients[i], error_message, strlen(error_message) + 1, 0);
                        }
                        fclose(fp);
                    }
                }
            }
        }
    }
    close(listener);

    return 0;
}