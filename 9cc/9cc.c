#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "引数の個数が正しくありません\n");
		return 1;
	}

	char *p = argv[1];

	printf(".text\n");
	printf(".align 2\n");
	printf(".global _main\n");
	printf("_main:\n");
	printf("  mov x0, %ld\n", strtol(p, &p, 10));

	while (*p) {
		if (*p == '+') {
			p++;
			printf("  add x0, x0, %ld\n", strtol(p, &p, 10));
			// strtol: 読み込んだ後、第２引数を＋１する。
			continue;
		}

		if (*p == '-') {
			p++;
			printf("  sub x0, x0, %ld\n", strtol(p, &p, 10));
			continue;
		}

		fprintf(stderr, "予期しない文字です: '%c'\n", *p);
		return 1;
	}

	printf("  ret\n");
	return 0;
}
