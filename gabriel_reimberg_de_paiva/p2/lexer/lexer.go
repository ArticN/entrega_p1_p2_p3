// lexer.go (refatorado)
package lexer

import (
    "unicode"
)

type TokenType string

type Token struct {
    Type    TokenType
    Literal string
}

const (
    ILLEGAL    TokenType = "ILLEGAL"
    EOF        TokenType = "EOF"
    IDENT      TokenType = "IDENT"
    INT_LIT    TokenType = "INT_LIT"
    FLOAT_LIT  TokenType = "FLOAT_LIT"
    STRING_LIT TokenType = "STRING_LIT"
    BOOL_LIT   TokenType = "BOOL_LIT"

    INICIO TokenType = "INICIO"
    FIM    TokenType = "FIM"
    FUNC   TokenType = "FUNC"
    IF     TokenType = "IF"
    ELSE   TokenType = "ELSE"
    WHILE  TokenType = "WHILE"
    RETURN TokenType = "RETURN"
    PRINT  TokenType = "PRINT"
    TYPE   TokenType = "TYPE"

    ASSIGN TokenType = ":="
    PLUS   TokenType = "+"
    MINUS  TokenType = "-"
    ASTER  TokenType = "*"
    SLASH  TokenType = "/"
    LT     TokenType = "<"
    GT     TokenType = ">"
    EQ     TokenType = "=="
    NOT_EQ TokenType = "!="
    LTE    TokenType = "<="
    GTE    TokenType = ">="

    COMMA    TokenType = ","
    SEMIC    TokenType = ";"
    LPAREN   TokenType = "("
    RPAREN   TokenType = ")"
    LBRACE   TokenType = "{"
    RBRACE   TokenType = "}"
    LBRACKET TokenType = "["
    RBRACKET TokenType = "]"
)

var keywords = map[string]TokenType{
    "inicio": INICIO,
    "fim":    FIM,
    "func":   FUNC,
    "if":     IF,
    "else":   ELSE,
    "while":  WHILE,
    "return": RETURN,
    "print":  PRINT,
    "int":    TYPE,
    "float":  TYPE,
    "string": TYPE,
    "bool":   TYPE,
    "true":   BOOL_LIT,
    "false":  BOOL_LIT,
}

var singleRuneTokens = map[rune]TokenType{
    '+': PLUS, '-': MINUS, '*': ASTER, '/': SLASH,
    ',': COMMA, ';': SEMIC, '(': LPAREN, ')': RPAREN,
    '{': LBRACE, '}': RBRACE, '[': LBRACKET, ']': RBRACKET,
    '=': ASSIGN, '<': LT, '>': GT,
}

type Lexer struct {
    input   []rune
    pos     int
    readPos int
    current rune
}

func New(input string) *Lexer {
    l := &Lexer{input: []rune(input)}
    l.readRune()
    return l
}

func (l *Lexer) readRune() {
    if l.readPos >= len(l.input) {
        l.current = 0
    } else {
        l.current = l.input[l.readPos]
    }
    l.pos = l.readPos
    l.readPos++
}

func (l *Lexer) peekRune() rune {
    if l.readPos >= len(l.input) {
        return 0
    }
    return l.input[l.readPos]
}


func (l *Lexer) NextToken() Token {
    l.skipWhitespace()

    // 1) reconhecimento de ":="
    if l.current == ':' && l.peekRune() == '=' {
        l.readRune() // consome ':'
        l.readRune() // consome '='
        return Token{Type: ASSIGN, Literal: ":="}
    }

    // 2) operadores de dois caracteres (==, !=, <=, >=) – mantidos
    if (l.current == '=' || l.current == '!' || l.current == '<' || l.current == '>') && l.peekRune() == '=' {
        first := l.current
        l.readRune()
        lit := string(first) + string(l.current)
        compMap := map[string]TokenType{
            "==": EQ, "!=": NOT_EQ, "<=": LTE, ">=": GTE,
        }
        tokType := compMap[lit]
        l.readRune()
        return Token{Type: tokType, Literal: lit}
    }

    // 3) tokens de um único rune (remova '=' daqui, pois agora só aparece via ":=" ou "==")
    if tokType, ok := singleRuneTokens[l.current]; ok {
        lit := string(l.current)
        l.readRune()
        return Token{Type: tokType, Literal: lit}
    }

    // 4) literais, identificadores, EOF, ILLEGAL…
    switch {
    case l.current == 0:
        return Token{Type: EOF, Literal: ""}
    case isLetter(l.current):
        ident := l.readIdentifier()
        if typ, isKw := keywords[ident]; isKw {
            return Token{Type: typ, Literal: ident}
        }
        return Token{Type: IDENT, Literal: ident}
    case unicode.IsDigit(l.current):
        num, typ := l.readNumber()
        return Token{Type: typ, Literal: num}
    case l.current == '"':
        str := l.readString()
        return Token{Type: STRING_LIT, Literal: str}
    default:
        lit := string(l.current)
        l.readRune()
        return Token{Type: ILLEGAL, Literal: lit}
    }
}

func (l *Lexer) skipWhitespace() {
    for unicode.IsSpace(l.current) {
        l.readRune()
    }
}

func (l *Lexer) readIdentifier() string {
    start := l.pos
    for isLetter(l.current) || unicode.IsDigit(l.current) {
        l.readRune()
    }
    return string(l.input[start:l.pos])
}

func (l *Lexer) readNumber() (string, TokenType) {
    start := l.pos
    tokType := INT_LIT
    for unicode.IsDigit(l.current) {
        l.readRune()
    }
    if l.current == '.' {
        tokType = FLOAT_LIT
        l.readRune()
        for unicode.IsDigit(l.current) {
            l.readRune()
        }
    }
    return string(l.input[start:l.pos]), tokType
}

func (l *Lexer) readString() string {
    l.readRune() // consome a aspa inicial
    start := l.pos
    for l.current != '"' && l.current != 0 {
        l.readRune()
    }
    lit := string(l.input[start:l.pos])
    l.readRune() // consome a aspa final
    return lit
}

func isLetter(ch rune) bool {
    return unicode.IsLetter(ch) && ch >= 'a' && ch <= 'z'
}
