#pragma once

#ifndef ACC_DEVICE_H
#define ACC_DEVICE_H


#include <windows.h>
     
/**
 *
 */
enum {
	CMD_CHANGCARD,
	CMD_SET_READKEY,
	CMD_GET_READKEY,
	CMD_SET_WRITEKEY,
	CMD_GET_WRITEKEY,
	CMD_MAX
};

typedef int (__stdcall *DllProbe)(void);
typedef int (__stdcall *DllOpen)(void);
typedef int (__stdcall *DllClose)(void);

typedef BOOL (__stdcall *DllAuthUDev)(void);
typedef int (__stdcall *DllScanCard)(void);
typedef int (__stdcall *DLLIOCtl)(int, void *);
typedef int (__stdcall *DllRead)(const unsigned char *, unsigned char *, int, int);
typedef int (__stdcall *DllWrite)(const unsigned char *, unsigned char *, int ,int, int);
typedef unsigned char (__stdcall *DLLChangePwdEx)(const unsigned char * pNewKeyA ,const unsigned char * ctrlword,
												  const unsigned char * pNewKeyB,const unsigned char * poldPin ,
												  unsigned char nsector,unsigned char keyA1B0,unsigned char changeflag);



/**
 * strurct 
 *
 * @name			���豸����
 * @hInstLibrary	�򿪶�̬����
 *
 * @iProbe			̽���豸����
 * @iOpen			���豸����
 * @iClose			�ر��豸����
 * @iScanCard		ɨ�迨����
 * @iRead			�豸������
 * @iWrite			�豸д����
 */
struct CardDevice
{
	HINSTANCE	hInstLibrary;

	DllProbe	iProbe;
	DllOpen		iOpen;
	DllClose	iClose;
	DllScanCard	iScanCard;
	DLLIOCtl	iIOCtl;
	DllRead		iRead;
	DllWrite	iWrite;
	DLLChangePwdEx  iChangePwdEx;
};


struct CardAuth 
{
	HINSTANCE	hAuthLibrary;
	DllAuthUDev iAuthUDev;
};
/*
 * �ɹ��� ���س����豸
 * ʧ�ܣ� NULL
 */
struct CardDevice* getCardDevice(const char *System);

/**
 * Func: putCardDevice
 * @device ����Ŀ��豸
 *
 *Return:
 * �ɹ��� 0
 * ʧ�ܣ� ����
 */
int putCardDevice(struct CardDevice *device);


//U����֤
int authUDev(const char *System);


#endif	//ACC_DEVICE_H