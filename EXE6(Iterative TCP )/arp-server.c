#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8082
#define BUFFER_SIZE 1024
#define MAX_ENTRIES 20

// ARP table structure
struct arp_entry
{
    char ip[20];
    char mac[20];
};

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    char ip[BUFFER_SIZE];
    char mac[BUFFER_SIZE];
    char message[BUFFER_SIZE];

    // Server ARP table
    struct arp_entry arp_table[MAX_ENTRIES] =
    {
        {"192.168.1.1", "AA:BB:CC:DD:EE:01"},
        {"192.168.1.2", "AA:BB:CC:DD:EE:02"},
        {"192.168.1.3", "AA:BB:CC:DD:EE:03"}
    };

    int table_size = 3;

    // Create TCP socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    printf("ARP Server socket created successfully.\n");

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

    printf("ARP Server listening on port %d...\n", PORT);

    // Iterative TCP server
    while (1)
    {
        printf("\nWaiting for client...\n");

        client_fd = accept(server_fd,
                           (struct sockaddr *)&client_addr,
                           &addr_len);

        if (client_fd < 0)
        {
            perror("Accept failed");
            continue;
        }

        printf("Client connected.\n");

        // Receive IP address
        memset(ip, 0, BUFFER_SIZE);

        int bytes_received = recv(client_fd,
                                  ip,
                                  BUFFER_SIZE - 1,
                                  0);

        if (bytes_received <= 0)
        {
            close(client_fd);
            continue;
        }

        ip[bytes_received] = '\0';

        printf("\nARP Request received for IP: %s\n", ip);

        // Search server ARP table
        int found = 0;
        int i;
        for (i = 0; i < table_size; i++)
        {
            if (strcmp(ip, arp_table[i].ip) == 0)
            {
                printf("IP found in SERVER ARP table.\n");
                printf("MAC Address: %s\n", arp_table[i].mac);

                // Send FOUND message
                sprintf(message, "FOUND %s", arp_table[i].mac);

                send(client_fd,
                     message,
                     strlen(message),
                     0);

                found = 1;
                break;
            }
        }

        // If IP not found
        if (!found)
        {
            printf("IP not found in SERVER ARP table.\n");

            // Tell client that server does not have it
            strcpy(message, "NOT_FOUND");

            send(client_fd,
                 message,
                 strlen(message),
                 0);

            // Ask server user for MAC
            printf("Enter MAC address for %s: ", ip);
            fgets(mac, BUFFER_SIZE, stdin);

            mac[strcspn(mac, "\n")] = '\0';

            // Add to server table
            if (table_size < MAX_ENTRIES)
            {
                strcpy(arp_table[table_size].ip, ip);
                strcpy(arp_table[table_size].mac, mac);

                table_size++;

                printf("New entry added to SERVER ARP table.\n");

                // Send new MAC to client
                sprintf(message, "ADDED %s", mac);

                send(client_fd,
                     message,
                     strlen(message),
                     0);
            }
            else
            {
                strcpy(message, "TABLE_FULL");

                send(client_fd,
                     message,
                     strlen(message),
                     0);
            }
        }

        // Display server ARP table
        printf("\n========== SERVER ARP TABLE ==========\n");
        printf("%-20s %-20s\n", "IP Address", "MAC Address");
        printf("----------------------------------------\n");

        for ( i = 0; i < table_size; i++)
        {
            printf("%-20s %-20s\n",
                   arp_table[i].ip,
                   arp_table[i].mac);
        }

        printf("========================================\n");

        // Close client connection
        close(client_fd);

        printf("Client disconnected.\n");
    }

    close(server_fd);

    return 0;
}
