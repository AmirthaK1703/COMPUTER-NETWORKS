
[24bcs132@mepcolinux exe1]$cat Receiverchar.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BYTES 256

#define SYN 22
#define SOH 1
#define STX 2
#define ETX 3

void print_binary(unsigned char byte) {
    int i;
    for (i = 7; i >= 0; i--) {
        printf("%d", (byte >> i) & 1);
    }
    printf(" ");
}

int main() {
    FILE *file = fopen("Outputchar.txt", "r");
    int i;
    if (file == NULL) {
        printf("Error: Could not open Outputchar.txt\n");
        return 1;
    }

    unsigned char stream[MAX_BYTES];
    int total_bytes = 0;
    char binary_str[16];

    while (fscanf(file, "%15s", binary_str) == 1) {
        if (binary_str[0] != '0' && binary_str[0] != '1') {
            break;
        }
        stream[total_bytes++] = (unsigned char)strtol(binary_str, NULL, 2);
    }
    fclose(file);

    int idx = 0;

    printf("SYN : 00010110 \nSYN : 00010110\n");
    while (idx < total_bytes && stream[idx] == SYN) {
        idx++;
    }

    if (idx < total_bytes && stream[idx] == SOH) {
        printf("SOH : 00000001\n");
        idx++;
    }

    printf("HEADER (Binary)   : ");
    while (idx < total_bytes && stream[idx] != STX) {
        print_binary(stream[idx]);
        idx++;
    }
    printf("\n");

    if (idx < total_bytes && stream[idx] == STX) {
        printf("STX : 00000010\n");
        idx++;
    }

    unsigned char raw_data[MAX_BYTES];
    int raw_len = 0;

    printf("DATA (Binary)     : ");
    while (idx < total_bytes && stream[idx] != ETX) {
        print_binary(stream[idx]);
        raw_data[raw_len++] = stream[idx];
        idx++;
    }
    printf("\n");

    printf("Data Text Format:   ");
    for (i = 0; i < raw_len; i++) {
        printf("%c", raw_data[i]);
    }
    printf("\n");

    unsigned char clean_data[MAX_BYTES];
    int clean_len = 0;

    for (i = 0; i < raw_len; ) {
        if (i + 7 < raw_len &&
            raw_data[i]   == 'D' && raw_data[i+1] == 'L' && raw_data[i+2] == 'E' && raw_data[i+3] == ' ' &&
            raw_data[i+4] == 'D' && raw_data[i+5] == 'L' && raw_data[i+6] == 'E' && raw_data[i+7] == ' ') {

            clean_data[clean_len++] = 'D';
            clean_data[clean_len++] = 'L';
            clean_data[clean_len++] = 'E';
            clean_data[clean_len++] = ' ';

            i += 8;

            while (i + 3 < raw_len &&
                   raw_data[i] == 'D' && raw_data[i+1] == 'L' && raw_data[i+2] == 'E' && raw_data[i+3] == ' ') {
                i += 4;
            }
        } else {
            clean_data[clean_len++] = raw_data[i];
            i++;
        }
    }

    printf("DATA (De-stuffed) : ");
    for (i = 0; i < clean_len; i++) {
        printf("%c", clean_data[i]);
    }
    printf("\n");

    if (idx < total_bytes && stream[idx] == ETX) {
        printf("ETX : 00000011\n");
        idx++;
    }

    if (idx < total_bytes) {
        printf("CRC : ");
        print_binary(stream[idx]);
        printf("\n");
    }

    return 0;
}

