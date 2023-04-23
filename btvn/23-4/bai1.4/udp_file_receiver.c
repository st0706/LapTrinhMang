#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len;
    char buf[1024];
    int file, bytes_received, bytes_written;
    long int file_size, total_received = 0;

    if (argc != 2)
    {
        printf("Sai tham số \n");
        exit(1);
    }

    // Create socket
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // Bind
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(argv[1]));

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind");
        exit(1);
    }

    while (1)
    {
        addr_len = sizeof(client_addr);

        // Receive file size from client
        bytes_received = recvfrom(sock, &file_size, sizeof(file_size), 0, (struct sockaddr *)&client_addr, &addr_len);
        if (bytes_received == -1)
        {
            perror("recvfrom");
            exit(1);
        }

        // Open file
        if ((file = open("received_file", O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
        {
            perror("open");
            exit(1);
        }

        // Receive file data from client
        while (total_received < file_size)
        {
            bytes_received = recvfrom(sock, buf, 1024, 0, (struct sockaddr *)&client_addr, &addr_len);
            if (bytes_received == -1)
            {
                perror("recvfrom");
                exit(1);
            }
            bytes_written = write(file, buf, bytes_received);
            if (bytes_written == -1)
            {
                perror("write");
                exit(1);
            }
            total_received += bytes_received;
        }

        printf("%d bytes received %s: %s\n", bytes_received, inet_ntoa(client_addr.sin_addr), buf);

        // Close file
        close(file);

        // Reset variables for next file transfer
        total_received = 0;
    }

    // Close socket
    close(sock);

    return 0;
}
