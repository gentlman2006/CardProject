#pragma once

#ifndef _BHGX_HOSPPROC_H_
#define _BHGX_HOSPPROC_H_

#define DLL_EXPORT _declspec(dllexport)

#ifdef __cplusplus
extern "C" {
#endif
	DLL_EXPORT int __stdcall iFormatHospInfo();

	DLL_EXPORT int __stdcall iWriteHospInfo(char *xml);

	DLL_EXPORT int __stdcall iReadIdentify(char *xml);

	DLL_EXPORT int __stdcall iReadHumanInfo(char *xml);

	DLL_EXPORT int __stdcall iReadConnInfo(char *xml);

	//������Ϣ�ͷ���������Ϣ
	DLL_EXPORT int __stdcall iReadCardInfo(char *xml);

	DLL_EXPORT int __stdcall iReadHealthInfo(char *xml);

	//����ժҪ
	DLL_EXPORT int __stdcall iReadClinicInfo(char *, char *xml);

	//������ҳ
	DLL_EXPORT int __stdcall iReadMedicalInfo(char *, char *xml);

	//���ý���
	DLL_EXPORT int __stdcall iReadFeeInfo(char *, char *xml);

	DLL_EXPORT int __stdcall iWriteHospInfoLog(char *xml, char *pszLogXml);

	//����ժҪ��־
	DLL_EXPORT int __stdcall iReadClinicInfoLog(char *, char *xml, char *pszLogXml);

	//������ҳ��־
	DLL_EXPORT int __stdcall iReadMedicalInfoLog(char *, char *xml, char *pszLogXml);

	//���ý�����־
	DLL_EXPORT int __stdcall iReadFeeInfoLog(char *, char *xml, char *pszLogXml);


	DLL_EXPORT int __stdcall iWriteHospInfoLocal(char *xml, char *pszLogXml);

	//����ժҪ��־
	DLL_EXPORT int __stdcall iReadClinicInfoLocal(char *, char *xml, char *pszLogXml);

	//������ҳ��־
	DLL_EXPORT int __stdcall iReadMedicalInfoLocal(char *, char *xml, char *pszLogXml);

	//���ý�����־
	DLL_EXPORT int __stdcall iReadFeeInfoLocal(char *, char *xml, char *pszLogXml);

	// HIS only local
	DLL_EXPORT int __stdcall iReadOnlyHospLocal(char *xml, char *pszLogXml);

#ifdef __cplusplus
}
#endif

#endif