#ifndef _DEC_LZW_H_
#define _DEC_LZW_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define EMPTY      0xFFFFFFFF
#define BufferSize 1024*1024*2
#define NEW_TABLE  256
#define START      257
#define END        258

typedef struct TABLE    //�����ֵ�
{
	uint32_t precode;  //ǰ׺��
	uint32_t code;     //��ǰ�ַ�
	bool used;     //�Ƿ�ʹ��
} TABLE;

typedef struct decompress
{
	uint8_t   *InBuffer;       //���뻺����256
	uint32_t  InBuffer_dex;    //����������
	uint32_t  InBuffer_Maxdex; //���������ֵ
	uint32_t  tempcode;
	uint8_t   *OutBuffer;      //д���ļ�������256
	uint32_t  OutBuffer_dex;   //����������
	TABLE     *table;          //�ֵ�
	uint32_t  Count;
	uint32_t  CurTable;
}DE_COMPRESS;

void lzw_decompresss_ex(uint8_t *src, uint32_t in_len, uint8_t *dst, uint32_t *out_len);

#endif