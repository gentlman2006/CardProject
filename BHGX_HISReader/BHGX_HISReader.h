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
	* ���������	
	* ���������	
	* �� �� ֵ��	0:�ɹ� ������ʧ��
	*******************************************************************************/
	DLL_EXPORT int __stdcall iReadHISInfo(char *pszCardCheckWSDL, char *pszCardServerURL, char *xml);

	/*******************************************************************************
	* �������ƣ�	iReadOnlyHIS	
	* ����������	xml			������Ϣ
	* ���������	
	* ���������	
	* �� �� ֵ��	0:�ɹ� ������ʧ��
	*******************************************************************************/
	DLL_EXPORT int __stdcall iReadOnlyHIS(char *xml);

	/*******************************************************************************
	* �������ƣ�	iReadInfoForXJ	
	* ����������	xml			������Ϣ
	* ���������	
	* ���������	
	* �� �� ֵ��	0:�ɹ� ������ʧ��
	*******************************************************************************/
	DLL_EXPORT int __stdcall iReadInfoForXJ(char *pszCardCheckWSDL, char *pszCardServerURL, char *xml);


#ifdef __cplusplus
}
#endif

#endif