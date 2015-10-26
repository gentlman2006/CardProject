#pragma once

#ifndef _BHGX_HISReader_H_
#define _BHGX_HISReader_H_

#define DLL_EXPORT _declspec(dllexport)

#ifdef __cplusplus
extern "C" {
#endif
	/*******************************************************************************
	* �������ƣ�	iReadHISInfo	
	* ����������	xml			������Ϣ
	*				pszCardCheckWSDL	WSDL�ļ���ַ
	*				pszCardServerURL	WSDL��������ַ
	*				bMsg				�Ƿ��Ͷ���
	*				msgSUrl				���ŷ��ͷ�������ַ
	* ���������	
	* ���������	
	* �� �� ֵ��	0:�ɹ� ������ʧ��
	*******************************************************************************/
	DLL_EXPORT int __stdcall iReadHISInfo(char *pszCardCheckWSDL, char *pszCardServerURL, char *xml,  bool bMsg=false, char *msgSUrl="");

	/*******************************************************************************
	* �������ƣ�	iReadOnlyHIS	
	* ���������	xml			������Ϣ
	*				bMsg		�Ƿ��Ͷ���
	*				msgSUrl		���ŷ��ͷ�������ַ
	* ��	
	* ���������	
	* �� �� ֵ��	0:�ɹ� ������ʧ��
	*******************************************************************************/
	DLL_EXPORT int __stdcall iReadOnlyHIS(char *xml,  bool bMsg=false, char *msgSUrl="");

	/*******************************************************************************
	* �������ƣ�	iReadInfoForXJ	
	* ����������	
	* ���������	xml			������Ϣ
	* ���������	
	* �� �� ֵ��	0:�ɹ� ������ʧ��
	*******************************************************************************/
	DLL_EXPORT int __stdcall iReadInfoForXJ(char *pszCardCheckWSDL, char *pszCardServerURL, char *xml);

	/*******************************************************************************
	* �������ƣ�	iSendMessage	
	* ����������	���Ͷ���webservice�ӿ�
	* ���������	url			���ŷ��ͷ�������ַ
	*				cardNO		����
	*				identity	���֤��
	*				farmID		ũ�Ϻ�
	*				name		����
	*				healthID	����������
	* ���������	
	* �� �� ֵ��	0:�ɹ� ������ʧ��
	*******************************************************************************/
	DLL_EXPORT int __stdcall iSendMessage(
			char *url, 
			const char *cardNO, 
			const char *identity, 
			const char *farmID, 
			const char *name, 
			const char *healthID
		);
#ifdef __cplusplus
}
#endif

#endif