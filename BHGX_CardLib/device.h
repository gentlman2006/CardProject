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

typedef int (__stdcall *DllProbe)();
typedef int (__stdcall *DllOpen)(void);
typedef int (__stdcall *DllClose)(void);
typedef int (__stdcall *DllScanCard)(void);
typedef unsigned char (__stdcall *DLLChangePwdEx)(const unsigned char * pNewKeyA ,const unsigned char * ctrlword,\
												  const unsigned char * pNewKeyB,const unsigned char * poldPin ,\
												  unsigned char nsector,unsigned char keyA1B0,unsigned char changeflag);

typedef int (__stdcall *DLLIOCtl)(int, void *,int);
typedef int (__stdcall *DLLICCSet)(unsigned char , unsigned char*, unsigned char*);
typedef int (__stdcall *DLLGetDevAuthGene)(unsigned char *);
typedef int (__stdcall *DLLDevAuthSys)(unsigned char *);
typedef int (__stdcall *DLLSysAuthDev)(unsigned char *, unsigned char *);
typedef int (__stdcall *DLLGetRandom)(unsigned char, unsigned char, unsigned char *);
typedef int (__stdcall *DLLSelectFile)(unsigned char, unsigned char *);
typedef int (__stdcall *DLLSysAuthUCard)(unsigned char *, unsigned char *, unsigned char *);
typedef int (__stdcall *DllReadBin)(unsigned char ,unsigned char *, unsigned char *, int, int);
typedef int (__stdcall *DllWriteBin)(unsigned char, unsigned char *, unsigned char *,unsigned char , int ,int);
typedef int (__stdcall *DllReadRec)(unsigned char ,unsigned char *, unsigned char *, unsigned long, int, int);

// 16/32 merge add unsigned char iSAMSeat

typedef int (__stdcall *DllWriteRec)(unsigned char, unsigned char, unsigned char *, unsigned char *, unsigned long ,int, int);

//add iCardType and iSAMSeat
typedef int (__stdcall *DLLUCardAuthSys)(unsigned char , unsigned char, int);
typedef int (_stdcall  *DllAppendRec)(unsigned char , unsigned char, unsigned char *, unsigned char *, unsigned long);


typedef int (_stdcall  *DllSignRec)(unsigned char iCardType, 
									unsigned char iSAMSeat,
									unsigned char *pFID, 
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
	int         type;

	DllProbe			iProbe;
	DllOpen				iOpen;
	DllClose			iClose;
	DllScanCard			iScanCard;
	DLLIOCtl			iIOCtl;
	DLLChangePwdEx		iChangePwdEx;
	DLLICCSet			ICCSet;
	DLLGetDevAuthGene	iGetDevAuthGene;
	DLLDevAuthSys		iDevAuthSys;
	DLLSysAuthDev		iSysAuthDev;
	DLLGetRandom		iGetRandom;
	DLLSelectFile		iSelectFile;
	DLLSysAuthUCard		iSysAuthUCard;
	DLLUCardAuthSys     iUCardAuthSys;
	DllReadRec			iReadRec;
	DllWriteRec			iWriteRec;
	DllReadBin			iReadBin;
	DllWriteBin			iWriteBin;
	DllAppendRec        iAppendRec;
	DllSignRec          iSignRec;
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


#endif	//ACC_DEVICE_H