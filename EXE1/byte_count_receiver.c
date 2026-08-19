
[24bcs132@mepcolinux exe1]$cat bytecountr.c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_BITS 10000

int binToByte(const char *binStr, int start) {
    int i,val = 0;
    for (i = 0; i < 8; i++) {
        val <<= 1;
        if (binStr[start + i] == '1') {
            val |= 1;
        }
    }
    return val;
}

int binToCount(const char *binStr, int start) {
    int i,val = 0;
    for (i = 0; i < 14; i++) {
        val <<= 1;
        if (binStr[start + i] == '1') {
            val |= 1;
        }
    }
    return val;
}

int main() {
   int i;
    FILE *fp = fopen("frame.txt", "r");
    if (fp == NULL) {
        printf("Error: Could not open frame1.txt source file.\n");
        return 1;
    }

    char rawStream[MAX_BITS];
    if (fgets(rawStream, sizeof(rawStream), fp) == NULL) {
        printf("Error: Frame file is empty.\n");
        fclose(fp);
        return 1;
    }
    fclose(fp);

    size_t streamLen = strlen(rawStream);
    while (streamLen > 0 && (rawStream[streamLen - 1] == '\n' || rawStream[streamLen - 1] == '\r')) {
        rawStream[streamLen - 1] = '\0';
        streamLen--;
    }
    int idx = 0;

    char syn1[9], syn2[9], classField[9], countField[15];

    strncpy(syn1, &rawStream[idx], 8); syn1[8] = '\0'; idx += 8;
    strncpy(syn2, &rawStream[idx], 8); syn2[8] = '\0'; idx += 8;
    strncpy(classField, &rawStream[idx], 8); classField[8] = '\0'; idx += 8;
    strncpy(countField, &rawStream[idx], 14); countField[14] = '\0'; idx += 14;

    int decodedByteCount = binToCount(countField, 0);

    int headerBitsCount = 160;
    char headerBuffer[200] = {0};
    int hBufIdx = 0;

    for (i = 0; i < headerBitsCount; i++) {
        headerBuffer[hBufIdx++] = rawStream[idx++];
        if ((i + 1) % 8 == 0 && i < headerBitsCount - 1) {
            headerBuffer[hBufIdx++] = ' ';
        }
    }
    headerBuffer[hBufIdx] = '\0';

    int bodyBitsCount = decodedByteCount * 8;
    char bodyBinStr[MAX_BITS] = {0};
    char messagePayload[MAX_BITS / 8] = {0};
    int bBufIdx = 0;

    for (i = 0; i < bodyBitsCount; i++) {
        bodyBinStr[bBufIdx++] = rawStream[idx++];
        if ((i + 1) % 8 == 0 && i < bodyBitsCount - 1) {
            bodyBinStr[bBufIdx++] = ' ';
        }
    }
    bodyBinStr[bBufIdx] = '\0';
    for (i = 0; i < decodedByteCount; i++) {
        messagePayload[i] = (char)binToByte(rawStream, (idx - bodyBitsCount) + (i * 8));
    }
    messagePayload[decodedByteCount] = '\0';

    char crcField[9];
    strncpy(crcField, &rawStream[idx], 8); crcField[8] = '\0';

    int actualBodyBytes = bodyBitsCount / 8;
    const char *countValidation = (decodedByteCount == actualBodyBytes) ? "Frame Valid" : "Frame Invalid";
    const char *crcValidation = (strcmp(crcField, "00000000") == 0) ? "CRC Valid" : "CRC Error";

    printf("==================================================\n");
    printf("            DDCMP RECEIVER OUTPUT PROCESSING      \n");
    printf("==================================================\n\n");

    printf("--- Message Text Payload ---\n");
    printf("Received Message     : %s\n\n", messagePayload);

    printf("--- Verification Status Metrics ---\n");
    printf("Received Byte Count  : %d\n", decodedByteCount);
    printf("Actual Body Bytes    : %d\n", actualBodyBytes);
    printf("Byte Count Validation: %s\n", countValidation);
    printf("CRC Verification     : %s\n\n", crcValidation);

    printf("--- Raw Frame Structural Binary Layout ---\n");
    printf("SYN   : %s\n", syn1);
    printf("SYN   : %s\n", syn2);
    printf("CLASS : %s\n", classField);
    printf("COUNT : %s\n", countField);
    printf("HEADER: %s\n", headerBuffer);
    printf("BODY  : %s\n", bodyBinStr);
    printf("CRC   : %s\n", crcField);

    return 0;
}

