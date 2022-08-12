#include "9cc.h"
#include <stdio.h>


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


