#ifndef __LZW_H__
#define __LZW_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#define UINT unsigned int
#define EMPTY 0xFFFFFFFF 
#define BufferSize 300
#define NEW_TABLE 256
#define START 257
#define END 258

typedef struct TABLE   //�����ֵ�
{
	uint32_t precode;  //ǰ׺��
	uint32_t code;     //��ǰ�ַ�
	bool used;         //�Ƿ�ʹ��
} TABLE;

typedef struct compress
{
	uint8_t  *InBuffer;
	uint32_t InBuffer_dex;
	uint32_t InBuffer_Maxdex; //���������ֵ
	uint32_t tempcode;
	uint8_t  *OutBuffer;      //д���ļ�������256
	uint32_t OutBuffer_dex;   //����������
	TABLE    *table;          //�ֵ�
	uint32_t table_dex;
	uint32_t Count;
	uint32_t _con;
}COMPRESS;

void lzw_compress_ex(uint8_t *in_buffer, uint32_t in_len, uint8_t *dst, uint32_t *out_len);

#endif