#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "lzw.h"

#define IN  "test_lzw.txt"
#define OUT "test_enc.lzw"
#define BUFFER_SIZE 2048

#pragma warning(disable:4996)

COMPRESS lzw_compress;


int main()
{
	uint8_t *in_buffer = NULL;
	uint8_t *out_buffer = NULL;
	FILE *fp = NULL;

	fp = fopen(IN, "rb");
	if (!fp) return -1;
	uint32_t in_pos = 0;

	in_buffer = (uint8_t*)malloc(sizeof(uint8_t) * BUFFER_SIZE);
	out_buffer = (uint8_t*)malloc(sizeof(uint8_t) * BUFFER_SIZE);

	fseek(fp, 0L, SEEK_END);
	uint32_t in_len = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	uint32_t read_len = fread(in_buffer, 1, in_len, fp);
	fclose(fp);

	int out_pos = 0;

	lzw_compress_ex(in_buffer, read_len, out_buffer, &out_pos);

	FILE *out = NULL;
	out = fopen("enc_lzw.lzw", "wb");
	fwrite(out_buffer, 1, out_pos, out);
	fclose(out);

	system("pause");
	return 0;
}