#pragma once

#ifndef _CARD_LIB_H_
#define _CARD_LIB_H_

#define DLL_EXPORT _declspec(dllexport)

#define F1		0x01		//�ƿ���Ϣ
#define F2		0x02		//�κϻ�����Ϣ
#define F3		0x04		//ũ�ϲ�����Ϣ
#define F4		0x08		//������־
#define F5		0x10		//����������Ϣ

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

#ifdef __cplusplus
extern "C" {
#endif
	// �õ����汾��
	DLL_EXPORT int __stdcall iGetCardVersion(char *pszVersion);

	// ��̬���ӿ��ʼ������
	DLL_EXPORT int __stdcall iCardInit(char *xml = "");

	DLL_EXPORT int __stdcall iCardClose();

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

	DLL_EXPORT int __stdcall iReadCardMessageForNH(
		char *pszCardCheckWSDL, 
		char *pszCardServerURL, 
		char* pszXml
		);

	DLL_EXPORT int __stdcall iEncryFile(char *filename);


#ifdef __cplusplus
}
#endif

#endif	// _CARD_LIB_H_