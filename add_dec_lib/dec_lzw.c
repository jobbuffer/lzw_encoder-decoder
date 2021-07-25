#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dec_lzw.h"

void lzw_decompress_init(DE_COMPRESS *decompress)
{
	decompress->InBuffer_dex = BufferSize;
	decompress->InBuffer_Maxdex = BufferSize;
	decompress->InBuffer = (uint8_t*)malloc(sizeof(uint8_t)* BufferSize);
	decompress->OutBuffer = (uint8_t*)malloc(sizeof(uint8_t)* BufferSize);
	decompress->tempcode = EMPTY;
	decompress->OutBuffer_dex = 0;
	decompress->Count = 0;
	decompress->CurTable = 0;
	decompress->table = (TABLE*)malloc(sizeof(TABLE) * 4096);
	
	for (uint32_t i = 0; i < 4096; i++)
	{
		if (i <= END)
		{
			decompress->CurTable = i;
			decompress->table[i].code = i;
			decompress->table[i].used = true;
		}
		else
		{
			decompress->table[i].precode = EMPTY;
			decompress->table[i].code = EMPTY;
			decompress->table[i].used = false;
		}
	}
}

void lzw_decompress_destroy(DE_COMPRESS *decompress)
{
	if (decompress->InBuffer)  free(decompress->InBuffer);
	if (decompress->OutBuffer) free(decompress->OutBuffer);
	if (decompress->table)     free(decompress->table);
}

bool lzw_check_dictionary(DE_COMPRESS *decompress, uint32_t code)
{
	if (decompress->table[code].used) //zhao
		return true;
	return false;
}

uint32_t lzw_read_data(DE_COMPRESS *decompress, uint8_t *src, uint32_t in_len, uint32_t *in_pos)
{
	decompress->Count++;
	if (decompress->Count % 10000 == 0)
	{
		if (decompress->Count % 100000 == 0)
			decompress->Count = 0;
	}
	if (decompress->InBuffer_dex >= BufferSize || decompress->InBuffer_dex >= decompress->InBuffer_Maxdex)
	{
		memcpy(decompress->InBuffer, src, in_len);
		decompress->InBuffer_Maxdex = in_len;
		decompress->InBuffer_dex = 0;
	}
	if (decompress->tempcode == EMPTY)
	{
		uint32_t  a = decompress->InBuffer[decompress->InBuffer_dex++];
		uint32_t  b = decompress->InBuffer[decompress->InBuffer_dex++];
		uint32_t  c = decompress->InBuffer[decompress->InBuffer_dex++];
		decompress->tempcode = ((c & 0xf) << 8) | b;
		(*in_pos)++;
		return ((c & 0xf0) << 4) | a;
	}
	else
	{
		uint32_t temp = decompress->tempcode;
		decompress->tempcode = EMPTY;
		(*in_pos)++;
		return temp;
	}
}

bool lzw_check_file(DE_COMPRESS *decompress, uint8_t *src, uint32_t in_len, uint32_t *in_pos)
{
	if (!src)return false;
	if (START != lzw_read_data(decompress, src, in_len, in_pos))
		return false;
	return true;
}

bool lzw_write_char(DE_COMPRESS *decompress, uint32_t code)
{
	if (code > END)
	{
		lzw_write_char(decompress, decompress->table[code].precode);  //递归输出
		lzw_write_char(decompress, decompress->table[code].code);
	}
	else
	{
		decompress->OutBuffer[decompress->OutBuffer_dex++] = code & 0xff;  //写入一个字节
		if (decompress->OutBuffer_dex >= BufferSize)
		{
			decompress->OutBuffer_dex = 0;
			return false;
		}
	}
	return true;
}

void lzw_write_end(DE_COMPRESS *decompress)
{
	if (decompress->OutBuffer_dex != 0){}
	// need deal;
	//memcpy(decompress->OutBuffer,  decompress->OutBuffer_dex);
}

uint8_t lzw_get_firstchar(DE_COMPRESS *decompress, uint32_t code)
{
	uint32_t temp;
	if (code > END)
	{
		temp = lzw_get_firstchar(decompress, decompress->table[code].precode);
		return temp;
	}
	else
	{
		return code;
	}
}

void lzw_add_table(DE_COMPRESS *decompress, uint32_t ocode, uint32_t firstchar)
{
	decompress->CurTable++;
	decompress->table[decompress->CurTable].precode = ocode;
	decompress->table[decompress->CurTable].code = firstchar;
	decompress->table[decompress->CurTable].used = true;
}

void lzw_decompresss_ex(uint8_t *src, uint32_t in_len, uint8_t *dst, uint32_t *out_len)
{
	DE_COMPRESS decompress;
	uint32_t in_pos = 0;

	lzw_decompress_init(&decompress);
	if (false == lzw_check_file(&decompress, src, in_len, &in_pos))
		goto failed;

	uint32_t OCODE, NCODE;
	uint8_t FirstChar;
	OCODE = lzw_read_data(&decompress, src, in_len, &in_pos);
	lzw_write_char(&decompress, OCODE);
	
	while (in_pos < in_len)
	{
		NCODE = lzw_read_data(&decompress, src, in_len, &in_pos);
		if (NCODE == END)	 //文件结束标志	
			break;
		else if (NCODE == NEW_TABLE)  //读入新字典标志
		{
			OCODE = lzw_read_data(&decompress, src, in_len, &in_pos);
			lzw_write_char(&decompress, OCODE);
			lzw_decompress_init(&decompress);
			continue;
		}
		if (lzw_check_dictionary(&decompress, NCODE))  //在字典中存在
		{
			lzw_write_char(&decompress, OCODE);
			FirstChar = lzw_get_firstchar(&decompress, NCODE);//更新NCODE的第一个字符
		}
		else  //字典中不存在
		{
			lzw_write_char(&decompress, OCODE);
			lzw_write_char(&decompress, FirstChar);
			FirstChar = lzw_get_firstchar(&decompress, OCODE);
		}

		lzw_add_table(&decompress, OCODE, FirstChar);  //OCODE+CHAR字典
		OCODE = NCODE;
	}
	memcpy(dst, decompress.OutBuffer, decompress.OutBuffer_dex);
	*out_len = decompress.OutBuffer_dex;

failed:
	lzw_decompress_destroy(&decompress);
}