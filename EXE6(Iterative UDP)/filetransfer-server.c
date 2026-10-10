#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8081
#define BUFFER_SIZE 1024

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    char filename[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];

    FILE *fp;
    long file_size;
    int bytes_read;

    // Create TCP socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    printf("Server socket created successfully.\n");

    // Server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind
    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(server_fd, 5) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", PORT);

    // Iterative TCP server
    while (1)
    {
        printf("\nWaiting for client...\n");

        // Accept client
        client_fd = accept(server_fd,
                           (struct sockaddr *)&client_addr,
                           &addr_len);

        if (client_fd < 0)
        {
            perror("Accept failed");
            continue;
        }

        printf("Client connected.\n");

        // Receive filename
        memset(filename, 0, BUFFER_SIZE);

        int bytes_received = recv(client_fd,
                                  filename,
                                  BUFFER_SIZE - 1,
                                  0);

        if (bytes_received <= 0)
        {
            printf("Failed to receive filename.\n");
            close(client_fd);
            continue;
        }

        filename[bytes_received] = '\0';

        // Remove newline
        filename[strcspn(filename, "\n")] = '\0';

        printf("Requested file: %s\n", filename);

        // Open file
        fp = fopen(filename, "rb");

        if (fp == NULL)
        {
            strcpy(buffer, "ERROR");

            send(client_fd,
                 buffer,
                 strlen(buffer),
                 0);

            printf("File not found.\n");

            close(client_fd);
            continue;
        }

        // Find file size
        fseek(fp, 0, SEEK_END);
        file_size = ftell(fp);
        rewind(fp);

        // Display file information
        printf("\n========== FILE INFORMATION ==========\n");
        printf("File Name : %s\n", filename);
        printf("File Size : %ld bytes\n", file_size);

        // Display file content
        printf("\n========== FILE CONTENT ==========\n");

        while ((bytes_read = fread(buffer,
                                   1,
                                   BUFFER_SIZE - 1,
                                   fp)) > 0)
        {
            buffer[bytes_read] = '\0';
            printf("%s", buffer);
        }

        printf("\n==================================\n");

        // Send file status
        strcpy(buffer, "FILE_FOUND");

        send(client_fd,
             buffer,
             strlen(buffer),
             0);

        // Send file size
        send(client_fd,
             &file_size,
             sizeof(file_size),
             0);

        // Reset file pointer
        rewind(fp);

        // Send file content
        printf("Sending file to client...\n");

        while ((bytes_read = fread(buffer,
                                   1,
                                   BUFFER_SIZE,
                                   fp)) > 0)
        {
            send(client_fd,
                 buffer,
                 bytes_read,
                 0);
        }

        printf("File transfer completed.\n");

        fclose(fp);

        // Close client connection
        close(client_fd);

        printf("Client disconnected.\n");
    }

    close(server_fd);

    return 0;
}



    
