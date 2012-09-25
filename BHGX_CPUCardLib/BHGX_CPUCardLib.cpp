// BHGX_CPUCardLib.cpp : ���� DLL Ӧ�ó������ڵ㡣
//
#include "stdafx.h"
#include "adapter.h"
#include "public/Markup.h"
#include "ns_pipeClient/n_USCOREapiSoap.nsmap"
#include "ns_pipeClient/soapn_USCOREapiSoapProxy.h"
#include "public/liberr.h"
#include "UtilProcess.h"
#include "resource.h"


#pragma warning (disable : 4996)
static BOOL g_CardInit = FALSE;
struct XmlProgramS *g_XmlListHead = NULL;

#define ASSERT_INIT(a)\
	if (a != TRUE)\
{\
	return -1;\
}\

// �õ����汾��
int __stdcall iGetCardVersion(char *pszVersion)
{
	strcpy(pszVersion, "Version 1.0.0.1");
	return 0;
}

// ��̬���ӿ��ʼ������
int __stdcall iCardInit(char *xml)
{
	if (g_CardInit)
		return 0;

	// ����Դ�ļ������ȡXML�ļ����ҳ�ʼ����
	HINSTANCE hInstance = ::LoadLibrary("BHGX_CPUCardLib.dll");
	HRSRC hResInfo = ::FindResource(hInstance, MAKEINTRESOURCE(IDR_XML1),"XML");
	HGLOBAL hgRes = ::LoadResource(hInstance, hResInfo);
	DWORD cbRes = ::SizeofResource(hInstance, hResInfo);
	char *pvRes = (char *)::LockResource(hgRes);
	if(!pvRes) 
	{	
		return CardInitErr;
	}
  
	char szSystem[256];
	memset(szSystem, 0, sizeof(szSystem));
	ReadConfigFromReg(szSystem);

	// ���豸���г�ʼ��
	g_CardInit = (initCoreDevice(szSystem)==0);

	if (g_CardInit)
	{
		CreateCPUData(pvRes);
	}
	return 0;
}

int __stdcall iCardDeinit()
{
	DestroyList(g_XmlListHead->SegHeader, 0);
	free(g_XmlListHead);
	g_CardInit = FALSE;
	g_XmlListHead = NULL;
	return closeCoreDevice();
}


int __stdcall iCardClose()
{
	return 0;
}

int __stdcall iCardOpen()
{
	return 0;
}

int __stdcall iCardIsEmpty()
{
	return 0;
}

// ��ȡflag��־�����xml
int __stdcall iReadInfo(int flag, char *xml)
{
	ASSERT_INIT(g_CardInit);
	struct XmlSegmentS	*list = NULL;
	struct RWRequestS	*RequestList = NULL;
	int length;
	int res;

	list = GetXmlSegmentByFlag(flag);
	if (list == NULL)
	{
		return CardXmlErr;
	}

	// ��ȡ��д����
	RequestList = CreateRequest(list, 0);

	// �豸����ʵ��ȡ
	res = iReadCard(RequestList);

	// ���ٶ�д��������
	DestroyRequest(RequestList, 0);

	// ͨ���������XML�ַ���
	iConvertXmlByList(list, xml, &length);

	DestroyList(list, 1);

	return res!=0 ? CardReadErr : 0;
}

int __stdcall iReadAnyInfo(int flag, char *xml, char *name)
{
	return 0;
}

// ��ȡname��Ϣ��������־�����
int __stdcall iQueryInfo(char *name, char *xml)
{
	return 0;
}

// д��xml����
int __stdcall iWriteInfo(char *xml)
{
	//ASSERT_INIT(g_CardInit);

	if (CheckCardXMLValid(xml) < 0)
	{
		printf("CardXML:Check Error\n");
		return CardXmlErr;
	}
	struct XmlSegmentS *XmlList;
	struct RWRequestS	*RequestList = NULL;
	unsigned int res = 0;

	// ����List
	XmlList = ConvertXmltoList(xml);

	if (XmlList == NULL)
	{
		return CardXmlErr;
	}

	// ������д����
	RequestList = CreateRequest(XmlList, 0);

	// ���豸������ʵ��д
	res = iWriteCard(RequestList);

	// ���ٶ�д����
	DestroyRequest(RequestList, 0);

	// ����XML����
	DestroyList(XmlList, 1);

	return res;
}

int __stdcall iScanCard()
{
	return 0;
}

//������Ϣ
char* __stdcall err(int errcode)
{
	return _err(errcode);
}


//�ƿ�
int __stdcall iCreateCard(char *pszCardDataXml)
{
	return 0;
}

int __stdcall iFormatCard()
{
	return 0;
}

int __stdcall iCardCtlCard(int cmd, void *data)
{
	return 0;
}

//��У��
int __stdcall iCheckMsgForNH(
				char *pszCardCheckWSDL, 
				char *pszCardServerURL, 
				char* pszXml
				)
{
	return 0;
}

//1|2|3|4 ��Ӧ ����|���֤��|��������|�Ա�
int __stdcall iReadConfigMsg(
				char *pszConfigXML,
				char *pszReadXML
				)
{
	return 0;
}

//��ע��
int __stdcall iRegMsgForNH(
			  char *pszCardServerURL, 
			  char* pszXml
			  )
{
	return 0;
}

int __stdcall iReadCardMessageForNH(
				char *pszCardCheckWSDL, 
				char *pszCardServerURL, 
				char* pszXml
				)
{
	return 0;
}

int __stdcall iEncryFile(char *filename)
{
	return 0;
}

//�ƿ�����ӡ
int __stdcall iPatchCard(
				 char *pszCardDataXml,
				 char *pszCardCoverDataXml,
				 char *pszPrinterType,
				 char *pszCardCoverXml 
				 )
{
	return 0;
}

//��ȡ��ӡ���б�����
int __stdcall iGetPrinterList(char *PrinterXML)
{
	return 0;
}

//��ӡ������
int __stdcall iPrintCard(
				 char *pszPrinterType,
				 char *pszCardCoverDataXml,
				 char *pszCardCoverXml
				 )
{
	return 0;
}

