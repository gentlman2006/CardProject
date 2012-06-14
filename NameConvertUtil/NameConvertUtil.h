#pragma once

#ifndef _NameConvertUtil_H_
#define _NameConvertUtil_H_

#define DLL_EXPORT _declspec(dllexport)

#ifdef __cplusplus
extern "C" {
#endif
	/*******************************************************************************
	* �������ƣ�	
	* ����������	����ת����������	
	* ���������	strName		-- ����
	* ���������	strCode		-- ������
	*				nLen		-- �����볤��
	* �� �� ֵ��	bool �ɹ� �� false ʧ��	
	*******************************************************************************/
	DLL_EXPORT int __stdcall Name2Code(const char *strName, char *strCode, int *nLen);

	/*******************************************************************************
	* �������ƣ�	
	* ����������	����ת����������	
	* ���������	strCode		-- ������
	* ���������	strName		-- ����
	*				nLen		-- ��������
	* �� �� ֵ��	bool �ɹ� �� false ʧ��	
	*******************************************************************************/
	DLL_EXPORT int __stdcall Code2Name(const char *strCode, char *strName, int *nLen);
#ifdef __cplusplus
}
#endif

#endif	//_CREATE_CARD_H