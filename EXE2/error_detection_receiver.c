#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define MAX_MSG 2000
#define MAX_FRAMES 9
#define MAX_STR 256

struct NetworkTable {
    char url[30];
    int ip[4];
    char mac[20];
};

struct FrameData {
    int frameNum;
    char srcMac[20];
    char destMac[20];
    int srcIp[4];
    int destIp[4];
    int srcPort;
    int destPort;
    size_t payloadBits;
    size_t paddingBits;
    unsigned char payloadBytes[512];
    size_t payloadBytesLen;
    char crcBinStr[128];
};

static struct FrameData frames[MAX_FRAMES];
static int totalFramesRead = 0;

uint64_t compute_crc_bitwise(const unsigned char *data, size_t len, int degree, uint64_t poly_input) {
    size_t i;
    int j;
    if (degree <= 0 || degree > 63) return 0;
    uint64_t poly_full = poly_input | (1ULL << degree);
    uint64_t reg = 0;
    for (i = 0; i < len; ++i) {
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

size_t destuff_bytes(const unsigned char *in, size_t inlen, unsigned char *out, size_t outcap) {
    size_t i = 0, o = 0;
    while (i < inlen && o < outcap) {
        if (i + 7 <= inlen && memcmp(&in[i], "DLE DLE", 7) == 0) {
            if (o + 3 > outcap) break;
            memcpy(&out[o], "DLE", 3);
            o += 3; i += 7;
        } else if (i + 7 <= inlen && memcmp(&in[i], "DLE ETX", 7) == 0) {
            if (o + 3 > outcap) break;
            memcpy(&out[o], "ETX", 3);
            o += 3; i += 7;
        } else if (i + 7 <= inlen && memcmp(&in[i], "DLE STX", 7) == 0) {
            if (o + 3 > outcap) break;
            memcpy(&out[o], "STX", 3);
            o += 3; i += 7;
        } else {
            out[o++] = in[i++];
        }
    }
    return o;
}

int main() {
    int fIdx, crcDegree = 8;
    uint64_t crcPolyInput = 0x07;
    char line[MAX_STR];

    printf("================== RECEIVER INITIALIZATION ==================\n");
    printf("CRC Config set automatically: Degree = %d, Polynomial = 0x%02X\n", crcDegree, (unsigned int)crcPolyInput);

    for (fIdx = 1; fIdx <= 9; fIdx++) {
        char fname[64];
        snprintf(fname, sizeof(fname), "frame_%d.txt", fIdx);
        FILE *f = fopen(fname, "rb");
        if (!f) {
            if (fIdx == 1) {
                printf("Note: frame_1.txt not found. Stopping file check.\n");
            }
            break;
        }

        struct FrameData *fd = &frames[totalFramesRead];
        fd->frameNum = fIdx;

        while (fgets(line, sizeof(line), f)) {
            if (strncmp(line, "SRC_MAC:", 8) == 0) {
                sscanf(line, "SRC_MAC: %19s", fd->srcMac);
            } else if (strncmp(line, "DST_MAC:", 8) == 0) {
                sscanf(line, "DST_MAC: %19s", fd->destMac);
            } else if (strncmp(line, "SRC_IP:", 7) == 0) {
                sscanf(line, "SRC_IP: %d.%d.%d.%d", &fd->srcIp[0], &fd->srcIp[1], &fd->srcIp[2], &fd->srcIp[3]);
            } else if (strncmp(line, "DST_IP:", 7) == 0) {
                sscanf(line, "DST_IP: %d.%d.%d.%d", &fd->destIp[0], &fd->destIp[1], &fd->destIp[2], &fd->destIp[3]);
            } else if (strncmp(line, "SRC_PORT:", 9) == 0) {
                sscanf(line, "SRC_PORT: %d", &fd->srcPort);
            } else if (strncmp(line, "Payload bits:", 13) == 0) {
                sscanf(line, "Payload bits: %zu", &fd->payloadBits);
                fd->payloadBytesLen = (fd->payloadBits + 7) / 8;
            } else if (strncmp(line, "Padding bits:", 13) == 0) {
                sscanf(line, "Padding bits: %zu", &fd->paddingBits);
            } else if (strncmp(line, "Payload (raw bytes):", 20) == 0) {
                fread(fd->payloadBytes, 1, fd->payloadBytesLen, f);
            } else if (strncmp(line, "CRC (bin):", 10) == 0) {
                sscanf(line, "CRC (bin): %127s", fd->crcBinStr);
            }
        }
        fclose(f);
        totalFramesRead++;
    }

    if (totalFramesRead == 0) {
        printf("❌Error: No structural frame_*.txt files discovered in this workspace.\n");
        return 1;
    }
    printf("Successfully mapped %d frames from local environment storage.\n", totalFramesRead);

    char simChoice = 'n';
    printf("\nDo you want to simulate a transmission error? (y/n): ");
    scanf(" %c", &simChoice);

    if (simChoice == 'y' || simChoice == 'Y') {
        int targetFrame = 0;
        int targetByte = 0;
        int targetBit = 0;

        while (1) {
            printf("Enter target frame to corrupt (Range: 1 to %d): ", totalFramesRead);
            scanf("%d", &targetFrame);
            if (targetFrame >= 1 && targetFrame <= totalFramesRead) break;
            printf("❌Frame out of valid sequence limit.\n");
        }

        struct FrameData *tfd = &frames[targetFrame - 1];
       // printf("Frame #%d contains %zu payload bytes (%zu total transmission bits).\n",
         //      targetFrame, tfd->payloadBytesLen, tfd->payloadBits);

        while (1) {
            printf("Enter byte index to corrupt:");
            //printf("Enter byte index to corrupt (0 to %zu): ", tfd->payloadBytesLen - 1);
            scanf("%d", &targetByte);
            printf("Enter target bit index to invert inside byte (0 to 7): ");
            scanf("%d", &targetBit);
            if (targetByte >= 0 && (size_t)targetByte < tfd->payloadBytesLen && targetBit >= 0 && targetBit <= 7) break;
            printf("Indices fall outside payload block boundary\n");
        }

        tfd->payloadBytes[targetByte] ^= (1 << (7 - targetBit));
        printf("\n*** Error injection applied safely to payload bit streams inside memory sequence ***\n");
    }

    printf("\n======================= RECEIVER PROCESSING LAYER =======================\n");
    unsigned char globalReconstructedBuffer[MAX_MSG * 3] = {0};
    size_t globalBufferOffset = 0;
    int cb,i;

    for (i = 0; i < totalFramesRead; i++) {
        struct FrameData *fd = &frames[i];
        uint64_t recomputedCrc = compute_crc_bitwise(fd->payloadBytes, fd->payloadBytesLen, crcDegree, crcPolyInput);

        char checkBinStr[128] = {0};
        for (cb = 0; cb < crcDegree; ++cb) {
            int pos = crcDegree - 1 - cb;
            checkBinStr[cb] = ((recomputedCrc >> pos) & 1) ? '1' : '0';
        }
        checkBinStr[crcDegree] = '\0';

        int isCrcValid = (strcmp(checkBinStr, fd->crcBinStr) == 0);
        printf("Frame %02d : CRC Evaluation status: %s \n", fd->frameNum, isCrcValid ? "PASS" : "FAIL");
        printf("  CRC: %s | Recomputed Value: %s\n", fd->crcBinStr, checkBinStr);
        if (isCrcValid){
           printf("  Remainder: 00000000");
        }
        else
        {
           printf("  Remainder:  %s",checkBinStr);
        }
            unsigned char localDestbuf[MAX_MSG * 3];
            size_t destLen = destuff_bytes(fd->payloadBytes, fd->payloadBytesLen, localDestbuf, sizeof(localDestbuf));

            if (globalBufferOffset + destLen < sizeof(globalReconstructedBuffer)) {
                memcpy(&globalReconstructedBuffer[globalBufferOffset], localDestbuf, destLen);
                globalBufferOffset += destLen;
            }
         else {
            printf(" Frame dropped. Integrity invalid inside error-checking execution matrices.\n");
        }
    }

    // Final Assembly Display Outputs to Shell Pipeline Stream targets
    //printf("\n====================== COMPRESSED OUTPUT ANALYSIS ======================\n");
    //globalReconstructedBuffer[globalBufferOffset] = '\0';
   // printf("Reconstructed Application-Layer Payload text: \n--> \"%s\"\n", globalReconstructedBuffer);
   // printf("========================================================================\n");

    return 0;

}
