#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8082
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
    char ip[BUFFER_SIZE];
    char message[BUFFER_SIZE];

    // Client ARP table
    struct arp_entry arp_table[MAX_ENTRIES] =
    {
        {"192.168.1.1", "AA:BB:CC:DD:EE:01"},
        {"192.168.1.4", "AA:BB:CC:DD:EE:04"}
    };

    int table_size = 2;

    while (1)
    {
        int sockfd;
        struct sockaddr_in server_addr;

        // Display client table
        printf("\n========== CLIENT ARP TABLE ==========\n");
        printf("%-20s %-20s\n", "IP Address", "MAC Address");
        printf("----------------------------------------\n");

        int i;
        for (i = 0; i < table_size; i++)
        {
            printf("%-20s %-20s\n",
                   arp_table[i].ip,
                   arp_table[i].mac);
        }

        printf("========================================\n");

        // Enter IP
        printf("\nEnter IP address to search: ");
        fgets(ip, BUFFER_SIZE, stdin);

        ip[strcspn(ip, "\n")] = '\0';

        // Exit
        if (strcmp(ip, "exit") == 0)
        {
            printf("Client exiting...\n");
            break;
        }

        // Search CLIENT ARP table first
        int found = 0;
        for (i = 0; i < table_size; i++)
        {
            if (strcmp(ip, arp_table[i].ip) == 0)
            {
                printf("\nIP address found in CLIENT ARP table.\n");
                printf("IP Address : %s\n", arp_table[i].ip);
                printf("MAC Address : %s\n", arp_table[i].mac);

                found = 1;
                break;
            }
        }

        // If found in client table
        if (found)
        {
            continue;
        }

        printf("\nIP not found in CLIENT ARP table.\n");
        printf("Sending ARP request to server...\n");

        // Create TCP socket
        sockfd = socket(AF_INET, SOCK_STREAM, 0);

        if (sockfd < 0)
        {
            perror("Socket creation failed");
            continue;
        }

        // Server address
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(SERVER_PORT);

        if (inet_pton(AF_INET,
                      SERVER_IP,
                      &server_addr.sin_addr) <= 0)
        {
            perror("Invalid server address");
            close(sockfd);
            continue;
        }

        // Connect
        if (connect(sockfd,
                    (struct sockaddr *)&server_addr,
                    sizeof(server_addr)) < 0)
        {
            perror("Connection failed");
            close(sockfd);
            continue;
        }

        // Send IP to server
        send(sockfd,
             ip,
             strlen(ip),
             0);

        // Receive response
        memset(message, 0, BUFFER_SIZE);

        int bytes_received = recv(sockfd,
                                  message,
                                  BUFFER_SIZE - 1,
                                  0);

        if (bytes_received <= 0)
        {
            printf("Server disconnected.\n");
            close(sockfd);
            continue;
        }

        message[bytes_received] = '\0';

        // Server found IP
        if (strncmp(message, "FOUND ", 6) == 0)
        {
            char mac[20];

            strcpy(mac, message + 6);

            printf("\nServer found the IP address.\n");
            printf("MAC Address received: %s\n", mac);

            // Add to client ARP table
            if (table_size < MAX_ENTRIES)
            {
                strcpy(arp_table[table_size].ip, ip);
                strcpy(arp_table[table_size].mac, mac);

                table_size++;

                printf("Entry added to CLIENT ARP table.\n");
            }
        }

        // Server did not find IP
        else if (strcmp(message, "NOT_FOUND") == 0)
        {
            printf("\nIP is not present in SERVER ARP table.\n");
            printf("Server will ask for the MAC address.\n");

            // Receive newly added MAC
            memset(message, 0, BUFFER_SIZE);

            bytes_received = recv(sockfd,
                                  message,
                                  BUFFER_SIZE - 1,
                                  0);

            if (bytes_received > 0)
            {
                message[bytes_received] = '\0';

                if (strncmp(message, "ADDED ", 6) == 0)
                {
                    char mac[20];

                    strcpy(mac, message + 6);

                    printf("\nNew ARP entry received from server.\n");
                    printf("IP Address : %s\n", ip);
                    printf("MAC Address : %s\n", mac);

                    // Add entry to client table
                    if (table_size < MAX_ENTRIES)
                    {
                        strcpy(arp_table[table_size].ip, ip);
                        strcpy(arp_table[table_size].mac, mac);

                        table_size++;

                        printf("New entry added to CLIENT ARP table.\n");
                    }
                }
            }
        }

        else if (strcmp(message, "TABLE_FULL") == 0)
        {
            printf("Server ARP table is full.\n");
        }

        close(sockfd);
    }

    return 0;
}
