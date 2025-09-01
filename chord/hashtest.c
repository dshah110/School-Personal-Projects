#include "hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int printHash(const char *lookup, struct sha1sum_ctx *ctx) {
	uint8_t checksum[20];

	int error = sha1sum_finish(ctx, (const uint8_t*)lookup, strlen(lookup), checksum);
	if (!error) {
		printf("%s ", lookup);
		for(size_t i = 0; i < 20; ++i) {
			printf("%02x", checksum[i]);
		}
		putchar('\n');
	}

	sha1sum_reset(ctx);

	return error;
}

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;

	struct sha1sum_ctx *ctx = sha1sum_create(NULL, 0);
	if (!ctx) {
		fprintf(stderr, "Error creating checksum\n");
		return 0;
	}

	printf("These should match the examples in the assignment spec:\n");

	printHash("Hello", ctx);
	printHash("World", ctx);

	sha1sum_destroy(ctx);

	return EXIT_SUCCESS;
}
