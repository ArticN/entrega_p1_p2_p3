## Brainfuck Compiler (`bfc`) & Executor (`bfe`)

Este repositório implementa dois códigos em Go para avaliar expressões numéricas e imprimir resultados usando Brainfuck em tempo de execução:

1. **`bfc`**: compila uma entrada no formato `VAR=EXPR`, gera um programa Brainfuck que realiza **todas** as operações de `EXPR` em tempo de execução.
2. **`bfe`**: interpreta o Brainfuck produzido pelo `bfc` e imprime o resultado final na saída padrão.

---

### Processo

* **Parser**: `bfc` faz parsing recursivo-descendente (LL(1)) de `EXPR` (`+`, `-`, `*`, parênteses, literais numéricos).
* **Codegen**: constrói código Brainfuck que executa soma, subtração, multiplicação (via loops), conversão para decimal e impressão de dígitos.
* **Execução**: `bfe` mapeia saltos (`[`/`]`), gerencia fita de 30.000 células e executa os comandos Brainfuck, produzindo o texto `VAR=valor`
---

### Como compilar
```bash
# Compilar o compilador Brainfuck
go build -o bfc bfc.go

# Compilar o executor Brainfuck
go build -o bfe bfe.go
```

---

### Exemplo de teste

```bash
go build -o bfc bfc.go
go build -o bfe bfe.go
echo 'CRÉDITO=2*5+10' | ./bfc
>>>>>>>>>>[-]+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.[-]++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.[-]+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.[-]+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.[-]++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.[-]+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.[-]++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.[-]+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.[-]+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.<<<<<<<<<<[-]++>[-]+++++>[-]>[-]<<<[->[->+>+<<]>>[-<<+>>]<<<]>>[-<<+>>]<<>[-]++++++++++[-<+>]<>[-]<[---------->+<]>[++++++++++++++++++++++++++++++++++++++++++++++++.[-]]<++++++++++++++++++++++++++++++++++++++++++++++++.[-]
echo 'CRÉDITO=2*5+10' | ./bfc | ./bfe
CRÉDITO=20
```
### Limitações
- Não aceita divisão
