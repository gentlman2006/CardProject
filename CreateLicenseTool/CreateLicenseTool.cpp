// CreateLicenseTool.cpp : �������̨Ӧ�ó������ڵ㡣
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

#define LICENSEFILE	"��������.license"
#define LICENSEHISFILE	"��������HIS.license"
void printHelp();

int main(int argc, char* argv[])
{
	printHelp();
	char szSelect[10];
	cout<<"�����빦�ܲ���ѡ��:";
	cin>>szSelect;

	int status = 0;
	char timeFMT[50];
	while (1)
	{
		if (strcmp(szSelect, "q") == 0 ||
			strcmp(szSelect, "quit") == 0)
			break;

		int nSelect = atoi(szSelect);
		switch (nSelect)
		{
		case 1:
			printf("�������ֹʱ��(��ʽ�� 2012-08-20_9:20:14):\n");
			scanf("%s", timeFMT);
			status = iCreateLicense(LICENSEFILE, timeFMT);
			if (status != 0)
				printf("%s\n",err(status));
			else 
				printf("Create license �ɹ�\n");
			break;
		case 2:
			status = iCreateLicense(LICENSEHISFILE);
			if (status != 0)
				printf("%s\n",err(status));
			else 
				printf("Create license �ɹ�\n");
			break;
		default:
			printHelp();
			break;
		}
		cout<<"�����빦�ܲ���ѡ��:";
		cin>>szSelect;
	}

	system("pause");

	return 0;
}

void printHelp()
{
	cout<<"************����license����******************"<<endl;
	cout<<"\t1:��ʱ�����License"<<endl;
	cout<<"\t2:������ȨLicense"<<endl;
	cout<<"\tq|quit:�˳�����"<<endl;
}



