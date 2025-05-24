package main

import (
    "fmt"
    "log"
    "os"

    "app/lexer"
)

func main() {

    data, err := os.ReadFile("code.lg")
    if err != nil {
        log.Fatalf("erro ao ler o arquivo .lg: %v", err)
    }
    sourceCode := string(data)

    l := lexer.New(sourceCode)

    for tok := l.NextToken(); tok.Type != lexer.EOF; tok = l.NextToken() {
        fmt.Printf("%+v\n", tok)
    }
}