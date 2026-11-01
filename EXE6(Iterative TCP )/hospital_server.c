#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define MAX 100
#define BUF 1024

typedef struct {
    int id, age;
    char name[50];
} Patient;

typedef struct {
    int id, patientId, doctorId, slot;
    int active;
} Appointment;

typedef struct {
    int id;
    char name[50];
    int slots;
} Doctor;

Patient patients[MAX];
Appointment appointments[MAX];
Doctor doctors[] = {
    {1, "Dr.Smith", 5},
    {2, "Dr.Jones", 5},
    {3, "Dr.Kumar", 5}
};

int patientCount = 0, appointmentCount = 0;
int doctorCount = 3, nextAppointmentId = 1;

void logActivity(const char *msg) {
    FILE *fp = fopen("hospital_log.txt", "a");
    if (fp) {
        time_t now = time(NULL);
        fprintf(fp, "%s | %s", msg, ctime(&now));
        fclose(fp);
    }
}

void sendMsg(int sock, const char *msg) {
    send(sock, msg, strlen(msg), 0);
}

int findPatient(int id) {
    for (int i = 0; i < patientCount; i++)
        if (patients[i].id == id) return i;
    return -1;
}

int findDoctor(int id) {
    for (int i = 0; i < doctorCount; i++)
        if (doctors[i].id == id) return i;
    return -1;
}

int slotAvailable(int doctorId, int slot) {
    int d = findDoctor(doctorId);
    if (d == -1 || slot < 1 || slot > doctors[d].slots)
        return 0;

    for (int i = 0; i < appointmentCount; i++)
        if (appointments[i].active &&
            appointments[i].doctorId == doctorId &&
            appointments[i].slot == slot)
            return 0;

    return 1;
}

void processRequest(int sock, char *req) {
    char cmd[20], name[50];
    int a, b, c;

    if (sscanf(req, "%19s", cmd) != 1) {
        sendMsg(sock, "Invalid request\n");
        return;
    }

    if (strcmp(cmd, "REGISTER") == 0) {
        if (sscanf(req, "%*s %49s %d", name, &a) != 2 ||
            patientCount >= MAX || a <= 0) {
            sendMsg(sock, "Invalid registration\n");
            return;
        }

        patients[patientCount].id = patientCount + 1;
        patients[patientCount].age = a;
        strcpy(patients[patientCount].name, name);

        char response[BUF];
        sprintf(response, "Registered. Patient ID: %d\n",
                patients[patientCount].id);
        patientCount++;

        logActivity("Patient registered");
        sendMsg(sock, response);
    }

    else if (strcmp(cmd, "BOOK") == 0) {
        if (sscanf(req, "%*s %d %d %d", &a, &b, &c) != 3 ||
            findPatient(a) == -1 || !slotAvailable(b, c) ||
            appointmentCount >= MAX) {
            sendMsg(sock, "Booking failed: invalid patient, doctor, or slot\n");
            return;
        }

        Appointment *ap = &appointments[appointmentCount++];
        ap->id = nextAppointmentId++;
        ap->patientId = a;
        ap->doctorId = b;
        ap->slot = c;
        ap->active = 1;

        char response[BUF];
        sprintf(response, "Appointment booked. Appointment ID: %d\n",
                ap->id);

        logActivity("Appointment booked");
        sendMsg(sock, response);
    }

    else if (strcmp(cmd, "CANCEL") == 0) {
        if (sscanf(req, "%*s %d", &a) != 1) {
            sendMsg(sock, "Invalid appointment ID\n");
            return;
        }

        for (int i = 0; i < appointmentCount; i++) {
            if (appointments[i].id == a && appointments[i].active) {
                appointments[i].active = 0;
                logActivity("Appointment cancelled");
                sendMsg(sock, "Appointment cancelled\n");
                return;
            }
        }
        sendMsg(sock, "Appointment not found\n");
    }

    else if (strcmp(cmd, "SEARCH") == 0) {
        if (sscanf(req, "%*s %d", &a) != 1) {
            sendMsg(sock, "Invalid doctor ID\n");
            return;
        }

        int d = findDoctor(a);
        if (d == -1) {
            sendMsg(sock, "Doctor not found\n");
            return;
        }

        char response[BUF] = "";
        char line[100];

        sprintf(line, "Doctor: %s\nAvailable slots: ", doctors[d].name);
        strcat(response, line);

        for (int s = 1; s <= doctors[d].slots; s++) {
            if (slotAvailable(a, s)) {
                sprintf(line, "%d ", s);
                strcat(response, line);
            }
        }
        strcat(response, "\n");
        sendMsg(sock, response);
    }

    else if (strcmp(cmd, "HISTORY") == 0) {
        if (sscanf(req, "%*s %d", &a) != 1) {
            sendMsg(sock, "Invalid patient ID\n");
            return;
        }

        char response[BUF] = "";
        char line[100];

        for (int i = 0; i < appointmentCount; i++) {
            if (appointments[i].patientId == a) {
                sprintf(line, "Appointment %d: Doctor %d, Slot %d, %s\n",
                        appointments[i].id,
                        appointments[i].doctorId,
                        appointments[i].slot,
                        appointments[i].active ? "Active" : "Cancelled");
                strcat(response, line);
            }
        }

        if (strlen(response) == 0)
            strcpy(response, "No appointments found\n");

        sendMsg(sock, response);
    }

    else {
        sendMsg(sock, "Unknown command\n");
    }
}

int main() {
    int server, client;
    struct sockaddr_in addr;

    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server, (struct sockaddr *)&addr, sizeof(addr)) < 0 ||
        listen(server, 5) < 0) {
        perror("bind/listen");
        close(server);
        return 1;
    }

    printf("Hospital server listening on port %d\n", PORT);

    while (1) {
        client = accept(server, NULL, NULL);
        if (client < 0) continue;

        printf("Client connected\n");
        sendMsg(client, "Hospital Appointment Server\n");

        char buffer[BUF];
        int n;

        while ((n = recv(client, buffer, sizeof(buffer) - 1, 0)) > 0) {
            buffer[n] = '\0';

            if (strncmp(buffer, "EXIT", 4) == 0)
                break;

            processRequest(client, buffer);
        }

        close(client);
        printf("Client disconnected\n");
    }

    close(server);
    return 0;
}
