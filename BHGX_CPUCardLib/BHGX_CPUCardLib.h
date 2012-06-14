#pragma once

#ifndef _CPUCARD_LIB_H_
#define _CPUCARD_LIB_H_

#define DLL_EXPORT _declspec(dllexport)

//#ifdef __cplusplus
//extern "C" {
//#endif

	// �õ����汾��
	DLL_EXPORT int __stdcall iGetCardVersion(char *pszVersion);

	// ��̬���ӿ��ʼ������
	DLL_EXPORT int __stdcall iCardInit(char *xml="");

	DLL_EXPORT int __stdcall iCardClose();

	DLL_EXPORT int __stdcall iCardIsEmpty();

	// ��ȡflag��־�����xml
	DLL_EXPORT int __stdcall iReadInfo(int flag, char *xml);

	DLL_EXPORT int __stdcall iReadAnyInfo(int flag, char *xml, char *name);

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

	DLL_EXPORT int __stdcall iReadCardMessageForNH(
		char *pszCardCheckWSDL, 
		char *pszCardServerURL, 
		char* pszXml
		);

	DLL_EXPORT int __stdcall iEncryFile(char *filename);


//#ifdef __cplusplus
//}
//#endif

#endif	