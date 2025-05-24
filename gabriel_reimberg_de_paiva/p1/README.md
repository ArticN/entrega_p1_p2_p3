[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/3XHcMjDV)
Gabriel Reimberg de Paiva

# Projeto Compilador + Assembler + Executor NEANDER

Este projeto implementa um pipeline de tradução e execução para a linguagem NEANDER. O processo parte de um código-fonte `.lpn`, passa por uma compilação para `.asm`, montagem para `.mem` (formato binário), e finalmente execução via executor/simulador do neander.

## Estrutura do Projeto

- `compilador.c` — Compila código `.lpn` para `.asm`
- `assembler.c` — Monta o `.asm` em um arquivo binário `.mem`
- `executor.c` — Executa o `.mem`, simulando a CPU NEANDER
- `programa.lpn` — Exemplo de código de entrada
- `Makefile` — Automatiza a compilação e execução
- `gramatica.pdf` — Documento com a gramática da linguagem

## Como Usar

1. **Compilar o projeto**

   ```bash
   make
   ```

2. **Executar o pipeline completo**
   Certifique de que o arquivo .lpn tenha o nome "programa". O código irá buscar pelo arquivo de nome "programa.lpn"

   ```bash
   make run
   ```

   Esse comando realiza:

   - Compilação de `programa.lpn` → `programa.asm`
   - Montagem de `programa.asm` → `programa.mem`
   - Execução de `programa.mem` e exibição do estado da memória

3. **Limpar arquivos gerados**

   ```bash
   make clean
   ```

## Exemplo de Código `.lpn`

```text
PROGRAMA "Exemplo":
INICIO
a = 2
b = 3
RES = a * b
FIM
```

## Limitações do Projeto

-  **Divisão (`/`) não está implementada.**
- Atribuições com expressões fixas como `a = 2 + 3` **não funcionam**.
  - Apenas números diretos ou variáveis são aceitas(`a = b + 3` também não serão aceitas)
- A variável `RES` deve obrigatoriamente estar no final e conter o resultado principal.
- Variáveis não inicializadas podem resultar em comportamento indefinido.
- .mem gerado não é compativel com o programa WNeander ()

##  Artefatos Gerados

- `programa.asm` — Código em assembly da linguagem NEANDER
- `programa.mem` — Arquivo binário com instruções e dados
- Saída do `executor` — Memória final, registrador AC e PC

## To Do
- Finalizar compatibilidade com o programa WNeander
- Possibilitar criação de variaveis com operações
- Implementar Divisão e outras operações básicas (pow)
---

## Importante
O Resultado aparecerá no AC
