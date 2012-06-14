// BHGX_PRINT_P330i.cpp : ���� DLL Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "BHGX_PRINT_P330i.h"
#include "dcrf32/dcrf32.h"
#include <stdio.h>
#include <string>

using namespace std;
std::string g_PrinterName;

#pragma comment(lib, "dcrf32/dcrf32.lib")



//Ѱ�Ҵ�ӡ��
typedef int (__stdcall *ProbePrinter)();

//�ͷŴ�ӡ��
typedef int (__stdcall *FreePrinter)();

//����
typedef int (__stdcall *FeedInCard_NonContract)();

//����
typedef int (__stdcall *BackCardToPrintHeader_NonContract)();

//��ʼ����ӡ��
typedef int (__stdcall *InitGraphics)(const char *strPrinter);

//���ı������ӡ����(ǰ��������Ϊ�ı����Ͻ����꣬
//����������Ϊ��ӡ���ݣ���������Ϊ��ӡ���壬�����С��������(0)��������ɫ(0))
typedef int (__stdcall *PrintDrawText)(long, long, const char *, const char *, long, long, long);

//��ӡ//�粻��������ӡ��˵����ӡ������ָ���������⣬���Ǵ�ӡ�ķ�Χ�����꣬�����˿�Ƭ��Χ
typedef int (__stdcall *PrintGraphics)();

//�豸��ӡ�ر�
typedef int (__stdcall *CloseGraphics)();

//�豸״̬
typedef int (__stdcall *IsPrinterReady)(const char *strPrinter);

//����,��ӡ�ɹ��Ļ�,�������,ֱ�ӳ���
typedef int (__stdcall *OutCard)();

struct Printer
{
	HINSTANCE	hInstLibrary;

	ProbePrinter		   iProbePrinter;
	FreePrinter			   iFreePrinter;
	FeedInCard_NonContract iFeedInCard;
	BackCardToPrintHeader_NonContract iBackCardToPrintHeader;
	InitGraphics iInitGraphics;
	PrintDrawText iDrawText;
	PrintGraphics iPrintGraphics;
	CloseGraphics iCloseGraphics;
	IsPrinterReady iIsPrinterReady;
	OutCard iOutCard;
	Printer()
		:iFeedInCard(NULL)
		,iFreePrinter(NULL)
		,iProbePrinter(NULL)
		,iBackCardToPrintHeader(NULL)
		,iInitGraphics(NULL)
		,iDrawText(NULL)
		,iPrintGraphics(NULL)
		,iCloseGraphics(NULL)
		,iIsPrinterReady(NULL)
		,iOutCard(NULL)
	{

	}
};

Printer m_iPrinter;

int __stdcall GetPrinterFunc()
{
	HINSTANCE hInstance = LoadLibrary("FOX_DC.dll");
	if (hInstance == NULL)
	{
		return -1;
	}
	m_iPrinter.iFeedInCard = (FeedInCard_NonContract)GetProcAddress(hInstance,
		"DC_USB_CardPrinter_FeedInCard_NonContract");

	m_iPrinter.iBackCardToPrintHeader = (BackCardToPrintHeader_NonContract)GetProcAddress(hInstance,
		"DC_USB_CardPrinter_BackCardToPrintHeader_NonContract");
	m_iPrinter.iInitGraphics = (InitGraphics)GetProcAddress(hInstance, 
		"DC_USB_CardPrinter_InitGraphics");

	m_iPrinter.iDrawText = (PrintDrawText)GetProcAddress(hInstance, 
		"DC_USB_CardPrinter_DrawText");
	m_iPrinter.iPrintGraphics = (PrintGraphics)GetProcAddress(hInstance,
		"DC_USB_CardPrinter_PrintGraphics");
	m_iPrinter.iCloseGraphics = (CloseGraphics)GetProcAddress(hInstance, 
		"DC_USB_CardPrinter_CloseGraphics");

	m_iPrinter.iIsPrinterReady = (IsPrinterReady)GetProcAddress(hInstance,
		"DC_USB_CardPrinter_IsPrinterReady");

	m_iPrinter.iOutCard = (OutCard)GetProcAddress(hInstance, "DC_USB_CardPrinter_OutCard");
	m_iPrinter.hInstLibrary = hInstance;

	return 0;
}

int __stdcall iProbePrinter()
{
	return (GetPrinterFunc()==0);
}	

int __stdcall iFreePrinter()
{
	FreeLibrary(m_iPrinter.hInstLibrary);
	return 0;
}

int __stdcall iFeedCardToM1(void)
{
	return m_iPrinter.iFeedInCard();
}
int __stdcall iBackCardFromM1(void)
{
	return m_iPrinter.iBackCardToPrintHeader();
}
int __stdcall iInitGraphics ( char *pszPrinterName )
{
	g_PrinterName = pszPrinterName;
	return m_iPrinter.iInitGraphics(pszPrinterName);
}
int __stdcall iPrintText(long nPosx, long nPosy, char *szContent,
						char *szFont, long nFontSize, 
						long nFontStyle, long nFontColoer)
{
	int nRet = m_iPrinter.iDrawText(nPosx, nPosy, szContent, szFont, nFontSize, nFontStyle, nFontColoer);
	return nRet;
}

int __stdcall iFlushGraphics ( void )
{
	return m_iPrinter.iPrintGraphics();
}

int __stdcall iCloseGraphics ( void )
{
	return m_iPrinter.iCloseGraphics();
}

int __stdcall iOutCard(void)
{
	return m_iPrinter.iOutCard();
}

int __stdcall iCheckPrinterStatus()
{
	return 1;//m_iPrinter.iIsPrinterReady(g_PrinterName.c_str());
}



