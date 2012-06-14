#pragma once

#ifndef _BHGX_CREATE_CARD_H_
#define _BHGX_CREATE_CARD_H_

#define DLL_EXPORT _declspec(dllexport)

#ifdef __cplusplus
extern "C" {
#endif
	/*******************************************************************************
	* �������ƣ�	iCreateCardData	
	* ����������	filename	�ƿ���Ϣ�ı��ļ�
	*				license		����Ϊ"��������.license"
					datafile	���ɿ����� Ĭ��"card.data"
					infofile	���࿨����
	* ���������	
	* ���������	
	* �� �� ֵ��	bool �ɹ� �� false ʧ��	
	*******************************************************************************/
	DLL_EXPORT int __stdcall iCreateCardData( 
							const char *filename, 
							const char *license,
							const char *datafile = "card.data", 
							const char *infofile = "cardpage.data" 
							);

	DLL_EXPORT int __stdcall iCreatePubSysCardData( 
							const char *filename, 
							const char *license,
							const char *datafile = "card.data", 
							const char *infofile = "cardpage.data" 
							);
#ifdef __cplusplus
}
#endif

#endif	//_CREATE_CARD_H