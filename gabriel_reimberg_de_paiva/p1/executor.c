#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define MEMORYSIZE 516
#define LINESIZE 16
#define HEADERSIZE 4

void print_memory(uint8_t *mem, size_t size) {
    for (size_t i = 0; i < size; i += LINESIZE) {
        printf("%08lx:", (unsigned long)i);
        for (int j = 0; j < LINESIZE && i + j < size; j++) {
            printf(" %02x", mem[i + j]);
        }
        printf("\n");
    }
}

int main() {
    uint8_t ac = 0, pc = 0;
    bool z = false, n = false;
    FILE *file = fopen("programa.mem", "rb");
    if (!file) {
        perror("Can't open the file");
        return 1;
    }

    uint8_t bytes[MEMORYSIZE] = {0};
    uint8_t fileHeader[HEADERSIZE];
    fread(fileHeader, 1, HEADERSIZE, file);
    const uint8_t expectedHeader[] = {0x03, 0x4E, 0x44, 0x52};
    if (memcmp(fileHeader, expectedHeader, HEADERSIZE) != 0) {
        printf("Wrong headers!\n");
        fclose(file);
        return 1;
    }

    fread(bytes + HEADERSIZE, 1, MEMORYSIZE - HEADERSIZE, file);
    fclose(file);

    while (bytes[pc] != 0xF0) {
        z = (ac == 0);
        n = ((ac & 0x80) != 0);
        uint16_t address = bytes[pc + 2] * 2 + HEADERSIZE;

        switch (bytes[pc]) {
            case 0x00: break;
            case 0x10: bytes[address] = ac; break;
            case 0x20: ac = bytes[address]; break;
            case 0x30: ac += bytes[address]; break;
            case 0x31: ac -= bytes[address]; break;
            case 0x40: ac |= bytes[address]; break;
            case 0x50: ac &= bytes[address]; break;
            case 0x60: ac = ~ac; pc += 2; continue;
            case 0x80: pc = address; continue;
            case 0x90: if (n) { pc = address; continue; } break; // JMN
            case 0xA0: if (z) { pc = address; continue; } break; // JMZ
            case 0xF0: break;
        }

        pc += 4;
    }

    print_memory(bytes, MEMORYSIZE);

    printf("Final AC: 0x%02X\n", ac);
    printf("Final PC: 0x%02X\n", pc);
    return 0;
}
