#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>

int main(int argc, char *argv[])
{
    struct sockaddr_in server_addr;
    socklen_t addr_len;
    char buf[1024];
    int file, bytes_read, bytes_sent;
    long int file_size, total_sent = 0;

    if (argc != 4)
    {
        printf("Nhập sai tham số \n");
        exit(1);
    }

    // Create socket
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // Set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(atoi(argv[2]));

    // Open file
    if ((file = open(argv[3], O_RDONLY)) == -1)
    {
        perror("open");
        exit(1);
    }

    // Get file size
    file_size = lseek(file, 0, SEEK_END);
    lseek(file, 0, SEEK_SET);

    // Send file size to server
    bytes_sent = sendto(sock, &file_size, sizeof(file_size), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (bytes_sent == -1)
    {
        perror("sendto");
        exit(1);
    }

    // Send file data to server
    while ((bytes_read = read(file, buf, 1024)) > 0)
    {
        bytes_sent = sendto(sock, buf, bytes_read, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (bytes_sent == -1)
        {
            perror("sendto");
            exit(1);
        }
        total_sent += bytes_sent;
        usleep(1000); // Sleep for 1ms
    }

    printf("Sent %ld bytes from file %s to server.\n", total_sent, argv[3]);

    // Close file and socket
    close(file);
    close(sock);

    return 0;
}
