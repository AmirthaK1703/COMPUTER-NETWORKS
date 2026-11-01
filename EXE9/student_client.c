#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>

#define PORT 8082
#define BUF 512

int main() {
    int sock;
    struct sockaddr_in server;
    socklen_t len = sizeof(server);
    char regno[BUF], response[BUF];

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    struct timeval timeout = {3, 0};
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
               &timeout, sizeof(timeout));

    while (1) {
        printf("\nEnter registration number (or EXIT): ");
        if (scanf("%511s", regno) != 1)
            break;

        if (strcmp(regno, "EXIT") == 0)
            break;

        if (strlen(regno) == 0) {
            printf("Invalid registration number\n");
            continue;
        }

        sendto(sock, regno, strlen(regno), 0,
               (struct sockaddr *)&server, len);

        int n = recvfrom(sock, response, BUF - 1, 0,
                         (struct sockaddr *)&server, &len);

        if (n > 0) {
            response[n] = '\0';
            printf("\n%s\n", response);
        } else {
            printf("Server timeout. Please try again.\n");
        }
    }

    close(sock);
    return 0;
}
