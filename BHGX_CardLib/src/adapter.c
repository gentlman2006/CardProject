#include <stdio.h>
#include <string.h>
#include "adapter.h"
#include "device.h"
#include "liberr.h"
#include "algorithm.h"
#include "debug.h"
#include <windows.h>
#include <time.h>
#include <stdio.h>
#pragma warning (disable : 4020)
#pragma warning (disable : 4996)


#define DEFAULT_CONTROL	0		//Ĭ������ KeyA�� KeyBд
#define KEYA_CONTROL	1		//KeyA��д
#define KEYB_CONTROL	2		//KeyB��д
char atmp[64];
time_t at;
#define DBGADAP(format, ...) \
	at = time(0);\
	memset(atmp,0, sizeof(atmp));\
	strftime( atmp, sizeof(atmp), "%Y-%m-%d %X  adapter: ",localtime(&at));\
	LogMessage(atmp ,__VA_ARGS__);		\
	LogMessage(format ,__VA_ARGS__);		\

/**
 * @ID 
 * @MASK 
 * @TYPE 
 * @CHECK 
 * @SOURCE 
 * @TARGET
 * @DEFAULT 
 * @ISWRITE 
 * @OFFSET 
 * @COLUMNBIT 
 * @INULLABLE= 
 * @WRITEBACK
 */
struct XmlColumnS
{
	int		ID;
	char	Source[50];
	char	Target[50];
	char	Value[100];
	char	Mask		:1;
	char	Type		:3;
	char	Check		:1;
	char	IsWrite		:1;
	char	INullLable	:1;
	char	WriteBack	:1;

	int		Offset;
	int		ColumnBit;

	struct XmlColumnS	*Next;
};

/**
 * @ID
 * @TARGET
 */
struct XmlSegmentS
{
	int		ID;
	char	Target[50];

	struct XmlColumnS	*Column;
	struct XmlSegmentS	*Next;
};

/**
 *@ID 
 *@TARGET
 */
struct XmlProgramS
{
	int		ID;
	char	Target[50];

	struct XmlSegmentS		*Segment;
	struct XmlProgramS		*Next;
};

static struct CardDevice *Instance = NULL;

//������Ϊ1  ����Ϊ0 
int IsGWCard(char *cardID)
{
	if (cardID == NULL)
		return 0;

	return cardID[0]=='1'? 1:0;
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
			//bcd[j] |= (str[i] - 48) << 4;
			bcd[j] |= (str[i-1] - 48) << 4;
		}
		else
		{
			//bcd[j] = str[i] - 48;
			bcd[j]=str[i+1] -48;
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
			//str[i] = (bcd[j] >> 4) + 48;
			str[i] |= (bcd[j] & 0x0f) + 48;
		}	
		else 
		{
			//str[i] = (bcd[j] & 0x0f) + 48;
			str[i] = (bcd[j] >> 4) + 48;
		}
	}

	return i;
}

/**
 * ������ initCoreDevice
 * ������
 *
 * ����ֵ��
 * �ɹ��� 0
 * ʧ�ܣ�����
 */
int __stdcall initCoreDevice(const char *System)
{
	if(!Instance) 
		Instance = getCardDevice(System);

	return (Instance == NULL?-1:0);
}

/**
 * ������closeCoreDevice
 * ������
 *
 * ����ֵ��
 * �ɹ��� 0
 * ʧ�ܣ� ����
 */
int __stdcall closeCoreDevice(void)
{
	int ret = 0;

	ret = putCardDevice(Instance);
	if(!ret) 
		Instance = NULL;

	return ret;
}

/**
 * ������iFindCard
 * ������
 *
 * ����ֵ��
 * �ɹ��� ����
 * ʧ�ܣ�0
 */
int __stdcall iCoreFindCard(void)
{

	// ̽�⿨�����û�п����Զ��˳�
	unsigned char ret = (unsigned char)Instance->iScanCard();
	printf("ScanCard: %d\n", ret);
	if(ret != 0)
		return CardScanErr; 
	return 0;
}
/**
 *
 */
int __stdcall iCtlCard(int cmd, void *data)
{
	// ��ʼ�����Ϊʧ��
	unsigned char result = 0;

	// �����������Щ����
	if(Instance && Instance->iIOCtl) 
	{
		result = (unsigned char)Instance->iIOCtl(cmd, data);
	}

	return result;
}

/**
 *
 */
struct RWRequestS* __stdcall CreateRWRequest(struct XmlSegmentS *listHead, int mode)
{
	struct XmlSegmentS	*SegmentElement = NULL;
	struct XmlColumnS	*ColumnElement = NULL;

	struct RWRequestS	*CurrRequest = NULL;
	struct RWRequestS	*TempRequest = NULL;
	struct RWRequestS	*result = NULL;

	SegmentElement = listHead;
	while(SegmentElement)
	{
		ColumnElement = SegmentElement->Column;
		while(ColumnElement)
		{
			// �����µĽڵ�
			TempRequest = (struct RWRequestS *)malloc(sizeof(struct RWRequestS));
			memset(TempRequest, 0, sizeof(struct RWRequestS));
			
			// �Խڵ���и�ֵ
			TempRequest->mode = mode;
			TempRequest->offset = ColumnElement->Offset;
			TempRequest->length = ColumnElement->ColumnBit;
			TempRequest->value = ColumnElement->Value;
			TempRequest->pri = (void *) ColumnElement;

			// ��������
			if(result)
			{
				CurrRequest->Next = TempRequest;
				CurrRequest = TempRequest;
			}
			else 
			{
				CurrRequest = TempRequest;
				result = CurrRequest;
			}

			// ������
			ColumnElement = ColumnElement->Next;
		}

		// ������
		SegmentElement = SegmentElement->Next;
	}


	return result;
}


/**
 *
 */
void __stdcall DestroyRWRequest(struct RWRequestS *list, int flag)
{
	struct RWRequestS	*CurrRequest = NULL;
	struct RWRequestS	*TempRequest = NULL;
	
	CurrRequest = list;
	while(CurrRequest)
	{
		TempRequest = CurrRequest;
		CurrRequest = CurrRequest->Next;
	
		if(flag)
		{
			free(TempRequest->value);
		}
		free(TempRequest);
	}

	return;
}

//��ȡд���ƣ� ��ʼ���Ŀ���λʱ����1���������Ϊ0
int __stdcall GetWriteWord(const unsigned char *pControl)
{
	unsigned char pInitControl[6] = {0xff, 0x07, 0x80, 0x69};
	int nRet = 0;
	int i = 0;

	for (; i<4; ++i)
	{
		if (pInitControl[i] != pControl[i])
		{
			return DEFAULT_CONTROL;
		}
	}
	return KEYA_CONTROL;
}

int __stdcall aGetControlBuff(unsigned char *pControl, int nSecr)
{
	//439704472047
	unsigned char keyA[] = {0x43, 0x97, 0x04, 0x47, 0x20, 0x47};
	unsigned char bRead = 0;
	int BlkNr = 0;

	/* ���û�ж����豸����*/
	if(!Instance) 
		return CardInitErr;

	if (iCoreFindCard() != 0)
		return CardScanErr;

	BlkNr = nSecr * 4 + 3;
	bRead = Instance->iRead(keyA, pControl, 4*8, BlkNr * 128 + 6*8);
	if (bRead != 0)
	{
		if (iCoreFindCard() != 0)
			return CardScanErr;

		memset(keyA, 0xff, sizeof(keyA));
		bRead = Instance->iRead(keyA, pControl, 4*8, BlkNr * 128 + 6*8);
	}
	return bRead;
}
/**

 *
 */
static struct RWRequestS  *_CreateReadList(struct RWRequestS *TmpReadRequest)
{
	struct RWRequestS *tmp = NULL, *current = NULL;
	struct RWRequestS *NCurrent;
	struct RWRequestS *ReadList = NULL;

	current = TmpReadRequest;
	while(current)
	{
		tmp = (struct RWRequestS *)malloc(sizeof(struct RWRequestS));
		memset(tmp, 0, sizeof(struct RWRequestS));

		tmp->offset = current->offset;
		tmp->length = current->length;
		
		//��������
		if(ReadList)
		{
			NCurrent ->Next = tmp;
			NCurrent = tmp;
		}
		else 
		{
			NCurrent = tmp;
			ReadList = NCurrent;
		}

		tmp = current->Next;
		while(tmp && (tmp->offset == (current->offset + current->length)))
		{
			// �����������ж�д�Ĵ���
			current->agent = NCurrent;
			NCurrent->length += tmp->length;

			current = current->Next ;
			tmp = current->Next;
		}
		
		current->agent = NCurrent;
		current = current->Next;
	}

	// �����ڴ�
	current = ReadList;
	while(current)
	{
		current->value = (unsigned char *) malloc((current->length + 7)/8);

		current = current->Next;
	}

	return ReadList;
}

/**
 *
 */
#define ID_OFFSET	1280
static void ListParseContent(struct RWRequestS *list)
{
	struct XmlColumnS *ColumnElement = NULL;
	struct RWRequestS *CurrRequest = NULL;
	struct RWRequestS *Agent = NULL;
	unsigned char *bcd = NULL;
	int nOffSet = 0;

	CurrRequest = list;
	while(CurrRequest)
	{
		ColumnElement = (struct XmlColumnS *)CurrRequest->pri;
		Agent = CurrRequest->agent;
		bcd = (Agent->value) + (CurrRequest->offset -Agent->offset)/8;
		memset(CurrRequest->value, 0, 100);
		// ����Ϊ�յĴ���: ���Ϊ0xff,�����������
		if(bcd[0] == 0xff)
		{
			goto Next;
		}

		nOffSet = (CurrRequest->offset -Agent->offset)/4%2;

		//add by yanggx	20110619
		////BCDת�����ַ��� �����⣬ ������4λ��BCD������˵���.
		if(CurrRequest->length == 4)
		{
			if (nOffSet == 0)
			{
				nOffSet = CurrRequest->offset/4%2;
			}
			Bcd2Ch(CurrRequest->value, bcd, nOffSet);	
		}
		else
		{
			//add by yanggx 20110706
			Bcd2StrOffSet(CurrRequest->value, (char *)bcd, (CurrRequest->length)/4, nOffSet);
			if (CurrRequest->offset == ID_OFFSET)
			{
				if (CurrRequest->value[(CurrRequest->length)/4-1] == 'D'
					|| CurrRequest->value[(CurrRequest->length)/4-1] == 'd')
				{
					CurrRequest->value[(CurrRequest->length)/4-1] = 'x';

				}else if (CurrRequest->value[(CurrRequest->length)/4-1] == 'e'
					|| CurrRequest->value[(CurrRequest->length)/4-1] == 'E')
				{
					CurrRequest->value[(CurrRequest->length)/4-1] = 'X';
				}

			}
		}

		// 256��Ϊ��ʶ�����֣����ִ����е�СС�Ĳ�ͬ

		if(CurrRequest->length == 256 || CurrRequest->length == 128) {
			CleanName((char *)CurrRequest->value);
		}
		else {
			iCleanStr((char *)CurrRequest->value);
		}
		Next:
		// ������
		CurrRequest = CurrRequest->Next;
	}
	return;
}

/**
 *
 */
static int _iReadCard(struct RWRequestS *list)
{
	//439704472047
	unsigned char keyA[] = {0x43, 0x97, 0x04, 0x47, 0x20, 0x47};
	unsigned char bRead = 0;

	struct RWRequestS *CurrRequest= NULL;
	struct ReadRequest *CurrList = NULL;
	
	/* ���û�ж����豸����*/
	if(!Instance) 
		return CardInitErr;

	CurrRequest = list;
	while(CurrRequest)
	{
		memset(CurrRequest->value, 0, (CurrRequest->length + 7)/8);
		bRead = Instance->iRead(keyA, CurrRequest->value, CurrRequest->length, CurrRequest->offset);

		// ������
		CurrRequest = CurrRequest->Next;
	}

	return bRead==0 ? 0:CardReadErr;
}

/**
 *
 */
int __stdcall iReadCard(struct RWRequestS *list)
{
	struct RWRequestS *AgentList = NULL;
	int res;

	// �������������

	// ������ʵ�Ķ�д����
	AgentList = _CreateReadList(list);

	// ��ʵ�����豸��д
	res = _iReadCard(AgentList);

	// �ⲿ�б���н���
	ListParseContent(list);

	// ɾ����д�б�
	DestroyRWRequest(AgentList, 1);;

	return res;
}


/**
 *
 */

static void ParseWriteContent(struct RWRequestS *list)
{
	struct XmlColumnS *ColumnElement = NULL;
	struct RWRequestS *CurrRequest = NULL;
	struct RWRequestS *Agent = NULL;
	unsigned char *bcd = NULL;
	int len = 0;
	int i=0;
	int nPos = 0, nBitPos = 0;
	int nByteLen = 0;
	int nAgentReLen = 0;

	CurrRequest = list;
	while(CurrRequest)
	{
		ColumnElement = (struct XmlColumnS *)CurrRequest->pri;
		Agent = CurrRequest->agent;

		// ����Ϊ�յĴ���,����Ĵ���
		if(!strlen(CurrRequest->value))
		{
			memset(CurrRequest->value, 0xff, (CurrRequest->length)/8);
		}

		//comment by yanggx	110612  ���ַ���ȡ����ÿ��Agent������ַ���¼����
		//Agent��һ��
		nByteLen = CurrRequest->length/4;
		if ((CurrRequest->offset - Agent->offset) == 0)
		{
			bcd = Agent->value;

			//add by yanggx 20110616 �жϿ�ʼλ�Ƿ�������
			nBitPos = CurrRequest->offset/4%2;
			if(nBitPos == 1)
			{
				memset(bcd, 0xff, 1);
			}
			nAgentReLen = nByteLen + nBitPos;
		}
		else
		{
			bcd = Agent->value + nAgentReLen/2;
			nAgentReLen += nByteLen;
		}

		// ����ĳ���Ӧ�����ַ����ĳ��ȣ���ӦΪ�ռ�ĳ��� 
		len = (int)strlen(CurrRequest->value);
		nPos = (nAgentReLen - nByteLen)%2;

		for(i=0; i<len; ++i)
		{
			//���ַ�תΪBCD���룬������bcd��
			StrCh2Bcd(CurrRequest->value[i], bcd+(i+nPos)/2, (i+nPos)%2);
		}

		//add by yanggx 20110615
		//����ַ�����û���ֽڳ��ȵ�4���������0xf
		nByteLen = CurrRequest->length/4;
		if (nByteLen > len)
		{
			for(; i<nByteLen; ++i)
			{
				StrCh2Bcd('f', bcd+(i+nPos)/2, (i+nPos)%2);
			}
		}
		CurrRequest = CurrRequest->Next;
	}

	return;
}

/**
 *
 */
static int iGetKeySeed(unsigned char *seed)
{
	unsigned char keyA[] = {0x43, 0x97, 0x04, 0x47, 0x20, 0x47};
	unsigned char tmp[32];

	//û��Ѱ����
	if(!Instance || !Instance->iRead)
		return -1;

	//��ȡseed
	memset(tmp, 0, 32);
	Instance->iRead(keyA, tmp, 56, 640);
	if (tmp[0]>>4 == 0)
	{
		memset(tmp, 0, sizeof(tmp));
		Instance->iRead(keyA, tmp, 72, 792);
		bcd2str(tmp, seed, 18);
	}
	else
	{
		bcd2str(tmp, seed, 14);
	}
	
	return 0;
}

int  __stdcall repairKeyForFault(unsigned char *ctrlword)
{
	unsigned char seed[32];
	unsigned char tmp[32];
	unsigned char oldKeyB[6];  //���ԭ����KeyB
	unsigned char curKeyB[0x6];//��ǰ�����keyB
	unsigned char NHKyeB[6];
	unsigned char KeyA[6] = {0x43, 0x97, 0x04, 0x47, 0x20, 0x47};
	int i=0, nRet = 0;
	unsigned char changeflag=2;
	memset(oldKeyB, 0, sizeof(oldKeyB));
	memset(seed, 0, sizeof(seed));
	memset(tmp, 0, sizeof(tmp));
	memset(curKeyB, 0x75, sizeof(curKeyB));
	memset(NHKyeB, 0, sizeof(NHKyeB));

	//û��Ѱ����
	if(!Instance || !Instance->iRead)
		return -1;

	if (iCoreFindCard() != 0)
		return CardScanErr;

	//��������,�ó��ɵ�KeyB
	Instance->iRead(KeyA, tmp, 56, 640);
	bcd2str(tmp, seed, 14);
	iGetKeyBySeed(seed, oldKeyB);

	//�����ũ�Ͽ���ֱ��ʧ��
	if (!IsGWCard(seed)) {
		printf("�˿�Ϊũ�Ͽ���ֱ��ʧ��\n");
		return -1;
	}

	//�õ���ũ�Ϻ�Ϊ���ݵ�keyB
	memset(seed, 0, sizeof(seed));
	memset(tmp, 0, sizeof(tmp));
	Instance->iRead(KeyA, tmp, 72, 792);
	bcd2str(tmp, seed, 18);
	iGetKeyBySeed(seed, NHKyeB);

	printf("[6-4�½������޲�����]:");
	for (i=0; i<16; ++i)
	{
		nRet = aChangePwdEx(KeyA, ctrlword, oldKeyB, curKeyB, i, 0, changeflag);
		if (nRet) {
			if (iCoreFindCard() != 0)
				break;
			nRet = aChangePwdEx(KeyA, ctrlword, oldKeyB, NHKyeB, i, 0, changeflag);
		}
		printf(" %d", nRet);
		if (nRet)
			break;
	}
	printf("\n");
	return nRet;

}
int __stdcall repairKeyB(unsigned char *ctrlword)
{
	unsigned char seed[32];
	unsigned char tmp[32];
	unsigned char oldKeyB[6];
	unsigned char newKeyB[0x6];
	unsigned char KeyA[6] = {0x43, 0x97, 0x04, 0x47, 0x20, 0x47};
	int i=0, nRet = 0;
	unsigned char changeflag=2;
	memset(oldKeyB, 0, sizeof(oldKeyB));
	memset(seed, 0, sizeof(seed));
	memset(tmp, 0, sizeof(tmp));

	//û��Ѱ����
	if(!Instance || !Instance->iRead)
		return -1;

	if (iCoreFindCard() != 0)
		return CardScanErr;

	//��������,�ó��ɵ�KeyB
	Instance->iRead(KeyA, tmp, 56, 640);
	bcd2str(tmp, seed, 14);
	iGetKeyBySeed(seed, oldKeyB);

	if (IsGWCard(seed)) {
		printf("����ǹ�������ֱ��ʧ��\n");
		return -1;
	}

	//�õ���ũ�Ϻ�Ϊ���ݵ�keyB
	memset(newKeyB, 0, sizeof(newKeyB));
	memset(seed, 0, sizeof(seed));
	memset(tmp, 0, sizeof(tmp));
	Instance->iRead(KeyA, tmp, 72, 792);
	bcd2str(tmp, seed, 18);
	iGetKeyBySeed(seed, newKeyB);

	printf("�޲�����:");
	for (i=0; i<16; ++i)
	{
		nRet = aChangePwdEx(KeyA, ctrlword, newKeyB, oldKeyB, i, 0, changeflag);
		printf("%d", nRet);
		if (nRet)
			break;
	}
	DBGADAP("�޲�����:%d\n", nRet);

	return nRet;
}

int __stdcall aFormatCard(unsigned char *pControl, unsigned char* szFormat, int nBlk, unsigned char *keyB)
{
	int nOffset = 128*nBlk;
	unsigned char bool_test = 0;
	unsigned char KeyA[6] = {0x43, 0x97, 0x04, 0x47, 0x20, 0x47};

	if(!Instance)
		return CardInitErr;

	if (GetWriteWord(pControl) == DEFAULT_CONTROL)
	{
		bool_test = Instance->iWrite(keyB, szFormat, DEFAULT_CONTROL, 128, nOffset);
	}
	else
	{
		bool_test = Instance->iWrite(KeyA, szFormat,  KEYA_CONTROL ,128, nOffset);
	}
	printf("%d", bool_test);
	return  bool_test==0 ? 0:-1;
}

int __stdcall aChangePwdEx(const unsigned char * pNewKeyA ,const unsigned char * ctrlword,
						 const unsigned char * pNewKeyB,const unsigned char * poldPin ,
						 unsigned char nsector,unsigned char keyA1B0,unsigned char changeflag)
{
	unsigned char  nRet = 0;
	unsigned char KeyA[6] = {0x43, 0x97, 0x04, 0x47, 0x20, 0x47};
	if (GetWriteWord(ctrlword) == DEFAULT_CONTROL)
	{
		nRet = Instance->iChangePwdEx(pNewKeyA, ctrlword, pNewKeyB, poldPin, nsector, keyA1B0, changeflag);
	}
	else
	{
		if (0 == IsAllTheSameFlag(pNewKeyA, 6, 0xff))
		{
			nRet = Instance->iChangePwdEx(pNewKeyA, ctrlword, pNewKeyB, KeyA, nsector, 1, changeflag);
		}
		else
		{
			memset(KeyA, 0xff, 6);
			nRet = Instance->iChangePwdEx(pNewKeyA, ctrlword, pNewKeyB, KeyA, nsector, 1, changeflag);
		}
	}
		
	return nRet ==0 ? 0:-1;
}

int __stdcall IsAllTheSameFlag(unsigned char *szBuf, int nLen, unsigned char cflag)
{
	int i=0;
	for (; i<nLen; ++i)
	{
		if (szBuf[i] != cflag)
		{
			return -1;
		}
	}
	return 0;
}




/**
 *
 */

#define FAILE_RETRY  2
static int _iWriteCard(struct RWRequestS *list)
{
	unsigned char seed[32];
	unsigned char  bool_test = -1;
	unsigned char keyNewB[6]={0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	unsigned char KeyA[6] = {0x43, 0x97, 0x04, 0x47, 0x20, 0x47};
	unsigned char Key[6];
	unsigned char pControl[4];
	int nWriteControl = DEFAULT_CONTROL;
	struct RWRequestS *CurrRequest= NULL;
	int faile_retry = 0;

	if(!Instance)
		return CardInitErr;

	aGetControlBuff(pControl, 0);
	nWriteControl = GetWriteWord(pControl);

	
	while (faile_retry < FAILE_RETRY)
	{
		memset(seed, 0, 32);
		iGetKeySeed(seed);

		if (IsAllTheSameFlag(seed, 14, 0x3f) == 0)
		{
			memcpy(Key, keyNewB, sizeof(keyNewB));
		}
		else if (IsAllTheSameFlag(seed, 14, 0x30) == 0)
		{
			memcpy(Key, keyNewB, sizeof(keyNewB));
		}
		else
		{
			if (nWriteControl == DEFAULT_CONTROL)
			{
				iGetKeyBySeed(seed, keyNewB);
				memcpy(Key, keyNewB, sizeof(keyNewB));
			}
			else
			{
				nWriteControl = KEYA_CONTROL;
				memcpy(Key, KeyA, sizeof(KeyA));
			}
		}

		if (iCoreFindCard() != 0)
			return CardScanErr;

		CurrRequest = list;
		while(CurrRequest)
		{
			bool_test = Instance->iWrite(Key, CurrRequest->value, nWriteControl , CurrRequest->length, CurrRequest->offset);
			CurrRequest = CurrRequest->Next;
			if (bool_test)
			{
				DBGADAP("д��ʧ�ܣ���Ҫ�޲�����\n");
				printf("д��ʧ�ܣ���Ҫ�޲�����\n");
				faile_retry++;
				break;
			}
		}

		//����keyb�����д�������޲�����
		if (bool_test)
		{
			bool_test = repairKeyB(pControl);
			if (bool_test)
				bool_test = repairKeyForFault(pControl);
			if (!bool_test)
			{
				DBGADAP( "�޲�����ɹ�������д��\n");
				printf("�޲�����ɹ�������д��\n");
				faile_retry = FAILE_RETRY-1;
			}
		}
		else
		{
			break;
		}
	}
	return bool_test==0 ? 0:CardWriteErr;
}

/**
 *
 */
int __stdcall iWriteCard(struct RWRequestS *list)
{
	struct RWRequestS *AgentList = NULL;
	int res = 0;

	// �������������

	// ������ʵ�Ķ�д����
	AgentList = _CreateReadList(list);

	// ����ʵд����������
	ParseWriteContent(list);

	// ��ʵ�����豸����д�빤��
	res = _iWriteCard(AgentList);

	// ɾ����д�б�
	DestroyRWRequest(AgentList, 1);

	return res;
}

