#pragma once

#ifndef _DEVICE_BHGX_MIFARE_H_
#define _DEVICE_BHGX_MIFARE_H_

#define DLL_EXPORT _declspec(dllexport)

#ifdef  __cplusplus
extern "C" {
#endif
	// ̽���Լ��豸
	DLL_EXPORT  int __stdcall bProbe(void);

	// ���豸
	DLL_EXPORT int __stdcall iOpen(void);

	// �ر��豸
	DLL_EXPORT int __stdcall iClose(void);

	// ��ɨ������
	DLL_EXPORT int __stdcall iScanCard(void);

	// �豸��������
	DLL_EXPORT int __stdcall iIOCtl(int cmd, void *);

	// ���豸����
	DLL_EXPORT int __stdcall iRead(const unsigned char *key, unsigned char *buf, int iSizeInBits, int iOffsetInBits);

	// д�豸����
	DLL_EXPORT int __stdcall iWrite(
		const unsigned char *key, 
		unsigned char *buf,
		int nCtrlWord,
		int iSizeInBits, 
		int iOffsetInBits
		);



	//�޸��û�Ȩ�ޣ�����������keya �� keyb ��ֵ��
	DLL_EXPORT int  __stdcall iChangePwdEx(const unsigned char * pNewKeyA ,const unsigned char * ctrlword,
		const unsigned char * pNewKeyB,const unsigned char * poldPin,unsigned char nsector
		,unsigned char keyA1B0,unsigned char changeflag);



#ifdef  __cplusplus
};
#endif

#endif	// _DEVICE_MIFARE_H