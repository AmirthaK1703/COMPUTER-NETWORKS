#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8082
#define BUF 512

typedef struct {
    char regno[20];
    char name[50];
    char department[30];
    int semester;
    float cgpa;
} Student;

Student students[] = {
    {"2026001", "Arun", "CSE", 5, 8.5},
    {"2026002", "Priya", "ECE", 5, 9.1},
    {"2026003", "Kumar", "IT",  3, 8.2},
    {"2026004", "Divya", "EEE", 7, 8.8},
    {"2026005", "Ravi", "CSE", 5, 7.9}
};

int studentCount = sizeof(students) / sizeof(students[0]);

int main() {
    int sock;
    struct sockaddr_in server, client;
    socklen_t len = sizeof(client);
    char regno[BUF], response[BUF];

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind");
        close(sock);
        return 1;
    }

    printf("Student server running on port %d\n", PORT);

    while (1) {
        int n = recvfrom(sock, regno, BUF - 1, 0,
                         (struct sockaddr *)&client, &len);

        if (n <= 0)
            continue;

        regno[n] = '\0';
        regno[strcspn(regno, "\r\n")] = '\0';

        printf("Lookup request: %s\n", regno);

        int found = 0;

        for (int i = 0; i < studentCount; i++) {
            if (strcmp(students[i].regno, regno) == 0) {
                sprintf(response,
                        "Registration: %s\nName: %s\nDepartment: %s\n"
                        "Semester: %d\nCGPA: %.2f\n",
                        students[i].regno,
                        students[i].name,
                        students[i].department,
                        students[i].semester,
                        students[i].cgpa);

                found = 1;
                break;
            }
        }

        if (!found)
            strcpy(response, "ERROR: Student not found");

        sendto(sock, response, strlen(response), 0,
               (struct sockaddr *)&client, len);
    }

    close(sock);
    return 0;
}
