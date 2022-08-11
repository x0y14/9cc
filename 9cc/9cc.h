#pragma once

typedef enum {
	TK_RESERVED,
	TK_NUM,
	TK_EOF,
} TokenKind;

typedef struct Token Token;

struct Token;
typedef enum {
	ND_ADD,
	ND_SUB,
	ND_MUL,
	ND_DIV,
	ND_NUM,
} NodeKind;

typedef struct Node Node;

struct Node;

void error(char*, ...);
void error_at(char*, char*, ...);

bool consume(char);

void expect(char);
int expect_number();

bool at_eof();

Token *new_token(TokenKind, Token*, char*);

Token *tokenize(char*);

Node *new_node(NodeKind, Node*, Node*);
Node *new_node_num(int);

// Parse
Node *expr();
Node *mul();
Node *primary();

void gen(Node*);
