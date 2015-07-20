#include <stdio.h>
#include "adapter.h"
#include "device.h"
#include "public/liberr.h"
#include "public/algorithm.h"
#include "public/debug.h"
#include "Card.h"

#pragma warning (disable : 4020)
#pragma warning (disable : 4996)

#define DBGADAP(format, ...)		\
	LogWithTime(0, format)			\	

struct CardDevice *Instance = NULL;

static CardType g_PreCardType = -1;


/**
 * ������ apt_InitCoreDevice
 * ������
 *
 * ����ֵ��
 * �ɹ��� 0
 * ʧ�ܣ�����
 */
int __stdcall apt_InitCoreDevice(const char *System)
{
	if(!Instance) 
		Instance = getCardDevice(System);

	return (Instance == NULL?-1:0);
}

/**
 * ������apt_CloseCoreDevice
 * ������
 *
 * ����ֵ��
 * �ɹ��� 0
 * ʧ�ܣ� ����
 */
int __stdcall apt_CloseCoreDevice(void)
{
	int ret = 0;

	ret = putCardDevice(Instance);
	if(!ret)  {
		Instance = NULL;
		g_PreCardType = -1;
	}
	return ret;
}

int __stdcall apt_OpenDev(void)
{
	int ret = 0;
	if (Instance) {
		Instance->iProbe();
		ret = Instance->iOpen();
	}
	return (ret == 0 ? 0 : CardNotOpen);
}
int __stdcall apt_CloseDev(void)
{
	int ret = 0;
	if (Instance) {
		ret = Instance->iClose();
	}
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
int __stdcall apt_ScanCard(char *card_info)
{
	unsigned char cardType = 0, psamType=0;
	int status = 0;
	unsigned short para = 30;

	char resp[260];
	char attr[260];
	memset(resp, 0, sizeof(resp));
	memset(attr, 0, sizeof(attr));


	// ̽�⿨�����û�п����Զ��˳�
	if (!Instance) {
		return CardInitErr;
	}

	//status = Instance->iIOCtl( CMD_BEEP, &para , 2);
	//status = Instance->iIOCtl( CMD_LED, &para , 2);
	status = Instance->ICCSet( CARDSEAT_RF, &cardType , attr);

	if (cardType == eCPUCard) {
		status = Instance->ICCSet(CARDSEAT_PSAM1, &psamType, resp);
	}
	
	if (status) {
		return CardScanErr; 
	}

	sprintf(card_info, "%d|%s|%s", (int)cardType, resp, attr);
	memcpy(card_info, card_info, strlen(card_info) + 1);
	card_info[strlen(card_info)] = 0;
	//*card_type = cardType;

	//����xml�ļ�
	apt_InitGList(cardType);
	return 0;
}

/**
 *
 */
int __stdcall apt_IOCtl(int cmd, void *data)
{
	// ��ʼ�����Ϊʧ��
	unsigned char result = 0;

	// �����������Щ����
	if(Instance && Instance->iIOCtl) {
		result = (unsigned char)Instance->iIOCtl(cmd, data, 0);
	}

	return result;
}

/**
*
*/
struct RWRequestS*  apt_CreateRWRequest(struct XmlSegmentS *listHead, int mode, CardType type)
{
	struct XmlSegmentS	*SegmentElement = NULL;
	struct XmlColumnS	*ColumnElement = NULL;

	struct RWRequestS	*CurrRequest = NULL;
	struct RWRequestS	*TempRequest = NULL;
	struct RWRequestS	*result = NULL;

	SegmentElement = listHead;
	while(SegmentElement)
	{
		ColumnElement = SegmentElement->ColumnHeader;
		while(ColumnElement)
		{
			// �����µĽڵ�
			TempRequest = (struct RWRequestS *)malloc(sizeof(struct RWRequestS));
			if (TempRequest == NULL) {
				return NULL;
			}

			memset(TempRequest, 0, sizeof(struct RWRequestS));

			// �Խڵ���и�ֵ
			TempRequest->mode = mode;
			TempRequest->value = ColumnElement->Value;
			TempRequest->pri = (void *) ColumnElement;
			TempRequest->offset = ColumnElement->Offset;

			//Ϊ��ͬ�Ŀ�������д����
			if (type == eCPUCard) {
				TempRequest->length = ColumnElement->CheckInfo.CpuInfo.ColumnByte;
				TempRequest->nID = SegmentElement->ID;
				TempRequest->nColumID = ColumnElement->ID;
				TempRequest->datatype = SegmentElement->datatype;
				TempRequest->itemtype = ColumnElement->CheckInfo.CpuInfo.itemtype;
			} else {
				TempRequest->length = ColumnElement->CheckInfo.M1Info.ColumnBit;
			}

			// ��������
			if(result){
				CurrRequest->Next = TempRequest;
				CurrRequest = TempRequest;
			} else {
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
void  apt_DestroyRWRequest(struct RWRequestS *list, int flag)
{
	struct RWRequestS	*CurrRequest = NULL;
	struct RWRequestS	*TempRequest = NULL;

	CurrRequest = list;
	while(CurrRequest)
	{
		TempRequest = CurrRequest;
		CurrRequest = CurrRequest->Next;

		if(flag){
			free(TempRequest->value);
		}
		free(TempRequest);
	}

	return;
}


