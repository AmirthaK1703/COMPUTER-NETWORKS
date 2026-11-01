#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define MAX 100
#define BUF 512

typedef struct {
    char location[50];
    int lastSeq, count;
    float tempSum, humiditySum, pressureSum, windSum;
} Weather;

Weather records[MAX];
int recordCount = 0;

int findLocation(char *loc) {
    for (int i = 0; i < recordCount; i++)
        if (strcmp(records[i].location, loc) == 0)
            return i;
    return -1;
}

int main() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in server, client;
    socklen_t len = sizeof(client);
    char buffer[BUF], response[BUF];

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind");
        return 1;
    }

    printf("Weather server running on port %d\n", PORT);

    while (1) {
        int n = recvfrom(sock, buffer, BUF - 1, 0,
                         (struct sockaddr *)&client, &len);
        if (n <= 0) continue;
        buffer[n] = '\0';

        char cmd[20], location[50];
        int seq;
        float t, h, p, w;

        if (sscanf(buffer, "%19s", cmd) != 1)
            continue;

        if (strcmp(cmd, "DATA") == 0) {
            if (sscanf(buffer, "%*s %49s %d %f %f %f %f",
                       location, &seq, &t, &h, &p, &w) != 6) {
                strcpy(response, "ERROR Invalid DATA packet");
            } else {
                int i = findLocation(location);

                if (i == -1 && recordCount < MAX) {
                    i = recordCount++;
                    strcpy(records[i].location, location);
                    records[i].lastSeq = 0;
                    records[i].count = 0;
                    records[i].tempSum = 0;
                    records[i].humiditySum = 0;
                    records[i].pressureSum = 0;
                    records[i].windSum = 0;
                }

                if (i == -1) {
                    strcpy(response, "ERROR Database full");
                } else if (seq <= records[i].lastSeq) {
                    sprintf(response, "ACK %d DUPLICATE", seq);
                } else {
                    if (seq > records[i].lastSeq + 1)
                        printf("Missing packets at %s: %d to %d\n",
                               location,
                               records[i].lastSeq + 1, seq - 1);

                    records[i].lastSeq = seq;
                    records[i].count++;
                    records[i].tempSum += t;
                    records[i].humiditySum += h;
                    records[i].pressureSum += p;
                    records[i].windSum += w;

                    sprintf(response, "ACK %d OK", seq);
                    printf("Received %s seq=%d\n", location, seq);
                }
            }
        }

        else if (strcmp(cmd, "QUERY") == 0) {
            if (sscanf(buffer, "%*s %49s", location) != 1) {
                strcpy(response, "ERROR Invalid query");
            } else {
                int i = findLocation(location);

                if (i == -1 || records[i].count == 0) {
                    strcpy(response, "ERROR No weather data");
                } else {
                    Weather *r = &records[i];
                    sprintf(response,
                        "STATS %s Count=%d AvgTemp=%.2f "
                        "AvgHumidity=%.2f AvgPressure=%.2f AvgWind=%.2f",
                        r->location, r->count,
                        r->tempSum / r->count,
                        r->humiditySum / r->count,
                        r->pressureSum / r->count,
                        r->windSum / r->count);
                }
            }
        }

        else {
            strcpy(response, "ERROR Unknown command");
        }

        sendto(sock, response, strlen(response), 0,
               (struct sockaddr *)&client, len);
    }

    close(sock);
    return 0;
}
