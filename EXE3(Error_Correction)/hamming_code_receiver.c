#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int isPowerOfTwo(int pos) {
    return (pos && !(pos & (pos - 1)));
}

// Calculates how many redundant bits (r) exist in a total code length of n
int calculateRedundantBitsFromTotal(int n) {
    int r = 0;
    while ((1 << r) < (n + 1)) {
        r++;
    }
    return r;
}

int* readHammingCodeFromFile(const char *filename, int *k, int *r, int *n) {
    FILE *fptr = fopen(filename, "r");
    if (fptr == NULL) {
        printf("Error: Could not open file %s.\n", filename);
        return NULL;
    }

    char buffer[1024];
    if (fscanf(fptr, "%1023s", buffer) != 1) {
        printf("Error: File is empty or invalid.\n");
        fclose(fptr);
        return NULL;
    }
    fclose(fptr);

    *n = strlen(buffer);
    *r = calculateRedundantBitsFromTotal(*n);
    *k = *n - *r;

    int i;
    for (i = 0; i < *n; i++) {
        if (buffer[i] != '0' && buffer[i] != '1') {
            printf("Error: File contains non-binary characters.\n");
            return NULL;
        }
    }

    int *code = (int *)malloc((*n + 1) * sizeof(int));
    if (!code) {
        return NULL;
    }

    for (i = 1; i <= *n; i++) {
        code[i] = buffer[i - 1] - '0';
    }

    return code;
}

void detectAndCorrectError(int *code, int n, int r) {
    int i, j;
    int error_pos = 0;
    int *syndromes = (int *)malloc(r * sizeof(int));

    printf("\n=== Parity Bit Calculations ===\n");
    for (i = 0; i < r; i++) {
        int parity_pos = 1 << i;
        int parity_val = 0;
        int first = 1;

        printf("P%d (Position %d) checks bits: ", i + 1, parity_pos);
        for (j = 1; j <= n; j++) {
            if (j & parity_pos) {
                if (!first) {
                    printf(" ^ ");
                }
                printf("b%d(%d)", j, code[j]);
                parity_val ^= code[j];
                first = 0;
            }
        }
        printf(" = %d\n", parity_val);

        // Save the result for the syndrome display (P1 is LSB, Pr is MSB)
        syndromes[i] = parity_val;

        if (parity_val != 0) {
            error_pos += parity_pos;
        }
    }

    printf("\n=== Error Position Detection ===\n");
    printf("Syndrome word : ");
    // Prints MSB to LSB
    for (i = r - 1; i >= 0; i--) {
        printf("%d", syndromes[i]);
    }
    printf("\nCalculation:  %d\n", error_pos);

    printf("\n=== Hamming Code Receiver Results ===\n");
    if (error_pos == 0) {
        printf("Status: No error detected. Data received successfully!\n");
    } else {
        printf("Status: Error detected at bit position: %d\n", error_pos);

        if (error_pos <= n) {
            code[error_pos] ^= 1;
            printf("Status: Error successfully corrected by flipping bit at position %d!\n", error_pos);
        } else {
            printf("Status: Error position out of bounds. Cannot correct.\n");
        }
    }

    free(syndromes);
}

int main() {
    int b, i, k = 0, r = 0, n = 0;

    int *code = readHammingCodeFromFile("output.txt", &k, &r, &n);
    if (code == NULL) {
        return 1;
    }

    printf("Successfully read from output.txt:\n");
    printf("k = %d, r = %d, n = %d\n", k, r, n);
    printf("Received Code (1-indexed): ");
    for (i = 1; i <= n; i++) {
        printf("%d", code[i]);
    }
    printf("\n");

    char choice;
    printf("\nDo you need to change any bit to simulate an error? (y/n): ");
    scanf(" %c", &choice);

    if (choice == 'y' || choice == 'Y') {
        int num_bits;
        printf("How many bits do you want to change?: ");
        scanf("%d", &num_bits);

        for (b = 0; b < num_bits; b++) {
            int pos_to_change;
            printf("Enter the bit position (1 to %d) to change: ", n);
            scanf("%d", &pos_to_change);

            if (pos_to_change >= 1 && pos_to_change <= n) {
                printf("Flipping bit at position %d (changed from %d to %d).\n",
                       pos_to_change, code[pos_to_change], code[pos_to_change] ^ 1);
                code[pos_to_change] ^= 1;
            } else {
                printf("Invalid position! Skipping.\n");
            }
        }

        printf("Modified Code after manual changes: ");
        for (i = 1; i <= n; i++) {
            printf("%d", code[i]);
        }
        printf("\n");
    }

    detectAndCorrectError(code, n, r);

    printf("Final Corrected Hamming Code: ");
    for (i = 1; i <= n; i++) {
        printf("%d", code[i]);
    }

    printf("\nExtracted Data Word: ");
    for (i = 1; i <= n; i++) {
        if (!isPowerOfTwo(i)) {
            printf("%d", code[i]);
        }
    }
    printf("\n=====================================\n");

    free(code);
    return 0;
}
