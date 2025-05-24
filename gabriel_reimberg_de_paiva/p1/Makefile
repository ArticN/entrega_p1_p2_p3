CC = gcc
CFLAGS = -Wall

COMPILADOR = compilador
ASSEMBLER = assembler
EXECUTOR = executor

SRC_COMPILADOR = compilador.c
SRC_ASSEMBLER = assembler.c
SRC_EXECUTOR = executor.c

INPUT_LPN = programa.lpn
OUTPUT_ASM = programa.asm
OUTPUT_MEM = programa.mem

.PHONY: all run clean

all: $(COMPILADOR) $(ASSEMBLER) $(EXECUTOR)

$(COMPILADOR): $(SRC_COMPILADOR)
	$(CC) $(CFLAGS) -o $@ $^

$(ASSEMBLER): $(SRC_ASSEMBLER)
	$(CC) $(CFLAGS) -o $@ $^

$(EXECUTOR): $(SRC_EXECUTOR)
	$(CC) $(CFLAGS) -o $@ $^

run: all
	@echo "Etapa 1: compilando .lpn -> .asm"
	./$(COMPILADOR) $(INPUT_LPN)
	@echo "Etapa 2: montando .asm -> .mem"
	./$(ASSEMBLER) $(OUTPUT_ASM) $(OUTPUT_MEM)
	@echo "Etapa 3: executando .mem"
	./$(EXECUTOR)

clean:
	rm -f $(COMPILADOR) $(ASSEMBLER) $(EXECUTOR)
	rm -f $(OUTPUT_ASM) $(OUTPUT_MEM)
