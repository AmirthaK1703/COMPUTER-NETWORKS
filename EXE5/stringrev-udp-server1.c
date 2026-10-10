#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main()
{
    int sockfd;
    char buffer[BUFFER_SIZE];

    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    // Initialize server address
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind socket
    if (bind(sockfd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(sockfd);
        exit(1);
    }

    printf("UDP Server is running...\n");
    printf("Waiting for message from client...\n");

    // Receive message
    int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                     (struct sockaddr *)&client_addr,
                     &client_len);

    if (n < 0)
    {
        perror("recvfrom failed");
        close(sockfd);
        exit(1);
    }

    buffer[n] = '\0';

    printf("\nMessage received from client: %s\n", buffer);

    // Reverse the message
    int i, j;
    char temp;

    i = 0;
    j = strlen(buffer) - 1;

    while (i < j)
    {
        temp = buffer[i];
        buffer[i] = buffer[j];
        buffer[j] = temp;

        i++;
        j--;
    }

    // Display reversed message
    printf("Reversed message: %s\n", buffer);

    // Close socket
    close(sockfd);

    return 0;
}
