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

	/*******************************************************************************
	* �������ƣ�	iReadHISInfoLog	
	* ����������	xml			������Ϣ
	*				pszCardCheckWSDL	WSDL�ļ���ַ
	*				pszCardServerURL	WSDL��������ַ
	*				pszLogXml			��־����xml
	* ���������	
	* ���������	
	* �� �� ֵ��	0:�ɹ� ������ʧ��
	*******************************************************************************/
	DLL_EXPORT int __stdcall iReadHISInfoLog(char *pszCardCheckWSDL, char *pszCardServerURL, 
										  char *pszLogXml, char *xml);

	/*******************************************************************************
	* �������ƣ�	iReadOnlyHISLog	
	* ����������	xml			������Ϣ
	*				pszLogXml			��־����xml
	* ���������	
	* ���������	
	* �� �� ֵ��	0:�ɹ� ������ʧ��
	*******************************************************************************/
	DLL_EXPORT int __stdcall iReadOnlyHISLog(char *pszLogXml, char *xml);

	/*******************************************************************************
	* �������ƣ�	iReadInfoForXJLog	
	* ����������	xml			������Ϣ
	*				pszLogXml			��־����xml
	* ���������	
	* ���������	
	* �� �� ֵ��	0:�ɹ� ������ʧ��
	*******************************************************************************/
	DLL_EXPORT int __stdcall iReadInfoForXJLog(char *pszCardCheckWSDL, char *pszCardServerURL, 
											char *pszLogXml, char *xml);

	/*******************************************************************************
	* �������ƣ�	iReadHISInfoLocal	
	* ����������	xml			������Ϣ
	*				pszCardCheckWSDL	WSDL�ļ���ַ
	*				pszCardServerURL	WSDL��������ַ
	*				pszLogXml			��־����xml
	* ���������	
	* ���������	
	* �� �� ֵ��	0:�ɹ� ������ʧ��
	*******************************************************************************/
	DLL_EXPORT int __stdcall iReadHISInfoLocal(char *pszLogXml, char *xml);

	/*******************************************************************************
	* �������ƣ�	iReadInfoForXJLocal	
	* ����������	xml			������Ϣ
	*				pszLogXml			��־����xml
	* ���������	
	* ���������	
	* �� �� ֵ��	0:�ɹ� ������ʧ��
	*******************************************************************************/
	DLL_EXPORT int __stdcall iReadInfoForXJLocal(char *pszLogXml, char *xml);

	/*******************************************************************************
	* �������ƣ�	iReadOnlyHISLocal
	* ����������	xml			������Ϣ
	*				pszLogXml			��־����xml
	* ���������	
	* ���������	
	* �� �� ֵ��	0:�ɹ� ������ʧ��
	*******************************************************************************/
	DLL_EXPORT int __stdcall iReadOnlyHISLocal(char *pszLogXml, char *xml);

#ifdef __cplusplus
}
#endif

#endif