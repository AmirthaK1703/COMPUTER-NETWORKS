
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

int main()
{
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    printf("Client socket created successfully.\n");

    // Server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP,
                  &server_addr.sin_addr) <= 0)
    {
        perror("Invalid server address");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if (connect(sockfd,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server %s:%d\n",
           SERVER_IP, SERVER_PORT);

    // Chat loop
    while (1)
    {
        // Send message to server
        printf("Client: ");
        fgets(buffer, BUFFER_SIZE, stdin);

        send(sockfd, buffer, strlen(buffer), 0);

        // Exit condition
        if (strncmp(buffer, "exit", 4) == 0)
        {
            printf("Chat ended.\n");
            break;
        }

        memset(buffer, 0, BUFFER_SIZE);

        // Receive message from server
        int bytes_received = recv(sockfd, buffer,
                                  BUFFER_SIZE - 1, 0);

        if (bytes_received <= 0)
        {
            printf("Server disconnected.\n");
            break;
        }

        buffer[bytes_received] = '\0';

        printf("Server: %s", buffer);

        if (strncmp(buffer, "exit", 4) == 0)
        {
            printf("Chat ended.\n");
            break;
        }
    }

    close(sockfd);

    return 0;
}
