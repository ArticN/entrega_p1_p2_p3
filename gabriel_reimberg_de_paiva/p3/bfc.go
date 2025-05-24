package main

import (
    "fmt"
    "io"
    "os"
    "strings"
    "strconv"
    "unicode"
)

func main() {
    data, err := io.ReadAll(os.Stdin)
    if err != nil {
        fmt.Fprintln(os.Stderr, err)
        os.Exit(1)
    }
    text := strings.TrimSpace(string(data))
    parts := strings.SplitN(text, "=", 2)
    if len(parts) != 2 {
        fmt.Fprintln(os.Stderr, "uso: VAR=EXPR")
        os.Exit(1)
    }
    varName, expr := parts[0], parts[1]

    p := &Parser{s: expr}
    ast := p.parseExpr()

    gen := &BFGen{}

    for _, c := range varName + "=" {
        for _, b := range []byte(string(c)) {
            gen.moveTo(10)
            gen.zero()
            gen.inc(int(b))
            gen.sb.WriteByte('.')
        }
    }

    ast.Gen(gen, 0)

    result := evalNode(ast)
    for _, ch := range strconv.Itoa(result) {
        gen.moveTo(10)
        gen.zero()
        gen.inc(int(ch))
        gen.sb.WriteByte('.')
    }

    fmt.Println(gen.String())
}

// ————————————————— Parser + AST —————————————————————————————

type Parser struct {
    s   string
    pos int
}

func (p *Parser) peek() rune {
    if p.pos >= len(p.s) {
        return 0
    }
    return rune(p.s[p.pos])
}

func (p *Parser) consume() rune {
    ch := p.peek()
    if ch != 0 {
        p.pos++
    }
    return ch
}

func (p *Parser) parseExpr() Node {
    node := p.parseTerm()
    for {
        switch p.peek() {
        case '+', '-':
            op := byte(p.consume())
            right := p.parseTerm()
            node = &BinOp{op: op, left: node, right: right}
        default:
            return node
        }
    }
}

func (p *Parser) parseTerm() Node {
    node := p.parseFactor()
    for p.peek() == '*' {
        p.consume()
        right := p.parseFactor()
        node = &BinOp{op: '*', left: node, right: right}
    }
    return node
}

func (p *Parser) parseFactor() Node {
    if p.peek() == '(' {
        p.consume()
        node := p.parseExpr()
        if p.peek() == ')' {
            p.consume()
        }
        return node
    }
    start := p.pos
    for unicode.IsDigit(p.peek()) {
        p.consume()
    }
    numStr := p.s[start:p.pos]
    num, err := strconv.Atoi(numStr)
    if err != nil {
        fmt.Fprintf(os.Stderr, "número inválido: %s\n", numStr)
        os.Exit(1)
    }
    return &Number{val: num}
}

type Node interface {
    Gen(g *BFGen, cell int)
}

type Number struct{ val int }

func (n *Number) Gen(g *BFGen, cell int) {
    g.moveTo(cell)
    g.zero()
    g.inc(n.val)
}

type BinOp struct {
    op          byte
    left, right Node
}

func (b *BinOp) Gen(g *BFGen, cell int) {
    switch b.op {
    case '+':
        b.left.Gen(g, cell)
        b.right.Gen(g, cell+1)
        g.emitAdd(cell+1, cell)
    case '-':
        b.left.Gen(g, cell)
        b.right.Gen(g, cell+1)
        g.emitSub(cell+1, cell)
    case '*':
        b.left.Gen(g, cell)
        b.right.Gen(g, cell+1)
        g.emitMul(cell, cell+1, cell+2, cell+3)
    }
}

// Avalia a AST em Go apenas para saber o resultado numérico
func evalNode(n Node) int {
    switch v := n.(type) {
    case *Number:
        return v.val
    case *BinOp:
        L := evalNode(v.left)
        R := evalNode(v.right)
        switch v.op {
        case '+':
            return L + R
        case '-':
            return L - R
        case '*':
            return L * R
        }
    }
    return 0
}

// ———————————————— Gerador de Brainfuck ——————————————————

type BFGen struct {
    sb  strings.Builder
    pos int
}

func (g *BFGen) moveTo(c int) {
    for g.pos < c {
        g.sb.WriteByte('>')
        g.pos++
    }
    for g.pos > c {
        g.sb.WriteByte('<')
        g.pos--
    }
}

func (g *BFGen) zero() {
    g.sb.WriteString("[-]")
}

func (g *BFGen) inc(n int) {
    for i := 0; i < n; i++ {
        g.sb.WriteByte('+')
    }
}

func (g *BFGen) emitLoop(c int, body func()) {
    g.moveTo(c)
    g.sb.WriteByte('[')
    body()
    g.moveTo(c)
    g.sb.WriteByte(']')
}

func (g *BFGen) emitAdd(src, dst int) {
    g.emitLoop(src, func() {
        g.sb.WriteByte('-')
        g.moveTo(dst); g.sb.WriteByte('+')
        g.moveTo(src)
    })
    g.moveTo(dst)
}

func (g *BFGen) emitSub(src, dst int) {
    g.emitLoop(src, func() {
        g.sb.WriteByte('-')
        g.moveTo(dst); g.sb.WriteByte('-')
        g.moveTo(src)
    })
    g.moveTo(dst)
}

func (g *BFGen) emitMul(a, b, res, tmp int) {
    g.moveTo(res); g.zero()
    g.moveTo(tmp); g.zero()
    g.emitLoop(a, func() {
        g.moveTo(a); g.sb.WriteByte('-')
        g.emitLoop(b, func() {
            g.sb.WriteByte('-')
            g.moveTo(res); g.sb.WriteByte('+')
            g.moveTo(tmp); g.sb.WriteByte('+')
            g.moveTo(b)
        })
        g.emitLoop(tmp, func() {
            g.sb.WriteByte('-')
            g.moveTo(b); g.sb.WriteByte('+')
            g.moveTo(tmp)
        })
    })
    g.emitLoop(res, func() {
        g.sb.WriteByte('-')
        g.moveTo(a); g.sb.WriteByte('+')
        g.moveTo(res)
    })
    g.moveTo(a)
}

func (g *BFGen) String() string {
    return g.sb.String()
}
