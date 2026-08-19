#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_MSG 1000

struct NetworkTable {
    char url[30];
    int ip[4];
    char mac[20];
};

struct NetworkTable table[20];
int entryCount = 0;

int Bin(int num, char *str) {
    int temp = num;
    int i;
    for(i = 0; i < 8; i++) {
        str[i] = '0';
    }
    str[8] = '\0';
    for (i = 7; i >= 0; i--) {
        if (temp % 2 == 1) {
            str[i] = '1';
        } else {
            str[i] = '0';
        }
        temp = temp / 2;
    }
    return 1;
}

void stringToBinary(const char *input, char *output) {
    int i;
    output[0] = '\0';
    for (i = 0; input[i] != '\0'; i++) {
        char binStr[9];
        Bin((int)input[i], binStr);
        strcat(output, binStr);
        strcat(output, " ");
    }
}

int portToBin(int port, char *str) {
    int highByte = port / 256;
    int lowByte = port % 256;
    int i;
    int index = 0;
    char hbin[9], lbin[9];
    Bin(highByte, hbin);
    Bin(lowByte, lbin);
    for(i = 0; i < 8; i++) {
        str[index] = hbin[i];
        index++;
    }
    str[index] = ' ';
    index++;
    for(i = 0; i < 8; i++) {
        str[index] = lbin[i];
        index++;
    }
    str[index] = '\0';
    return 1;
}

int macToBin(const char *mac, char *str) {
    int len = (int)strlen(mac);
    int bIdx = 0;
    int digitCount = 0;
    int i, j;

    for (i = 0; i < len; i++) {
        char c;
        int digit = 0;
        int temp;

        if (mac[i] == ':') {
            continue;
        }
        c = mac[i];
        if (c >= '0' && c <= '9') digit = c - '0';
        else if (c >= 'A' && c <= 'F') digit = c - 'A' + 10;
        else if (c >= 'a' && c <= 'f') digit = c - 'a' + 10;

        temp = digit;
        for (j = 3; j >= 0; j--) {
            if (temp % 2 == 1) {
                str[bIdx + j] = '1';
            } else {
                str[bIdx + j] = '0';
            }
            temp = temp / 2;
        }
        bIdx += 4;
        digitCount++;

        if (digitCount % 2 == 0 && i < len - 1) {
            str[bIdx] = ' ';
            bIdx++;
        }
    }
    str[bIdx] = '\0';
    return 1;
}

unsigned char calculateCRC(const char *data) {
   return 0;
}


void displayTable() {
    int i;
    printf("\n==================== NETWORK ROUTING TABLE ====================\n");
    printf("%-20s | %-15s | %-15s\n", "URL", "IP Address", "MAC Address");
    printf("------------------------------------------------------------\n");
    for (i = 0; i < entryCount; i++) {
        printf("%-20s | %3d.%-3d.%-3d.%-3d | %-15s\n",
               table[i].url, table[i].ip[0], table[i].ip[1], table[i].ip[2], table[i].ip[3], table[i].mac);
    }
    printf("===============================================================\n");
}

int printApplicationLayer(const char *buffer, int totalLen) {
    int i;
    printf("========================================================================\n");
    printf(" LAYER 1 : APPLICATION LAYER\n");
    printf("------------------------------------------------------------------------\n");
    printf("  Message        : %s\n", buffer);
    printf("  Binary Format  : ");
    for (i = 0; i < totalLen; i++) {
        char binStr[9];
        Bin((int)buffer[i], binStr);
        printf("%s%s", binStr, (i < totalLen - 1) ? " " : "");
    }
    printf("\n\n========================================================================\n");
    return 1;
}

int printTransportLayer(int srcPort, const char *buffer, int totalLen) {
    int destPort = 50054;
    int i;
    char srcPortBin[30], destPortBin[30];

    portToBin(srcPort, srcPortBin);
    portToBin(destPort, destPortBin);

    printf("========================================================================\n");
    printf(" LAYER 2 : TRANSPORT LAYER (SEGMENT)\n");
    printf("------------------------------------------------------------------------\n");
    printf("  Source Port              : %d \n", srcPort);
    printf("  Destination Port         : %d\n", destPort);
    printf("------------------------------------------------------------------------\n");
    printf(" Binary Format\n");
    printf(" Source Port               : %s\n", srcPortBin);
    printf(" Destination Port          : %s\n", destPortBin);
    printf("------------------------------------------------------------------------\n");
    printf(" Layer Output:\n");
    printf("%s %s ", srcPortBin, destPortBin);
    for (i = 0; i < totalLen; i++) {
        char binStr[9];
        Bin((int)buffer[i], binStr);
        printf("%s%s", binStr, (i < totalLen - 1) ? " " : "");
    }
    printf("\n========================================================================\n");
    return 1;
}

int printNetworkLayer(int sIdx, int dIdx, const char *buffer, int totalLen, int srcPort) {
    int i;
    char ipBinSrc[50], ipBinDest[50], tempIp[9];
    char srcPortBin[30], destPortBin[30];

    portToBin(srcPort, srcPortBin);
    portToBin(50054, destPortBin);

    ipBinSrc[0] = '\0';
    for(i = 0; i < 4; i++) {
        Bin(table[sIdx].ip[i], tempIp); strcat(ipBinSrc, tempIp); if(i < 3) strcat(ipBinSrc, " ");
    }
    ipBinDest[0] = '\0';
    for(i = 0; i < 4; i++) {
        Bin(table[dIdx].ip[i], tempIp); strcat(ipBinDest, tempIp); if(i < 3) strcat(ipBinDest, " ");
    }

    printf("========================================================================\n");
    printf(" LAYER 3 : NETWORK LAYER\n");
    printf("----------------------------------------------------------------------\n");
    printf("  Source IP        : %d.%d.%d.%d\n", table[sIdx].ip[0], table[sIdx].ip[1], table[sIdx].ip[2], table[sIdx].ip[3]);
    printf("  Destination IP   : %d.%d.%d.%d\n", table[dIdx].ip[0], table[dIdx].ip[1], table[dIdx].ip[2], table[dIdx].ip[3]);
    printf("----------------------------------------------------------------------\n");
    printf(" Packet Binary Stream:\n ");
    printf("%s %s %s %s ", ipBinSrc, ipBinDest, srcPortBin, destPortBin);
    for (i = 0; i < totalLen; i++) {
        char pBin[9];
        Bin((int)buffer[i], pBin);
        printf("%s%s", pBin, (i < totalLen - 1) ? " " : "");
    }
    printf("\n========================================================================\n");
    return 1;
}

int printDataLinkLabel(int sIdx, int dIdx, const char *buffer, int totalLen, int srcPort) {
    int destPort = 50054;
    char stuffedMessage[MAX_MSG * 2] = {0};
    int sIdxMsg = 0;
    int i; /* Declared at top of scope for ANSI C compatibility */

    for (i = 0; buffer[i] != '\0'; ) {
        if (strncmp(&buffer[i], "DLE", 3) == 0) {
            strcpy(&stuffedMessage[sIdxMsg], "DLE DLE ");
            sIdxMsg += 8;
            i += 3;
            if (buffer[i] == ' ') i++; /* drop trailing spaces to avoid duplicate spacing */
        } else if (strncmp(&buffer[i], "ETX", 3) == 0) {
            strcpy(&stuffedMessage[sIdxMsg], "DLE ETX ");
            sIdxMsg += 8;
            i += 3;
            if (buffer[i] == ' ') i++;
        } else if (strncmp(&buffer[i], "STX", 3) == 0) {
            strcpy(&stuffedMessage[sIdxMsg], "DLE STX ");
            sIdxMsg += 8;
            i += 3;
            if (buffer[i] == ' ') i++;
             }else {
            stuffedMessage[sIdxMsg++] = buffer[i++];
        }
    }
    stuffedMessage[sIdxMsg] = '\0';

    char binSrcMac[100] = {0}, binDestMac[100] = {0};
    char binSrcIP[50] = {0}, binDestIP[50] = {0}, tempIp[9];
    char binSrcPort[30] = {0}, binDestPort[30] = {0};

    macToBin(table[sIdx].mac, binSrcMac);
    macToBin(table[dIdx].mac, binDestMac);
    portToBin(srcPort, binSrcPort);
    portToBin(destPort, binDestPort);

    for(i = 0; i < 4; i++) {
        Bin(table[sIdx].ip[i], tempIp); strcat(binSrcIP, tempIp); if(i < 3) strcat(binSrcIP, " ");
    }
    for(i = 0; i < 4; i++) {
        Bin(table[dIdx].ip[i], tempIp); strcat(binDestIP, tempIp); if(i < 3) strcat(binDestIP, " ");
    }

    char headerText[MAX_MSG];
    sprintf(headerText, "SRC_MAC:%s, DST_MAC:%s, SRC_IP:%d.%d.%d.%d, DST_IP:%d.%d.%d.%d, SRC_PORT:%d, DST_PORT:%d",
            table[sIdx].mac, table[dIdx].mac,
            table[sIdx].ip[0], table[sIdx].ip[1], table[sIdx].ip[2], table[sIdx].ip[3],
            table[dIdx].ip[0], table[dIdx].ip[1], table[dIdx].ip[2], table[dIdx].ip[3],
            srcPort, destPort);

    char headerBinary[MAX_MSG * 2];
    sprintf(headerBinary, "MACs:[%s | %s] IPs:[%s | %s] Ports:[%s | %s]",
            binSrcMac, binDestMac, binSrcIP, binDestIP, binSrcPort, binDestPort);

    char binStuffedMessage[MAX_MSG * 16] = {0};
    stringToBinary(stuffedMessage, binStuffedMessage);

    unsigned char crcVal = calculateCRC(stuffedMessage);
    char crcText[20];
    sprintf(crcText, "0x%02X", crcVal);

    printf("\n========================================================================\n");
    printf(" LAYER 4 : DATA LINK LAYER (BISYNC CHARACTER STUFFING SEND)\n");
    printf("========================================================================\n");
    printf("Original Message  : %s\n", buffer);
    printf("Stuffed Message   : %s\n", stuffedMessage);
    printf("------------------------------------------------------------------------\n");
    printf("=== COMPLETE CONSTRUCTED BISYNC FRAME ===\n");
    printf("SYN\nSYN\nSOH\n");
    printf("HEADER (Text)     : %s\n", headerText);
    printf("HEADER (Binary)   : %s\n", headerBinary);
    printf("STX\n");
    printf("DATA (Text)       : %s\n", stuffedMessage);
    printf("DATA (Binary)     : %s\n", binStuffedMessage);
    printf("ETX\n");
    printf("CRC               : %s\n", crcText);
    printf("========================================================================\n");

    FILE *frameFile = fopen("frame.txt", "w");
    if (frameFile) {
        fprintf(frameFile, "SYN\nSYN\nSOH\n%s\nSTX\n%s\n%s\nETX\n%s",
                headerText, stuffedMessage, binStuffedMessage, crcText);
        fclose(frameFile);
    }

    // --- STEP 18: Save beautiful output display context to output.txt ---
/*    FILE *outputFile = fopen("output.txt", "w");
    if (outputFile) {
        fprintf(outputFile, "=== BISYNC TRANSMISSION OUTPUT LOG ===\n\n");
        fprintf(outputFile, "Original Message : %s\n", buffer);
        fprintf(outputFile, "Stuffed Data     : %s\n\n", stuffedMessage);
        fprintf(outputFile, "--- PROTOCOL FRAME DATA LAYOUT ---\n");
        fprintf(outputFile, "SYN\nSYN\nSOH\n");
        fprintf(outputFile, "HEADER TEXT      : %s\n", headerText);
        fprintf(outputFile, "HEADER BINARY    : %s\n", headerBinary);
        fprintf(outputFile, "STX\n");
        fprintf(outputFile, "DATA TEXT        : %s\n", stuffedMessage);
        fprintf(outputFile, "DATA BINARY      : %s\n", binStuffedMessage);
        fprintf(outputFile, "ETX\n");
        fprintf(outputFile, "CRC              : %s\n", crcText);
        fclose(outputFile);
    }

    printf("Frame Created Successfully.\n");
    return 1;*/
}

int printPhysicalLayer(int sIdx, int dIdx, const char *buffer, int srcPort) {
    int destPort = 50054;
    char stuffedMessage[MAX_MSG * 2] = {0};
    int sIdxMsg = 0;
    int i;

    for (i = 0; buffer[i] != '\0'; ) {
        if (strncmp(&buffer[i], "DLE", 3) == 0) {
            strcpy(&stuffedMessage[sIdxMsg], "DLE DLE "); sIdxMsg += 8; i += 3;
            if (buffer[i] == ' ') i++;
        } else if (strncmp(&buffer[i], "ETX", 3) == 0) {
            strcpy(&stuffedMessage[sIdxMsg], "DLE ETX "); sIdxMsg += 8; i += 3;
            if (buffer[i] == ' ') i++;
        } else {
            stuffedMessage[sIdxMsg++] = buffer[i++];
        }
    }
    stuffedMessage[sIdxMsg] = '\0';

    char binSrcMac[100] = {0}, binDestMac[100] = {0};
    char binSrcIP[50] = {0}, binDestIP[50] = {0}, tempIp[9];
    char binSrcPort[30] = {0}, binDestPort[30] = {0};
    char binStuffedMessage[MAX_MSG * 16] = {0};

    macToBin(table[sIdx].mac, binSrcMac);
    macToBin(table[dIdx].mac, binDestMac);
    portToBin(srcPort, binSrcPort);
    portToBin(destPort, binDestPort);
    stringToBinary(stuffedMessage, binStuffedMessage);

    for(i = 0; i < 4; i++) {
        Bin(table[sIdx].ip[i], tempIp); strcat(binSrcIP, tempIp); if(i < 3) strcat(binSrcIP, " ");
    }
    for(i = 0; i < 4; i++) {
        Bin(table[dIdx].ip[i], tempIp); strcat(binDestIP, tempIp); if(i < 3) strcat(binDestIP, " ");
    }

    unsigned char crcVal = calculateCRC(stuffedMessage);
    char binCrc[30];
    Bin(crcVal, binCrc);

    printf("\n========================================================================\n");
    printf("LAYER 5 : PHYSICAL LAYER\n");
    printf("========================================================================\n");
    printf("Complete Combined Synchronous Transmission Stream:\n\n");

    printf("00010110 00010110 00000001 %s %s %s %s %s %s 00000010 %s 00000011 %s\n",
           binSrcMac, binDestMac, binSrcIP, binDestIP, binSrcPort, binDestPort, binStuffedMessage, binCrc);
   FILE *fp =fopen("Outputchar.txt","w");
    fprintf(fp,"00010110 00010110 00000001 %s %s %s %s %s %s 00000010 %s 00000011 %s\n",
           binSrcMac, binDestMac, binSrcIP, binDestIP, binSrcPort, binDestPort, binStuffedMessage, binCrc);
    fclose(fp);
    printf("\n========================================================================\n");
    printf("TRANSMISSION COMPLETED SUCCESSFULLY\n");
    printf("========================================================================\n");
    return 1;
}

int printTransmissionSummary(int sIdx, int dIdx, const char *buffer, int srcPort) {
    printf("\n========================================================================\n");
    printf(" TRANSMISSION SUMMARY\n");
    printf("========================================================================\n");
    printf("  Total Frames Sent  : 1 (BISYNC Superframe Bundle)\n");
    printf("  Complete Message   : %s\n", buffer);
    printf("  Source Port        : %-14d | Destination Port    : 50054\n", srcPort);
    printf("  Source IP          : %d.%d.%d.%d  | Destination IP      : %d.%d.%d.%d\n",
           table[sIdx].ip[0], table[sIdx].ip[1], table[sIdx].ip[2], table[sIdx].ip[3],
           table[dIdx].ip[0], table[dIdx].ip[1], table[dIdx].ip[2], table[dIdx].ip[3]);
    printf("  Source MAC         : %s  | Destination MAC     : %s\n", table[sIdx].mac, table[dIdx].mac);
    printf("  Delivery Status    : Transmission Successful\n");
    printf("========================================================================\n");
    return 1;
}

void executeNetworkSimulation(int sIdx, int dIdx, const char *buffer, int totalLen, int srcPort) {
    printApplicationLayer(buffer, totalLen);
    printTransportLayer(srcPort, buffer, totalLen);
    printNetworkLayer(sIdx, dIdx, buffer, totalLen, srcPort);
    printDataLinkLabel(sIdx, dIdx, buffer, totalLen, srcPort);
    printPhysicalLayer(sIdx, dIdx, buffer, srcPort);
    printTransmissionSummary(sIdx, dIdx, buffer, srcPort);
}

int main() {
    int choice, i, j, sIdx, dIdx, len;
    char src[30], dest[30], fileName[30], buffer[100], delUrl[30];
    FILE *fp;

    srand((unsigned int)time(NULL));

    strcpy(table[0].url, "client.lan");
    table[0].ip[0] = 192; table[0].ip[1] = 168; table[0].ip[2] = 1; table[0].ip[3] = 10;
    strcpy(table[0].mac, "AA:BB:CC:DD");

    strcpy(table[1].url, "google.com");
    table[1].ip[0] = 142; table[1].ip[1] = 250; table[1].ip[2] = 190; table[1].ip[3] = 46;
    strcpy(table[1].mac, "11:22:33:44");

    entryCount = 2;

    while(1) {
        printf("\n=================== NETWORK TOOL MENU ===================\n");
        printf(" 1. Add Network Node\n");
        printf(" 2. Delete Network Node\n");
        printf(" 3. Generate & Stream Frame\n");
        printf(" 4. View Network Table\n");
        printf(" 5. Exit Program\n");
        printf("---------------------------------------------------------\n");
        printf("Enter your choice (1-5): ");

        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            continue;
        }

        if (choice == 5) {
            printf("\nExiting program. Goodbye!\n");
            break;
        }

        if (choice == 1) {
            printf("\n--- ADD NEW NODE ARCHITECTURE ---\n");
            if (entryCount >= 20) {
                printf("\n[ERROR] Registry limits exceeded.\n");
                continue;
            }
            printf("Enter Domain URL : ");
            scanf("%29s", table[entryCount].url);
            printf("Enter IP Address (e.g., 192.168.1.1) : ");
            scanf("%d.%d.%d.%d", &table[entryCount].ip[0], &table[entryCount].ip[1], &table[entryCount].ip[2], &table[entryCount].ip[3]);
            printf("Enter MAC Address (XX:XX:XX:XX) : ");
            scanf("%19s", table[entryCount].mac);
            entryCount++;
            printf("\n>>> Node added successfully!\n");
        }
        else if (choice == 2) {
            printf("\n--- DELETE NODE INTERFACE ---\n");
            printf("Enter the URL to delete: ");
            scanf("%29s", delUrl);
            int foundIdx = -1;
            for(i = 0; i < entryCount; i++) {
                if(strcmp(table[i].url, delUrl) == 0) {
                    foundIdx = i;
                    break;
                }
            }
            if(foundIdx == -1) {
                printf("\n[ERROR] URL not found.\n");
            } else {
                for(j = foundIdx; j < entryCount - 1; j++) {
                    table[j] = table[j + 1];
                }
                entryCount--;
                printf("\n>>> Node successfully deleted.\n");
            }
        }
        else if (choice == 4) {
            displayTable();
        }
        else if (choice == 3) {
            printf("\n--- FRAME CONFIGURATION WINDOW ---\n");
            printf("Source URL : ");
            scanf("%29s", src);
            printf("Destination URL : ");
            scanf("%29s", dest);

            printf("Source File Name : ");
            scanf("%29s", fileName);

            fp = fopen(fileName, "r");
            if (!fp) {
                printf("\n[ERROR] File missing or inaccessible.\n");
                continue;
            }
            if (fgets(buffer, sizeof(buffer), fp) == NULL) {
                buffer[0] = '\0';
            }
            fclose(fp);

            len = (int)strlen(buffer);
            if(len > 0 && buffer[len-1] == '\n') {
                buffer[len-1] = '\0';
                len--;
            }

            sIdx = -1; dIdx = -1;
            for(i = 0; i < entryCount; i++) {
                if(strcmp(table[i].url, src) == 0) sIdx = i;
                if(strcmp(table[i].url, dest) == 0) dIdx = i;
            }

            if(sIdx == -1 || dIdx == -1) {
                printf("\n[ERROR] Routing conflict: Match paths not verified.\n");
                continue;
            }

            int randomSrcPort = (rand() % 49152) + 1024;
            executeNetworkSimulation(sIdx, dIdx, buffer, len, randomSrcPort);
        }
    }
    return 0;
 }
