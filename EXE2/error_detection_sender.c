#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#define MAX_MSG 2000
#define MAX_FRAME_FILE_NAME 64

struct NetworkTable {
    char url[30];
    int ip[4];
    char mac[20];
};

struct NetworkTable table[20];
int entryCount = 0;

void BinByte(unsigned int num, char *str) {
    unsigned int tmp = num & 0xFFu;
    int i;
    for (i = 7; i >= 0; --i) {
        str[7 - i] = (tmp & (1u << i)) ? '1' : '0';
    }
    str[8] = '\0';
}

void portToBin(int port, char *str) {
    char a[9], b[9];
    BinByte((port >> 8) & 0xFF, a);
    BinByte(port & 0xFF, b);
    sprintf(str, "%s %s", a, b);
}

void macToBin(const char *mac, char *str) {
    int len = (int)strlen(mac);
    int b = 0,j,i;
    int digitCount = 0;
    for (i = 0; i < len; ++i) {
        char c = mac[i];
        if (c == ':') continue;
        int v = 0;
        if (c >= '0' && c <= '9') v = c - '0';
        else if (c >= 'A' && c <= 'F') v = c - 'A' + 10;
        else if (c >= 'a' && c <= 'f') v = c - 'a' + 10;
        for (j = 3; j >= 0; --j) {
            str[b + j] = (v & 1) ? '1' : '0';
            v >>= 1;
        }
        b += 4;
        digitCount++;
        if (digitCount % 2 == 0) {
            str[b++] = ' ';
        }
    }
    if (b > 0 && str[b - 1] == ' ') b--;
    str[b] = '\0';
}

size_t stuff_bytes(const unsigned char *in, size_t inlen, unsigned char *out, size_t outcap) {
    size_t i = 0, o = 0;
    while (i < inlen && o < outcap) {
        if (i + 3 <= inlen && memcmp(&in[i], "DLE", 3) == 0) {
            const unsigned char rep[] = "DLE DLE";
            size_t rl = sizeof(rep) - 1;
            if (o + rl > outcap) break;
            memcpy(&out[o], rep, rl);
            o += rl;
            i += 3;
        } else if (i + 3 <= inlen && memcmp(&in[i], "ETX", 3) == 0) {
            const unsigned char rep[] = "DLE ETX";
            size_t rl = sizeof(rep) - 1;
            if (o + rl > outcap) break;
            memcpy(&out[o], rep, rl);
            o += rl;
            i += 3;
        } else if (i + 3 <= inlen && memcmp(&in[i], "STX", 3) == 0) {
            const unsigned char rep[] = "DLE STX";
            size_t rl = sizeof(rep) - 1;
            if (o + rl > outcap) break;
            memcpy(&out[o], rep, rl);
            o += rl;
            i += 3;
        } else {
            out[o++] = in[i++];
        }
    }
    return o;
}

size_t destuff_bytes(const unsigned char *in, size_t inlen, unsigned char *out, size_t outcap) {
    size_t i = 0, o = 0;
    while (i < inlen && o < outcap) {
        if (i + 7 <= inlen && memcmp(&in[i], "DLE DLE", 7) == 0) {
            if (o + 3 > outcap) break;
            memcpy(&out[o], "DLE", 3);
            o += 3;
            i += 7;
        } else if (i + 7 <= inlen && memcmp(&in[i], "DLE ETX", 7) == 0) {
            if (o + 3 > outcap) break;
            memcpy(&out[o], "ETX", 3);
            o += 3;
            i += 7;
        } else if (i + 7 <= inlen && memcmp(&in[i], "DLE STX", 7) == 0) {
            if (o + 3 > outcap) break;
            memcpy(&out[o], "STX", 3);
            o += 3;
            i += 7;
        } else {
            out[o++] = in[i++];
        }
    }
    return o;
}

uint64_t compute_crc_bitwise(const unsigned char *data, size_t len, int degree, uint64_t poly_input) {
    size_t i;
    int j;
    if (degree <= 0 || degree > 63) return 0;
    uint64_t poly_full = poly_input | (1ULL << degree);
    uint64_t reg = 0;
    for ( i = 0; i < len; ++i) {
        unsigned char cur = data[i];
        for (j = 0; j < 8; ++j) {
            int bit = (cur >> (7 - j)) & 1;
            reg = (reg << 1) | (uint64_t)bit;
            if (reg & (1ULL << degree)) reg ^= poly_full;
        }
    }
    uint64_t mask = (degree == 64) ? ~0ULL : ((1ULL << degree) - 1ULL);
    return reg & mask;
}

void displayTable() {
    int i;
    printf("\n==================== NETWORK ROUTING TABLE ====================\n");
    printf("%-20s | %-15s | %-15s\n", "URL", "IP Address", "MAC Address");
    printf("------------------------------------------------------------\n");
    for (i = 0; i < entryCount; ++i) {
        printf("%-20s | %3d.%-3d.%-3d.%-3d | %-15s\n",
               table[i].url, table[i].ip[0], table[i].ip[1], table[i].ip[2], table[i].ip[3], table[i].mac);
    }
    printf("===============================================================\n");
}

int printApplicationLayer(const char *buffer, int totalLen) {
    printf("========================================================================\n");
    printf(" LAYER 1 : APPLICATION LAYER\n");
    printf("------------------------------------------------------------------------\n");
    printf("  Message        : %s\n", buffer);
    printf("  Binary Format  : ");
    int i;
    for (i = 0; i < totalLen; ++i) {
        char b[9];
        BinByte((unsigned char)buffer[i], b);
        printf("%s%s", b, (i < totalLen - 1) ? " " : "");
    }
    printf("\n========================================================================\n");
    return 1;
}

int printTransportLayer(int srcPort, const char *buffer, int totalLen) {
    printf("========================================================================\n");
    printf(" LAYER 2 : TRANSPORT LAYER (SEGMENT)\n");
    printf("------------------------------------------------------------------------\n");
    printf("  Source Port : %d\n", srcPort);
    printf("  Destination Port : %d\n", 50054);
    printf("------------------------------------------------------------------------\n");
    return 1;
}

int printDataLinkAndFrames(int sIdx, int dIdx, const char *buffer, int totalLen,
                           int srcPort, int packetBitSize, int frameBitSize,
                           int crcDegree, uint64_t crcPolyInput) {
    int i,pIdx;
    size_t b;int packetBytes = packetBitSize / 8;
    if (packetBytes < 1) packetBytes = 1;

    int totalPackets = (totalLen + packetBytes - 1) / packetBytes;
    int frameCounter = 0;

    char srcMacBin[256], destMacBin[256];
    char ipBinSrc[128], ipBinDest[128], tempIp[9];
    char srcPortBin[32], destPortBin[32];

    macToBin(table[sIdx].mac, srcMacBin);
    macToBin(table[dIdx].mac, destMacBin);
    portToBin(srcPort, srcPortBin);
    portToBin(50054, destPortBin);

    ipBinSrc[0] = '\0'; ipBinDest[0] = '\0';
    for (i = 0; i < 4; ++i) {
        BinByte(table[sIdx].ip[i], tempIp); strcat(ipBinSrc, tempIp); if (i < 3) strcat(ipBinSrc, " ");
    }
    for (i = 0; i < 4; ++i) {
        BinByte(table[dIdx].ip[i], tempIp); strcat(ipBinDest, tempIp); if (i < 3) strcat(ipBinDest, " ");
    }

    printf("\n========================================================================\n");
    printf(" LAYER 4 : DATA LINK LAYER (STUFFING, FRAMING & CRC per frame)\n");
    printf("========================================================================\n");

    for (pIdx = 0; pIdx < totalPackets; ++pIdx) {
        int byteStart = pIdx * packetBytes;
        int bytesAvail = totalLen - byteStart;
        if (bytesAvail < 0) bytesAvail = 0;
        int thisPacketLen = (bytesAvail >= packetBytes) ? packetBytes : bytesAvail;

        unsigned char packetBytesBuf[MAX_MSG];
        if (thisPacketLen > 0) memcpy(packetBytesBuf, &buffer[byteStart], thisPacketLen);

        unsigned char stuffed[MAX_MSG * 3];
        size_t stuffedLen = stuff_bytes(packetBytesBuf, thisPacketLen, stuffed, sizeof(stuffed));

        size_t stuffedBits = stuffedLen * 8;
        size_t bitOffset = 0;
        while (bitOffset < stuffedBits) {
            size_t bitsRemaining = stuffedBits - bitOffset;
            size_t thisFrameBits = (bitsRemaining >= (size_t)frameBitSize) ? (size_t)frameBitSize : bitsRemaining;
            size_t paddingBits = 0;
            if (thisFrameBits < (size_t)frameBitSize) paddingBits = (size_t)frameBitSize - thisFrameBits;

            size_t payloadBytesLen = (thisFrameBits + 7) / 8;
            unsigned char payloadBytes[512];
            memset(payloadBytes, 0, sizeof(payloadBytes));
            for (b = 0; b < thisFrameBits; ++b) {
                size_t overallBitPos = bitOffset + b;
                size_t byteIdx = overallBitPos / 8;
                int bitInByte = 7 - (overallBitPos % 8);
                int bit = (stuffed[byteIdx] >> bitInByte) & 1;
                int outByte = b / 8;
                int outBit = 7 - (b % 8);
                payloadBytes[outByte] |= (unsigned char)(bit << outBit);
            }

            uint64_t crcVal = 0;
            if (crcDegree > 0 && payloadBytesLen > 0) {
                crcVal = compute_crc_bitwise(payloadBytes, payloadBytesLen, crcDegree, crcPolyInput);
            } else {
                crcVal = 0;
            }

            char crcBinStr[128] = {0};
            int cb;
            for (cb = 0; cb < crcDegree; ++cb) {
                int pos = crcDegree - 1 - cb;
                crcBinStr[cb] = ((crcVal >> pos) & 1) ? '1' : '0';
            }
         crcBinStr[crcDegree] = '\0';

            ++frameCounter;

            printf("\n-------------------- FRAME %d --------------------\n", frameCounter);
            printf("SRC_MAC: %s    DST_MAC: %s\n", table[sIdx].mac, table[dIdx].mac);
            printf("SRC_IP: %d.%d.%d.%d    DST_IP: %d.%d.%d.%d\n",
                   table[sIdx].ip[0], table[sIdx].ip[1], table[sIdx].ip[2], table[sIdx].ip[3],
                   table[dIdx].ip[0], table[dIdx].ip[1], table[dIdx].ip[2], table[dIdx].ip[3]);
            printf("SRC_PORT: %d    DST_PORT: 50054\n", srcPort);
            printf("Payload bits: %zu  Padding bits: %zu\n", thisFrameBits, paddingBits);

            size_t pb;
            for (pb = 0; pb < payloadBytesLen; ++pb) {
                unsigned char ch = payloadBytes[pb];
                if (ch >= 32 && ch <= 126) printf("%c", ch);
                else printf(".");
            }
            printf("\nCRC (bin): %s\n", crcBinStr);

            {
                char fname[MAX_FRAME_FILE_NAME];
                snprintf(fname, sizeof(fname), "frame_%d.txt", frameCounter);
                FILE *f = fopen(fname, "wb");
                if (f) {
                    fprintf(f, "FRAME %d\n\n", frameCounter);
                    fprintf(f, "SRC_MAC: %s\nDST_MAC: %s\nSRC_IP: %d.%d.%d.%d\nDST_IP: %d.%d.%d.%d\nSRC_PORT: %d\nDST_PORT: 50054\n\n",
                            table[sIdx].mac, table[dIdx].mac,
                            table[sIdx].ip[0], table[sIdx].ip[1], table[sIdx].ip[2], table[sIdx].ip[3],
                            table[dIdx].ip[0], table[dIdx].ip[1], table[dIdx].ip[2], table[dIdx].ip[3],
                            srcPort);
                    fprintf(f, "Payload bits: %zu\nPadding bits: %zu\n", thisFrameBits, paddingBits);
                    fprintf(f, "Payload (raw bytes):\n");
                    fwrite(payloadBytes, 1, payloadBytesLen, f);
                    fprintf(f, "\n\nCRC (bin): %s\n", crcBinStr);
                    fclose(f);
                }
            }

            /* Receiver simulation: verify CRC and destuff the received payload bytes (length payloadBytesLen)
            {
                uint64_t recomputed = 0;
                if (crcDegree > 0 && payloadBytesLen > 0) recomputed = compute_crc_bitwise(payloadBytes, payloadBytesLen, crcDegree, crcPolyInput);
                printf("[Receiver] CRC verification: ");
                if (recomputed == crcVal) printf("PASSED\n");
                else printf("FAILED (expected %s, got 0x%llX)\n", crcBinStr, (unsigned long long)recomputed);

                unsigned char destbuf[MAX_MSG * 3];
                size_t destlen = destuff_bytes(payloadBytes, payloadBytesLen, destbuf, sizeof(destbuf));

               // printf("[Receiver] Destuffed payload (printable where possible):\n");
                size_t d;
                for (d = 0; d < destlen; ++d) {
                    unsigned char ch = destbuf[d];
                    if (ch >= 32 && ch <= 126) putchar(ch);
                    else putchar('.');
                }
                putchar('\n');

            }*/
            bitOffset += thisFrameBits;
        }

    }

    printf("\n========================================================================\n");
    printf("Total frames generated: %d\n", frameCounter);
    printf("========================================================================\n");
    return frameCounter;
}
int printPhysicalLayer(int sIdx, int dIdx, const char *buffer, int totalLen,
                       int srcPort, int packetBitSize, int frameBitSize,
                       int crcDegree, uint64_t crcPolyInput) {
    int i,pIdx,cb;
    size_t b,pb;
    int packetBytes = packetBitSize / 8;
    if (packetBytes < 1) packetBytes = 1;
    int totalPackets = (totalLen + packetBytes - 1) / packetBytes;

    char srcMacBin[256], destMacBin[256];
    char ipBinSrc[128], ipBinDest[128], tempIp[9];
    char srcPortBin[32], destPortBin[32];

    macToBin(table[sIdx].mac, srcMacBin);
    macToBin(table[dIdx].mac, destMacBin);
    portToBin(srcPort, srcPortBin);
    portToBin(50054, destPortBin);

    ipBinSrc[0] = ipBinDest[0] = '\0';
    for (i = 0; i < 4; ++i) {
        BinByte(table[sIdx].ip[i], tempIp); strcat(ipBinSrc, tempIp); if (i < 3) strcat(ipBinSrc, " ");
    }
    for (i = 0; i < 4; ++i) {
        BinByte(table[dIdx].ip[i], tempIp); strcat(ipBinDest, tempIp); if (i < 3) strcat(ipBinDest, " ");
    }

    printf("\n========================================================================\n");
    printf(" LAYER 5 : PHYSICAL LAYER (BINARY FORMAT PER FRAME)\n");
    printf("========================================================================\n");

    int frameCounter = 0;

    for ( pIdx = 0; pIdx < totalPackets; ++pIdx) {
        int byteStart = pIdx * packetBytes;
        int bytesAvail = totalLen - byteStart;
        if (bytesAvail < 0) bytesAvail = 0;
        int thisPacketLen = (bytesAvail >= packetBytes) ? packetBytes : bytesAvail;

        unsigned char packetBytesBuf[MAX_MSG];
        if (thisPacketLen > 0) memcpy(packetBytesBuf, &buffer[byteStart], thisPacketLen);

        unsigned char stuffed[MAX_MSG * 3];
        size_t stuffedLen = stuff_bytes(packetBytesBuf, thisPacketLen, stuffed, sizeof(stuffed));
        size_t stuffedBits = stuffedLen * 8;
        size_t bitOffset = 0;
        while (bitOffset < stuffedBits) {
            size_t bitsRemaining = stuffedBits - bitOffset;
            size_t thisFrameBits = (bitsRemaining >= (size_t)frameBitSize) ? (size_t)frameBitSize : bitsRemaining;
            size_t paddingBits = 0;
            if (thisFrameBits < (size_t)frameBitSize) paddingBits = (size_t)frameBitSize - thisFrameBits;

            size_t payloadBytesLen = (thisFrameBits + 7) / 8;
            unsigned char payloadBytes[512];
            memset(payloadBytes, 0, sizeof(payloadBytes));
            for (b = 0; b < thisFrameBits; ++b) {
                size_t overallBitPos = bitOffset + b;
                size_t byteIdx = overallBitPos / 8;
                int bitInByte = 7 - (overallBitPos % 8);
                int bit = (stuffed[byteIdx] >> bitInByte) & 1;
                int outByte = b / 8;
                int outBit = 7 - (b % 8);
                payloadBytes[outByte] |= (unsigned char)(bit << outBit);
            }

            uint64_t crcVal = 0;
            if (crcDegree > 0 && payloadBytesLen > 0) crcVal = compute_crc_bitwise(payloadBytes, payloadBytesLen, crcDegree, crcPolyInput);
            char crcBinStr[128] = {0};
            for (cb = 0; cb < crcDegree; ++cb) {
                int pos = crcDegree - 1 - cb;
                crcBinStr[cb] = ((crcVal >> pos) & 1) ? '1' : '0';
            }
            crcBinStr[crcDegree] = '\0';

            ++frameCounter;

            printf("FRAME %d BINARY:\n", frameCounter);
            printf("%s %s %s %s %s %s ", srcMacBin, destMacBin, ipBinSrc, ipBinDest, srcPortBin, destPortBin);

            for (b = 0; b < thisFrameBits; ++b) {
                size_t overallBitPos = bitOffset + b;
                size_t byteIdx = overallBitPos / 8;
                int bitInByte = 7 - (overallBitPos % 8);
                int bit = (stuffed[byteIdx] >> bitInByte) & 1;
                putchar(bit ? '1' : '0');
            }
            for (pb = 0; pb < paddingBits; ++pb) putchar('0');

            printf(" %s\n", crcBinStr);

            bitOffset += thisFrameBits;
        }
    }

    printf("========================================================================\n");
    return 1;
}

int printTransmissionSummary(int sIdx, int dIdx, const char *buffer, int totalFrames, int srcPort) {
    printf("\n========================================================================\n");
    printf(" TRANSMISSION SUMMARY\n");
    printf("========================================================================\n");
    printf("  Total Frames Sent  : %d\n", totalFrames);
    printf("  Complete Message   : %s\n", buffer);
    printf("  Source Port        : %-14d | Destination Port    : 50054\n", srcPort);
    printf("  Source IP          : %d.%d.%d.%d  | Destination IP      : %d.%d.%d.%d\n",
           table[sIdx].ip[0], table[sIdx].ip[1], table[sIdx].ip[2], table[sIdx].ip[3],
           table[dIdx].ip[0], table[dIdx].ip[1], table[dIdx].ip[2], table[dIdx].ip[3]);
    printf("  Source MAC         : %s  | Destination MAC     : %s\n", table[sIdx].mac, table[dIdx].mac);
    printf("========================================================================\n");
    return 1;
}

void executeNetworkSimulation(int sIdx, int dIdx, const char *buffer, int totalLen, int srcPort,
                              int packetBitSize, int frameBitSize, int crcDegree, uint64_t crcPolyInput) {
    printApplicationLayer(buffer, totalLen);
    printTransportLayer(srcPort, buffer, totalLen);

    int packetBytes = packetBitSize / 8;
    if (packetBytes < 1) packetBytes = 1;
    int totalPackets = (totalLen + packetBytes - 1) / packetBytes;
    printf("========================================================================\n");
    printf(" LAYER 3 : NETWORK LAYER (packet fragmentation -> %d bytes per packet)\n", packetBytes);
    printf(" Total application bytes: %d -> Total packets: %d\n", totalLen, totalPackets);
    printf("========================================================================\n");

    int totalFrames = printDataLinkAndFrames(sIdx, dIdx, buffer, totalLen, srcPort, packetBitSize, frameBitSize, crcDegree, crcPolyInput);
    printPhysicalLayer(sIdx, dIdx, buffer, totalLen, srcPort, packetBitSize, frameBitSize, crcDegree, crcPolyInput);
    printTransmissionSummary(sIdx, dIdx, buffer, totalFrames, srcPort);
}

int main() {
    int choice;
    int i, j, sIdx, dIdx, len;
    int packetBitSize = 16, frameBitSize = 8;
    char src[30], dest[30], fileName[256], buffer[MAX_MSG];
    char delUrl[30];
    int crcDegree = 8;
    uint64_t crcPolyInput = 0x07;

    FILE *fp;
    srand((unsigned int)time(NULL));

    strcpy(table[0].url, "client.lan");
    table[0].ip[0] = 192; table[0].ip[1] = 168; table[0].ip[2] = 1; table[0].ip[3] = 10;
    strcpy(table[0].mac, "AA:BB:CC:DD");

    strcpy(table[1].url, "google.com");
    table[1].ip[0] = 142; table[1].ip[1] = 250; table[1].ip[2] = 190; table[1].ip[3] = 46;
    strcpy(table[1].mac, "11:22:33:44");

    entryCount = 2;

    while (1) {
        printf("\n=================== NETWORK TOOL MENU ===================\n");
        printf(" 1. Add Network Node\n");
        printf(" 2. Delete Network Node\n");
        printf(" 3. Generate & Stream Frame\n");
        printf(" 4. View Network Table\n");
        printf(" 5. Exit Program\n");
        printf("---------------------------------------------------------\n");
        printf("Enter your choice (1-5): ");

        if (scanf("%d", &choice) != 1) { while (getchar() != '\n'); continue; }
        if (choice == 5) { printf("\nExiting program. Goodbye!\n"); break; }

        if (choice == 1) {
            if (entryCount >= 20) { printf("[ERROR] Registry limits exceeded.\n"); continue; }
            printf("Enter Domain URL : "); scanf("%29s", table[entryCount].url);
            printf("Enter IP Address (e.g., 192.168.1.1) : ");
            scanf("%d.%d.%d.%d", &table[entryCount].ip[0], &table[entryCount].ip[1], &table[entryCount].ip[2], &table[entryCount].ip[3]);
            printf("Enter MAC Address (XX:XX:XX:XX) : "); scanf("%19s", table[entryCount].mac);
            entryCount++; printf("Node added successfully.\n");
        }
        else if (choice == 2) {
            printf("Enter the URL to delete: "); scanf("%29s", delUrl);
            int foundIdx = -1;
            for (i = 0; i < entryCount; ++i) if (strcmp(table[i].url, delUrl) == 0) { foundIdx = i; break; }
            if (foundIdx == -1) { printf("[ERROR] URL not found.\n"); }
            else { for (j = foundIdx; j < entryCount - 1; ++j) table[j] = table[j + 1]; entryCount--; printf("Node deleted.\n"); }
        }
        else if (choice == 4) {
            displayTable();
        }
        else if (choice == 3) {
            printf("\n--- FRAME CONFIGURATION WINDOW ---\n");
            printf("Source URL : "); scanf("%29s", src);
            printf("Destination URL : "); scanf("%29s", dest);

            printf("Packet Bit Size : ");
            scanf("%d", &packetBitSize);
            if (packetBitSize <= 0 || (packetBitSize % 8) != 0) {
                printf("[WARN] Packet bit size must be multiple of 8. Using default 16.\n");
                packetBitSize = 16;
            }

            printf("Frame Bit Size : ");
            scanf("%d", &frameBitSize);
            if (frameBitSize <= 0) { printf("[WARN] invalid frame size; using 8\n"); frameBitSize = 8; }

            printf("Enter CRC degree (e.g. 8, 16, 32): ");
            if (scanf("%d", &crcDegree) != 1) { while (getchar() != '\n'); crcDegree = 8; }
            if (crcDegree <= 0 || crcDegree > 63) { printf("[WARN] invalid degree -> using 8\n"); crcDegree = 8; }

            printf("Enter polynomial binary string \n", crcDegree);
            char polyStr[128];
            scanf("%127s", polyStr);
            uint64_t poly_input = 0;
            int L = (int)strlen(polyStr);
            int cnt = 0,k;
            for (k = 0; k < L && cnt < 64; ++k) {
                if (polyStr[k] == '0' || polyStr[k] == '1') {
                    poly_input = (poly_input << 1) | (uint64_t)(polyStr[k] - '0');
                    cnt++;
                }
            }
            if (cnt < crcDegree) {
                poly_input <<= (crcDegree - cnt);
            } else if (cnt > crcDegree) {
                poly_input &= ((1ULL << crcDegree) - 1ULL);
            }
            crcPolyInput = poly_input;

            printf("Source File Name : "); scanf("%255s", fileName);
            fp = fopen(fileName, "r");
            if (!fp) { printf("[ERROR] File missing or inaccessible.\n"); continue; }
            if (fgets(buffer, sizeof(buffer), fp) == NULL) buffer[0] = '\0';
            fclose(fp);
            len = (int)strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') { buffer[len - 1] = '\0'; len--; }

            sIdx = dIdx = -1;
            for (i = 0; i < entryCount; ++i) {
                if (strcmp(table[i].url, src) == 0) sIdx = i;
                if (strcmp(table[i].url, dest) == 0) dIdx = i;
            }
            if (sIdx == -1 || dIdx == -1) { printf("[ERROR] Routing conflict: match paths not verified.\n"); continue; }

            int randomSrcPort = (rand() % 49152) + 1024;
            executeNetworkSimulation(sIdx, dIdx, buffer, len, randomSrcPort, packetBitSize, frameBitSize, crcDegree, crcPolyInput);
        }
    }

    return 0;
}
