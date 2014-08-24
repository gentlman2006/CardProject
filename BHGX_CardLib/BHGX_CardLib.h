#pragma once

#ifndef _CARD_LIB_H_
#define _CARD_LIB_H_

#define DLL_EXPORT _declspec(dllexport)

#define F1		0x01		//�ƿ���Ϣ
#define F2		0x02		//�κϻ�����Ϣ
#define F3		0x04		//ũ�ϲ�����Ϣ
#define F4		0x08		//������־
#define F5		0x10		//����������Ϣ

#ifdef __cplusplus
extern "C" {
#endif
	// �õ����汾��
	DLL_EXPORT int __stdcall iGetCardVersion(char *pszVersion);

	DLL_EXPORT int __stdcall iCardCompany(char *szCompanyXml);

	// ��̬���ӿ��ʼ������
	DLL_EXPORT int __stdcall iCardInit(char *xml = "");

	DLL_EXPORT int __stdcall iCardDeinit();

	DLL_EXPORT int __stdcall iCardClose();

	DLL_EXPORT int __stdcall iCardOpen();

	DLL_EXPORT int __stdcall iCardIsEmpty();

	// ��ȡflag��־�����xml
	DLL_EXPORT int __stdcall iReadInfo(int flag, char *xml);

	DLL_EXPORT int __stdcall iReadAnyInfo(int flag, char *xml, char *name="");

	// ��ȡname��Ϣ��������־�����
	DLL_EXPORT int __stdcall iQueryInfo(char *name, char *xml);

	// д��xml����
	DLL_EXPORT int __stdcall iWriteInfo(char *xml);

	DLL_EXPORT int __stdcall iScanCard();

	//������Ϣ
	DLL_EXPORT char* __stdcall err(int errcode);

	//�ƿ�����ӡ
	DLL_EXPORT int __stdcall iPatchCard(
							char *pszCardDataXml,
							char *pszCardCoverDataXml,
							char *pszPrinterType,
							char *pszCardCoverXml 
							);

	//��ȡ��ӡ���б�����
	DLL_EXPORT int __stdcall iGetPrinterList(char *PrinterXML);

	//��ӡ������
	DLL_EXPORT int __stdcall iPrintCard(
							char *pszPrinterType,
							char *pszCardCoverDataXml,
							char *pszCardCoverXml 
							);


	//�ƿ�
	DLL_EXPORT int __stdcall iCreateCard(char *pszCardDataXml);

	DLL_EXPORT int __stdcall iFormatCard();

	DLL_EXPORT int __stdcall iCardCtlCard(int cmd, void *data);

	//��У��
	DLL_EXPORT int __stdcall iCheckMsgForNH(
		char *pszCardCheckWSDL, 
		char *pszCardServerURL, 
		char* pszXml
		);

	//1|2|3|4 ��Ӧ ����|���֤��|��������|�Ա�
	DLL_EXPORT int __stdcall iReadConfigMsg(
		char *pszConfigXML,
		char *pszReadXML
		);

	//��ע��
	DLL_EXPORT int __stdcall iRegMsgForNH(
		char *pszCardServerURL, 
		char* pszXml
		);

	DLL_EXPORT int __stdcall iReadOnlyCardMessageForNH( 
		char* pszXml
		);

	DLL_EXPORT int __stdcall iReadCardMessageForNH(
		char *pszCardCheckWSDL, 
		char *pszCardServerURL, 
		char* pszXml
		);

	//��ע��
	DLL_EXPORT int __stdcall iRegMsgForNHLog(
		char *pszCardServerURL, 
		char* pszLogXml,
		char* pszXml
		);

	DLL_EXPORT int __stdcall iReadCardMessageForNHLog(
		char *pszCardCheckWSDL, 
		char *pszCardServerURL, 
		char* pszLogXml,
		char* pszXml
		);

	DLL_EXPORT int __stdcall iReadCardMessageForNHLocal(
		char* pszLogXml,
		char* pszXml
		);

	//��У�� ������У��
	DLL_EXPORT int __stdcall iCheckMsgForNHLocal(
		char* pszLogXml,
		char* pszXml
		);

	DLL_EXPORT int __stdcall iEncryFile(char *filename);

	DLL_EXPORT int __stdcall iCreateLicense(
		char *filename, 
		char *timeStr
		);  

#define TIMELICENSE		0
#define OTHERLICENSE	1
#define COUNTS			2
	DLL_EXPORT int __stdcall iCheckLicense(
		char *filename,
		int type					//0��ʱ��  1����
		);

	// ������У��
	DLL_EXPORT int __stdcall iCheckException(
		char *pszLogXml,
		char *pszXml
		);


#ifdef __cplusplus
}
#endif

#endif	// _CARD_LIB_H_