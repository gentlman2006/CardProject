#pragma once

#ifndef _CARD_LIB_ERR_H_
#define _CARD_LIB_ERR_H_

#ifdef __cplusplus
extern "C" {
#endif	// __cplusplus

enum ErrType
{
	CardProcSuccess = 0,
	CardInitErr,
	CardXmlErr,
	CardReadErr,
	CardScanErr,
	CardWriteErr,
	CardCoverPrintErr,
	CardDLLLoadErr,
	CardCreateErr,
	CardFormatErr,
	CardCreateDateErr,
	FeedCardError,
	CardCheckError,
	CardRegError,
	EncryFileError,
	DescryFileError,
	CardIsNotEmpty,
	CardNotOpen,
	CardNoAuthority,
	CardAuthExpired,
	CardCreateLicenseFailed,
	CardNoSupport,
	CardMallocFailed,
	CardForbidden,
	CardWarnning,
	CardMedicalFailed,
	CardNotMedicalCard,
	ConnectWebServerFailed,
	CardSQLError,
	CardDBFileNotFound,
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
	"�����ļ�ʧ��,û�д��ļ������߲��Ǽ����ļ�",				//-15
	"�����ǿտ�",
	"�豸û�б���",
	"û�б���Ȩ����������ϵ��Ӧ��",
	"��Ȩ�Ѿ����ڣ�����ϵ��Ӧ��",
	"����license�ļ���������ʱ���ʽ",
	"�˿���û�д˹���",
	"�����ڴ����",
	"�ÿ���ע��,�޷�ʹ��",
	"�ÿ��ѹ�ʧ����ʱ�޷�ʹ��",
	"ũ�Ϻ�Ϊ��",
	"��ũ�Ͽ��������»���",
	"����webserverʧ��",
	"��ѯ���ݿ�ʧ��",
	"���ݿ��ļ������ڣ���ȷ�����ݿ�*.db����",
};

char *_err(int errcode);

#ifdef __cplusplus 
};
#endif	// __cplusplus

#endif	// _CARD_LIB_ERR_H_