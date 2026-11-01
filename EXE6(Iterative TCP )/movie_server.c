#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8081
#define MAX 100
#define BUF 1024
#define SEATS 10

typedef struct {
    int id;
    char title[50];
    char show[20];
    int price;
    int seats[SEATS];
} Movie;

typedef struct {
    int id, movieId, seat;
    char customer[50];
    int active;
} Booking;

Movie movies[] = {
    {1, "Avatar", "10AM", 150, {0}},
    {2, "Titanic", "2PM", 120, {0}},
    {3, "Inception", "6PM", 180, {0}}
};

Booking bookings[MAX];
int movieCount = 3, bookingCount = 0, nextBookingId = 1;

void logActivity(const char *msg) {
    FILE *fp = fopen("movie_log.txt", "a");
    if (fp) {
        time_t now = time(NULL);
        fprintf(fp, "%s | %s", msg, ctime(&now));
        fclose(fp);
    }
}

void sendMsg(int sock, const char *msg) {
    send(sock, msg, strlen(msg), 0);
}

int findMovie(int id) {
    for (int i = 0; i < movieCount; i++)
        if (movies[i].id == id) return i;
    return -1;
}

void processRequest(int sock, char *req) {
    char cmd[20], customer[50];
    int a, b;

    if (sscanf(req, "%19s", cmd) != 1) {
        sendMsg(sock, "Invalid request\n");
        return;
    }

    if (strcmp(cmd, "LIST") == 0) {
        char response[BUF] = "";
        char line[150];

        for (int i = 0; i < movieCount; i++) {
            int available = 0;
            for (int s = 0; s < SEATS; s++)
                if (!movies[i].seats[s]) available++;

            sprintf(line, "ID:%d Movie:%s Show:%s Price:%d Available:%d\n",
                    movies[i].id, movies[i].title, movies[i].show,
                    movies[i].price, available);
            strcat(response, line);
        }

        sendMsg(sock, response);
    }

    else if (strcmp(cmd, "SEATS") == 0) {
        if (sscanf(req, "%*s %d", &a) != 1) {
            sendMsg(sock, "Invalid movie ID\n");
            return;
        }

        int m = findMovie(a);
        if (m == -1) {
            sendMsg(sock, "Movie not found\n");
            return;
        }

        char response[BUF] = "Available seats: ";
        char line[20];

        for (int s = 0; s < SEATS; s++) {
            if (!movies[m].seats[s]) {
                sprintf(line, "%d ", s + 1);
                strcat(response, line);
            }
        }
        strcat(response, "\n");
        sendMsg(sock, response);
    }

    else if (strcmp(cmd, "BOOK") == 0) {
        if (sscanf(req, "%*s %d %d %49s", &a, &b, customer) != 3 ||
            bookingCount >= MAX) {
            sendMsg(sock, "Invalid booking request\n");
            return;
        }

        int m = findMovie(a);
        if (m == -1 || b < 1 || b > SEATS || movies[m].seats[b - 1]) {
            sendMsg(sock, "Booking failed: invalid movie or unavailable seat\n");
            return;
        }

        movies[m].seats[b - 1] = 1;

        Booking *bk = &bookings[bookingCount++];
        bk->id = nextBookingId++;
        bk->movieId = a;
        bk->seat = b;
        bk->active = 1;
        strcpy(bk->customer, customer);

        char response[BUF];
        sprintf(response, "Ticket booked. Booking ID: %d, Price: %d\n",
                bk->id, movies[m].price);

        logActivity("Ticket booked");
        sendMsg(sock, response);
    }

    else if (strcmp(cmd, "CANCEL") == 0) {
        if (sscanf(req, "%*s %d", &a) != 1) {
            sendMsg(sock, "Invalid booking ID\n");
            return;
        }

        for (int i = 0; i < bookingCount; i++) {
            if (bookings[i].id == a && bookings[i].active) {
                int m = findMovie(bookings[i].movieId);
                if (m != -1)
                    movies[m].seats[bookings[i].seat - 1] = 0;

                bookings[i].active = 0;
                logActivity("Ticket cancelled");
                sendMsg(sock, "Booking cancelled\n");
                return;
            }
        }

        sendMsg(sock, "Booking not found\n");
    }

    else if (strcmp(cmd, "DETAILS") == 0) {
        if (sscanf(req, "%*s %d", &a) != 1) {
            sendMsg(sock, "Invalid booking ID\n");
            return;
        }

        for (int i = 0; i < bookingCount; i++) {
            if (bookings[i].id == a) {
                int m = findMovie(bookings[i].movieId);
                char response[BUF];

                sprintf(response,
                        "Booking ID:%d Customer:%s Movie:%s Show:%s Seat:%d Status:%s\n",
                        bookings[i].id,
                        bookings[i].customer,
                        m == -1 ? "Unknown" : movies[m].title,
                        m == -1 ? "Unknown" : movies[m].show,
                        bookings[i].seat,
                        bookings[i].active ? "Confirmed" : "Cancelled");

                sendMsg(sock, response);
                return;
            }
        }

        sendMsg(sock, "Booking not found\n");
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

    printf("Movie server listening on port %d\n", PORT);

    while (1) {
        client = accept(server, NULL, NULL);
        if (client < 0) continue;

        printf("Client connected\n");
        sendMsg(client, "Movie Ticket Booking Server\n");

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
