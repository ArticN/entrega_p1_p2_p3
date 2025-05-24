#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define HEADERSIZE 4         
#define MEMORYSIZE 512       
#define LINESIZE 16          
#define DATA_START 0x100     

#define OPCODE_NOP  0x00
#define OPCODE_STA  0x10
#define OPCODE_LDA  0x20
#define OPCODE_ADD  0x30
#define OPCODE_SUB  0x31
#define OPCODE_OR   0x40
#define OPCODE_AND  0x50
#define OPCODE_NOT  0x60
#define OPCODE_JMP  0x80
#define OPCODE_JMN  0x90
#define OPCODE_JMZ  0xA0
#define OPCODE_HLT  0xF0

#define RESULTOFFSET (DATA_START + 4)

typedef struct {
    char name[32];
    int address;
    int value;
    bool defined;
} Symbol;

#define MAX_SYMBOLS 256
Symbol symbols[MAX_SYMBOLS];
int symbolCount = 0;

void addSymbol(const char* name, int address, int value, bool defined) {
    if (symbolCount < MAX_SYMBOLS) {
        strncpy(symbols[symbolCount].name, name, sizeof(symbols[symbolCount].name) - 1);
        symbols[symbolCount].name[sizeof(symbols[symbolCount].name) - 1] = '\0';
        symbols[symbolCount].address = address;
        symbols[symbolCount].value = value;
        symbols[symbolCount].defined = defined;
        symbolCount++;
    } else {
        fprintf(stderr, "Erro: tabela de símbolos cheia\n");
    }
}

int findSymbol(const char* name) {
    for (int i = 0; i < symbolCount; i++) {
        if (strcmp(symbols[i].name, name) == 0)
            return symbols[i].address;
    }
    return -1;
}

bool symbolExists(const char* name) {
    for (int i = 0; i < symbolCount; i++) {
        if (strcmp(symbols[i].name, name) == 0)
            return true;
    }
    return false;
}

int parseNumber(const char* str) {
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
        return (int)strtol(str, NULL, 16);
    else
        return atoi(str);
}

void cleanLine(char* line) {
    char* comment = strchr(line, ';');
    if (comment) *comment = '\0';
    
    int len = strlen(line);
    while (len > 0 && isspace((unsigned char)line[len-1])) {
        line[--len] = '\0';
    }
}

bool assemble(const char* inputFile, const char* outputFile) {
    FILE *fin = fopen(inputFile, "r");
    if (!fin) {
        perror("Erro ao abrir o arquivo assembly");
        return false;
    }
    
    uint8_t memory[MEMORYSIZE] = {0};
    uint8_t header[HEADERSIZE] = {0x03, 0x4E, 0x44, 0x52};
    memcpy(memory, header, HEADERSIZE);
    
    int dataAddr = DATA_START;
    int codeOrigin = 0;
    int codeStart = HEADERSIZE + codeOrigin * 2; 
    int codeAddr = codeStart; 
    
    enum { NONE, DATA_SECTION, CODE_SECTION } section = NONE;
    
    addSymbol("RES", RESULTOFFSET, 0, false);
    
    char line[256];
    int tempCodeAddr = codeStart;
    while (fgets(line, sizeof(line), fin)) {
        cleanLine(line);
        
        char *p = line;
        while (isspace((unsigned char)*p)) p++;
        if (*p == '\0') continue;
        
        char label[32] = {0};
        if (strchr(p, ':')) {
            sscanf(p, "%31[^:]:", label);
            if (strlen(label) > 0 && section == CODE_SECTION) {
                if (!symbolExists(label)) {
                    addSymbol(label, tempCodeAddr, 0, true);
                    printf("Rótulo encontrado: %s (endereço: %d)\n", label, tempCodeAddr);
                }
                continue;
            }
        }
        
        if (strncasecmp(p, ".DATA", 5) == 0) {
            section = DATA_SECTION;
            continue;
        }
        if (strncasecmp(p, ".CODE", 5) == 0) {
            section = CODE_SECTION;
            continue;
        }
        
        if (section == DATA_SECTION) {
            char directive[16], valueStr[32];
            int result = sscanf(p, "%31s %15s %31s", label, directive, valueStr);
            
            if (result >= 2 && strcasecmp(directive, "DB") == 0) {
                int value = 0;
                bool defined = true;
                
                if (result < 3 || strcmp(valueStr, "?") == 0) {
                    defined = false;
                    value = 0;
                } else {
                    value = parseNumber(valueStr);
                }
                
                if (dataAddr % 2 != 0) dataAddr++;
                
                if (!symbolExists(label)) {
                    addSymbol(label, dataAddr, value, defined);
                }
                
                memory[dataAddr] = (uint8_t)value;
                memory[dataAddr+1] = 0;
                dataAddr += 2;
            }
        } else if (section == CODE_SECTION) {
            if (strncasecmp(p, ".ORG", 4) == 0) {
                int org;
                if (sscanf(p, ".ORG %d", &org) == 1) {
                    codeOrigin = org;
                    codeStart = HEADERSIZE + codeOrigin * 2;
                    tempCodeAddr = codeStart;;
                }
                continue;
            }
            
            char mnemonic[16], operand[32];
            if (sscanf(p, "%15s %31s", mnemonic, operand) >= 1) {
                tempCodeAddr += 4;
            }
        }
    }
    rewind(fin);
    
    section = NONE;
    codeAddr = codeStart;
    while (fgets(line, sizeof(line), fin)) {
        cleanLine(line);
        
        char *p = line;
        while (isspace((unsigned char)*p)) p++;
        if (*p == '\0') continue;
        
        if (strchr(p, ':')) {
            continue;
        }
        
        if (strncasecmp(p, ".DATA", 5) == 0) {
            section = DATA_SECTION;
            continue;
        }
        if (strncasecmp(p, ".CODE", 5) == 0) {
            section = CODE_SECTION;
            continue;
        }
        
        if (section == DATA_SECTION) {
        } else if (section == CODE_SECTION) {
            if (strncasecmp(p, ".ORG", 4) == 0) {
                int org;
                if (sscanf(p, ".ORG %d", &org) == 1) {
                    codeOrigin = org;
                    codeStart = HEADERSIZE + codeOrigin * 2;
                    codeAddr = codeStart;
                }
                continue;
            }
            
            char mnemonic[16], operand[32];
            int items = sscanf(p, "%15s %31s", mnemonic, operand);
            
            if (items < 1) continue;
            
            uint8_t opcode = 0;
            uint8_t operandByte = 0;
            
            if (strcasecmp(mnemonic, "LDA") == 0) {
                opcode = OPCODE_LDA;
            } else if (strcasecmp(mnemonic, "ADD") == 0) {
                opcode = OPCODE_ADD;
            } else if (strcasecmp(mnemonic, "SUB") == 0) {
                opcode = OPCODE_SUB;
            } else if (strcasecmp(mnemonic, "STA") == 0) {
                opcode = OPCODE_STA;
            } else if (strcasecmp(mnemonic, "HLT") == 0) {
                opcode = OPCODE_HLT;
            } else if (strcasecmp(mnemonic, "NOP") == 0) {
                opcode = OPCODE_NOP;
            } else if (strcasecmp(mnemonic, "NOT") == 0) {
                opcode = OPCODE_NOT;
            } else if (strncasecmp(mnemonic, "JMP", 3) == 0) {
                opcode = OPCODE_JMP;
            } else if (strncasecmp(mnemonic, "JMN", 3) == 0) {
                opcode = OPCODE_JMN;
            } else if (strncasecmp(mnemonic, "JMZ", 3) == 0) {
                opcode = OPCODE_JMZ;
            } else if (strcasecmp(mnemonic, "OR") == 0) {
                opcode = OPCODE_OR;
            } else if (strcasecmp(mnemonic, "AND") == 0) {
                opcode = OPCODE_AND;
            } else {
                fprintf(stderr, "Mnemônico desconhecido: %s\n", mnemonic);
                continue;
            }
            
            if (opcode != OPCODE_HLT && opcode != OPCODE_NOP && opcode != OPCODE_NOT && items == 2) {
                int symAddr = findSymbol(operand);
                if (symAddr < 0) {
                    if (dataAddr % 2 != 0) dataAddr++;
                    addSymbol(operand, dataAddr, 0, false);
                    symAddr = dataAddr;
                    dataAddr += 2;
                }
                operandByte = (uint8_t)((symAddr - HEADERSIZE) / 2);
            }
            
            memory[codeAddr]   = opcode;
            memory[codeAddr+1] = 0;
            memory[codeAddr+2] = operandByte;
            memory[codeAddr+3] = 0;
            
            codeAddr += 4;
        }
    }
    fclose(fin);
    
    /*for (int i = 0; i < symbolCount; i++) {
        if (!symbols[i].defined) {
        }
    }
    */

    FILE *fout = fopen(outputFile, "wb");
    if (!fout) {
        perror("Erro ao criar o arquivo de memória");
        return false;
    }
    fwrite(memory, 1, MEMORYSIZE, fout);
    fclose(fout);
    return true;
}

int main(int argc, char *argv[]) {
    char inputFile[256] = "programa.asm";
    char outputFile[256] = "programa.mem";
    
    if (argc > 1) {
        strncpy(inputFile, argv[1], sizeof(inputFile) - 1);
        inputFile[sizeof(inputFile) - 1] = '\0';
    }
    if (argc > 2) {
        strncpy(outputFile, argv[2], sizeof(outputFile) - 1);
        outputFile[sizeof(outputFile) - 1] = '\0';
    }
    
    printf("Montando arquivo: %s -> %s\n", inputFile, outputFile);
    
    if (!assemble(inputFile, outputFile)) {
        fprintf(stderr, "Falha na montagem.\n");
        return 1;
    }
    
    return 0;
}