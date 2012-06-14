#pragma once
#ifndef _VIRT_CARD_H_
#define _VIRT_CARD_H_

#define DLL_EXPORT _declspec(dllexport)

#ifdef  __cplusplus
extern "C" {
#endif
	// ̽���Լ��豸
	DLL_EXPORT int __stdcall bProbe(void);

	// ���豸
	DLL_EXPORT int __stdcall iOpen(void);

	// �ر��豸
	DLL_EXPORT int __stdcall iClose(void);

	// ��ɨ������
	DLL_EXPORT int __stdcall iScanCard(void);

	// �豸��������
	DLL_EXPORT int __stdcall iIOCtl(int cmd, void *);

	// ���豸����
	//DLL_EXPORT int iRead(unsigned char *key, unsigned char *buf, int iSizeInBits, int iOffsetInBits);
	DLL_EXPORT int __stdcall iRead(const unsigned char *key, unsigned char *buf, int iSizeInBits, int iOffsetInBits);

	// д�豸����
	//DLL_EXPORT int iWrite(unsigned char *key, unsigned char *buf, int iSizeInBits, int iOffsetInBits);
	DLL_EXPORT int __stdcall iWrite(const unsigned char *key, unsigned char *buf, int iSizeInBits, int iOffsetInBits);

#ifdef  __cplusplus
};
#endif

#endif	// _VIRT_CARD_H_
