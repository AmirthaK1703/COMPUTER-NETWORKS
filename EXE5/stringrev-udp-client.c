
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main()
{
    int sockfd;
    char message[BUFFER_SIZE];

    struct sockaddr_in server_addr;

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
    server_addr.sin_port = htons(PORT);

    // Server IP address
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    // Get message from user
    printf("Enter message to send: ");
    fgets(message, BUFFER_SIZE, stdin);

    // Remove newline
    message[strcspn(message, "\n")] = '\0';

    // Send message to server
    if (sendto(sockfd, message, strlen(message), 0,
               (struct sockaddr *)&server_addr,
               sizeof(server_addr)) < 0)
    {
        perror("sendto failed");
        close(sockfd);
        exit(1);
    }

    printf("Message sent successfully.\n");

    // Close socket
    close(sockfd);

    return 0;
}

[24bcs132@mepcolinux exe5]$cat server2.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024

// Accept command-line arguments
int main(int argc, char *argv[])
{
    // Check if the user provided the port number
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    // Convert string argument to integer port
    int port = atoi(argv[1]);

    int sockfd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port); // Dynamic port used here

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(sockfd);
        exit(1);
    }

    printf("UDP Server is running on port %d...\n", port);
    printf("Waiting for message from client...\n");

    int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                     (struct sockaddr *)&client_addr, &client_len);
    if (n < 0)
    {
        perror("recvfrom failed");
        close(sockfd);
        exit(1);
    }

    buffer[n] = '\0';
    printf("\nMessage received from client: %s\n", buffer);

    close(sockfd);
    return 0;
}

[24bcs132@mepcolinux exe5]$cat client2.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

// Accept command-line arguments
int main(int argc, char *argv[])
{
    // Check if both server IP and port are provided
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        exit(1);
    }

    char *server_ip = argv[1];   // Target IP string
    int port = atoi(argv[2]);    // Target port int

    int sockfd;
    char message[BUFFER_SIZE];
    struct sockaddr_in server_addr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port); // Dynamic port used here

    // Dynamically convert the passed IP string to network format
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0)
    {
        fprintf(stderr, "Invalid IP address format: %s\n", server_ip);
        close(sockfd);
        exit(1);
    }

    printf("Enter message to send: ");
    fgets(message, BUFFER_SIZE, stdin);
    message[strcspn(message, "\n")] = '\0';

    if (sendto(sockfd, message, strlen(message), 0,
               (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("sendto failed");
        close(sockfd);
        exit(1);
    }

    printf("Message sent successfully to %s:%d\n", server_ip, port);

    close(sockfd);
    return 0;
}

[24bcs132@mepcolinux exe5]$cat server3.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

void perform_byte_destuffing(const char *msg, char *destuffed_bin) {
    char header_prefix[] = "10101010 10111011 11001100 00010001 00100010 00110011 10101010 11101110 11001100 01010110 01100111 00110011 00001000 00001000 00001000 00001000 00001000 00001000 00001000 00001000 ";
    strcpy(destuffed_bin, header_prefix);

    char data_bin[256] = "";
    int i,b;
    for (i = 0; msg[i] != '\0'; i++) {
        unsigned char c = msg[i];
        for (b = 7; b >= 0; b--) {
            strcat(data_bin, ((c >> b) & 1) ? "1" : "0");
        }
        if (msg[i + 1] != '\0') strcat(data_bin, " ");
    }
    strcat(destuffed_bin, data_bin);
    strcat(destuffed_bin, " 00000000");
}

int verify_ppp_checksum(const char raw_binary_message[], int wordSize, const char expected_fcs[]) {
    char message[8192] = "";
    int pos = 0;
    int i;

    for (i = 0; raw_binary_message[i] != '\0'; i++) {
        if (raw_binary_message[i] != ' ' && raw_binary_message[i] != '\n' && raw_binary_message[i] != '\r') {
            message[pos++] = raw_binary_message[i];
        }
    }
    message[pos] = '\0';

    int len = strlen(message);
    int rem = len % wordSize;

    if (rem != 0) {
        int pad = wordSize - rem;
        for (i = 0; i < pad; i++) {
            strcat(message, "0");
        }
        len = strlen(message);
    }

    int words = len / wordSize;
    int j,sum[128] = {0};

    for ( i = 0; i < wordSize; i++) {
        sum[i] = message[i] - '0';
    }

    for (i = 1; i < words; i++) {
        int carry = 0;
        for (j = wordSize - 1; j >= 0; j--) {
            int bit = sum[j] + (message[i * wordSize + j] - '0') + carry;
            sum[j] = bit % 2;
            carry = bit / 2;
        }
        while (carry) {
            for (j = wordSize - 1; j >= 0; j--) {
                int bit = sum[j] + carry;
                sum[j] = bit % 2;
                carry = bit / 2;
                if (carry == 0) break;
            }
        }
    }

    char clean_fcs[128] = "";
    for (i = 0; expected_fcs[i] != '\0'; i++) {
        if (expected_fcs[i] != ' ') {
            clean_fcs[pos++] = expected_fcs[i];
        }
    }
    clean_fcs[pos] = '\0';

    int check[128];
    for ( i = 0; i < wordSize; i++) {
        check[i] = clean_fcs[i] - '0';
    }

    int result[128];
    for (i = 0; i < wordSize; i++) result[i] = sum[i];

    int carry = 0;
    for (i = wordSize - 1; i >= 0; i--) {
        int bit = result[i] + check[i] + carry;
        result[i] = bit % 2;
        carry = bit / 2;
    }

    while (carry) {
        for (i = wordSize - 1; i >= 0; i--) {
            int bit = result[i] + carry;
            result[i] = bit % 2;
            carry = bit / 2;
            if (carry == 0) break;
        }
    }

    printf("\nReceiver Side Checksum Verification");
    printf("\n-----------------------------------");
    printf("\nResult : ");
    for (i = 0; i < wordSize; i++) {
        printf("%d", result[i]);
        if (wordSize == 16 && i == 7) printf(" ");
    }
    printf("\n");

    for ( i = 0; i < wordSize; i++) {
        if (result[i] != 1) return 0;
    }
    return 1;
}
