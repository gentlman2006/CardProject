// CardProject.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include "../BHGX_CardLib/BHGX_CardLib.h"

#ifdef _DEBUG
#pragma comment(lib, "../debug/BHGX_CardLib.lib")
#else
#pragma comment(lib, "../release/BHGX_CardLib.lib")
#endif

#pragma warning (disable : 4996)
using namespace std;

#define BIRTHSTR   "BIRTHDAY"
int repairKey(char *xml);
void printHelp();

int main(int argc, char* argv[])
{
	printHelp();
	char szSelect[10];
	cout<<"�����빦�ܲ���ѡ��:";
	cin>>szSelect;

	char szxml[1024];
	int status = iCardInit();
	while (1)
	{
		if (strcmp(szSelect, "q") == 0 ||
			strcmp(szSelect, "quit") == 0)
			break;

		int nSelect = atoi(szSelect);
		switch (nSelect)
		{
		case 1:
			if (!status) {
				memset(szxml, 0, sizeof(szxml));
				status = repairKey(szxml);
				if (status == 0) {
					cout<<"�޲�����ɹ�"<<endl;
				} else {
					cout<<"�޲�����ʧ��"<<endl;
				}
			}
			printHelp();
			break;
		default:
			printHelp();
			break;
		}
		cout<<"�����빦�ܲ���ѡ��:";
		cin>>szSelect;
	}

	iCardDeinit();
	
	system("pause");

	return 0;
}

void printHelp()
{
	cout<<"*************�޲���*********************"<<endl;
	cout<<"\t1:д�����޸�"<<endl;
	cout<<"\tq|quit:�˳�����"<<endl;
}

int repairKey(char *xml) 
{
	int status = iQueryInfo(BIRTHSTR, xml);
	status = iWriteInfo(xml);
	return status;
}
