#include <stdio.h>
#include <stdlib.h>

#define MAX_ROWS 20
#define MAX_COLS 20

int main() {
    FILE *fp = fopen("output.txt", "r");
    if (fp == NULL) {
        printf("Error: Could not open output.txt file.\n");
        return 1;
    }
    int i,j;
    int total_rows, total_cols;

    if (fscanf(fp, "%d %d", &total_rows, &total_cols) != 2) {
        printf("Error: Invalid matrix dimensions in output.txt.\n");
        fclose(fp);
        return 1;
    }

    int matrix[MAX_ROWS][MAX_COLS];

    for ( i = 0; i < total_rows; i++) {
        for (j = 0; j < total_cols; j++) {
            if (fscanf(fp, "%d", &matrix[i][j]) != 1) {
                printf("Error reading matrix data from output.txt at [%d][%d].\n", i, j);
                fclose(fp);
                return 1;
            }
        }
    }
    fclose(fp);

    int data_rows = total_rows - 1;
    int data_cols = total_cols - 1;

    printf("=========================================\n");
    printf("     2D PARITY RECEIVER & TEST\n");
    printf("=========================================\n");
    printf("Received Matrix Dimensions: %d Rows x %d Columns (Data + Parity)\n\n", total_rows, total_cols);

    printf("Received Matrix:\n");
    for ( i = 0; i < total_rows; i++) {
        for (j = 0; j < total_cols; j++) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");

    char choice;
    printf("Do you want to simulate a transmission error? (y/n): ");
    scanf(" %c", &choice);

    if (choice == 'y' || choice == 'Y') {
        int k,flips;
        printf("Enter number of bits to flip (1 for Single-bit, >1 for Multiple): ");
        scanf("%d", &flips);

        for ( k = 1; k <= flips; k++) {
            int r, c;
            printf("\n[Flip %d/%d] Enter row number to modify (Range: 1 to %d): ", k, flips, total_rows);
            scanf("%d", &r);
            printf("[Flip %d/%d] Enter column number to modify (Range: 1 to %d): ", k, flips, total_cols);
            scanf("%d", &c);

            matrix[r - 1][c - 1] ^= 1;
            printf(" Bit at Row %d, Column %d flipped \n", r, c);
        }
    }

    printf("\n=========================================\n");
    printf("          RECEIVER PROCESSING\n");
    printf("=========================================\n");

    int recalcRowParity[MAX_ROWS];
    int recalcColParity[MAX_COLS];

    for ( i = 0; i < data_rows; i++) {
        int sum = 0;
        for ( j = 0; j < data_cols; j++) {
            sum += matrix[i][j];
        }
        recalcRowParity[i] = sum % 2;
    }

    for ( j = 0; j < data_cols; j++) {
        int sum = 0;
        for (i = 0; i < data_rows; i++) {
            sum += matrix[i][j];
        }
        recalcColParity[j] = sum % 2;
    }

    int cornerSum = 0;
    for (i = 0; i < data_rows; i++) {
        cornerSum += recalcRowParity[i];
    }
    int recalcCornerParity = cornerSum % 2;

    printf("Matrix with Recalculated Parity Bits:\n");
    for (i = 0; i < data_rows; i++) {
        for (j = 0; j < data_cols; j++) {
            printf("%d ", matrix[i][j]);
        }
        printf("%d [%d]\n", matrix[i][data_cols], recalcRowParity[i]);
    }

    for (j = 0; j < data_cols; j++) {
        printf("[%d] ", recalcColParity[j]);
    }
    printf("[%d] - Recalculated Column & Corner Parity\n\n", recalcCornerParity);

    // Arrays to record which indices failed for later single-bit isolation
    int failed_rows[MAX_ROWS];
    int failed_cols[MAX_COLS];
    int failed_row_count = 0;
    int failed_col_count = 0;

    printf("Row Parity check:\n");
    for ( i = 0; i < data_rows; i++) {
        if (recalcRowParity[i] == matrix[i][data_cols]) {
            printf("Row %d: PASS (Bit: %d)\n", i + 1, matrix[i][data_cols]);
        } else {
            printf("Row %d: FAIL (Bit Changed: Received %d -> Recalculated %d)\n",
                   i + 1, matrix[i][data_cols], recalcRowParity[i]);
            failed_rows[failed_row_count++] = i;
        }
    }

    printf("\nColumn Parity check:\n");
    for ( j = 0; j < data_cols; j++) {
        if (recalcColParity[j] == matrix[data_rows][j]) {
            printf("Column %d: PASS (Bit: %d)\n", j + 1, matrix[data_rows][j]);
        } else {
            printf("Column %d: FAIL (Bit Changed: Received %d -> Recalculated %d)\n",
                   j + 1, matrix[data_rows][j], recalcColParity[j]);
            failed_cols[failed_col_count++] = j;
        }
    }

    // Single-bit error identification and fix announcement
    if (failed_row_count == 1 && failed_col_count == 1) {
        int r_err = failed_rows[0];
        int c_err = failed_cols[0];
        printf("\n=========================================\n");
        printf("         ERROR CORRECTION REPORT\n");
        printf("=========================================\n");
        printf("Single-bit error isolated at: Row %d, Column %d\n", r_err + 1, c_err + 1);
        printf("Data bit transformation: %d -> %d\n", matrix[r_err][c_err], matrix[r_err][c_err] ^ 1);
    }

    return 0;
}
