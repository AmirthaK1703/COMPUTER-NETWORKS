#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int isPowerOfTwo(int pos) {
    return (pos && !(pos & (pos - 1)));
}

int calculateRedundantBits(int k) {
    int r = 0;
    while ((1 << r) < (k + r + 1)) {
        r++;
    }
    return r;
}

int* generateHammingCode(const char *data_str, int k, int r, int n) {
    int i;
    int *code = (int *)malloc((n + 1) * sizeof(int));
    if (!code) {
       return NULL;
    }

    int j, data_idx = 0;
    for (i = 1; i <= n; i++) {
        if (isPowerOfTwo(i)) {
            code[i] = 0;
        } else {
            code[i] = data_str[data_idx] - '0';
            data_idx++;
        }
    }

    for (i = 0; i < r; i++) {
        int parity_pos = 1 << i;
        int parity_val = 0;

        for (j = 1; j <= n; j++) {
            if (j & parity_pos) {
                parity_val ^= code[j];
            }
        }

        code[parity_pos] = parity_val;
    }

    return code;
}

void saveToFile(int n, const int *code) {
    FILE *fptr = fopen("output.txt", "w");
    if (fptr == NULL) {
        printf("Error opening file output.txt for writing.\n");
        return;
    }
    int i;
    // Saves ONLY the raw binary string for the receiver to easily read
    for (i = 1; i <= n; i++) {
        fprintf(fptr, "%d", code[i]);
    }
    fprintf(fptr, "\n");
    fclose(fptr);
    printf("Hamming code successfully saved to output.txt\n");
}

int main() {
    FILE *infile = fopen("input.txt", "r");
    if (infile == NULL) {
        printf("Error: Could not open input.txt. Create it with text first.\n");
        return 1;
    }

    char input_text[256];
    if (fgets(input_text, sizeof(input_text), infile) == NULL) {
        printf("Error: input.txt is empty.\n");
        fclose(infile);
        return 1;
    }
    fclose(infile);

    // Remove trailing newline if present
    input_text[strcspn(input_text, "\r\n")] = 0;
    printf("Read text from input.txt: \"%s\"\n", input_text);

    int text_len = strlen(input_text);
    if (text_len == 0) {
        printf("Error: Input text is empty.\n");
        return 1;
    }

    // Convert text string to binary string (8 bits per character)
    char *data_str = (char *)malloc((text_len * 8 + 1) * sizeof(char));
    if (!data_str) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    int bit_idx = 0;
    int i, b;
    for (i = 0; i < text_len; i++) {
        char ch = input_text[i];
        for (b = 7; b >= 0; b--) {
            data_str[bit_idx++] = ((ch >> b) & 1) ? '1' : '0';
        }
    }
    data_str[bit_idx] = '\0';

    printf("Converted Binary Data String: %s\n", data_str);

    int k = strlen(data_str);
    int r = calculateRedundantBits(k);
    int n = k + r;

    int *code = generateHammingCode(data_str, k, r, n);
    if (code == NULL) {
        printf("Memory allocation failed.\n");
        free(data_str);
        return 1;
    }

    printf("\n=== Hamming Code Sender Results ===\n");
    printf("Data word length (k): %d bits\n", k);
    printf("Redundant bits (r)  : %d bits\n", r);
    printf("Total length (n)    : %d bits\n", n);

    saveToFile(n, code);

    free(data_str);
    free(code);
    return 0;
}

[24bcs132@mepcolinux exe3]$cat hreceiver3.c
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
