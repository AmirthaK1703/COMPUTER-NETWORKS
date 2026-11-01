#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>

#define PORT 8080
#define BUF 512

int main() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in server;
    char buffer[BUF], response[BUF];
    socklen_t len = sizeof(server);

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    struct timeval timeout = {2, 0};
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
               &timeout, sizeof(timeout));

    char location[50];
    int seq = 0, choice;

    printf("Enter location (no spaces): ");
    scanf("%49s", location);

    while (1) {
        printf("\n1. Send weather data\n2. Query statistics\n3. Exit\n");
        scanf("%d", &choice);

        if (choice == 1) {
            float t, h, p, w;
            printf("Temperature, humidity, pressure, wind speed: ");
            if (scanf("%f %f %f %f", &t, &h, &p, &w) != 4) {
                printf("Invalid input\n");
                break;
            }

            seq++;
            sprintf(buffer, "DATA %s %d %.2f %.2f %.2f %.2f",
                    location, seq, t, h, p, w);

            int acknowledged = 0;

            for (int attempt = 0; attempt < 3; attempt++) {
                sendto(sock, buffer, strlen(buffer), 0,
                       (struct sockaddr *)&server, len);

                int n = recvfrom(sock, response, BUF - 1, 0,
                                 (struct sockaddr *)&server, &len);

                if (n > 0) {
                    response[n] = '\0';
                    printf("Server: %s\n", response);
                    acknowledged = 1;
                    break;
                }

                printf("Timeout. Retransmitting packet...\n");
            }

            if (!acknowledged)
                printf("Transmission failed after 3 attempts\n");
        }

        else if (choice == 2) {
            sprintf(buffer, "QUERY %s", location);

            sendto(sock, buffer, strlen(buffer), 0,
                   (struct sockaddr *)&server, len);

            int n = recvfrom(sock, response, BUF - 1, 0,
                             (struct sockaddr *)&server, &len);

            if (n > 0) {
                response[n] = '\0';
                printf("%s\n", response);
            } else {
                printf("Server timeout\n");
            }
        }

        else if (choice == 3) {
            break;
        }

        else {
            printf("Invalid choice\n");
        }
    }

    close(sock);
    return 0;
}
