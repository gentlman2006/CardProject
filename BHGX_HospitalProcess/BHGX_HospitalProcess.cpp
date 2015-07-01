#include <stdlib.h>
#include <iostream>
#include <vector>
#include <map>
#include <windows.h>
#include "Markup.h"
#include "BHGX_HospitalProcess.h"
#include "../BHGX_CardLib/BHGX_CardLib.h"
#include "../BHGX_CardLib/public/liberr.h"
#include "../BHGX_HISReader/BHGX_HISReader.h"

#pragma warning (disable : 4996)

#ifdef _DEBUG
#pragma comment(lib, "../Debug/BHGX_CardLib.lib")
#pragma comment(lib, "../Debug/BHGX_HISReader.lib")
#else
#pragma comment(lib, "../Release/BHGX_CardLib.lib")
#pragma comment(lib, "../Release/BHGX_HISReader.lib")
#endif

using namespace std;

typedef unsigned char BYTE;
struct OrderInfo 
{
	BYTE	order;	//����
	BYTE	bUsed;	//�Ƿ�ռ��
};

struct LocationInfo
{
	int		startID;	//��ʼ��ID
	int		endID;		//������ID
	int		counts;		//����
	int		startPos;	//��ʼλ��
	int		length;		//�ֶγ���
};

#define		MAX_TIMES		20

#define		SEG_IDENTITY		1
#define		SEG_PEPBASEINFO		2
#define		SEG_CARDBASEINFO	3
#define		SEG_HEALTHINFO		4
#define		SEG_CLINICINFO		5
#define		SEG_HOSPINFO		6
#define		SEG_FEEINFO			7

#define BASELEN		1024*12
char	g_BaseBuff[BASELEN];

int			g_SectionRead[8] = {-1, -1, -1, -1, -1, 18, 15, 15};
LocationInfo	g_HospLocaltion[8] = {
	{1, 9, 1, 0, 0},
	{10, 57, 3, 0, 0},
	{58, 69, 4, 0, 0},
	{70, 77, 1, 0, 0},
	{78, 137, 20, 0, 0},
	{138, 147, 1, 0, 0},
	{148, 149, 1, 0, 0},
	{-1, -1, -1, 0, 0},
};

LocationInfo	g_ClinicLocaltion[8] = {
	{1, 6, 1, 0, 0},
	{7, 41, 5, 0, 0},
	{42, 91, 10, 0, 0},
	{92, 131, 5, 0, 0},
	{132, 140, 3, 0, 0},
	{141, 170, 10, 0, 0},
	{171, 172, 1, 0, 0},
	{-1, -1, -1, 0, 0},
};


/************************************************************************/
/*     ������Ϣ                                                         */
/************************************************************************/
#define		CLINIC_BASE		1
#define		SYM_BASE		5
#define		CHECK_BASE		10
#define		MEDICAL_BASE	5
#define		DRUG_BASE		3
#define		CLINICFEE_BASE	10
#define		END_BASE		1

int		g_Clinic[CLINIC_BASE];			//���������Ϣ
int		g_Symptom[SYM_BASE];			//֢״����
int		g_Check[CHECK_BASE];			//�����Ŀ
int		g_Medical[MEDICAL_BASE];		//ҩ����Ϣ
int		g_Drug[DRUG_BASE];				//������Ϣ
int		g_ClinicFee[CLINICFEE_BASE];	//�������
int		g_ClinicEnd[END_BASE];			//�������

/************************************************************************/
/*	סԺ��Ϣ				                                            */
/************************************************************************/
#define		IN_BASE			1
#define		INFO_BASE		3	
#define		INBLOOD_BASE	4
#define		COUNTS_BASE		1
#define		INFEE_BASE		20
#define		ALLFEE_BASE		1
#define		HOSP_END		1

int		g_InBase[IN_BASE];				//סԺ������Ϣ
int		g_InfoBase[INFEE_BASE];			//סԺ���������Ϣ
int		g_InBlood[INBLOOD_BASE];		//��Ѫ��Ϣ
int		g_Counts[COUNTS_BASE];			//ͳ����Ϣ
int		g_ItemFee[INFEE_BASE];			//���������Ϣ
int		g_AllFee[ALLFEE_BASE];			//���Ʒ�����Ϣ
int		g_HospEnd[HOSP_END];			//סԺ����

static int _HospReadInfo(int section, char *pszLogXml, char *xml, bool bLog, bool bLocal, char *pName);

static int __stdcall _geneLog(char *pszLogXml, int rw, char *pName);

int __stdcall iFormatHospInfo()
{
	return iFormatCard();
}

int __stdcall iWriteHospInfo(char *xml)
{
	int status = 0;
	status = iWriteInfo(xml);
	return status;
}

//����ժҪ
int __stdcall iReadClinicInfo(char *pszClinicCode, char *xml)
{
	int status = 0;
	status = _HospReadInfo(SEG_CLINICINFO, NULL, xml, false, false, "");
	return status;
}

//������ҳ
int __stdcall iReadMedicalInfo(char *pszHospCode, char *xml)
{
	int status = 0;
	status = _HospReadInfo(SEG_HOSPINFO, NULL, xml, false, false, "");
	return status;
}

//���ý���
int __stdcall iReadFeeInfo(char *pszClinicCode, char *xml)
{
	int status = 0;
	status = _HospReadInfo(SEG_FEEINFO, NULL, xml, false, false, "");
	return status;
}

static int __stdcall _geneLog(char *pszLogXml, int rw, char *pName)
{
	char logInfo[2048];
	char szQuery[2048];
	memset(szQuery, 0, sizeof(szQuery));
	memset(logInfo, 0 , sizeof(logInfo));
	strcpy(logInfo, pszLogXml);
	int status = iReadInfo(3, szQuery);
	if (status != CardProcSuccess){
		return CardReadErr;
	}
	iGeneLog(logInfo, rw, pName, szQuery);
	return CardProcSuccess;
}

int __stdcall iWriteHospInfoLog(char *xml, char *pszLogXml)
{
	int status = 0;
	status = iWriteInfo(xml);
	if (status != CardProcSuccess) {
		return _geneLog(pszLogXml, 1, "iWriteHospInfoLog");
	}
	return CardProcSuccess;
}

//����ժҪ
int __stdcall iReadClinicInfoLog(char *pszClinicCode, char *xml, char *pszLogXml)
{
	int status = 0;
	status = _HospReadInfo(SEG_CLINICINFO, pszLogXml, xml, true, false, "iReadClinicInfoLog");

	return status;
}

//������ҳ
int __stdcall iReadMedicalInfoLog(char *pszHospCode, char *xml, char *pszLogXml)
{

	int status = _HospReadInfo(SEG_HOSPINFO, pszLogXml, xml, true, false, "iReadMedicalInfoLog");
	return status;
}

//���ý���
int __stdcall iReadFeeInfoLog(char *pszClinicCode, char *xml, char *pszLogXml)
{
	int status = 0;
	status = _HospReadInfo(SEG_FEEINFO, pszLogXml, xml, true, false, "iReadFeeInfoLog");

	return status;
}

int __stdcall iWriteHospInfoLocal(char *xml, char *pszLogXml)
{
	int status = iCheckException(pszLogXml, xml);
	if (status != CardProcSuccess) {
		return status;
	}
	status = iWriteInfo(xml);
	if (status != CardProcSuccess) {
		return _geneLog(pszLogXml, 1, "iWriteHospInfoLocal");
	}
	return CardProcSuccess;
}

//����ժҪ��־
int __stdcall iReadClinicInfoLocal(char *pszClinicCode, char *xml, char *pszLogXml)
{
	int status = 0;
	status = _HospReadInfo(SEG_CLINICINFO, pszLogXml, xml, true, true, "iReadClinicInfoLocal");
	return status;
}

//������ҳ��־
int __stdcall iReadMedicalInfoLocal(char *pszClinicCode, char *xml, char *pszLogXml)
{
	int status = 0;
	status = _HospReadInfo(SEG_HOSPINFO, pszLogXml, xml, true, true, "iReadMedicalInfoLocal");
	return status;
}

//���ý�����־
int __stdcall iReadFeeInfoLocal(char *pszClinicCode, char *xml, char *pszLogXml)
{
	int status = 0;
	status = _HospReadInfo(SEG_FEEINFO, pszLogXml, xml, true, true, "iReadFeeInfoLocal");
	return status;
}

int __stdcall iWriteHospInfoOnlyLog(char *xml, char *pszLogXml)
{
	int status = iWriteInfo(xml);
	if (status != CardProcSuccess) {
		return _geneLog(pszLogXml, 1, "iWriteHospInfoOnlyLog");
	}
	return CardProcSuccess;
}

//����ժҪ��־
int __stdcall iReadClinicInfoOnlyLog(char *, char *xml, char *pszLogXml)
{
	int status = 0;
	status = _HospReadInfo(SEG_CLINICINFO, pszLogXml, xml, true, false, "iReadClinicInfoOnlyLog");
	return status;
}

//������ҳ��־
int __stdcall iReadMedicalInfoOnlyLog(char *, char *xml, char *pszLogXml)
{
	int status = 0;
	status = _HospReadInfo(SEG_HOSPINFO, pszLogXml, xml, true, false, "iReadMedicalInfoOnlyLog");
	return status;
}

//���ý�����־
int __stdcall iReadFeeInfoOnlyLog(char *, char *xml, char *pszLogXml)
{
	int status = 0;
	status = _HospReadInfo(SEG_FEEINFO,pszLogXml, xml, true, false, "iReadFeeInfoOnlyLog");
	return status;
}

int __stdcall iReadIdentify(char *xml)
{
	return 0;
}

int __stdcall iReadHumanInfo(char *xml)
{
	return 0;
}

int __stdcall iReadConnInfo(char *xml)
{
	return 0;
}

//������Ϣ�ͷ���������Ϣ
int __stdcall iReadCardInfo(char *xml)
{
	return 0;
}

int __stdcall iReadHealthInfo(char *xml)
{
	return 0;
}

int __stdcall iReadOnlyHospLocal(char *xml, char *pszLogXml)
{
	return iReadOnlyHISLocal(pszLogXml, xml);
}

/************************************************************************/
/*	 �жϽڵ��Ƿ���ڣ������ڵĻ���ɾ����
/*   ���ò�ͬ�ĵ��ȷ�������ȡ��ͬid��λ�ã�
/*   ��ǰ���û�ȡ��һ����Ϣ                                             
/* 
/************************************************************************/
static bool isExist(int id, const LocationInfo *location, int len)
{
	for (int i=0; i<len; i++)
	{
		const LocationInfo pLocal = location[i];
		if (pLocal.startID == -1) {
			continue;
		}
		int counts = pLocal.counts;

		//С�ڿ�ʼID������
		if (id < pLocal.startID) {
			break;
		}

		//�ҵ������еĵ�һ�н���ID
		int endPos = pLocal.startID + (pLocal.endID - pLocal.startID + 1)/counts - 1;
		if (id <= endPos) {
			return true;
		}
	}
	return false;
}

//���ཻ��positon��extraLocation��remove��
static int _doIntersect(const LocationInfo *location, 
						 int len, 
						 int *extraLoaction,
						 int extraLen)
{
	return 0;
}

//�ж�id�Ƿ�����extraLocation��
static bool _isExtra(int id, const int *extraLoaction, int extraLen)
{
	if (extraLoaction == NULL || extraLen == -1) {
		return false;
	}

	for (int i=0; i<extraLen; i++)
	{
		if (extraLoaction[i] == -1) {
			return false;
		}

		if (extraLoaction[i] == id) {
			return true;
		}
	}
	return false;
}


static int _ParseSegXml(const char *src, 
						const LocationInfo *location, 
						int len, 
						int *extraLoaction,
						int extraLen,
						char *dst)
{
	CMarkup xml;
	xml.SetDoc(src);						
	if (!xml.FindElem("SEGMENTS")){		
		return -1;								
	}										
	xml.IntoElem();						
	if (! xml.FindElem("SEGMENT")){		
		return -1;								
	}										
	xml.IntoElem();
	_doIntersect(location, len, extraLoaction, extraLen);
	while (xml.FindElem("COLUMN"))
	{
		int id = atoi(xml.GetAttrib("ID").c_str());
		if (_isExtra(id, extraLoaction, extraLen)) {
			continue;
		}

		if (!isExist(id, location, len)) {
			xml.RemoveElem();
		}
	}
	xml.OutOfElem();						
	xml.OutOfElem();
	strcpy(dst, xml.GetDoc().c_str());
	return 0;
}


static int _HospReadInfo(int section, char *pszLogXml, char *xml, bool bLog, bool bLocal, char *pName)
{
	int status = 0;
	memset(g_BaseBuff, 0, BASELEN);
	int extraLocation[8];
	memset(extraLocation, -1, sizeof(extraLocation));
	
	if (bLocal) {
		int status = iCheckException(pszLogXml, xml);
		if (status != CardProcSuccess) {
			return status;
		}
	}

	//����sectionȥ��ȡ��ȡ�ֶε�flag��Ϣ
	int flag = g_SectionRead[section];
	if (section < 0) {
		return -1;
	} 

	status = iReadInfo(0x1<<(flag-1), g_BaseBuff);
	if (status) {
		strcpy(xml, err(status));
		return status;
	}

	switch (section)
	{
	case SEG_IDENTITY:                
		break;
	case SEG_PEPBASEINFO:
		break;
	case SEG_CARDBASEINFO:
		break;
	case SEG_HEALTHINFO:
		break;
	case SEG_CLINICINFO:
		status = _ParseSegXml(g_BaseBuff, g_ClinicLocaltion, 6, NULL, -1, xml);
		break;
	case SEG_HOSPINFO:
		status = _ParseSegXml(g_BaseBuff, g_HospLocaltion, 5, NULL, -1, xml);
		break;
	case SEG_FEEINFO:
		extraLocation[0] = 3;
		extraLocation[1] = 5;
		extraLocation[2] = 72;
		status = _ParseSegXml(g_BaseBuff, g_HospLocaltion + 5, 1, 
			extraLocation, sizeof(extraLocation), xml);
		break;
	default:
		break;
	}

	if (bLog) {
		return _geneLog(pszLogXml, 0, pName);

	}
	return status;
}	

static int ReadFile(char *filename, char **xml)
{
	FILE *handle;
	int size;

	fopen_s(&handle, filename, "r");

	if (NULL == handle)
		return -1;
	fseek(handle, 0, SEEK_END);
	size = ftell(handle) + 1;
	rewind(handle);
	*xml = (char *) malloc(size);
	memset(*xml, 0, size);
	fread(*xml,size,1,handle);
	fclose(handle);

	return size;
}

typedef void (*procfunc)();

struct procItem {
	int index;
	procfunc func;
};

char szError[64];

void procCreateGWCard()
{
	char szRead[2048*6];
	memset(szError, 0, 64);
	memset(szRead, 0, sizeof(szRead));
	//int nStart = GetTickCount();
	int ret = iReadMedicalInfo("", szRead);
	//int nEnd = GetTickCount();
	//printf("iReadMedicalInfo: %d����ʱ��: %d \n", ret, nEnd - nStart);
	if (ret != 0){
		strcpy(szError, err(ret));
		printf("%s\n", szError);
	}
	else{
		printf("%s\n", szRead);
	}
}

void procCreateNHCard()
{
	char szRead[2048*6];
	memset(szError, 0, 64);
	memset(szRead, 0, sizeof(szRead));
	//int nStart = GetTickCount();
	int ret = iReadFeeInfo("", szRead);
	//int nEnd = GetTickCount();
	//printf("iReadFeeInfo: %d����ʱ��: %d \n", ret, nEnd - nStart);
	if (ret != 0){
		strcpy(szError, err(ret));
		printf("%s\n", szError);
	} else {
		printf("%s\n", szRead);
	}
}

void procRead()
{
	char szRead[2048*6];
	memset(szError, 0, 64);
	memset(szRead, 0, sizeof(szRead));
	//int nStart = GetTickCount();
	int ret = iReadClinicInfo("", szRead);
	//int nEnd = GetTickCount();
	//printf("iReadClinicInfo: %d����ʱ��: %d \n", ret, nEnd - nStart);
	if (ret != 0){
		strcpy(szError, err(ret));
		printf("%s\n", szError);
	}else{
		printf("%s\n", szRead);
	}
}

void procWrite()
{
	char *buf;
	char file[256];
	memset(szError, 0, sizeof(szError));
	cout<<"������д���ļ���"<<endl;
	cin>>file;
	int ret = ReadFile(file, &buf);
	if (ret == -1) {
		printf("ũ��д������.xml�ļ�������\n");
		return;
	}
	ret = iWriteHospInfo(buf);
	printf("д����:%d\n", ret);
	if (ret != 0)
	{
		strcpy(szError, err(ret));
		printf("%s\n", szError);
	}
	free(buf);
}

void procFormate()
{
		//int nStart = GetTickCount();
		int ret = iFormatCard();
		if (ret != 0){
			strcpy(szError, err(ret));
			printf("%s\n", szError);
		} else {
			printf("format�ɹ�\n");
		}
}

void procWriteHospInfoLog()
{
	char *buf, *configBuffer;
	char file[256];
	memset(szError, 0, sizeof(szError));
	cout<<"������д���ļ���"<<endl;
	cin>>file;
	int ret = ReadFile(file, &buf);
	if (ret == -1) {
		printf("ũ��д������.xml�ļ�������\n");
		return;
	}
	ret = ReadFile("��־������ʽ.xml", &configBuffer);
	if (ret == -1) {
		cout << "��־������ʽ.xml������" <<endl;
		return;
	}
	ret = iWriteHospInfoLocal(buf, configBuffer);
	printf("д����:%d\n", ret);
	if (ret != 0)
	{
		strcpy(szError, err(ret));
		printf("%s\n", szError);
	}
	free(buf);
}

void procReadMedicalInfoLog()
{
	char szRead[2048*6];
	memset(szError, 0, 64);
	memset(szRead, 0, sizeof(szRead));
	char *buf;
	int ret = ReadFile("��־������ʽ.xml", &buf);
	if (ret == -1) {
		cout << "��־������ʽ.xml������" <<endl;
		return;
	}
	int nStart = GetTickCount();
	ret = iReadMedicalInfoOnlyLog("", szRead, buf);
	int nEnd = GetTickCount();
	printf("iReadClinicInfo: %d����ʱ��: %d \n", ret, nEnd - nStart);
	if (ret != 0){
		strcpy(szError, err(ret));
		printf("%s\n", szError);
	}else{
		printf("%s\n", szRead);
	}
	free(buf);
}

void procReadFeeInfoLog()
{
	char szRead[2048*6];
	memset(szError, 0, 64);
	memset(szRead, 0, sizeof(szRead));
	char *buf;
	int ret = ReadFile("��־������ʽ.xml", &buf);
	if (ret == -1) {
		cout << "��־������ʽ.xml������" <<endl;
		return;
	}
	//int nStart = GetTickCount();
	ret = iReadFeeInfoLog("", szRead, buf);
	//int nEnd = GetTickCount();
	//printf("iReadClinicInfo: %d����ʱ��: %d \n", ret, nEnd - nStart);
	if (ret != 0){
		strcpy(szError, err(ret));
		printf("%s\n", szError);
	}else{
		printf("%s\n", szRead);
	}
	free(buf);
}	

void procReadClinicInfoLog()
{
	char szRead[2048*6];
	memset(szError, 0, 64);
	memset(szRead, 0, sizeof(szRead));
	char *buf;
	int ret = ReadFile("��־������ʽ.xml", &buf);
	if (ret == -1) {
		cout << "��־������ʽ.xml������" <<endl;
		return;
	}
	//int nStart = GetTickCount();
	ret = iReadClinicInfoLog("", szRead, buf);
	//int nEnd = GetTickCount();
	//printf("iReadClinicInfo: %d����ʱ��: %d \n", ret, nEnd - nStart);
	if (ret != 0){
		strcpy(szError, err(ret));
		printf("%s\n", szError);
	}else{
		printf("%s\n", szRead);
	}
	free(buf);
}

void procReadClinicInfoLocal()
{
	char szRead[2048*6];
	memset(szError, 0, 64);
	memset(szRead, 0, sizeof(szRead));
	char *buf;
	int ret = ReadFile("��־������ʽ.xml", &buf);
	if (ret == -1) {
		cout << "��־������ʽ.xml������" <<endl;
		return;
	}
	//int nStart = GetTickCount();
	ret = iReadClinicInfoLocal("", szRead, buf);
	//int nEnd = GetTickCount();
	//printf("iReadClinicInfo: %d����ʱ��: %d \n", ret, nEnd - nStart);
	if (ret != 0){
		strcpy(szError, err(ret));
		printf("%s\n", szError);
	}else{
		printf("%s\n", szRead);
	}
	free(buf);
}

void printHelp()
{
	cout<<"*************ҽԺ����*********************"<<endl;
	cout<<"���ܲ��ԣ�\n\t1:iReadMedicalInfo\n\t2:iReadFeeInfo\n\t3:iReadClinicInfo"<<endl;
	cout<<"\t4:iWriteHospInfo\n\t5:FormateCard\n\t6:iWriteHospInfoLog"<<endl;
	cout<<"\t7:iReadMedicalInfoLog\n\t8:iReadFeeInfoLog\n\t9:iReadClinicInfoLog"<<endl;
	cout<<"\t10:iReadClinicInfoLocal"<<endl;
}

procItem procMethods[] = {
	{0, procCreateGWCard},
	{1, procCreateNHCard},
	{2, procRead},
	{3, procWrite},
	{4, procFormate},
	{5, procWriteHospInfoLog},
	{6, procReadMedicalInfoLog},
	{7, procReadFeeInfoLog},
	{8, procReadClinicInfoLog},
	{9, procReadClinicInfoLocal},
	{10, NULL},
};


int main(int argc, char **argv)
{

	printHelp();
	char xml[BASELEN];
	memset(xml, 0, BASELEN);
	int status = 0;
	
	char szSelect[10];
	cout<<"�����빦�ܲ���ѡ��:";
	cin>>szSelect;
	std::string strSelect = szSelect;
	int ret = iCardInit();
	if (ret != 0){
		cout<<"����ʼ��ʧ��:"<<ret<<endl;
		return -1;
	}
	while (1)
	{
		if (strSelect == "q" || strSelect == "quit")
			break;

		int nSelect = atoi(szSelect);
		nSelect -- ;
		if (nSelect < 0 || 
			nSelect > sizeof(procMethods)/sizeof(struct procItem) ) {
				printHelp();
		} else {
			if (procMethods[nSelect].func != NULL) {
				procMethods[nSelect].func();
			}
		}

		memset(szSelect, 0 , sizeof(szSelect));
		cout<<"\n�����빦�ܲ���ѡ��:";
		cin>>szSelect;
		strSelect = szSelect;
	}
	iCardDeinit();
	
	getchar();
}