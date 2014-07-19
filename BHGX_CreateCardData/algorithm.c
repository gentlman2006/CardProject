
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "algorithm.h"

#pragma warning (disable : 4996)
/**
 *
 */
void DbgMem(unsigned char *mem, int len)
{
	int i = 0;
	for(i=0; i<len; i++)
		printf("%02x ",mem[i]);

	printf("\n");
	
	return;
}

/**
 *
 */
int Str2Bcd(const char *str, unsigned  char *bcd, int *len)
{
	int slen;
	int i, j;

	// ���������ֵ
	if((str == NULL) || (bcd == NULL) || (len == 0))
		return -1;
	       
	// У�鳤������
	slen = *len;
	slen -= (slen%2);
	if(slen == 0)
		return -1;

	for(i=0; i < slen; i++)
	{
		if(!((str[i]>='0') && (str[i]<='9')))
			return -1;
	}

	for(i=0, j=0; i<slen/2; i++,j+=2)
	{
		bcd[i] = ((str[j] - '0') << 4) | (str[j+1] - '0');
	}

	//���س���ֵ
	if(len)
		 *len=slen/2;

	 return 0;
}

/**
 *
 */
int Bcd2Str(char *str, const unsigned char *bcd, int len)
{	
	int i,j;

	// ��֤�������ݵ���ȷ��
	if((str==NULL) || (bcd==NULL) || (len==0))
		return -1;

	for(i=0,j=0; i<len; i++,j+=2)
	{
		str[j]= (bcd[i]>>4) > 9 ? (bcd[i]>>4)-10+'A' : (bcd[i]>>4)+'0';
		str[j+1]=(bcd[i] & 0x0F) >9 ? (bcd[i] & 0x0F)-10+'A' : (bcd[i] & 0x0F)+'0';
	}
	str[j]='\0';

	return 0;
}

/**
 *
 */
static int IsNumber(char c)
{
	return (c >= '0') && (c <= '9');
}

/**
 *
 */
static int Number(char c) 
{
	return c - '0';
}

/**
 *
 */
static int ShiftLeft(char *string, int count)
{
	int index = 0;
	char c = string[index];
	for(index = 1; index  < (int)strlen(string); index ++)
	{
		string[index -1] = string[index];
	}

	string[index -1] = c;

	return 0;
}

/**
 *
 */
static int ShiftExchange(char *string, int block)
{
	int len = (int)strlen(string);
	int index = 0;
	char tmp;

	len -= 1;
	tmp = string[block -1];
	string[block -1] = string[len - block + 1];
	string[len-block +1] = tmp;

	return 0;
}

/**
 *
 */
static int ConvertKeyB(const char *seed, char *keyB)
{
	char *string = NULL;
	int length = 0;
	int index = 0;
	int round = 0;

	string = _strdup(seed);
	length = (int)strlen(string);

	// 1:ѭ���ۼ�
	for(index =0; index < length; index ++)
	{
		if(IsNumber(string[index]))
			round += Number(string[index]);
	}
	
	// 2:ѭ������
	for(index = 0; index < round; index ++)
	{
		ShiftLeft(string, index);
	}

	// 3: Ӣ���ַ�26��ĸ�ԳƷ���ԭ�������ַ�9����������ԭ
	for(index = 0; index < length; index ++)
	{
		if(IsNumber(string[index]))
		{	
			string[index] = '9' - string[index] + '0';
		}
		else
		{
			string[index] = 219 - string[index];
		}
	}

	// ������λ
	for(index = 1; index <= (length/2); index ++)
	{
		if(index%2)
		{
			ShiftExchange(string,index);
		}
	}

	// // ȡǰ12λ��ΪKEYB
	memset(keyB, 0, 16);
	memcpy(keyB, string, 12);
	free(string);

	return 0;
}

/**
 *
 */
//static int str2bcd(const char *str, unsigned  char *bcd, int len)
//{
//	int i, j;
//	
//	for(i = 0; i < len; i++) 
//	{		
//		j = i >> 1;	
//		
//		if(i & 1) 
//		{
//			bcd[j] = str[i] - 48;
//			
//		}
//		else
//		{
//			bcd[j] |= (str[i] - 48) << 4;
//		}
//	}
//	
//	return i;
//}

int iGetKeyBySeed(const unsigned char *_seed, unsigned char *key)
{
	unsigned char seed[18];
	unsigned char tkey[16];
	int length;

	// ׼������
	memcpy(seed, _seed, 18);
	memset(tkey, 0, 16);

	// ת������
	
	ConvertKeyB(_seed, tkey);

	// �±�Ϊ���Դ���
	printf("[iGetKeyBySeed tkey]:");
	printf("%s\n", tkey);
	//DBG(0, "[iGetKeyBySeed tkey]:%s", tkey);

	//�������
	length = 12;
	Str2Bcd(tkey, key, &length);

	return 0;
}