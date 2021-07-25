#include <string.h>
#include "lzw.h"

void lzw_init(COMPRESS *lzw, uint8_t *src, uint32_t src_len)
{
	lzw->Count = 0;
	lzw->_con = 0;
	lzw->OutBuffer_dex = 0;
	lzw->tempcode = EMPTY;
	lzw->InBuffer = (uint8_t*)malloc(sizeof(uint8_t) * BufferSize);
	lzw->OutBuffer = (uint8_t*)malloc(sizeof(uint8_t) * BufferSize);
	lzw->InBuffer_dex = lzw->InBuffer_Maxdex = BufferSize;
	lzw->table = (TABLE*)malloc(sizeof(TABLE) * 4096);

	memcpy(lzw->InBuffer, src, src_len);
	for (uint32_t i = 0; i < 4096; i++)
	{
		if (i <= END)
		{
			lzw->table[i].code = i;
			lzw->table[i].used = true;
		}
		else
		{
			lzw->table[i].precode = EMPTY;
			lzw->table[i].code = EMPTY;
			lzw->table[i].used = false;
		}
	}
}

uint8_t lzw_readchar(COMPRESS *lzw, uint32_t *in_pos)
{
	lzw->Count++;
	if (lzw->Count % 10000 == 0)
	{
		if (lzw->Count % 100000 == 0)
		{
			lzw->Count = 0;
		}
	}
	if (lzw->InBuffer_dex >= BufferSize || lzw->InBuffer_dex >= lzw->InBuffer_Maxdex)
	{
		lzw->InBuffer_dex = 0;
	}
	return lzw->InBuffer[(*in_pos)++];
}

uint32_t lzw_checktable(COMPRESS *lzw, uint32_t pre_code, uint32_t code)
{
	for (uint32_t i = END + 1; i < 4096; i++)
	{
		if (lzw->table[i].used == true)
		{
			if (lzw->table[i].precode == pre_code && lzw->table[i].code == code)
				return i;
		}
		else   //加入到字典中
		{
			if (i >= 4095)
			{
				lzw->_con++;
				return NEW_TABLE;
			}
			else
			{
				lzw->table[i].precode = pre_code;
				lzw->table[i].code = code;
				lzw->table[i].used = true;
				return EMPTY;
			}
		}
	}
	return EMPTY;
}
bool lzw_write_char(COMPRESS *lzw, uint32_t code)
{
	if (lzw->tempcode == EMPTY)
	{
		lzw->OutBuffer[lzw->OutBuffer_dex++] = code & 0xff;  //写入低8位
		lzw->tempcode = (code >> 4) & 0xF0;
	}
	else
	{
		lzw->OutBuffer[lzw->OutBuffer_dex++] = code & 0xff;
		lzw->OutBuffer[lzw->OutBuffer_dex++] = (lzw->tempcode | ((code >> 8) & 0xf)) & 0xff;
		lzw->tempcode = EMPTY;
	}

	if (lzw->OutBuffer_dex >= BufferSize)
	{
		memcpy(lzw->OutBuffer, lzw->OutBuffer, lzw->OutBuffer_dex);
		lzw->OutBuffer_dex = 0;
	}
	return true;
}

void lzw_write_end(COMPRESS *lzw)
{
	if (lzw->tempcode != EMPTY)
	{
		lzw_write_char(lzw, END);
	}
	else
	{
		lzw_write_char(lzw, END);
		lzw_write_char(lzw, END);
	}
}

void lzw_destory(COMPRESS *lzw)
{
	if (lzw->InBuffer) free(lzw->InBuffer);
	if (lzw->OutBuffer) free(lzw->OutBuffer);
	if (lzw->table) free(lzw->table);
}

void lzw_compress_ex(uint8_t *in_buffer, uint32_t in_len, uint8_t *dst, uint32_t *out_len)
{
	COMPRESS lzw_compress;
	uint32_t in_pos = 0;

	lzw_init(&lzw_compress, in_buffer, in_len);
	lzw_write_char(&lzw_compress, START);
	uint32_t  pre_code = lzw_readchar(&lzw_compress, &in_pos);

	while (in_pos < in_len)
	{
		uint32_t code = lzw_readchar(&lzw_compress, &in_pos);        //读入新字符
		uint32_t temp = lzw_checktable(&lzw_compress, pre_code, code);//查字典
		if (temp == EMPTY)
		{
			lzw_write_char(&lzw_compress, pre_code);
			pre_code = code;        //更新前缀
		}
		else if (temp == NEW_TABLE) //字典已满，重新初始化字典
		{
			lzw_write_char(&lzw_compress, pre_code);
			lzw_write_char(&lzw_compress, NEW_TABLE);
			lzw_init(&lzw_compress, in_buffer, in_len);
			pre_code = code;
		}
		else
		{
			pre_code = temp;
		}
	}

	lzw_write_char(&lzw_compress, pre_code);
	lzw_write_end(&lzw_compress);

	if (dst)
	{
		memcpy(dst, lzw_compress.OutBuffer, lzw_compress.OutBuffer_dex);
		*out_len = lzw_compress.OutBuffer_dex;
	}

	lzw_destory(&lzw_compress);
}