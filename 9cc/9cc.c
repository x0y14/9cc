#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"


char *user_input;

//トークン型
struct Token {
	TokenKind kind;
	Token *next;
	int val;       // kindがTK_NUMの時、数値が入る
	char *str;     // トークン文字列; おそらく生文字
};

// 現在着目しているトークン
Token *token;

struct Node {
	NodeKind kind; // ノードの型
	Node *lhs;     // 左辺
	Node *rhs;     // 右辺
	int val;       // kindがND_NUMの場合のみ使う
};

// エラーを報告するための関数
// printfと同じ引数をとる
void error(char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

void error_at(char *loc, char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	int pos = loc - user_input;
	fprintf(stderr, "%s\n", user_input);
	fprintf(stderr, "%*s", pos, " "); // pos個空白を出力
	fprintf(stderr, "^ ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// 次のトークンが期待している記号の時には、トークンを読み進め
// 真を返す
// それ以外は偽を返す
bool consume(char op) {
	if (token->kind != TK_RESERVED || token->str[0] != op)
		return false;
	token = token->next;
	return true;
}

// 次のトークンが期待している記号の機器には、トークンを読み進め
// それ以外の場合はエラー報告をする。
void expect(char op) {
	if (token->kind != TK_RESERVED || token->str[0] != op) {
		error_at(token->str, "'%c'ではありません", op);
	}
	token = token->next;
}

// 次のトークンが数値の場合、トークンを１つ読み進めてその数値を返す
// それ以外の場合はエラー
int expect_number() {
	if (token->kind != TK_NUM)
		error_at(token->str, "数ではありません");
	int val = token->val;
	token = token->next;
	return val;
}

bool at_eof() {
	return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str) {
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	cur->next = tok;
	return tok;
}

Token *tokenize(char *p) {
	Token head;
	head.next = NULL;
	Token *cur = &head;

	// 見た感じheadはnextだけ代入されて、かつhead.nextが返却なので
	// next以外いじられてない

	while (*p) {
		// whitespace skip
		if (isspace(*p)) {
			p++;
			continue;
		}

		if (*p == '+' || *p == '-' || *p == '*' || *p == '/' ||
				*p == '(' || *p == ')') {
			cur = new_token(TK_RESERVED, cur, p++);
			continue;
		}

		if (isdigit(*p)) {
			// strtolがp++を担うので、ここではインクリメントしない
			cur = new_token(TK_NUM, cur, p);
			cur->val = strtol(p, &p, 10);
			continue;
		}

		error_at(token->str,"トークナイズできません");
	}

	new_token(TK_EOF, cur, p); // ここでインクリメントすると多分セグフォる
	return head.next;
}

// 左辺と右辺を持つノード
Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

// 直接数値を持つノード
Node *new_node_num(int val) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = ND_NUM;
	node->val = val;
	return node;
}

// パース
Node *expr() {
	Node *node = mul();

	for (;;) {
		if (consume('+')) {
			node = new_node(ND_ADD, node, mul());
		} else  if (consume('-')) {
			node = new_node(ND_SUB, node, mul());
		} else {
			return node;
		}
	}
}

Node *mul() {
	Node *node = primary();

	for (;;) {
		if (consume('*')) {
			node = new_node(ND_MUL, node, primary());
		} else if (consume('/')) {
			node = new_node(ND_DIV, node, primary());
		} else {
			return node;
		}
	}
}

Node *primary() {
	if (consume('(')) {
		Node *node = expr();
		expect(')');
		return node;
	}
	return new_node_num(expect_number());
}

void gen(Node *node) {
	if (node->kind == ND_NUM) {
		// spは16Nづつ動かすこと
		printf("  sub sp, sp, #16\n");
		// w8 <- val
		printf("  mov w8, #%d\n", node->val);
		// x8の内容をspから始まる領域に書き込む
		printf("  str w8, [sp]\n");
		// 注意:spは戻していない
		return;
	}

	gen(node->lhs);
	gen(node->rhs);

	// w9 <- spから32bit * 4読み込んだもの
	printf("  ldr w9, [sp]\n");
	// sp+=16で、読み終わったデータ分戻る
	printf("  add sp, sp, #16\n");
	// w8 <- spから始まる32bit *4の領域のデータ
	printf("  ldr w8, [sp]\n");
	// sp+=16で、読み終わったデータ分戻る
	printf("  add sp, sp, #16\n");

	switch (node->kind) {
		case ND_ADD:
			// w8 <- w8 + w9
			printf("  add w8, w8, w9\n");
			break;
		case ND_SUB:
			// w8 <- w8 - w9
			printf("  sub w8, w8, w9\n");
			break;
		case ND_MUL:
			// w8 <- w8 * w9
			printf("  mul w8, w8, w9\n");
			break;
		case ND_DIV:
			// w8 <- w8 / w9
			printf("  sdiv w8, w8, w9\n");
			break;
		case ND_NUM:
			error("記号ではなく数字トークンを発見しました");
			break;
	}

	// 16Nづつ動かす
	printf("  sub sp, sp, #16\n");
	// w8のデータをspから始まる32bit * 4の領域に書き込む
	// wNは32bit*4のレジスタ
	// w8は計算結果
	printf("  str w8, [sp]\n");

}

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "引数の個数が正しくありません\n");
		return 1;
	}

	user_input = argv[1];
	token = tokenize(user_input);
	Node *node = expr();

	printf(".text\n");
	printf(".align 2\n");
	printf(".global _main\n");
	printf("_main:\n");
	// 16Nづつ
	printf("  sub sp, sp, #16\n");
	// 32bitのzr(zero register)をspに書き込む
	// 用途は不明
	printf("  str wzr, [sp]\n");

	gen(node);

	// 最終的な計算結果がスタックに残っているはずなので回収する
	printf("  ldr w8, [sp]\n");
	// spを戻してあげる
	printf("  add sp, sp, #16\n"); // 計算結果を保存したw8のスタック

	// 戻り値w0にw8のデータを書き込んであげる
	printf("  sub sp, sp, #16\n");
	printf("  str w8, [sp]\n");
	printf("  ldr w0, [sp]\n");
	printf("  add sp, sp, #16\n"); // w8->w0に使用したスタック
	printf("  add sp, sp, #16\n"); // wzrに使用したスタック

	printf("  ret\n");
	return 0;
}
