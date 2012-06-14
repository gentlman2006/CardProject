#include "stdafx.h"
#include <stdio.h>

#include "BHGX_MF_LXING.h"
#include "./dcrf32/dcrf32.h"
#include "debug.h"

#pragma  comment(lib, "./dcrf32/dcrf32.lib")

static HANDLE mGHandle;

#define NR_MASK(nr) (1 << nr)
#define NOT_NR_MASK(nr) ~(1 << nr)

#define SETBIT(byte, nr) byte |= NR_MASK(nr)
#define CLRBIT(byte, nr) byte &= NOT_NR_MASK(nr)

unsigned char GKeyB[6];


/**
*
*/
struct CreateKeyInfoS
{
	int offset;
	int length;
	unsigned char ID[9];
	unsigned char token[6];
	unsigned char key[16];
};


#define DEFAULT_CONTROL	0		//Ĭ������ KeyA�� KeyBд
#define KEYA_CONTROL	1		//KeyA��д
#define KEYB_CONTROL	2		//KeyB��д
int g_Control = DEFAULT_CONTROL;

int GetWriteControl(int nCtrol)
{
	int nWrite = 0;
	switch (nCtrol)
	{
	case DEFAULT_CONTROL:
	case KEYB_CONTROL:
		nWrite = 4;
		break;
	case KEYA_CONTROL:
	default:
		nWrite = 0;
		break;
	}
	g_Control = nCtrol;
	return nWrite;
}
/**
*
*
*/
static int SetBits(unsigned char *bits, const unsigned char *value, int offset, int len )
{
	int si, sr, di, dr;
	int i, j, k=0;
	for(i = 0; i < len ;) 
	{
		si = (offset + i) / 8;
		sr = (offset + i) % 8;

		dr = offset%8;
		di = (i+dr) / 8;
		for (j=sr; j<8 && k<len; ++j, ++k)
		{	
			if(value[di] & NR_MASK((7-sr)))
				SETBIT(bits[si], (7-j));
			else 
				CLRBIT(bits[si], (7-j));
			++sr;
			sr %= 8;
		}
		i = k;
	}
	return i;
}

/**
*
*
*/
static int GetBits(const unsigned char *bits, unsigned char *value, int offset, int len)
{
	int si, sr, di, dr;
	int i, j, k=0;
	for(i = 0; i < len ;) 
	{
		si = (offset + i) / 8;
		sr = (offset + i) % 8;

		di = i / 8;
		dr = i % 8;
		for (j=sr; j<8 && k<len; ++j, ++k)
		{	
			if(bits[si] & NR_MASK((7-j)))
				SETBIT(value[di], (7-j));
			else 
				CLRBIT(value[di], (7-j));
		}
		i = k;
	}
	return i;
}

/**
*
*/
static int str2bcd(const char *str, unsigned  char *bcd, int len)
{
	int i, j;

	for(i = 0; i < len; i++) 
	{		
		j = i >> 1;	

		if(i & 1) 
		{
			bcd[j] |= (str[i] - 48) << 4;
		}
		else
		{
			bcd[j] = str[i] - 48;
		}
	}

	return i;
}

/**
*
*/
static int bcd2str(const unsigned char *bcd, char *str, int len)
{	
	int i, j;

	for(i=0; i < len; i++) 
	{
		j = i >> 1;
		if(i & 1) 
		{
			str[i] = (bcd[j] >> 4) + 48;
		}	
		else 
		{
			str[i] = (bcd[j] & 0x0f) + 48;
		}
	}

	return i;
}


/**
*
*/
static void dbgmem(unsigned char *begin, int len)
{
	int i = 0;
	for(i=0; i<len; i++)
		printf("%02x ",begin[i]);

	printf("\n");

	return;
}


/**
*
*/
int __stdcall bProbe(void)
{
	int res = 0;

	for (int i=0; i<19; ++i)
	{
		mGHandle = dc_init(i, 115200);
		if((int)mGHandle > 0)
		{
			res = 1;
			break;
		}
	}

	if (res == 0)
	{
		mGHandle = dc_init(100, 115200);
		if((int)mGHandle > 0)
		{
			res = 1;
		}
	}
	return res;
}

/**
*
*/
int __stdcall iOpen(void)
{
	int res = 0;

	// ��ʼ���豸
	//mGHandle = dc_init(100, 115200);
	if(!mGHandle)
	{
		res = -1;

		goto out;
	}

	// �豸����ʱ�䳤��
	dc_beep(mGHandle, 10);

out:
	return res;
}

/**
*
*/
int __stdcall iClose(void)
{
	return dc_exit(mGHandle);
}

/**
*
*/
int __stdcall iScanCard(void)
{
	unsigned long cardid;

	if(0 == dc_card(mGHandle, 0, &cardid))
	{
		cardid = 0;
	}
	else
	{
		cardid = 1;
	}

	return cardid;
}

static void SetKey(struct CreateKeyInfoS *key)
{
	int SecNr, BlkNr, BitNr;
	unsigned char data[16];
	int secs = 0;
	int result = 0;

	//// �±�Ϊ������Ϣ
	//dbgmem(key->token, 6);
	//dbgmem(key->key, 16);

	/**
	* �±ߵĴ�����Bug������ʱ�����⣬�Ƚ����ͬһ�����ܹ������Ļ���
	*/
	// д��ؼ���Ϣ
	SecNr = (key->offset) >> 9;
	BlkNr = (key->offset)>> 7;
	BitNr = (key->offset) & 127;

	result = dc_load_key(mGHandle, 4, SecNr, key->token);
	result = dc_authentication(mGHandle, 4, SecNr);
	if(result)
		DBG(8, "[SetKey �ؼ���Ϣ]: Authentication=%d\n", result);

	memset(data, 0, 16);
	dc_read(mGHandle, BlkNr, data);
	SetBits(data, key->ID, BitNr, key->length);
	dc_write(mGHandle, BlkNr, data);

	//д��ÿ�������Ŀ����ֽ���
	for(secs = 0; secs < 16; secs++)
	{
		dbgmem(key->token, 6);
		result = dc_load_key(mGHandle, 4, secs, key->token);
		result = dc_authentication(mGHandle, 4, secs);
		if(result)
			DBG(8, "[SetKey ����]: %d Authentication=%d\n", secs,result);

		dc_write(mGHandle, (secs<<2 + 3), key->key);
	}
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
		break;
	case 1:
		break;
	case 2:
		SetKey((struct CreateKeyInfoS *)data);
		break;
	case 3:
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
	unsigned char data[16], *now = NULL;
	unsigned char SecNr, BlkNr, BitNr;
	int iReadDataInBits = 0;
	int iTotalReadDataInBits = 0;
	int result = 0;

	// ̽�⿨�����û�п����Զ��˳�
	if(iScanCard())
		return -1; 

	while(iSizeInBits != 0) {

		SecNr = iOffsetInBits >> 9;
		BlkNr = iOffsetInBits >> 7;
		BitNr = iOffsetInBits & 127;

		if((BitNr + iSizeInBits) >= 128) 
		{
			iReadDataInBits = 128 - BitNr;
		}
		else 
		{
			iReadDataInBits = iSizeInBits;
		}

		now = buf + iTotalReadDataInBits/8;

		// ��ȡ������֤
		result = dc_load_key(mGHandle, 0, SecNr, (unsigned char *)key);
		result = dc_authentication(mGHandle, 0, SecNr);
		if(result)
			DBG(8, "[Read]: Authentication=%d\n", result);

		// ��ȡһ��������
		if((BitNr==0) && (iReadDataInBits == 128))
		{
			dc_read(mGHandle, BlkNr, now);
		}
		// ��ȡ��������
		else 
		{
			memset(data, 0, 16);
			dc_read(mGHandle, BlkNr, data);
	/*		dbgmem(data, 16);*/
			GetBits(data, now, BitNr, iReadDataInBits);
			//SetBits(now, data, BitNr, iReadDataInBits);
		}

		// ������
		iSizeInBits -= iReadDataInBits;
		iOffsetInBits += iReadDataInBits;

		// �ۼ��Ѿ���ȡ������
		iTotalReadDataInBits += iReadDataInBits;
	}

	return result;
}



/**
*
*/
int __stdcall iWrite(const unsigned char *key, unsigned char *buf,
					 int nCtrlWord,int iSizeInBits, int iOffsetInBits)
{
	unsigned char data[16], *now = NULL;
	unsigned char SecNr, BlkNr, BitNr;
	int iWriteDataInBits = 0;
	int iTotalWriteDataInBits = 0;
	int result = 0;
	int nRealWrite = 0;

	// ̽�⿨,���û�п����Զ��˳�
	if(iScanCard())
	{
		return -1;
	}

	while(iSizeInBits != 0)
	{
		SecNr = iOffsetInBits >> 9;
		BlkNr = iOffsetInBits >> 7;
		BitNr = iOffsetInBits & 127; 

		if((BitNr + iSizeInBits) >= 128)
		{
			iWriteDataInBits = 128 - BitNr;
		}
		else 
		{
			iWriteDataInBits = iSizeInBits;
		}

		now = buf + iTotalWriteDataInBits/8;

		// д����֤
		result = dc_load_key(mGHandle, GetWriteControl(nCtrlWord), SecNr,(unsigned char *)key);
		result = dc_authentication(mGHandle, GetWriteControl(nCtrlWord), SecNr);
		if(result)
			DBG(8, "[Write]: Authentication = %d\n", result);

		// ����д��
		if((BitNr == 0) && (iWriteDataInBits == 128))
		{
			dc_write(mGHandle, BlkNr, now);
		}
		// ����д��
		else 
		{
			memset(data, 0, 16);
			dc_read(mGHandle, BlkNr, data);
			SetBits(data, now, BitNr, iWriteDataInBits);
			dc_write(mGHandle, BlkNr, data);
		}

		// ������
		iSizeInBits -= iWriteDataInBits;
		iOffsetInBits += iWriteDataInBits;

		// �ۼ��Ѿ�д�������
		iTotalWriteDataInBits += iWriteDataInBits;
	}

	return result;
}

//�޸��û�Ȩ�ޣ�����������keya �� keyb ��ֵ��
int  __stdcall iChangePwdEx(
							const unsigned char * pNewKeyA ,const unsigned char * ctrlword,
							const unsigned char * pNewKeyB,const unsigned char * poldPin,
							unsigned char nsector,unsigned char keyA1B0,unsigned char changeflag
							)
{
	unsigned char Key[16];
	unsigned char OldKey[16];
	unsigned char KeyB[6];
	unsigned char SecNr, BlkNr;
	short result = -1;
	memset(Key, 0, sizeof(Key));
	memset(KeyB, 0, sizeof(KeyB));

	// ̽�⿨,���û�п����Զ��˳�
	//if(iScanCard())
	//{
	//	return -1;
	//}

	int nWControl = GetWriteControl(g_Control);

	//�����ƿ�Ŀ�����
	SecNr = nsector;
	BlkNr= nsector*4+3;
	memcpy(KeyB, poldPin, 6);

	// ��ȡ������֤
	result = dc_load_key(mGHandle, nWControl, SecNr, KeyB);
	result = dc_authentication(mGHandle, nWControl, SecNr);
	if(result)
		DBG(8, "[Read]: Authentication=%d\n", result);

	dc_read(mGHandle, BlkNr, OldKey);

	switch(changeflag)
	{
	case 0://ֵΪ0��ʾֻ��A����
		{
			memcpy(Key, pNewKeyA, 6);
			memcpy(Key+6, OldKey+6, 4);
			memcpy(Key+10, KeyB, 6);
			//memcpy(KeyB, OldKey+10, 6);
			result = dc_load_key(mGHandle, nWControl, SecNr, KeyB);
			result = dc_authentication(mGHandle, nWControl, SecNr);
			if(result)
				DBG(8, "[Write]: Authentication=%d\n", result);
			return dc_write(mGHandle, BlkNr, Key);
		}
		break;
	case 2://ֵΪ2��ʾ����Ȩ�޷����ֲ����ģ�ֻ��A��B����
		{
			memcpy(Key, pNewKeyA, 6);
			memcpy(Key+6, OldKey+6, 4);
			memcpy(Key+10, pNewKeyB, 6);
			//memcpy(KeyB, OldKey+10, 6);
			result = dc_load_key(mGHandle, nWControl, SecNr, KeyB);
			result = dc_authentication(mGHandle, nWControl, SecNr);
			if(result)
				DBG(8, "[Write]: Authentication=%d\n", result);
			return dc_write(mGHandle, BlkNr, Key);
		}
		break;
	case 3:
		break;
	default:
		return -1;
	}
	return 0;
}
