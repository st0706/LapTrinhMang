#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

#define MAX_CLIENTS 10
#define MAX_USERNAME_LENGTH 64

void *client_thread(void *);

struct Client
{
    int socket;
    char username[MAX_USERNAME_LENGTH];
};

struct Client clients[MAX_CLIENTS];
int num_clients = 0;

int is_username_taken(const char *username)
{
    for (int i = 0; i < num_clients; i++)
    {
        if (strcmp(clients[i].username, username) == 0)
        {
            return 1; // Username already taken
        }
    }
    return 0; // Username available
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

    while (1)
    {
        int client = accept(listener, NULL, NULL);
        if (client == -1)
        {
            perror("accept() failed");
            continue;
        }

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, client_thread, &client);
        pthread_detach(thread_id);
    }

    close(listener);

    return 0;
}

void *client_thread(void *param)
{
    int client = *(int *)param;
    char buf[256];
    int isLog = 0;

    while (1)
    {
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
            break;

        buf[ret] = 0;
        if (!isLog)
        {
            if (strncmp(buf, "JOIN ", 5) == 0)
            {
                char username[MAX_USERNAME_LENGTH];
                strncpy(username, buf + 5, MAX_USERNAME_LENGTH - 1);
                username[MAX_USERNAME_LENGTH - 1] = '\0';

                if (is_username_taken(username))
                {
                    // Send error message back to client
                    send(client, "200 NICKNAME IN USE\n", strlen("200 NICKNAME IN USE\n"), 0);
                }
                else
                {
                    // Save the client and username
                    struct Client new_client;
                    new_client.socket = client;
                    strncpy(new_client.username, username, MAX_USERNAME_LENGTH);
                    clients[num_clients] = new_client;
                    num_clients++;

                    // Broadcast join message to all clients
                    for (int i = 0; i < num_clients; i++)
                    {
                        char res1[256];
                        sprintf(res1, "JOIN %s", username);
                        send(clients[i].socket, res1, strlen(res1), 0);
                    }

                    isLog = 1;
                }
            }
        }
    }

    // Remove client from the list of connected clients
    for (int i = 0; i < num_clients; i++)
    {
        if (clients[i].socket == client)
        {
            for (int j = i; j < num_clients - 1; j++)
            {
                clients[j] = clients[j + 1];
            }
            num_clients--;
            break;
        }
    }

    close(client);
}
