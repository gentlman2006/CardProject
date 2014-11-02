#ifndef CPUCARD_H
#define CPUCARD_H
#include "Card.h"

#ifdef  __cplusplus
extern "C" {
#endif

//���ڼ�¼�ļ�������
struct RecFolder{
	BYTE	section[10];      //DDF
	BYTE    subSection[10];   //DF
	BYTE	fileName[10];     //EF, ED....
};
 
adapter* __stdcall InitCpuAdapter();

int __stdcall FormatCpuCard(char c);

int __stdcall get_sec_counts(int sec);

#ifdef  __cplusplus
}
#endif
#endif