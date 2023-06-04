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
#include <sys/wait.h>
#include <signal.h>

#define BUFFER_SIZE 1024

void signalHandler(int signo)
{
    int pid = wait(NULL);
    printf("Client %d đã thoát khỏi chương trình!\n", pid);
}

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

void handle_client(int client)
{
    char username[50], password[50];
    bool visited = false;
    char buf[1024];
    char *request = "Vui lòng nhập tài khoản và mật khẩu của bạn (đúng dạng username password): ";
    send(client, request, strlen(request), 0);

    while (1)
    {
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
        {
            // Client đã ngắt kết nối
            break;
        }

        buf[ret] = 0;
        if (!visited)
        {
            if (strchr(buf, ' ') == NULL)
            {
                send(client, request, strlen(request), 0);
            }
            else
            {
                char name[20];
                char pass[20];
                sscanf(buf, "%s %s", name, pass);
                if (authenticate(name, pass))
                {
                    strcpy(username, name);
                    strcpy(password, pass);

                    printf("%s đã kết nối tới Server chat.\n", name);
                    visited = true;
                    send(client, "Kết nối thành công, vui lòng nhập lệnh: ", strlen("Kết nối thành công, vui lòng nhập lệnh: "), 0);
                }
                else
                {
                    send(client, "Tài khoản hoặc mật khẩu không đúng! Vui lòng nhập lại: ", strlen("Tài khoản hoặc mật khẩu không đúng! Vui lòng nhập lại: "), 0);
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
                send(client, result_buffer, strlen(result_buffer), 0);
                fseek(fp, 0, SEEK_END);
                long size = ftell(fp);
                if (size == 0)
                {
                    char *error_message = "Error executing command\n";
                    send(client, error_message, strlen(error_message) + 1, 0);
                }
                fclose(fp);
            }
        }
    }

    close(client);
    exit(0);
}

int main()
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    signal(SIGCHLD, signalHandler);

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
        }

        pid_t pid = fork();
        if (pid == -1)
        {
            perror("fork() failed");
            close(client);
            continue;
        }
        else if (pid == 0)
        {
            // Child process
            close(listener);
            handle_client(client);
            break;
        }
        else
        {
            // Parent process
            close(client);
        }
    }

    close(listener);
    return 0;
}
