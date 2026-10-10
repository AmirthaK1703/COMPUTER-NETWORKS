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

int main() {
    int server_fd;
    struct sockaddr_in address, client_address;
    int opt = 1;
    socklen_t client_addrlen = sizeof(client_address);

    // 1. Structural change: Establish a UDP socket using SOCK_DGRAM
    server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_fd < 0) {
        perror("[ERROR] Socket creation failed");
        exit(EXIT_FAILURE);
    }

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("[ERROR] Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // TCP operational blocks dropped (listen() and accept() removed for connectionless UDP)
    printf("UDP Server listening on port 8080...\n");

    while (1) {
       int i;
        char buffer[8192] = {0};

        // 2. Structural change: Fetch connectionless incoming streams via recvfrom()
        int received_bytes = recvfrom(server_fd, buffer, sizeof(buffer) - 1, 0,
                                      (struct sockaddr *)&client_address, &client_addrlen);
        if (received_bytes < 0) {
            perror("[ERROR] Data read mismatch");
            continue;
        }
        buffer[received_bytes] = '\0';

        char flag1[16] = {0}, addr[16] = {0}, ctrl[16] = {0}, proto[64] = {0}, info[4096] = {0}, fcs[128] = {0}, flag2[16] = {0}, raw_msg[256] = {0};
        int word_size = 16;

        char *token = strtok(buffer, "|");
        if (token) strcpy(flag1, token);
        token = strtok(NULL, "|");
        if (token) strcpy(addr, token);
        token = strtok(NULL, "|");
        if (token) strcpy(ctrl, token);
        token = strtok(NULL, "|");
        if (token) strcpy(proto, token);
        token = strtok(NULL, "|");
        if (token) strcpy(info, token);
        token = strtok(NULL, "|");
        if (token) strcpy(fcs, token);
        token = strtok(NULL, "|");
        if (token) strcpy(flag2, token);
        token = strtok(NULL, "|");
        if (token) word_size = atoi(token);
        token = strtok(NULL, "|");
        if (token) strcpy(raw_msg, token);

        char full_payload[8192] = "";
        strcat(full_payload, addr);
        strcat(full_payload, ctrl);

        for (i = 0; proto[i] != '\0'; i++) {
            if (proto[i] != ' ') strncat(full_payload, &proto[i], 1);
        }
        for (i = 0; info[i] != '\0'; i++) {
            if (info[i] != ' ') strncat(full_payload, &info[i], 1);
        }

        int is_valid = verify_ppp_checksum(full_payload, word_size, fcs);

        if (is_valid) {
            printf("\nData Status: VALID\n");

            char byte_destuffed_info[4096];
            perform_byte_destuffing(raw_msg, byte_destuffed_info);

            printf("\n========================================================\n");
            printf("          PPP FRAME DE-FORMAT (RECEIVER)\n");
            printf("========================================================\n\n");
            printf("Flag\n-----\n%s\n\n", flag1);
            printf("Address\n-------\n%s\n\n", addr);
            printf("Control\n-------\n%s\n\n", ctrl);
            printf("Protocol\n--------\n%s\n\n", proto);
            printf("Information (Data)\n-------------------\n%s\n\n", byte_destuffed_info);
            printf("FCS (Verified Checksum)\n------------------------\n%s\n\n", fcs);
            printf("Flag\n-----\n%s\n", flag2);

            printf("\n========================================================\n");
            printf("                  FINAL DESTUFFED MESSAGE\n");
            printf("========================================================\n");
            printf("Received Message: %s\n", raw_msg);
            printf("========================================================\n");

            // 3. Structural change: Route the acknowledgement verification layout using sendto()
            sendto(server_fd, "ACK: Frame Accepted", 19, 0,
                   (struct sockaddr *)&client_address, client_addrlen);
        } else {
            printf("\nData Status: INVALID\n");
            printf("Transmission Error Detected\n");

            // 4. Structural change: Route error frame response drops using sendto()
            sendto(server_fd, "SOCKET DISCARDED", 16, 0,
                   (struct sockaddr *)&client_address, client_addrlen);
        }
        printf("\nProcessing completed. Re-initializing loop context...\n");
    }

    close(server_fd);
    return 0;
}

