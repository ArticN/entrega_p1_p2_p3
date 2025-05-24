#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

typedef enum { AST_NUM, AST_VAR, AST_BINOP } ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    union {
        int num;
        char var[64];
        struct {
            char op;
            struct ASTNode *left;
            struct ASTNode *right;
        } binop;
    };
} ASTNode;

typedef struct Statement {
    char var[64];
    ASTNode* expr;
    struct Statement* next;
} Statement;

Statement* statements = NULL;
Statement* lastStmt = NULL;

typedef struct {
    char name[64];
    Statement* stmts;
    ASTNode* resultExpr;
} Program;

Program program;

typedef enum {
    TOKEN_PROGRAMA,
    TOKEN_INICIO,
    TOKEN_FIM,
    TOKEN_RES,
    TOKEN_IDENT,
    TOKEN_NUM,
    TOKEN_EQ,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_TIMES,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_COLON,
    TOKEN_EOF,
    TOKEN_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[64];
} Token;

#define MAX_TOKENS 1024
Token tokens[MAX_TOKENS];
int tokenCount = 0;
int currentToken = 0;

char* source;
int sourcePos = 0;

ASTNode* parseExpression();
ASTNode* parseTerm();
ASTNode* parseFactor();

typedef struct {
    char name[64];
    int value;
    bool defined;
} Var;

#define MAX_VARS 256
Var varTable[MAX_VARS];
int varCount = 0;
FILE* asmOut;

void skipWhitespace() {
    while (source[sourcePos] == ' ' || source[sourcePos] == '\t')
        sourcePos++;
}

int isLetter(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

int isDigit(char c) {
    return (c >= '0' && c <= '9');
}

void addToken(TokenType type, const char *lexeme) {
    if (tokenCount < MAX_TOKENS) {
        tokens[tokenCount].type = type;
        strncpy(tokens[tokenCount].lexeme, lexeme, sizeof(tokens[tokenCount].lexeme)-1);
        tokens[tokenCount].lexeme[sizeof(tokens[tokenCount].lexeme)-1] = '\0';
        tokenCount++;
    }
}

void tokenize() {
    tokenCount = 0;
    currentToken = 0;
    sourcePos = 0;
    while (source[sourcePos] != '\0') {
        if (source[sourcePos] == ' ' || source[sourcePos] == '\t') {
            skipWhitespace();
            continue;
        }
        if (source[sourcePos] == '\n' || source[sourcePos] == '\r') {
            sourcePos++;
            continue;
        }
        if (strncmp(&source[sourcePos], "PROGRAMA", 8) == 0 && !isLetter(source[sourcePos+8])) {
            addToken(TOKEN_PROGRAMA, "PROGRAMA");
            sourcePos += 8;
            continue;
        }
        if (strncmp(&source[sourcePos], "INICIO", 6) == 0 && !isLetter(source[sourcePos+6])) {
            addToken(TOKEN_INICIO, "INICIO");
            sourcePos += 6;
            continue;
        }
        if (strncmp(&source[sourcePos], "FIM", 3) == 0 && !isLetter(source[sourcePos+3])) {
            addToken(TOKEN_FIM, "FIM");
            sourcePos += 3;
            continue;
        }
        if (strncmp(&source[sourcePos], "RES", 3) == 0 && !isLetter(source[sourcePos+3])) {
            addToken(TOKEN_RES, "RES");
            sourcePos += 3;
            continue;
        }
        if (source[sourcePos] == '=') {
            addToken(TOKEN_EQ, "=");
            sourcePos++;
            continue;
        }
        if (source[sourcePos] == '+') {
            addToken(TOKEN_PLUS, "+");
            sourcePos++;
            continue;
        }
        if (source[sourcePos] == '-') {
            addToken(TOKEN_MINUS, "-");
            sourcePos++;
            continue;
        }
        if (source[sourcePos] == '*') {
            addToken(TOKEN_TIMES, "*");
            sourcePos++;
            continue;
        }
        if (source[sourcePos] == '(') {
            addToken(TOKEN_LPAREN, "(");
            sourcePos++;
            continue;
        }
        if (source[sourcePos] == ')') {
            addToken(TOKEN_RPAREN, ")");
            sourcePos++;
            continue;
        }
        if (source[sourcePos] == ':') {
            addToken(TOKEN_COLON, ":");
            sourcePos++;
            continue;
        }
        if (source[sourcePos] == '\"') {
            sourcePos++;
            int start = sourcePos;
            while (source[sourcePos] != '\"' && source[sourcePos] != '\0')
                sourcePos++;
            int len = sourcePos - start;
            char ident[64];
            strncpy(ident, &source[start], len);
            ident[len] = '\0';
            addToken(TOKEN_IDENT, ident);
            if (source[sourcePos] == '\"')
                sourcePos++;
            continue;
        }
        if (isLetter(source[sourcePos])) {
            int start = sourcePos;
            while (isLetter(source[sourcePos]) || isDigit(source[sourcePos]) || source[sourcePos]=='_')
                sourcePos++;
            int len = sourcePos - start;
            char ident[64];
            strncpy(ident, &source[start], len);
            ident[len] = '\0';
            addToken(TOKEN_IDENT, ident);
            continue;
        }
        if (isDigit(source[sourcePos])) {
            int start = sourcePos;
            while (isDigit(source[sourcePos]))
                sourcePos++;
            int len = sourcePos - start;
            char num[64];
            strncpy(num, &source[start], len);
            num[len] = '\0';
            addToken(TOKEN_NUM, num);
            continue;
        }
        sourcePos++;
    }
    addToken(TOKEN_EOF, "EOF");
}

Token* peekToken() {
    if (currentToken < tokenCount)
        return &tokens[currentToken];
    return NULL;
}

Token* getToken() {
    if (currentToken < tokenCount)
        return &tokens[currentToken++];
    return NULL;
}

ASTNode* newNumNode(int value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_NUM;
    node->num = value;
    return node;
}

ASTNode* newVarNode(const char* name) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_VAR;
    strncpy(node->var, name, sizeof(node->var)-1);
    node->var[sizeof(node->var)-1] = '\0';
    return node;
}

ASTNode* newBinOpNode(char op, ASTNode* left, ASTNode* right) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_BINOP;
    node->binop.op = op;
    node->binop.left = left;
    node->binop.right = right;
    return node;
}

ASTNode* parseExpression() {
    ASTNode* node = parseTerm();
    Token* t;
    while ((t = peekToken()) && (t->type == TOKEN_PLUS || t->type == TOKEN_MINUS)) {
        t = getToken();
        ASTNode* right = parseTerm();
        node = newBinOpNode(t->lexeme[0], node, right);
    }
    return node;
}

ASTNode* parseTerm() {
    ASTNode* node = parseFactor();
    Token* t;
    while ((t = peekToken()) && (t->type == TOKEN_TIMES)) {
        t = getToken();
        ASTNode* right = parseFactor();
        node = newBinOpNode(t->lexeme[0], node, right);
    }
    return node;
}

ASTNode* parseFactor() {
    Token* t = peekToken();
    if (!t) return NULL;
    if (t->type == TOKEN_LPAREN) {
        getToken();
        ASTNode* node = parseExpression();
        getToken();
        return node;
    }
    if (t->type == TOKEN_NUM) {
        t = getToken();
        return newNumNode(atoi(t->lexeme));
    }
    if (t->type == TOKEN_IDENT) {
        t = getToken();
        return newVarNode(t->lexeme);
    }
    return NULL;
}

void parseAssignment() {
    Token* t = getToken();
    if (!t || t->type != TOKEN_IDENT) {
        printf("ERRP: token esperado -> IDENT\n");
        return;
    }
    char varName[64];
    strncpy(varName, t->lexeme, sizeof(varName));
    Token* eq = getToken();
    if (!eq || eq->type != TOKEN_EQ) {
        printf("ERRO: token esperado -> '='\n");
        return;
    }
    ASTNode* expr = parseExpression();
    
    Statement* stmt = malloc(sizeof(Statement));
    strncpy(stmt->var, varName, sizeof(stmt->var));
    stmt->expr = expr;
    stmt->next = NULL;
    if (statements == NULL) {
        statements = stmt;
        lastStmt = stmt;
    } else {
        lastStmt->next = stmt;
        lastStmt = stmt;
    }
}

void freeAST(ASTNode* node) {
    if (!node) return;
    if (node->type == AST_BINOP) {
        freeAST(node->binop.left);
        freeAST(node->binop.right);
    }
    free(node);
}

void freeStatements(Statement* stmt) {
    while (stmt) {
        Statement* next = stmt->next;
        freeAST(stmt->expr);
        free(stmt);
        stmt = next;
    }
}

void parseProgram() {
    Token* t = getToken();
    if (!t || t->type != TOKEN_PROGRAMA) { 
        printf("Erro: esperado PROGRAMA\n"); 
        exit(1); 
    }
    
    t = getToken();
    if (!t || t->type != TOKEN_IDENT) { 
        printf("Erro: esperado nome do programa\n"); 
        exit(1); 
    }
    strncpy(program.name, t->lexeme, sizeof(program.name));
    
    t = getToken();
    if (!t || t->type != TOKEN_COLON) { 
        printf("Erro: esperado ':' após nome\n"); 
        exit(1); 
    }
    
    t = getToken();
    if (!t || t->type != TOKEN_INICIO) { 
        printf("Erro: esperado INICIO\n"); 
        exit(1); 
    }
    
    while (1) {
        t = peekToken();
        if (!t) break;
        if (t->type == TOKEN_RES)
            break;
        parseAssignment();
    }
    
    t = getToken();
    if (!t || t->type != TOKEN_RES) { 
        printf("Erro: esperado RES\n"); 
        exit(1); 
    }    
    t = getToken();
    if (!t || t->type != TOKEN_EQ) { 
        printf("Erro: esperado '=' após RES\n"); 
        exit(1); 
    }
    program.resultExpr = parseExpression();
    
    t = getToken();
    if (!t || t->type != TOKEN_FIM) { 
        printf("Erro: esperado FIM\n"); 
        exit(1); 
    }
}

int addVar(const char* name) {
    for (int i = 0; i < varCount; i++) {
        if (strcmp(varTable[i].name, name) == 0)
            return i;
    }
    strncpy(varTable[varCount].name, name, sizeof(varTable[varCount].name)-1);
    varTable[varCount].name[sizeof(varTable[varCount].name)-1] = '\0';
    varTable[varCount].value = 0;
    varTable[varCount].defined = false;
    varCount++;
    return varCount - 1;
}

void updateVarValue(const char* name, int value) {
    for (int i = 0; i < varCount; i++) {
        if (strcmp(varTable[i].name, name) == 0) {
            varTable[i].value = value;
            varTable[i].defined = true;
            return;
        }
    }
    addVar(name);
    updateVarValue(name, value);
}

void ensureConstantExists(int value) {
    char constName[64];
    sprintf(constName, "CONST_%d", value);
    for (int i = 0; i < varCount; i++) {
        if (strcmp(varTable[i].name, constName) == 0)
            return;
    }
    addVar(constName);
    updateVarValue(constName, value);
}

int tempCount = 0;
char tempBuffer[64];

void newTemp(char* buffer) {
    sprintf(buffer, "TEMP_%d", tempCount++);
    addVar(buffer);
}

void genExpr(ASTNode* node) {
    if (node->type == AST_NUM) {
        char constName[64];
        sprintf(constName, "CONST_%d", node->num);
        ensureConstantExists(node->num);
        fprintf(asmOut, "LDA %s\n", constName);
    } else if (node->type == AST_VAR) {
        addVar(node->var);
        fprintf(asmOut, "LDA %s\n", node->var);
    } else if (node->type == AST_BINOP) {
        char op = node->binop.op;
        if (op == '+') {
            if (node->binop.left->type == AST_VAR && node->binop.right->type == AST_VAR) {
                fprintf(asmOut, "LDA %s\n", node->binop.left->var);
                fprintf(asmOut, "ADD %s\n", node->binop.right->var);
            } else if (node->binop.left->type == AST_VAR && node->binop.right->type == AST_NUM) {
                char constName[64];
                sprintf(constName, "CONST_%d", node->binop.right->num);
                ensureConstantExists(node->binop.right->num);
                fprintf(asmOut, "LDA %s\n", node->binop.left->var);
                fprintf(asmOut, "ADD %s\n", constName);
            } else if (node->binop.left->type == AST_NUM && node->binop.right->type == AST_VAR) {
                char constName[64];
                sprintf(constName, "CONST_%d", node->binop.left->num);
                ensureConstantExists(node->binop.left->num);
                fprintf(asmOut, "LDA %s\n", constName);
                fprintf(asmOut, "ADD %s\n", node->binop.right->var);
            } else if (node->binop.left->type == AST_NUM && node->binop.right->type == AST_NUM) {
                int result = node->binop.left->num + node->binop.right->num;
                char constName[64];
                sprintf(constName, "CONST_%d", result);
                ensureConstantExists(result);
                fprintf(asmOut, "LDA %s\n", constName);
            } else {
                genExpr(node->binop.left);
                if (node->binop.right->type == AST_VAR) {
                    fprintf(asmOut, "ADD %s\n", node->binop.right->var);
                } else if (node->binop.right->type == AST_NUM) {
                    char constName[64];
                    sprintf(constName, "CONST_%d", node->binop.right->num);
                    ensureConstantExists(node->binop.right->num);
                    fprintf(asmOut, "ADD %s\n", constName);
                } else {
                    newTemp(tempBuffer);
                    fprintf(asmOut, "STA %s\n", tempBuffer);

                    genExpr(node->binop.right);

                    fprintf(asmOut, "ADD %s\n", tempBuffer);
                }
            }
        } else if (op == '-') {
            if (node->binop.left->type == AST_VAR && node->binop.right->type == AST_VAR) {
                fprintf(asmOut, "LDA %s\n", node->binop.left->var);
                fprintf(asmOut, "SUB %s\n", node->binop.right->var);
            } else if (node->binop.left->type == AST_VAR && node->binop.right->type == AST_NUM) {
                char constName[64];
                sprintf(constName, "CONST_%d", node->binop.right->num);
                ensureConstantExists(node->binop.right->num);
                fprintf(asmOut, "LDA %s\n", node->binop.left->var);
                fprintf(asmOut, "SUB %s\n", constName);
            } else if (node->binop.left->type == AST_NUM && node->binop.right->type == AST_VAR) {
                char constName[64];
                sprintf(constName, "CONST_%d", node->binop.left->num);
                ensureConstantExists(node->binop.left->num);
                fprintf(asmOut, "LDA %s\n", constName);
                fprintf(asmOut, "SUB %s\n", node->binop.right->var);
            } else if (node->binop.left->type == AST_NUM && node->binop.right->type == AST_NUM) {
                int result = node->binop.left->num - node->binop.right->num;
                char constName[64];
                sprintf(constName, "CONST_%d", result);
                ensureConstantExists(result);
                fprintf(asmOut, "LDA %s\n", constName);
            } else {
                genExpr(node->binop.left);
                if (node->binop.right->type == AST_VAR) {
                    fprintf(asmOut, "SUB %s\n", node->binop.right->var);
                } else if (node->binop.right->type == AST_NUM) {
                    char constName[64];
                    sprintf(constName, "CONST_%d", node->binop.right->num);
                    ensureConstantExists(node->binop.right->num);
                    fprintf(asmOut, "SUB %s\n", constName);
                } else {
                    newTemp(tempBuffer);
                    fprintf(asmOut, "STA %s\n", tempBuffer);
                    genExpr(node->binop.right);

                    char rightTemp[64];
                    newTemp(rightTemp);
                    fprintf(asmOut, "STA %s\n", rightTemp);
                    
                    fprintf(asmOut, "LDA %s\n", tempBuffer);
                    fprintf(asmOut, "SUB %s\n", rightTemp);
                }
            }
        } else if (op == '*') {
            char tempResult[64];
            newTemp(tempResult);
            fprintf(asmOut, "LDA CONST_0\n");
            fprintf(asmOut, "STA %s\n", tempResult);

            if (node->binop.right->type == AST_NUM) {
                int multiplier = node->binop.right->num;
                for (int i = 0; i < multiplier; i++) {
                    genExpr(node->binop.left);
                    fprintf(asmOut, "ADD %s\n", tempResult);
                    fprintf(asmOut, "STA %s\n", tempResult);
                }
            } else if (node->binop.right->type == AST_VAR) {
                int multiplier = -1;
                for (int i = 0; i < varCount; i++) {
                    if (strcmp(varTable[i].name, node->binop.right->var) == 0 && varTable[i].defined) {
                        multiplier = varTable[i].value;
                        break;
                    }
                }
                if (multiplier >= 0) {
                    for (int i = 0; i < multiplier; i++) {
                        genExpr(node->binop.left);
                        fprintf(asmOut, "ADD %s\n", tempResult);
                        fprintf(asmOut, "STA %s\n", tempResult);
                    }
                }
            }
            fprintf(asmOut, "LDA %s\n", tempResult);
        }
    }
}


void genAssignment(Statement* stmt) {
    if (stmt->expr && stmt->expr->type == AST_NUM) {
        updateVarValue(stmt->var, stmt->expr->num);
        char constName[64];
        sprintf(constName, "CONST_%d", stmt->expr->num);
        ensureConstantExists(stmt->expr->num);
        fprintf(asmOut, "LDA %s\n", constName);
        fprintf(asmOut, "STA %s\n", stmt->var);
    } else {
        genExpr(stmt->expr);
        addVar(stmt->var);
        fprintf(asmOut, "STA %s\n", stmt->var);
    }
}

void generateAssembly() {
    fprintf(asmOut, ".DATA\n");

    fprintf(asmOut, "ONE DB 1\n");
    fprintf(asmOut, "CONST_0 DB 0\n");
    fprintf(asmOut, "CONST_1 DB 1\n");
    fprintf(asmOut, "NEG_1 DB 255\n");
    fprintf(asmOut, "RES DB ?\n");
    
    Statement* stmt = statements;
    while (stmt) {
        if (stmt->expr && stmt->expr->type == AST_NUM) {
            ensureConstantExists(stmt->expr->num);
        }
        stmt = stmt->next;
    }
    
    for (int i = 0; i < varCount; i++) {
        if (strcmp(varTable[i].name, "ONE") == 0 ||
            strcmp(varTable[i].name, "CONST_0") == 0 ||
            strcmp(varTable[i].name, "CONST_1") == 0 ||
            strcmp(varTable[i].name, "NEG_1") == 0 ||
            strcmp(varTable[i].name, "RES") == 0)
            continue;
        
        if (strncmp(varTable[i].name, "TEMP_", 5) == 0) {
            fprintf(asmOut, "%s DB ?\n", varTable[i].name);
        } else if (strncmp(varTable[i].name, "CONST_", 6) == 0) {
            fprintf(asmOut, "%s DB %d\n", varTable[i].name, varTable[i].value);
        } else if (!varTable[i].defined) {
            fprintf(asmOut, "%s DB ?\n", varTable[i].name);
        } else {
            fprintf(asmOut, "%s DB %d\n", varTable[i].name, varTable[i].value);
        }
    }
    
    fprintf(asmOut, "\n.CODE\n");
    fprintf(asmOut, ".ORG 0\n");
    
    stmt = statements;
    while (stmt) {
        genAssignment(stmt);
        stmt = stmt->next;
    }
    genExpr(program.resultExpr);
    fprintf(asmOut, "STA RES\n");
    fprintf(asmOut, "HLT\n");
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Uso: %s programa.lpn\n", argv[0]);
        return 1;
    }

    FILE* fp = fopen(argv[1], "r");
    if (!fp) {
        perror("Erro ao abrir o arquivo .lpn");
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    source = (char*)malloc(fileSize + 1);
    if (!source) {
        perror("Erro ao alocar memória");
        fclose(fp);
        return 1;
    }

    size_t bytesRead = fread(source, 1, fileSize, fp);
    source[bytesRead] = '\0';
    fclose(fp);

    char outputFile[256];
    strncpy(outputFile, argv[1], sizeof(outputFile)-5);
    outputFile[sizeof(outputFile)-5] = '\0';
    char* dot = strrchr(outputFile, '.');
    if (dot) *dot = '\0';
    strcat(outputFile, ".asm");
    
    asmOut = fopen(outputFile, "w");
    if (!asmOut) {
        perror("Erro ao criar arquivo de saída .asm");
        free(source);
        return 1;
    }

    statements = NULL;
    lastStmt = NULL;
    varCount = 0;
    tempCount = 0;

    tokenize();
    parseProgram();
    generateAssembly();
    
    freeStatements(statements);
    freeAST(program.resultExpr);
    free(source);
    fclose(asmOut);

    return 0;
}