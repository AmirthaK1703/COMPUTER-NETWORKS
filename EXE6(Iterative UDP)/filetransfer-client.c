
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8081
#define BUFFER_SIZE 1024

int main()
{
    int sockfd;
    struct sockaddr_in server_addr;

    char filename[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];

    FILE *fp;
    long file_size;
    long total_received = 0;

    // Create TCP socket
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

    if (inet_pton(AF_INET,
                  SERVER_IP,
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

    printf("Connected to server.\n");

    // Enter filename
    printf("Enter filename to download: ");
    fgets(filename, BUFFER_SIZE, stdin);

    // Remove newline
    filename[strcspn(filename, "\n")] = '\0';

    // Send filename
    send(sockfd,
         filename,
         strlen(filename),
         0);

    // Receive file status
    memset(buffer, 0, BUFFER_SIZE);

    int bytes_received = recv(sockfd,
                              buffer,
                              BUFFER_SIZE - 1,
                              0);

    if (bytes_received <= 0)
    {
        printf("Server disconnected.\n");
        close(sockfd);
        return 0;
    }

    buffer[bytes_received] = '\0';

    // Check if file exists
    if (strcmp(buffer, "ERROR") == 0)
    {
        printf("File not found on server.\n");
        close(sockfd);
        return 0;
    }

    // Receive file size
    bytes_received = recv(sockfd,
                           &file_size,
                           sizeof(file_size),
                           MSG_WAITALL);

    if (bytes_received <= 0)
    {
        printf("Failed to receive file size.\n");
        close(sockfd);
        return 0;
    }

    printf("\n========== FILE INFORMATION ==========\n");
    printf("File Name : %s\n", filename);
    printf("File Size : %ld bytes\n", file_size);

    // Create output file
    char output_filename[BUFFER_SIZE];

    snprintf(output_filename,
             BUFFER_SIZE,
             "received_%s",
             filename);

    fp = fopen(output_filename, "wb");

    if (fp == NULL)
    {
        perror("Cannot create output file");
        close(sockfd);
        return 0;
    }

    // Receive file content
    printf("\nReceiving file...\n");

    while (total_received < file_size)
    {
        bytes_received = recv(sockfd,
                              buffer,
                              BUFFER_SIZE,
                              0);

        if (bytes_received <= 0)
        {
            break;
        }

        fwrite(buffer,
               1,
               bytes_received,
               fp);

        total_received += bytes_received;
    }

    fclose(fp);

    printf("File received successfully.\n");

    // Display received file content
    fp = fopen(output_filename, "rb");

    if (fp == NULL)
    {
        perror("Cannot open received file");
        close(sockfd);
        return 0;
    }

    printf("\n========== RECEIVED FILE CONTENT ==========\n");

    while ((bytes_received = fread(buffer,
                                   1,
                                   BUFFER_SIZE - 1,
                                   fp)) > 0)
    {
        buffer[bytes_received] = '\0';
        printf("%s", buffer);
    }

    printf("\n===========================================\n");

    fclose(fp);

    printf("\nSaved as : %s\n", output_filename);
    printf("Received size : %ld bytes\n", total_received);

    // Close socket
    close(sockfd);

    return 0;
}
