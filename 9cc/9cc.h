#pragma once
#include <stdbool.h>

extern char *user_input;

typedef enum {
	TK_RESERVED,
	TK_NUM,
	TK_EOF,
} TokenKind;

typedef struct Token Token;

struct Token {
	TokenKind kind;
	Token *next;
	int val;
	char *str;
};

typedef enum {
	ND_ADD,
	ND_SUB,
	ND_MUL,
	ND_DIV,
	ND_NUM,
} NodeKind;

typedef struct Node Node;

struct Node {
	NodeKind kind;
	Node *lhs;
	Node *rhs;
	int val;
};

// parser
void error(char *fmt, ...);
void error_at(char *loc, char* fmt, ...);

bool consume(char op);

void expect(char op);
int expect_number();

bool at_eof();

Token *new_token(TokenKind kind, Token *cur, char *str);

Token *tokenize(char *p);

Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);

Node *expr();
Node *mul();
Node *primary();

Node *parse(Token *tok);


// codegen
void gen(Node *node);
