// CardProject.cpp : 定义控制台应用程序的入口点。
//

#include <stdio.h>
#include <stdarg.h>
#include "stdafx.h"
#include <iostream>
#include <string>
#include <windows.h>
#include <vector>
#include "ConvertUtil.h"
#include <fstream>
#include <map>
#include <time.h>
#include <stdlib.h>
#include "tinyxml/headers/tinyxml.h"
#include "Markup.h"


#define CPU_CARD 0
#define M1_CARD  1

#include "../BHGX_HISReader/BHGX_HISReader.h"
#include "../BHGX_CardLib/BHGX_CardLib.h"
#include "../BHGX_HospitalProcess/BHGX_HospitalProcess.h"

#ifdef _DEBUG
#pragma comment(lib, "../debug/BHGX_CardLib.lib")
#pragma comment(lib, "../debug/BHGX_HISReader.lib")
#pragma comment(lib, "../debug/BHGX_HospitalProcess.lib")
#else
#pragma comment(lib, "../release/BHGX_HISReader.lib")
#pragma comment(lib, "../release/BHGX_CardLib.lib")
#pragma comment(lib, "../release/BHGX_HospitalProcess.lib")
#endif

#pragma comment(lib, "tinyxml/libs/tinyxmld.lib")
#pragma warning (disable : 4996)
using namespace std;

typedef void (*procfunc)();

struct procItem {
	int index;
	procfunc func;
	bool bNeedInit;
};

#define SAFE_DELETE(a)	\
	if (a != NULL)		\
{						\
	delete a;			\
	a = NULL;			\
}						\

static int WriteFile(char *filename, char *xml)
{
	FILE *handle;

	fopen_s(&handle, filename, "w");
	fwrite(xml, strlen(xml), 1, handle);
	fclose(handle);

	return 0;
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

static void DbgMem(unsigned char *mem, int size)
{
	int i = 0;
	for(i=0; i< size; i++)
		printf("%02x ", mem[i]);
	printf("\n");
}


char szError[64];

char NH_URL[] = "http://222.84.227.235:8080/nh_webservice/nhcardWebService?wsdl";

char CARD_URL[] = "http://222.84.227.237:8081/ccss/ws/oneCardWebService?wsdl";

#define MAX_BUFF_SIZE   (1024 * 24)
void procCreateGWCard()
{
	char szRead[MAX_BUFF_SIZE];
	memset(szRead, 0, sizeof(szRead));
	char *buf;
	int ret = ReadFile("log.xml", &buf);
	if (ret == -1) {
		printf("log.xml文件不存在\n");
		return;
	}
	int nStart = GetTickCount();
	ret = iReadOnlyHISLocal(buf, szRead);
	int nEnd = GetTickCount();
	printf("iReadAll: %d所需时间: %d \n", ret, nEnd - nStart);
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
	char *buf;
	char szRead[2048];
	memset(szRead, 0, sizeof(szRead));
	int index = 0;
	int ret = ReadFile("log.xml", &buf);
	if (ret == -1) {
		printf("log.xml文件不存在\n");
		return;
	}
	int nStart = GetTickCount();
	ret = iCheckException(buf, szRead);
	int nEnd = GetTickCount();
	printf("iCheckException: %d所需时间: %d \n", ret, nEnd - nStart);

	if (ret != 0){
		strcpy(szError, err(ret));
		printf("%s\n", szError);
	}
	else{
		printf("%s\n", szRead);
	}
}

void procRead() 
{
	char szRead[2048*6];
	memset(szError, 0, 64);
	memset(szRead, 0, sizeof(szRead));
	int index = 0, ret;
	cout<<"请选择读的区域,从第1块开始:"<<endl;
	cin>>index;
	int nStart = GetTickCount();
	ret = iReadInfo(0x1<<(index - 1), szRead);
	int nEnd = GetTickCount();
	printf("iReadInfo: %d所需时间: %d \n", ret, nEnd - nStart);
	if (ret != 0){
		strcpy(szError, err(ret));
		printf("%s\n", szError);
	}
	else{
		printf("%s\n", szRead);
	}
}

void procWrite()
{
	char *buf;
	char file[256];
	memset(szError, 0, sizeof(szError));
	//cout<<"请输入写卡文件："<<endl;
	//cin>>file;
	int ret = ReadFile("农合写卡数据.xml", &buf);
	if (ret == -1) {
		printf("农合写卡数据.xml文件不存在\n");
		return;
	}
	int nStart = GetTickCount();
	ret = iWriteInfo(buf);
	int nEnd = GetTickCount();
	printf("iWriteInfo: %d所需时间: %d ->", ret, nEnd - nStart);
	printf("写操作:%d\n", ret);
	if (ret != 0)
	{
		strcpy(szError, err(ret));
		printf("%s\n", szError);
	}
	free(buf);
}

void procFormat()
{
	int nStart = GetTickCount();
	int ret = iFormatCard();
	int nEnd = GetTickCount();
	printf("iFormatCard: %d所需时间: %d ", ret, nEnd - nStart);
	if (ret != 0)
	{
		strcpy(szError, err(ret));
		printf("%s\n", szError);
	}
}

void procCreate()
{
	char *CreateXML;
	cout<<"制卡数据默认文件CreateCard.xml"<<endl;
	int ret = ReadFile("农合制卡数据.xml", &CreateXML);
	if (ret == -1) {
		printf("农合制卡数据.xml文件不存在\n");
		return;
	}
	printf("*************开始制卡*************\n");

	int nStart = GetTickCount();
	ret = iCreateCard(CreateXML);

	int nEnd = GetTickCount();
	printf("iCreateCard: %d所需时间: %d\n ", ret, nEnd - nStart);
	if (ret != 0)
	{
		strcpy(szError, err(ret));
		printf("%s\n", szError);
	}
	free(CreateXML);
}

void procReadHIS()
{
	char szRead[2048 * 4];
	char strCheckWSDL[100], strURL[100];
	memset(szRead, 0, sizeof(szRead));
	char *buff;
	int nStart = GetTickCount();
	int ret = iReadOnlyHIS(szRead);
	int nEnd = GetTickCount();
	
	printf("iCreateCard: %d所需时间: %d\n ", ret, nEnd - nStart);
	if (ret != 0){
		strcpy(szError, err(ret));
		cout << szError <<endl;
	} else {
		cout<<szRead<<endl;
	}
}

void procRegNHMessage()
{
	char szRead[2048 * 2];
	memset(szRead, 0, sizeof(szRead));
	iReadCardMessageForNH(NH_URL, CARD_URL, szRead);	
	printf("%s", szRead);
}

void procPatchCard()
{
	char *CreateXML, *buf, *readBuf;
	char szPrinter[100];

	cout<<"制卡数据默认文件CreateCard.xml"<<endl;
	int ret = ReadFile("CreateCard.xml", &CreateXML);
	if (ret == -1) {
		printf("CreateCard.xml文件不存在\n");
		return;
	}

	cout<<"卡面打印数据默认文件cardprintData.xml"<<endl;
	ret = ReadFile("cardprintData.xml", &buf);
	if (ret == -1) {
		printf("cardprintData.xml文件不存在\n");
		return;
	}

	cout<<"卡面风格数据默认文件cardprint_652822.xml"<<endl;
	ret = ReadFile("cardprint_652822.xml", &readBuf);
	if (ret == -1) {
		printf("cardprint_652822.xml文件不存在\n");
		return;
	}

	printf("*************开始制卡*************\n");
	iGetPrinterList(szPrinter);
	printf("探测的打印机列表%s:\n", szPrinter);
	printf("***********************************\n");

	for(int i=0; i<4; ++i)
	{
		int nStart = GetTickCount();
		ret = iPatchCard(CreateXML,buf,"Zebra P330i Card Printer USB", readBuf);

		int nEnd = GetTickCount();
		printf("PatchCard: %d    所需时间: %d \n", ret, nEnd - nStart);
	}
	free(buf);
	free(readBuf);
	free(CreateXML);
}

int get_printer_list(char *pinters, std::vector<std::string> &vecPrinter) {
	CMarkup xml;
	xml.SetDoc(pinters);
	if (!xml.FindElem("PrinterList")){
		return -1;
	}
	xml.IntoElem();
	while ( xml.FindElem("PrinterName")){
		vecPrinter.push_back(xml.GetAttrib("NAME"));
	}
	xml.OutOfElem();
	return vecPrinter.size();
}

void procPrintCard()
{
	char printers[1024];
	std::vector<std::string> vecPrinter;
	int size = iGetPrinterList(printers);
	if (size < 0) {
		cout << "无法获取打印机列表" << endl;
		return;
	}

	get_printer_list(printers, vecPrinter);
	for (int i=0 ; i < vecPrinter.size(); i++) {
		cout << i+1 << ":\t" << vecPrinter[i] << endl;
	}
	cout << "请输入打印机编号[1|2..]" <<endl;
	int printIndex;
	cin >> printIndex;

	char *buf, *readBuf;
	cout<<"卡面打印数据默认文件cardprintData.xml"<<endl;
	int ret = ReadFile("cardprintData.xml", &buf);
	if (ret == -1) {
		printf("cardprintData.xml文件不存在\n");
		return;
	}

	cout<<"卡面风格数据默认文件cardprint_652822.xml"<<endl;
	ret = ReadFile("cardprint_652822.xml", &readBuf);
	if (ret == -1) {
		printf("cardprint_652822.xml文件不存在\n");
		return;
	}
	printf("*************开始打印卡*************\n");

	printf("***********************************\n");
	//for(int i=0; i<vecPrinter.size(); ++i)
	//{
	int nStart = GetTickCount();
	ret = iPrintCard("", buf, readBuf);
	int nEnd = GetTickCount();
	printf("iPrintCard: %d    所需时间: %d \n", ret, nEnd - nStart);
	//}
	free(buf);
	free(readBuf);
}

void procIsEmpty()
{
	int ret = iCardIsEmpty();
	if (ret != 0)
	{
		strcpy(szError, err(ret));
		printf("%s\n", szError);
	}
}

void procClinicInfo()
{
	char szRead[2048 * 4];
	memset(szRead, 0, sizeof(szRead));
	int nStart = GetTickCount();
	int status = iReadClinicInfo("", szRead);
	int nEnd = GetTickCount();
	printf("iReadClinicInfo: %d    所需时间: %d \n", status, nEnd - nStart);
	if (status == 0) {
		cout << szRead <<endl;
	}

}

void procMedicalInfo()
{
	char szRead[2048 * 4];
	memset(szRead, 0, sizeof(szRead));
	int nStart = GetTickCount();
	int status = iReadMedicalInfo("", szRead);
	int nEnd = GetTickCount();
	printf("iReadMedicalInfo: %d    所需时间: %d \n", status, nEnd - nStart);
	if (status == 0) {
		cout << szRead <<endl;
	}
}


void procQueryInfo()
{
	char szRead[2048];
	memset(szRead, 0, sizeof(szRead));

	std::string queryItems = "CARDNO|CARDSEQ";
	int nStart = GetTickCount();
	int status = iQueryInfo((char*)queryItems.c_str(), szRead);
	int nEnd = GetTickCount();
	printf("iQueryInfo: %d    所需时间: %d \n", status, nEnd - nStart);
	if (status == 0) {
		cout << szRead <<endl;
	}
}

void procScanCard()
{
	char szRead[2048];
	memset(szRead, 0, sizeof(szRead));

	int nStart = GetTickCount();
	int status = iScanCard(szRead);

	status = iScanCard();
	int nEnd = GetTickCount();
	printf("iScanCard: %d    所需时间: %d \n", status, nEnd - nStart);
	cout << szRead <<endl;
}

void procWriteHospinfo()
{
	char *buf;
	char file[256];
	memset(szError, 0, sizeof(szError));
	char *logXml;
	int ret = ReadFile("log.xml", &logXml);
	if (ret == -1) {
		printf("log.xml文件不存在\n");
		return;
	}

	ret = ReadFile("门诊摘要.xml", &buf);
	if (ret == -1) {
		printf("门诊摘要.xml文件不存在\n");
		return;
	}
	int nStart = GetTickCount();
	ret = iWriteHospInfoLocal(buf, logXml);
	int nEnd = GetTickCount();
	printf("iWriteHospInfoLocal: %d所需时间: %d ->", ret, nEnd - nStart);
	printf("写操作:%d\n", ret);
	if (ret != 0){
		strcpy(szError, err(ret));
		printf("%s\n", szError);
	}
	free(buf);
	free(logXml);
}

void procReadCardMessageForNH()
{
	char *buf;
	char szRead[2048 * 4];
	memset(szError, 0, sizeof(szError));
	memset(szRead, 0, sizeof(szRead));

	char *logXml;
	int ret = ReadFile("log.xml", &logXml);
	if (ret == -1) {
		printf("log.xml文件不存在\n");
		return;
	}
	int nStart = GetTickCount();
	ret = iReadCardMessageForNHLocal(logXml, szRead);
	int nEnd = GetTickCount();
	printf("iReadCardMessageForNHLocal: %d所需时间: %d \n", ret, nEnd - nStart);
	if (ret != 0){
		strcpy(szError, err(ret));
		printf("%s\n", szError);
	} 
	cout << szRead << endl;
	free(logXml);
}


void procReadClinicInfoLocal()
{
	char *buf;
	char szRead[2048 * 4];
	memset(szError, 0, sizeof(szError));
	memset(szRead, 0, sizeof(szRead));

	char *logXml;
	int ret = ReadFile("log.xml", &logXml);
	if (ret == -1) {
		printf("log.xml文件不存在\n");
		return;
	}
	int nStart = GetTickCount();
	ret = iReadClinicInfoLocal("", szRead, logXml);
	int nEnd = GetTickCount();
	printf("iReadClinicInfoLocal: %d所需时间: %d \n", ret, nEnd - nStart);
	if (ret != 0){
		strcpy(szError, err(ret));
		printf("%s\n", szError);
	} 
	cout << szRead << endl;
	free(logXml);
}

void printHelp()
{
	cout<<"*************一卡通系统测试*********************"<<endl;
	cout<<"功能测试："<<endl;
	cout<<"\t1:HIS只读"<<endl;
	cout<<"\t2:黑名单校验"<<endl;
	cout<<"\t3:ReadInfo"<<endl;
	cout<<"\t4:WriteInfo"<<endl;
	cout<<"\t5:FormateCard"<<endl;
	cout<<"\t6:CreateCard"<<endl;
	cout<<"\t7:ReadHISInfo"<<endl;
	cout<<"\t8:卡注册"<<endl;
	cout<<"\t9:PatchCard"<<endl;
	cout<<"\t10:PrintCard"<<endl;
	cout<<"\t11:CardIsEmpty"<<endl;
	cout<<"\t12:iReadClinicInfo"<<endl;
	cout<<"\t13:iReadMedicalInfo"<<endl;
	cout<<"\t14:iQueryInfo"<<endl;
	cout<<"\t15:iScanCard"<<endl;
	cout<<"\t16:iWriteHospInfoLocal"<<endl;
	cout<<"\t17:iReadCardMessageForNHLocal" <<endl;
	cout<<"\t18:iReadClinicInfoLocal" <<endl;
}

procItem procMethods[] = {
	{0, procCreateGWCard, false},
	{1, procCreateNHCard, true},
	{2, procRead, true},
	{3, procWrite, true},
	{4, procFormat, true},
	{5, procCreate, true},
	{6, procReadHIS, false},
	{7, procRegNHMessage, false},
	{8, procPatchCard, true},
	{9, procPrintCard, false},
	{10, procIsEmpty, false},
	{11, procClinicInfo, false},
	{12, procMedicalInfo, false},
	{13, procQueryInfo, true},
	{14, procScanCard, true},
	{15, procWriteHospinfo, true},
	{16, procReadCardMessageForNH, true},
	{17, procReadClinicInfoLocal, true},
	{-1, NULL},
};


// 8D
#define AUTH_FLAG	141  
int main(int argc, char* argv[])
{
	std::locale::global(std::locale(""));
	
	printHelp();
	char szSelect[10];
	cout<<"请输入功能测试选项:";
	cin>>szSelect;
	std::string strSelect = szSelect;

	while (1)
	{
		if (strSelect == "q" || strSelect == "quit")
			break;

		int nSelect = atoi(szSelect);
		nSelect -- ;
		if (nSelect < 0 || 
			nSelect > sizeof(procMethods)/sizeof(struct procItem) ) {
		} else {
			if (procMethods[nSelect].bNeedInit) {
				int ret = iCardInit();
				if (ret != 0){
					cout<<"卡初始化失败:"<<ret<<endl;
					continue;
				}
			}
			if (procMethods[nSelect].func != NULL) {
				procMethods[nSelect].func();
			}
			if (procMethods[nSelect].bNeedInit) {
				iCardDeinit();
			}
		}

		memset(szSelect, 0 , sizeof(szSelect));
		printHelp();
		cout<<"\n请输入功能测试选项:";
		cin>>szSelect;
		strSelect = szSelect;
	}
	
	getchar();
	return 0;
}

