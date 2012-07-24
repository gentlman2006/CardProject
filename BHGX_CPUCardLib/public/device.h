#pragma once

#ifndef CPU_DEVICE_H
#define CPU_DEVICE_H

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
typedef int (__stdcall *DLLIOCtl)(int, void *,int);
typedef int (__stdcall *DLLICCSet)(unsigned char , unsigned char*, unsigned char*);
typedef int (__stdcall *DLLGetDevAuthGene)(unsigned char *);
typedef int (__stdcall *DLLDevAuthSys)(unsigned char *);
typedef int (__stdcall *DLLSysAuthDev)(unsigned char *, unsigned char *);
typedef int (__stdcall *DLLGetRandom)(unsigned char, unsigned char, unsigned char *);
typedef int (__stdcall *DLLSelectFile)(unsigned char, unsigned char *);
typedef int (__stdcall *DLLSysAuthUCard)(unsigned char *, unsigned char *, unsigned char *);
typedef int (__stdcall *DLLUCardAuthSys)(int);
typedef int (__stdcall *DllReadBin)(unsigned char ,unsigned char *, unsigned char *, int, int);
typedef int (__stdcall *DllWriteBin)(unsigned char, unsigned char *, unsigned char *, int ,int);
typedef int (__stdcall *DllReadRec)(unsigned char ,unsigned char *, unsigned char *, int, int, int);
typedef int (__stdcall *DllWriteRec)(unsigned char, unsigned char *, unsigned char *, int ,int,int);

//add at 0624

typedef int (_stdcall  *DllAppendRec)(unsigned char *, unsigned char *, unsigned long);


typedef int (_stdcall  *DllSignRec)(unsigned char *pFID, 
								 int		iRecNo,
								 int		sign );

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

	DllProbe			iProbe;
	DllOpen				iOpen;
	DllClose			iClose;
	DLLIOCtl			iIOCtl;
	DLLICCSet			ICCSet;
	//DLLGetDevAuthGene	iGetDevAuthGene;
	//DLLDevAuthSys		iDevAuthSys;
	//DLLSysAuthDev		iSysAuthDev;
	DLLGetRandom		iGetRandom;
	DLLSelectFile		iSelectFile;
	//DLLSysAuthUCard		iSysAuthUCard;
	DLLUCardAuthSys     iUCardAuthSys;
	DllReadRec			iReadRec;
	DllWriteRec			iWriteRec;
	DllReadBin			iReadBin;
	DllWriteBin			iWriteBin;
	DllAppendRec        iAppendRec;
	DllSignRec          iSignRec;
};

#ifdef  __cplusplus
extern "C" {
#endif
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

#ifdef  __cplusplus
};
#endif

#endif	//ACC_DEVICE_H