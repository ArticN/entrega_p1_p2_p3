# Linguagem LG - Gramática + Lexer

Este projeto é um lexer escrito em Go para a linguagem definida no`bnfgramatica.txt`. O programa lê um arquivo `.lg` contendo código fonte e imprime a sequência de tokens reconhecidos.

## Estrutura

- `main.go`: Lê o arquivo `code.lg`, inicializa o lexer e printa os tokens
- `lexer.go`: Identifica tokens válidos da linguagem.
- `bnfgramatica.txt`: Definição da linguagem em formato BNF.

## Executando o projeto

1. Certifique-se de ter o Go instalado em seu sistema.
2. Certifique-se de ter o arquivo code.lg
3. Execute o programa com:

```bash
go run main.go
```