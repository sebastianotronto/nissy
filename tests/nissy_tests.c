#include <stdio.h>

void fst_pre_init_testall();
void fst_post_init_testall();

int main() {
	fst_pre_init_testall();
	fst_post_init_testall();

	printf("All tests passed.\n");
	return 0;
}
