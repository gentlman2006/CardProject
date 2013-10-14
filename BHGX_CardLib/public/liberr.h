#pragma once

#ifndef _CARD_LIB_ERR_H_
#define _CARD_LIB_ERR_H_

#ifdef __cplusplus
extern "C" {
#endif	// __cplusplus

enum ErrType
{
	CardProcSuccess = 0,
	CardInitErr = 1,
	CardXmlErr = 2,
	CardReadErr = 3,
	CardScanErr = 4,
	CardWriteErr = 5,
	CardCoverPrintErr = 6,
	CardDLLLoadErr = 7,
	CardCreateErr = 8,
	CardFormatErr = 9,
	CardCreateDateErr = 10,
	FeedCardError = 11,
	CardCheckError = 12,
	CardRegError = 13,
	EncryFileError = 14,
	DescryFileError = 15,
	CardIsNotEmpty = 16,
	CardNotOpen = 17,
	CardNoAuthority = 18,
	CardAuthExpired = 19,
	CardCreateLicenseFailed = 20,
	CardUAuthFailed = 21,
	SendMsgFailed   = 22,

};

static char *errString[] =
{
	"�������ɹ�",				// 0
	"����ʼ��ʧ��",			// -1
	"XML�ļ���ʽ����",		// -2
	"����ȡʧ��",		// -3
	"Ѱ��ʧ��",		// -4
	"��д��ʧ��",		//-5
	"��ӡ����ʧ��",	//-6
	"����ӡ��̬���ӿ�δ����",	//-7
	"�ƿ�ʧ��",			//-8
	"����ʽ��ʧ��",				//-9
	"���ɿ�����ʧ��",			//-10
	"����ʧ��",					// -11
	"��У��ʧ��",				// -12	
	"��ע��ʧ��",				//-13
	"�����ļ�ʧ��",				//-14
	"�����ļ�ʧ��",				//-15
	"�����ǿտ�",
	"�豸û�б���",
	"û�б���Ȩ����������ϵ��Ӧ��",
	"��Ȩ�Ѿ����ڣ�����ϵ��Ӧ��",
	"����license�ļ���������ʱ���ʽ",
	"�豸û�б���Ȩ�������U��",
	"���Ͷ���wsdlʧ��",
};

char *_err(int errcode);

#ifdef __cplusplus 
};
#endif	// __cplusplus

#endif	// _CARD_LIB_ERR_H_