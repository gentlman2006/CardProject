#include "stdafx.h"
#include <stdio.h>
#include <malloc.h>
#include "public/device.h"
#include "public/liberr.h"
#include "adapter.h"
#include ".\public\algorithm.h"
#include "UtilProcess.h"

#pragma warning (disable : 4996)

#define IFD_OK	0				//����
#define IFD_ICC_Type_Error	1	//��Ƭ���Ͳ���
#define IFD_NO_ICC	2			//�޿�
#define IFD_ICC_Clash	3		//�࿨Ƭ��ͻ
#define ICC_NO_Response	4		//��Ƭ��Ӧ��
#define IFD_Error	5			//�ӿ��豸����
#define IFD_Bad_Command	6		//��֧�ָ�����
#define IFD_Length_Error	7	//����ȴ���
#define IFD_Parameter_Error	8	//�����������
#define IFD_CheckSum_Error	9	//��ϢУ��ͳ���
#define IFD_RD_CONNECT_ERROR 10	//


/************************************************************************/
/* ��ԿID ����                                                                     */
/************************************************************************/
#define KEY_RK_DDF1			0
#define KEY_RK_DF01			1
#define KEY_RK_DF02			2
#define KEY_RK_DF03			3

#define KEY_UK_DDF1			4
#define KEY_UK_DF01			5
#define KEY_UK_DF02_1		6
#define KEY_UK_DF02_2		7
#define KEY_UK_DF02_3		8
#define KEY_UK_DF03_1		9
#define KEY_UK_DF03_2		10

#define  USERCARD	0
#define  SAMCARD	1

#define CMD_BEEP	1
#define CMD_LED		2

#define PADDING     2

#define BIN_START   15
#define BIN_END		22

extern  struct RecFolder g_recIndex[30];

//Bin�ļ���дȨ��
static int g_BinAccessMap[BIN_START] = {0};

//�ж϶����ļ��Ƿ�������
static BOOL g_SureFill[BIN_START] = {0};

//ÿ���ֶε�����¼����
static int g_RecMap[BIN_START] = {0, 10, 5, 1, 6, 4, 9, 3, 4, 15, 1, 2, 2, 3, 5};


//mode 0:reader 1:writer
static struct RWRequestS  *_CreateReadList(struct RWRequestS *, int mode);

static int _iReadCard(struct RWRequestS *);

static void ListParseContent(struct RWRequestS *list);

// ��ʵ�����豸����д�빤��
static int _iWriteCard(struct RWRequestS *list);

// ����ʵд����������
static void ParseWriteContent(struct RWRequestS *list);

static int CheckActionValide();

static void* SpliteSegments(struct RWRequestS *list);

static BOOL IsSameFile(struct RWRequestS *oldReq, struct RWRequestS *newReq);


//�����¼�����
extern struct XmlProgramS *g_XmlListHead;
static struct CardDevice *Instance = NULL;


static int GetFloderKeyID(char *folder)
{
	if (strcmp (folder, "DDF1") == 0)
	    return KEY_RK_DDF1;
	else if (strcmp (folder, "DF01") == 0)
		return KEY_RK_DF01;
	else if (strcmp (folder, "DF02") == 0) 
		return KEY_RK_DF02;
	else if (strcmp (folder, "DF03") == 0)
		return KEY_RK_DF03;
	return -1;
}

//modeΪ��д��־ 0Ϊ����
static int GetUpdateKeyID(int SegID,int mode)
{
	if (/*SegID > 2 && */SegID < 5)
		return KEY_UK_DDF1;
	else if (SegID < 9) 
		return KEY_UK_DF01;
	else if (SegID < 10)
		return KEY_UK_DF02_1;
	else if (SegID < 11)
		return KEY_UK_DF02_2;
	else if (SegID < 13)
		return KEY_UK_DF02_3;
	else {
	    if (!mode && SegID < 15)
			return KEY_UK_DF03_2;
		return KEY_UK_DF03_1;
	}
	return -1;
}



int __stdcall initCoreDevice(const char *System)
{
	unsigned char resp[260];
	unsigned char len = 0;
	unsigned short para = 30;
	unsigned char *presp = resp;
	int status = 0;
	if(!Instance) 
		Instance = getCardDevice(System);

	//�����ն������豸����֤
	if (!Instance)
		return -1;

	Instance->iIOCtl(CMD_LED, &para, 2);
	Instance->iIOCtl(CMD_BEEP, &para, 2);
	status = Instance->ICCSet(SAMCARD, &len, presp);
	return (status == 0 ? 0 : -1) ;  
}

int __stdcall closeCoreDevice(void)
{
	int ret = 0;

	ret = putCardDevice(Instance);
	if(!ret) 
		Instance = NULL;

	return ret;
}

//Ѱ������������Ӧ�Ŀ�Ȩ������
int __stdcall iCoreFindCard(void)
{
	// ̽�⿨�����û�п����Զ��˳�
	unsigned char ret = 0;// = (unsigned char)Instance->ICCSet(
	return ret==0 ? 0 : CardScanErr ;
}


int __stdcall DestroyCPUData()
{
	return 0;
}

int __stdcall iReadCard(struct RWRequestS *list)
{
	struct RWRequestS *AgentList = NULL;
	int res;

	// ������ʵ�Ķ�д����
	AgentList = _CreateReadList(list, 0);

	// ��ʵ�����豸��д
	res = _iReadCard(AgentList);

	// �ⲿ�б���н���
	ListParseContent(list);

	// ɾ����д�б�
	DestroyRequest(AgentList, 1);

	return res;

}

int __stdcall iWriteCard(struct RWRequestS *list)
{
	struct RWRequestS *AgentList = NULL;
	int res = 0;

	SpliteSegments(list);

	// ������ʵ�Ķ�д����
	AgentList = _CreateReadList(list, 1);

	// ����ʵд����������
	ParseWriteContent(list);

	// ��ʵ�����豸����д�빤��
	res = _iWriteCard(AgentList);

	// ɾ����д�б�
	DestroyRequest(AgentList, 1);
	return res;
}

static struct RWRequestS  *_CreateReadList(struct RWRequestS *ReqList, int mode)
{
	struct RWRequestS *tmp = NULL, *current = NULL;
	struct RWRequestS *NCurrent;
	struct RWRequestS *ReadList = NULL;

	current = ReqList;
	while(current)
	{
		tmp = (struct RWRequestS *)malloc(sizeof(struct RWRequestS));
		memset(tmp, 0, sizeof(struct RWRequestS));
		memcpy(tmp, current, sizeof(struct RWRequestS));
		tmp->Next = NULL;

		//��������
		if(ReadList)
		{
			NCurrent->Next = tmp;
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
		if (current->datatype == eRecType) {
			current->length += g_RecMap[current->nID] * 2;
			current->value = (BYTE*)malloc(current->length);
		} 
		else 
		{
			current->value = (BYTE*)malloc(current->length);
		}
		memset(current->value, 0, current->length);
		current = current->Next;
	}
	return ReadList;
}


static int _iReadCard(struct RWRequestS *list)
{
	struct RWRequestS *pReq = list;
	int status = 0;
	int UCardFlag = 0;
	if (Instance)
	{
		if (!CheckActionValide())
			return -1;

		while (pReq)
		{
			if (strlen((char*)(g_recIndex[pReq->nID-1].section)) > 0) {
				status = Instance->iSelectFile(USERCARD, g_recIndex[pReq->nID-1].section);
				UCardFlag = GetFloderKeyID(g_recIndex[pReq->nID-1].section);
				status |= Instance->iUCardAuthSys(UCardFlag);
			}

			if (strlen((char*)(g_recIndex[pReq->nID-1].subSection)) > 0) {
				status |= Instance->iSelectFile(USERCARD, g_recIndex[pReq->nID-1].subSection);
				UCardFlag = GetFloderKeyID(g_recIndex[pReq->nID-1].subSection);
				status |= Instance->iUCardAuthSys(UCardFlag);
			}

			if (status)
				goto done;

			switch (pReq->datatype)
			{
			case eSureType:
			case eCycType:
			case eRecType:
				status |= Instance->iReadRec(USERCARD, g_recIndex[pReq->nID-1].fileName,pReq->value,
					pReq->length, 0xff, g_RecMap[pReq->nID]);
				break;
			case eBinType:
				if (g_SureFill[pReq->nID - BIN_START]) {
					if (g_BinAccessMap[pReq->nID - BIN_START] == 1) {
						status |= Instance->iReadBin(USERCARD, g_recIndex[pReq->nID-1].fileName,pReq->value, 
						155, pReq->offset);
					} else {
						status = 107;
					}
				} else {
					
				}

				break;
			}
			pReq = pReq->Next;
		}
	}

done:
	return status;
}

static void ListParseContent(struct RWRequestS *list)
{
	struct XmlColumnS *ColumnElement = NULL;
	struct RWRequestS *CurrRequest = list;
	struct RWRequestS *Agent = NULL;
	BYTE *bcd = NULL;
	eFileType eType;
	int len = 0;
	BYTE padding[2];

	while (CurrRequest)
	{
		ColumnElement = (struct XmlColumnS *)CurrRequest->pri;
		Agent = CurrRequest->agent;
		eType = CurrRequest->datatype;
		if (CurrRequest->offset == Agent->offset){
		    bcd = Agent->value;
		} 

		if (eType == eRecType) {//��¼�ļ�
			memcpy(padding, bcd, sizeof(padding));
			bcd += sizeof(padding);
			len = padding[1];
			memcpy(CurrRequest->value, bcd, len > CurrRequest->length ? CurrRequest->length : len);
			bcd += len;
		
		} else { //�������ļ�
			memcpy(CurrRequest->value , bcd, CurrRequest->length);
			bcd += CurrRequest->length;
		}

		//����ת��
		if (CurrRequest->itemtype != eAnsType) {
			BinToHexstr(CurrRequest->value, CurrRequest->value, CurrRequest->length);
			if (CurrRequest->datatype != eSureType)
				trimRightF(CurrRequest->value, 2*CurrRequest->length);

			if (CurrRequest->datatype == eSureType) {
				g_BinAccessMap[13] = BIN_START - 1;
				g_BinAccessMap[14] = g_RecMap[13] + g_BinAccessMap[13];
				g_SureFill[g_BinAccessMap[CurrRequest->nID] + CurrRequest->nColumID - BIN_START] = 1;
				if (strcmp((char*)CurrRequest->value, "00") == 0)
					g_BinAccessMap[g_BinAccessMap[CurrRequest->nID] + CurrRequest->nColumID - BIN_START] = 1;
				else
					g_BinAccessMap[g_BinAccessMap[CurrRequest->nID] + CurrRequest->nColumID - BIN_START] = 0;
			}
		}

		CurrRequest = CurrRequest->Next;
	}
}

// ����ʵд����������
static void ParseWriteContent(struct RWRequestS *list)
{
	struct XmlColumnS *ColumnElement = NULL;
	struct RWRequestS   *CurrRequest = list;
	struct RWRequestS *Agent = NULL;
	BYTE *bcd = NULL;
	int nByteLen = 0;
	eFileType datatype ;

	while(CurrRequest)
	{
		Agent= CurrRequest->agent;
		datatype = CurrRequest->datatype;
		ColumnElement = (struct XmlColumnS *)CurrRequest->pri;

		if ((CurrRequest->offset - Agent->offset) == 0)
		{
			bcd = Agent->value;
		}

		if (datatype == eRecType) { //��¼�ļ�
			memcpy(bcd, CurrRequest->value, CurrRequest->length+PADDING);
			*bcd++ = (BYTE)CurrRequest->nColumID;
			*bcd++ =(BYTE)CurrRequest->length;
			bcd += CurrRequest->length;

		} else /*if (datatype == eBinType)*/ {
			memcpy(bcd, CurrRequest->value, CurrRequest->length);
			bcd += CurrRequest->length;
		} /*else if (datatype == eCycType) {
			
		} else {
		}*/
		CurrRequest = CurrRequest->Next;
	}

}

// ��ʵ�����豸����д�빤��
static int _iWriteCard(struct RWRequestS *list)
{
	struct RWRequestS *pReq = list;
	struct RWRequestS *pOldReq = NULL;
	int status = 1;
	int UKey = 0;
	int mode = 0;
	if (Instance)
	{
		if (!CheckActionValide())
			goto done;

		while (pReq)
		{
			if (strlen((char*)(g_recIndex[pReq->nID-1].section)) > 0) {
				status = Instance->iSelectFile(USERCARD, g_recIndex[pReq->nID-1].section);
				UKey = GetFloderKeyID(g_recIndex[pReq->nID-1].section);
				status |= Instance->iUCardAuthSys(UKey);
			}

			if (strlen((char*)(g_recIndex[pReq->nID-1].subSection)) > 0) {
				status |= Instance->iSelectFile(USERCARD, g_recIndex[pReq->nID-1].subSection);
				UKey = GetFloderKeyID(g_recIndex[pReq->nID-1].subSection);
				status |= Instance->iUCardAuthSys(UKey);
			}

			pOldReq = pReq;
			while (IsSameFile(pOldReq, pReq))
			{
				if (pReq->datatype == eSureType) {
					mode = *(BYTE*)(pReq->value);
					mode = (mode==0 ? 1 : 0);
				}

				UKey = GetUpdateKeyID(pReq->nID, mode);
				status |= Instance->iUCardAuthSys(UKey);
				if (status) 
					goto done;

				switch (pReq->datatype)
				{
				case eRecType:
					status |= Instance->iWriteRec(USERCARD, g_recIndex[pReq->nID-1].fileName, pReq->value,
						pReq->length , 0xff, g_RecMap[pReq->nID]);
					break;
				case eBinType:
					status |= Instance->iWriteBin(USERCARD,g_recIndex[pReq->nID-1].fileName , pReq->value, 
						pReq->length, pReq->offset);
					break;
				case eCycType:
					status |= Instance->iAppendRec(g_recIndex[pReq->nID-1].fileName, pReq->value, pReq->length);
					break;
				case eSureType:
					status |= Instance->iSignRec(g_recIndex[pReq->nID-1].fileName, pReq->nColumID, mode);
					break;

				}
				pOldReq = pReq;
				pReq = pReq->Next;
			}
		}
	}
done:
	return status;
}

static int CheckActionValide()
{
	unsigned char len = 0;
	unsigned char resp[260];
	int status = 0;
	if (Instance)
	{
		status = Instance->ICCSet(USERCARD, &len , resp);
		status = Instance->ICCSet(SAMCARD, &len, resp);
		//status |= Instance->iGetRandom(USERCARD , 8 , resp);
		//memset(resp, 0, sizeof(resp));
		//status |= Instance->iGetRandom(SAMCARD , 4 , resp);

		return (status==0);
	}
	return FALSE;
}

static void* SpliteSegments(struct RWRequestS *list)
{
	struct RWRequestS *cur = list;
	int span = 1;
	int pos = 0;
	if (cur->datatype == eSureType 
		|| cur->datatype == eCycType) {
		if (cur->datatype == eCycType)
			span = g_RecMap[list->nID];
		
		while (cur)
		{
			if (pos == span){
				pos = 0;
				cur->offset = 0;
			}
			pos++;
			cur = cur->Next;
		}
	}
	return list;
}

static BOOL IsSameFile(struct RWRequestS *oldReq, struct RWRequestS *newReq)
{
	if (!newReq)
		return FALSE;

	return (oldReq->nID == newReq->nID 
		    && oldReq->datatype == newReq->datatype);
}