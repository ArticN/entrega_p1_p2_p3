package main

import (
    "fmt"
    "io"
    "os"
)

func main() {
    data, err := io.ReadAll(os.Stdin)
    if err != nil {
        fmt.Fprintln(os.Stderr, err)
        os.Exit(1)
    }
    code := string(data)

    jumps := map[int]int{}
    var stack []int
    for i, c := range code {
        switch c {
        case '[':
            stack = append(stack, i)
        case ']':
            if len(stack) == 0 {
                fmt.Fprintf(os.Stderr, "']' sem par em %d\n", i)
                os.Exit(1)
            }
            j := stack[len(stack)-1]
            stack = stack[:len(stack)-1]
            jumps[j] = i
            jumps[i] = j
        }
    }
    if len(stack) != 0 {
        fmt.Fprintln(os.Stderr, "'[' sem par")
        os.Exit(1)
    }

    tape := make([]byte, 30000)
    ptr := 0

    for ip := 0; ip < len(code); ip++ {
        switch code[ip] {
        case '>':
            ptr++
            if ptr >= len(tape) {
                ptr = 0
            }
        case '<':
            ptr--
            if ptr < 0 {
                ptr = len(tape) - 1
            }
        case '+':
            tape[ptr]++
        case '-':
            tape[ptr]--
        case '.':
            os.Stdout.Write([]byte{tape[ptr]})
        case '[':
            if tape[ptr] == 0 {
                ip = jumps[ip]
            }
        case ']':
            if tape[ptr] != 0 {
                ip = jumps[ip]
            }
        }
    } 
    os.Stdout.Write([]byte{'\n'})
}
