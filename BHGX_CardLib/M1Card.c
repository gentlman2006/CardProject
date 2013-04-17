#include "M1Card.h"
#include "device.h"
#include "public/liberr.h"
#include "public/algorithm.h"
#include "public/debug.h"
#include "adapter.h"


#pragma warning (disable : 4020)
#pragma warning (disable : 4996)

#define DEFAULT_CONTROL	0		//Ĭ������ KeyA�� KeyBд
#define KEYA_CONTROL	1		//KeyA��д
#define KEYB_CONTROL	2		//KeyB��д

#define KEY_LEN			16
#define BLK_LEN			16

#define KEYA			0
#define KEYB			1

#define DEFAULT			0
#define NHCARD			1
#define GWCARD			2

#define DBGADAP(format, ...)  LogWithTime(0, format)

static unsigned char defKey[6] = 
{0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

static unsigned char defKeyA[0x6] = 
{0x43, 0x97, 0x04, 0x47, 0x20, 0x47};

static unsigned char defCtrl[0x4]=
{0x08,0x77,0x8f,0x69};

static unsigned char initCtrl[0x4]=
{0xff, 0x07, 0x80, 0x69};

#define ISGWCARD(cardno)   (cardno == 1) 
#define ISAPTSCANCARD {if (apt_ScanCard() != 0) return CardScanErr;}

static int GetControlBuff(unsigned char *pControl, int nSecr);
static int GetWriteWord(const unsigned char *pControl);
static int iGetKeySeed(int type, unsigned char *seed);
static int repairKeyForFault(unsigned char *ctrlword);
static int repairKeyB(unsigned char *ctrlword);
static int repairKeyBAllF(unsigned char *ctrlword);
static int ChangePwdEx(const unsigned char * pNewKeyA ,
					   const unsigned char * ctrlword,
					   const unsigned char * pNewKeyB,
					   const unsigned char * poldPin ,
					   unsigned char nsector,
					   unsigned char keyA1B0,
					   unsigned char changeflag);


static int _FormatCard(unsigned char *pControl, 
					   unsigned char* szFormat, 
					   int nBlk, 
					   unsigned char *keyB);

//M1����ʼ����Ĺ���
static int	M1LastInit(void*);

//Ϊ��������ڴ�
static int	M1CallocForList(struct RWRequestS*);

extern  struct CardDevice *Instance;

adapter M1Adapter;

/**
*
*/
static int str2bcd(const unsigned char *str, unsigned  char *bcd, int len)
{
	int i, j;

	for(i = 0; i < len; i++) 
	{		
		j = i >> 1;	

		if(i & 1) 
		{
			bcd[j] |= (str[i-1] - 48) << 4;
		}
		else
		{
			bcd[j]=str[i+1] -48;
		}
	}

	return i;
}

/**
*
*/
static int bcd2str(const unsigned char *bcd, unsigned char *str, int len)
{	
	int i, j;

	for(i=0; i < len; i++) 
	{
		j = i >> 1;
		if(i & 1) 
		{
			str[i] |= (bcd[j] & 0x0f) + 48;
		}	
		else 
		{
			str[i] = (bcd[j] >> 4) + 48;
		}
	}

	return i;
}

//��ȡд���ƣ� ��ʼ���Ŀ���λʱ����1���������Ϊ0
static int GetWriteWord(const unsigned char *pControl)
{
	int nRet = 0;
	int i = 0;

	for (; i<4; ++i)
	{
		if (initCtrl[i] != pControl[i]){
			return DEFAULT_CONTROL;
		}
	}
	return KEYA_CONTROL;
}

static int GetControlBuff(unsigned char *pControl, int nSecr)
{
	//439704472047
	unsigned char keyA[6];
	unsigned char bRead = 0;
	int BlkNr = 0;
	memset(keyA, 0, sizeof(keyA));

	/* ���û�ж����豸����*/
	if(!Instance) 
		return CardInitErr;
	ISAPTSCANCARD;

	BlkNr = nSecr * 4 + 3;
	memcpy(keyA, defKeyA, sizeof(keyA));
	bRead = Instance->iReadBin(CARDSEAT_M1, keyA, pControl, 4*8, BlkNr * 128 + 6*8);
	if (bRead != 0){
		ISAPTSCANCARD;

		memset(keyA, 0xff, sizeof(keyA));
		bRead = Instance->iReadBin(CARDSEAT_M1, keyA, pControl, 4*8, BlkNr * 128 + 6*8);
	}
	return bRead;
}

#define FAILE_RETRY  2
int __stdcall aFormatCard(unsigned char cFlag)
{
	unsigned char seed[20];
	unsigned char keyB[6];
	unsigned char newKeyA[0x6];
	unsigned char newKeyB[0x6];
	unsigned char changeflag=2;
	unsigned char ctrlWork[0x4]={0x08,0x77,0x8f,0x69};
	unsigned char szFormat[KEY_LEN];
	int nLen=0, i;
	int faile_retry = 0;
	int nRet = 0;
	memset(newKeyA, 0xff, 6);
	memset(newKeyB, 0xff, 6);
	memset(seed, 0, sizeof(seed));
	nLen = iGetKeySeed(DEFAULT, seed);
	if (nRet == -1 || nLen == 0 ||
		IsAllTheSameFlag(seed, nLen/2, 0x30)== 0 ||
		IsAllTheSameFlag(seed, nLen/2, 0x3f)== 0 ){
		memset(keyB, 0xff, 6);
	}else{
		iGetKeyBySeed(seed, keyB);
	}

	LogPrinter("��ʼ��ʽ������:");
	GetControlBuff(ctrlWork, 0);
	memset(szFormat, cFlag, KEY_LEN);
	while (faile_retry < FAILE_RETRY)
	{
		for (i=4; i<64; ++i)
		{
			if ((i+1) % 4 == 0){
				continue;
			}
			nRet = _FormatCard(ctrlWork, szFormat, i, keyB);
			if (nRet){
				faile_retry++;
				DBGADAP( "��ʽ��ʧ�ܣ���Ҫ�޲�����\n");
				break;
			}
		}

		if (nRet){
			nRet = repairKeyB(ctrlWork);
			if (nRet) {
				nRet = repairKeyForFault(ctrlWork);
			}

			if (!nRet)
			{
				faile_retry = FAILE_RETRY-1;
				DBGADAP( "�޲�����ɹ������¸�ʽ������\n");
				LogPrinter("\n���¸�ʽ������:");
			}
		}else{
			break;
		}
	}

	if (nRet) {
		goto done;
	}

	LogPrinter("��ʽ������:");
	for (i=0; i<BLK_LEN; ++i)
	{
		nRet = ChangePwdEx(newKeyA, ctrlWork, newKeyB, keyB, i, 0, changeflag);
		if (nRet != 0) {
			break;
		}
	}
	DBGADAP( "��ʽ��������:%d\n", nRet);
	LogPrinter("%d\n", nRet);

done:
	return nRet==0?CardProcSuccess:CardFormatErr;
}


static int _FormatCard(unsigned char *pControl, unsigned char* szFormat,
					   int nBlk, unsigned char *keyB)
{
	int nOffset = 128*nBlk;
	unsigned char bool_test = 0;

	if(!Instance)
		return CardInitErr;

	if (GetWriteWord(pControl) == DEFAULT_CONTROL)
	{
		bool_test = Instance->iWriteBin(CARDSEAT_M1, keyB, szFormat, DEFAULT_CONTROL, 128, nOffset);
	}else{
		bool_test = Instance->iWriteBin(CARDSEAT_M1, defKeyA, szFormat,  KEYA_CONTROL ,128, nOffset);
	}
	LogPrinter("%d", bool_test);
	return  bool_test==0 ? 0:-1;
}

static int ChangePwdEx(const unsigned char * pNewKeyA ,const unsigned char * ctrlword,
					   const unsigned char * pNewKeyB,const unsigned char * poldPin ,
					   unsigned char nsector,unsigned char keyA1B0,unsigned char changeflag)
{
	unsigned char  nRet = 0;
	unsigned char KeyA[6] = {0x43, 0x97, 0x04, 0x47, 0x20, 0x47};
	if (GetWriteWord(ctrlword) == DEFAULT_CONTROL)
	{
		nRet = Instance->iChangePwdEx(pNewKeyA, ctrlword, pNewKeyB, poldPin, 
			nsector, keyA1B0, changeflag);
	}
	else
	{
		if (0 == IsAllTheSameFlag(pNewKeyA, 6, 0xff))
		{
			nRet = Instance->iChangePwdEx(pNewKeyA, ctrlword, pNewKeyB, KeyA, 
				nsector, 1, changeflag);
		}
		else
		{
			memset(KeyA, 0xff, 6);
			nRet = Instance->iChangePwdEx(pNewKeyA, ctrlword, pNewKeyB, KeyA, 
				nsector, 1, changeflag);
		}
	}

	return nRet ==0 ? 0:-1;
}
static int iGetKeySeed(int type, unsigned char *seed)
{
	unsigned char tmp[32];
	int cardtype = 0;

	//û��Ѱ����
	if(!Instance || !Instance->iReadBin)
		return -1;

	//��ȡseed
	memset(tmp, 0, 32);
	Instance->iReadBin(CARDSEAT_M1, defKeyA, tmp, 56, 640);
	cardtype = tmp[0] >> 4;
	if (type == GWCARD) {
		bcd2str(tmp, seed, 14);
		goto done;
	} else if (type == NHCARD) {
		memset(tmp, 0, sizeof(tmp));
		Instance->iReadBin(CARDSEAT_M1, defKeyA, tmp, 72, 792);
		bcd2str(tmp, seed, 18);
		goto done;
	} else {
		if (tmp[0]>>4 == 0){
			memset(tmp, 0, sizeof(tmp));
			Instance->iReadBin(CARDSEAT_M1, defKeyA, tmp, 72, 792);
			bcd2str(tmp, seed, 18);
		} else {
			bcd2str(tmp, seed, 14);
		}
	}

done:
	return strlen(seed);
}


static int repairKeyForFault(unsigned char *ctrlword)
{
	unsigned char seed[32];
	unsigned char oldKeyB[6];  //���ԭ����KeyB
	unsigned char curKeyB[0x6];//��ǰ�����keyB
	unsigned char NHKyeB[6];
	int i=0, nRet = 0, type;
	unsigned char changeflag=2;
	memset(curKeyB, 0x75, sizeof(curKeyB));

	//û��Ѱ����
	if(!Instance || !Instance->iReadBin)
		return -1;

	ISAPTSCANCARD;

	//��������,�ó��ɵ�KeyB
	memset(oldKeyB, 0, sizeof(oldKeyB));
	memset(seed, 0, sizeof(seed));
	type = iGetKeySeed(GWCARD, seed);
	iGetKeyBySeed(seed, oldKeyB);

	//�����ũ�Ͽ���ֱ��ʧ��
	if (!ISGWCARD(type)) {
		LogPrinter("�˿�Ϊũ�Ͽ���ֱ��ʧ��\n");
		return -1;
	}

	//�õ���ũ�Ϻ�Ϊ���ݵ�keyB
	memset(seed, 0, sizeof(seed));
	memset(NHKyeB, 0, sizeof(NHKyeB));
	iGetKeySeed(NHCARD, seed);
	iGetKeyBySeed(seed, NHKyeB);

	LogPrinter("[6-4�½������޲�����]:");
	for (i=0; i<BLK_LEN; ++i)
	{
		nRet = ChangePwdEx(defKeyA, ctrlword, oldKeyB, curKeyB, i, 0, changeflag);
		if (nRet) {
			ISAPTSCANCARD;
			nRet = ChangePwdEx(defKeyA, ctrlword, oldKeyB, NHKyeB, i, 0, changeflag);
		}
		LogPrinter(" %d", nRet);
		if (nRet)
			break;
	}
	LogPrinter("\n");
	return nRet;

}

static int repairKeyBAllF(unsigned char *ctrlword)
{
	unsigned char seed[32];
	unsigned char oldKeyB[6];
	unsigned char newKeyB[6];
	int i=0, nRet = 0, type;
	unsigned char changeflag=2;
	memset(oldKeyB, 0xFF, sizeof(oldKeyB));

	//û��Ѱ����
	if(!Instance || !Instance->iReadBin)
		return -1;
	ISAPTSCANCARD;

	//��������,�ó��ɵ�KeyB
	memset(newKeyB, 0, sizeof(newKeyB));
	memset(seed, 0, sizeof(seed));
	type = iGetKeySeed(GWCARD, seed);
	iGetKeyBySeed(seed, newKeyB);

	if (!ISGWCARD(type)) {

		//�õ���ũ�Ϻ�Ϊ���ݵ�keyB
		memset(newKeyB, 0, sizeof(newKeyB));
		memset(seed, 0, sizeof(seed));
		iGetKeySeed(NHCARD, seed);
		iGetKeyBySeed(seed, newKeyB);
	}

	LogPrinter("�޲�����:");
	for (i=0; i<BLK_LEN; ++i){
		nRet = ChangePwdEx(defKeyA, ctrlword, newKeyB, oldKeyB, i, 0, changeflag);
		LogPrinter("%d", nRet);
		if (nRet)
			break;
	}
	DBGADAP("�޲�����:%d\n", nRet);

	return nRet;
}

static int repairKeyB(unsigned char *ctrlword)
{
	unsigned char seed[32];
	unsigned char oldKeyB[6];
	unsigned char newKeyB[0x6];
	int i=0, nRet = 0, type;
	unsigned char changeflag=2;

	//û��Ѱ����
	if(!Instance || !Instance->iReadBin)
		return -1;
	ISAPTSCANCARD;

	//��������,�ó��ɵ�KeyB
	memset(oldKeyB, 0, sizeof(oldKeyB));
	memset(seed, 0, sizeof(seed));
	type = iGetKeySeed(GWCARD, seed);
	iGetKeyBySeed(seed, oldKeyB);

	if (ISGWCARD(type)) {
		LogPrinter("����ǹ�������ֱ��ʧ��\n");
		return -1;
	}

	//�õ���ũ�Ϻ�Ϊ���ݵ�keyB
	memset(newKeyB, 0, sizeof(newKeyB));
	memset(seed, 0, sizeof(seed));
	iGetKeySeed(NHCARD, seed);
	iGetKeyBySeed(seed, newKeyB);

	LogPrinter("�޲�����:");
	for (i=0; i<BLK_LEN; ++i)
	{
		nRet = ChangePwdEx(defKeyA, ctrlword, newKeyB, oldKeyB, i, 0, changeflag);
		LogPrinter("%d", nRet);
		if (nRet)
			break;
	}
	DBGADAP("�޲�����:%d\n", nRet);

	return nRet;
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
		//memset(CurrRequest->value, 0, 100);

		// ����Ϊ�յĴ���: ���Ϊ0xff,�����������
		if(bcd[0] == 0xff){
			goto Next;
		}

		nOffSet = (CurrRequest->offset - Agent->offset)/4%2;

		//add by yanggx	20110619
		////BCDת�����ַ��� �����⣬ ������4λ��BCD������˵���.
		if(CurrRequest->length == 4){
			if (nOffSet == 0){
				nOffSet = CurrRequest->offset/4%2;
			}
			Bcd2Ch(CurrRequest->value, bcd, nOffSet);	
		} else {
			//add by yanggx 20110706
			Bcd2StrOffSet(CurrRequest->value, bcd, (CurrRequest->length)/4, nOffSet);
			if (CurrRequest->offset == ID_OFFSET) {
				if (CurrRequest->value[(CurrRequest->length)/4-1] == 'D'
					|| CurrRequest->value[(CurrRequest->length)/4-1] == 'd'){
					CurrRequest->value[(CurrRequest->length)/4-1] = 'x';

				}else if (CurrRequest->value[(CurrRequest->length)/4-1] == 'e'
					|| CurrRequest->value[(CurrRequest->length)/4-1] == 'E') {

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
int _iReadCard(struct RWRequestS *list)
{
	unsigned char bRead = 0;

	struct RWRequestS *CurrRequest= NULL;

	/* ���û�ж����豸����*/
	if(!Instance) 
		return CardInitErr;

	//memset(defKeyA, 0xff, sizeof(defKeyA));
	CurrRequest = list;
	while(CurrRequest)
	{
		bRead = Instance->iReadBin(CARDSEAT_M1, defKeyA, CurrRequest->value, CurrRequest->length, 
			CurrRequest->offset);

		// ������
		CurrRequest = CurrRequest->Next;
	}

	return bRead==0 ? 0:CardReadErr;
}

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
		if(!strlen((char*)CurrRequest->value)){
			memset(CurrRequest->value, 0xff, (CurrRequest->length)/8);
		}

		//comment by yanggx	110612  ���ַ���ȡ����ÿ��Agent������ַ���¼����
		nByteLen = CurrRequest->length/4;
		if ((CurrRequest->offset - Agent->offset) == 0){
			bcd = Agent->value;

			//add by yanggx 20110616 �жϿ�ʼλ�Ƿ�������
			nBitPos = CurrRequest->offset/4%2;
			if(nBitPos == 1){
				memset(bcd, 0xff, 1);
			}
			nAgentReLen = nByteLen + nBitPos;
		} else {
			bcd = Agent->value + nAgentReLen/2;
			nAgentReLen += nByteLen;
		}

		// ����ĳ���Ӧ�����ַ����ĳ��ȣ���ӦΪ�ռ�ĳ��� 
		len = (int)strlen((char*)CurrRequest->value);
		nPos = (nAgentReLen - nByteLen)%2;

		for(i=0; i<len; ++i){

			//���ַ�תΪBCD���룬������bcd��
			StrCh2Bcd(CurrRequest->value[i], bcd+(i+nPos)/2, (i+nPos)%2);
		}

		//add by yanggx 20110615
		//����ַ�����û���ֽڳ��ȵ�4���������0xf
		nByteLen = CurrRequest->length/4;
		if (nByteLen > len) {
			for(; i<nByteLen; ++i)
			{
				StrCh2Bcd('f', bcd+(i+nPos)/2, (i+nPos)%2);
			}
		}
		CurrRequest = CurrRequest->Next;
	}

	return;
}

int _iWriteCard(struct RWRequestS *list)
{
	unsigned char seed[32];
	unsigned char  bool_test = -1;
	unsigned char keyNewB[6]={0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	unsigned char Key[6];
	unsigned char pControl[4];
	int nWriteControl = DEFAULT_CONTROL;
	struct RWRequestS *CurrRequest= NULL;
	int faile_retry = 0;

	if(!Instance)
		return CardInitErr;

	//��ȡ����λ���жϿ�Ƭ��д��ʽ
	GetControlBuff(pControl, 0);
	nWriteControl = GetWriteWord(pControl);


	while (faile_retry < FAILE_RETRY)
	{
		memset(seed, 0, 32);

		//���ݿ�����λ��ȡ����Keyb������
		iGetKeySeed(DEFAULT, seed);   

		//ȫFF/00�������Ϊ��ʼ����ΪȫF
		if (IsAllTheSameFlag(seed, 14, 0x3f) == 0 ||
			IsAllTheSameFlag(seed, 14, 0x30) == 0){
			memcpy(Key, keyNewB, sizeof(keyNewB));
		} else {
			//Ĭ�϶�д��ʽ
			if (nWriteControl == DEFAULT_CONTROL){
				iGetKeyBySeed(seed, keyNewB);
				memcpy(Key, keyNewB, sizeof(keyNewB));
			} else {
				//KeyA��ȫ����дȨ��
				nWriteControl = KEYA_CONTROL;
				memcpy(Key, defKeyA, sizeof(defKeyA));
			}
		}

		ISAPTSCANCARD;

		CurrRequest = list;
		while(CurrRequest)
		{
			bool_test = Instance->iWriteBin(CARDSEAT_M1, Key, CurrRequest->value, nWriteControl , 
				CurrRequest->length, CurrRequest->offset);
			CurrRequest = CurrRequest->Next;
			if (bool_test) {
				DBGADAP("д��ʧ�ܣ���Ҫ�޲�����\n");
				LogPrinter("д��ʧ�ܣ���Ҫ�޲�����\n");
				faile_retry++;
				break;
			}
		}

		//����keyb�����д�������޲�����
		if (bool_test) {
			bool_test = repairKeyBAllF(pControl);
			if (bool_test) {
				//�޲�ũ�Ͽ�KeyB����
				bool_test = repairKeyB(pControl);
			}
			if (bool_test) {
				//�޲��½�����Ϊ0x75�Ĵ���
				bool_test = repairKeyForFault(pControl);
			}
			if (!bool_test) {
				DBGADAP( "�޲�����ɹ�������д��\n");
				LogPrinter("�޲�����ɹ�������д��\n");
				faile_retry = FAILE_RETRY-1;
			}
		} else{
			break;
		}
	}
	return bool_test==0 ? 0:CardWriteErr;
}

int __stdcall InitPwd(unsigned char *newKeyB)
{
	unsigned char oldKeyB[6], ctrlWord[6];
	unsigned char changeflag=2, i, nRet;
	memset(oldKeyB, 0xff, 6);
	memset(ctrlWord, 0, 6);

	LogPrinter("��������:");
	GetControlBuff(ctrlWord, 0);
	for (i=0; i<BLK_LEN; ++i)
	{
		nRet = ChangePwdEx(defKeyA, ctrlWord, newKeyB, 
			oldKeyB, i, 0, changeflag);

		LogPrinter("%d", nRet); 
		if (nRet != 0)
			break;
	}
	LogPrinter("\n");
	return nRet;
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
		memset(current->value, 0, (current->length + 7)/8);

		current = current->Next;
	}

	return ReadList;
}



int  M1ReadCard(struct RWRequestS *list, void *apt)
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
	apt_DestroyRWRequest(AgentList, 1);;

	return res;
}

int M1WriteCard(struct RWRequestS *list,  adapter *apt)
{
	struct RWRequestS *AgentList = NULL;
	int res = 0;

	// ������ʵ�Ķ�д����
	AgentList = _CreateReadList(list);

	// ����ʵд����������
	ParseWriteContent(list);

	// ��ʵ�����豸����д�빤��
	res = _iWriteCard(AgentList);

	// ɾ����д�б�
	apt_DestroyRWRequest(AgentList, 1);

	return res;
}

//M1����ʼ����Ĺ���
static int	M1LastInit(void* data)
{
	return 0;
}

//Ϊ��������ڴ�
static int	M1CallocForList(struct RWRequestS* request)
{
	return 0;
}

int __stdcall InitM1CardOps()
{
	M1Adapter.type = eM1Card;
	M1Adapter.iLastInit = M1LastInit;
	M1Adapter.iCallocForList = M1CallocForList;
	M1Adapter.iReadCard = M1ReadCard;
	M1Adapter.iWriteCard = M1WriteCard;
	return 0;
}
