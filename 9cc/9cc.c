#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"


char *user_input;


int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "引数の個数が正しくありません\n");
		return 1;
	}

	user_input = argv[1];
	Token *token = tokenize(user_input);
	Node *node = parse(token);

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
