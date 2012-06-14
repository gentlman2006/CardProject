
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <windows.h>

#include "vcard.h"

#define DEFAULT_FILENAME  "vcard.data"
#define VCARD_MAX_SIZE 8*1024

static int GOffset = 0;
static FILE *Handle = NULL;


/**
 * �ƿ���Ҫ�Ĺؼ���Ϣ
 */
struct CreateKeyInfoS
{
	int offset;
	int length;
	unsigned char ID[9];
	unsigned char token[6];
	unsigned char key[16];
};


/**
 *
 */
static void DbgMem(unsigned char *mem, int size)
{
	int i = 0;
	for(i=0; i< size; i++)
		printf("%02x ", mem[i]);
	printf("\n");
}

/**
 * �����ļ�̽���ļ�������ʼ�շ���Ϊ1
 */
int __stdcall bProbe(void)
{
	/* ����̽���� */
	int res = 1;

	return res;
}

/**
 *
 */
static FILE * iOpenFile(const char *filename)
{
	FILE *res = NULL;

	//�������ļ�������
	if(_access(filename, 6))
	{
		fopen_s(&res, filename, "ab+");
		fclose(res);
	}

	// ���ļ�
	fopen_s(&res, filename, "rb+");
	if(res != NULL)
	{
		rewind(res);
	}

	return res;
}

/**
 *
 */
int __stdcall iOpen(void)
{
	//�Ѵ��ļ��������

	return 0;
}

/**
 *
 */
int __stdcall iClose(void)
{
	if(Handle)
		fclose(Handle);

	Handle = 0;

	return 0;
}

/**
 *
 */
int __stdcall iScanCard(void)
{
	// �����⿨���û��ʵ��Ѱ������
	return 0;
}

/**
 * ģ�⻻������
 */
static void ChangeCard(void)
{
	GOffset += 1024;

	return;
}

/**
 * ��ʼ��������
 */
static void InitCard(void)
{
	unsigned char buf[1024];

	// ��Buf�г�ʼ��Ϊoxff
	memset(buf, 0xff, 1024);

	rewind(Handle);
	fseek(Handle, GOffset, SEEK_SET);
	fwrite(buf, 1024, 1, Handle);
}



/**
 *
 */
static void SetKey(struct CreateKeyInfoS *KeyInfo)
{
	int secs = 0;

	//printf("[VCard]:");
	//DbgMem(KeyInfo->ID, KeyInfo->length>>3);
	printf("Good:");
	DbgMem(KeyInfo->key, 16);
	
	//д��ÿ�������Ŀ����ֽ���
	for(secs = 0; secs < 16; secs++)
	{
		rewind(Handle);
		fseek(Handle, GOffset + secs*64 + 48, SEEK_SET);
		fwrite(KeyInfo->key, 16, 1, Handle);
	}
}


/**
 *
 */
static int ChangeVirtualFile(char *filename)
{
	//����Ѿ����ļ�����ر��ļ����
	if(Handle != NULL)
	{
		fclose(Handle);
	}

	Handle = iOpenFile(filename);
	GOffset = 0;

	return 0;
}


/**
 *
 */
int __stdcall iIOCtl(int cmd, void *data)
{
	int result = 0;


	switch(cmd)
	{
	case 0:
		ChangeCard();
		break;

	case 1:
		InitCard();
		break;

	case 2:
		SetKey((struct CreateKeyInfoS *)data);
		break;

	case 3:
		ChangeVirtualFile((char *)data);
		break;

	default:
		break;
	}

	return result;
}



/**
 *
 */
int __stdcall iRead(const unsigned char *key, unsigned char *buf, int iSizeInBits, int iOffsetInBits)
{
	int result;

	//����ļ�������
	if(Handle == NULL)
		ChangeVirtualFile(DEFAULT_FILENAME);

	// ���ļ����õ���ͷ
	rewind(Handle);

	result = fseek(Handle, iOffsetInBits/8 + GOffset, SEEK_SET);
	result = fread(buf,  1, iSizeInBits/8,Handle);

	DbgMem(buf, iSizeInBits/8);

	return 0;
}

/**
 *
 */
int __stdcall iWrite(const unsigned char *key, unsigned char *buf, int iSizeInBits, int iOffsetInBits)
{

	// ����ļ�������
	if(Handle == NULL)
		ChangeVirtualFile(DEFAULT_FILENAME);  

	// ���ļ����õ���ͷ
	rewind(Handle);

	fseek(Handle, iOffsetInBits/8 + GOffset, SEEK_SET);
	fwrite(buf, 1,iSizeInBits/8,Handle);
	if (iSizeInBits/4 %2 == 1)
	{
		fwrite(buf + iSizeInBits/8,1,1,Handle);
	}

	return 0;
}