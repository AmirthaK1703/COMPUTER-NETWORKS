#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STRING_LEN 100
#define BITS_PER_CHAR 8

int main() {
    char str[MAX_STRING_LEN];
    int i,j;
    printf("=========================================\n");
    printf("         2D PARITY SENDER PROGRAM\n");
    printf("=========================================\n");
    printf("Enter input string: ");

    if (fgets(str, sizeof(str), stdin) == NULL) {
        printf("Error reading input string.\n");
        return 1;
    }

    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
        len--;
    }

    if (len == 0) {
        printf("Error: Input string cannot be empty.\n");
        return 1;
    }

    int data_rows = (int)len;
    int data_cols = BITS_PER_CHAR;
    int total_rows = data_rows + 1;
    int total_cols = data_cols + 1;

    int matrix[total_rows][total_cols];

    for ( i = 0; i < data_rows; i++) {
        unsigned char c = (unsigned char)str[i];
        for ( j = 0; j < data_cols; j++) {
            matrix[i][j] = (c >> (7 - j)) & 1;
        }
    }

    for ( i = 0; i < data_rows; i++) {
        int sum = 0;
        for ( j = 0; j < data_cols; j++) {
            sum += matrix[i][j];
        }
        matrix[i][data_cols] = sum % 2;
    }

    for (j = 0; j < data_cols; j++) {
        int sum = 0;
        for (i = 0; i < data_rows; i++) {
            sum += matrix[i][j];
        }
        matrix[data_rows][j] = sum % 2;
    }

    int cornerSum = 0;
    for ( i = 0; i < data_rows; i++) {
        cornerSum += matrix[i][data_cols];
    }
    matrix[data_rows][data_cols] = cornerSum % 2;

    printf("\nGenerated 2D Parity Matrix (%d Rows x %d Columns):\n\n", total_rows, total_cols);
    printf("  Data Bits (ASCII)       | Row Parity\n");
    printf("--------------------------+-----------\n");

    for (i = 0; i < data_rows; i++) {
        printf(" ");
        for ( j = 0; j < data_cols; j++) {
            printf("%d ", matrix[i][j]);
        }
        printf("| %d    Row %d ('%c')\n", matrix[i][data_cols], i + 1, str[i]);
    }

    printf("--------------------------+-----------\n ");
    for ( j = 0; j < data_cols; j++) {
        printf("%d ", matrix[data_rows][j]);
    }
    printf("| %d   <-- Column & Corner Parity\n", matrix[data_rows][data_cols]);

    FILE *fp = fopen("output.txt", "w");
    if (fp == NULL) {
        printf("\nError: Could not create output.txt file.\n");
        return 1;
    }

    fprintf(fp, "%d %d\n", total_rows, total_cols);
    for (i = 0; i < total_rows; i++) {
        for ( j = 0; j < total_cols; j++) {
            fprintf(fp, "%d", matrix[i][j]);
            if (j < total_cols - 1) fprintf(fp, " ");
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

    printf("\n[+] Successfully saved frame to 'output.txt'\n");

    return 0;
}

