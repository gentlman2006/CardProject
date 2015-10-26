#include <windows.h>
#include <string.h>
#include <stdio.h>

#include "device.h"
#include "public/debug.h"
#pragma warning (disable : 4996)
#pragma warning (disable : 4020)
#pragma warning (disable : 4013)

#define MAX_RESULT 256


/**
 *
 */
static char **EnumDLL( __in const char *directory, __out int *count)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	TCHAR tStringArray[MAX_RESULT][MAX_PATH];
	TCHAR **result;
	TCHAR Pattern[MAX_PATH];
	int i = 0, j = 0;

	// ��ʼ����
	//lstrcpy(Pattern, directory);
	lstrcpy(Pattern, "D:\\card_drivers\\");
	lstrcat(Pattern,"BHGX*.dll");
	hFind = FindFirstFile(Pattern, &FindFileData);
	if(hFind == INVALID_HANDLE_VALUE)
	{
		*count = 0;
		return NULL;
	}
	else 
	{
		do 
		{
			lstrcpy(tStringArray[i++], FindFileData.cFileName);
		} while(FindNextFile(hFind, &FindFileData) != 0);
	}

	// ���ҽ���
	FindClose(hFind);

	// ���Ƶ������
	result = (char **)malloc(i, sizeof(char *));
	for (j = 0; j < i; j++)
	{
		result[j] = (char *)malloc(MAX_PATH, sizeof(char));
		lstrcpy(result[j], tStringArray[j]);
	}

	*count = i;
	return result;
}


/**
 *
 */
static struct CardDevice *_InitDevice(HINSTANCE hInstLibrary)
{
	struct CardDevice *result = NULL;
	
	// ����CardDevice���ݽṹ�ռ�
	result = (struct CardDevice *)malloc(sizeof(struct CardDevice));

	// ��ʼ���������
	result->hInstLibrary = hInstLibrary;

	// ��ʼ���豸�ṹ�Ļص�����
	result->iProbe = (DllProbe)GetProcAddress(hInstLibrary, "bProbe");
	result->iOpen = (DllOpen)GetProcAddress(hInstLibrary, "iOpen");
	result->iClose = (DllClose)GetProcAddress(hInstLibrary, "iClose");
	result->iScanCard = (DllScanCard)GetProcAddress(hInstLibrary,"iScanCard");
	result->iIOCtl = (DLLIOCtl)GetProcAddress(hInstLibrary, "iIOCtl");
	result->iRead = (DllRead)GetProcAddress(hInstLibrary, "iRead");
	result->iWrite = (DllWrite)GetProcAddress(hInstLibrary, "iWrite");
	result->iChangePwdEx = (DLLChangePwdEx)GetProcAddress(hInstLibrary, "iChangePwdEx");
	return result;
}

static struct CardAuth *InitAuthDev(const char *System)
{
	char Pattern[MAX_PATH];
	HINSTANCE hInstLibrary;
	struct CardAuth *pAuth = NULL;
	pAuth = (struct CardAuth*)malloc(sizeof(struct CardAuth));
	memset(pAuth, 0, sizeof(struct CardAuth));

	memset(Pattern, 0, MAX_PATH);
	strcpy(Pattern, System);
	strcat(Pattern, "BHGX_HHUkey.dll");
	hInstLibrary = LoadLibrary(Pattern);
	if (hInstLibrary != NULL) {
		pAuth->hAuthLibrary = hInstLibrary;
		pAuth->iAuthUDev = (DllAuthUDev)GetProcAddress(hInstLibrary, "AuthenticDevice");
	} 
	return pAuth;
}

/**
 * ����: getCardDevice 
 * ������ 
 *
 * ����ֵ:
 * �ɹ��� ���س����豸
 * ʧ�ܣ� NULL
 */
struct CardDevice *getCardDevice(const char *System)
{
	DllProbe FunProbe;
	struct CardDevice *result = NULL;

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	char Pattern[MAX_PATH];
	int nProbe = 0;

	// ��ʼ����
	strcpy(Pattern, "D:\\card_drivers\\");
	strcat(Pattern, "BHGX_MF_*.dll");
	hFind = FindFirstFile(Pattern, &FindFileData);
	while (hFind != INVALID_HANDLE_VALUE)
	{
		HINSTANCE hInstLibrary;
		hInstLibrary = LoadLibrary(FindFileData.cFileName);
		if (hInstLibrary != NULL)
		{
			FunProbe = (DllProbe)GetProcAddress(hInstLibrary, "bProbe");
			nProbe = FunProbe();
			LogPrinter("filename:%s, bProbe:%d\n",FindFileData.cFileName, nProbe);
			if((FunProbe != NULL) && nProbe)
			{
				result = _InitDevice(hInstLibrary);
				break;
			}
			FreeLibrary(hInstLibrary);
		}
		if (0 == FindNextFile(hFind, &FindFileData))
			break;
	}
	FindClose(hFind);

	///**
	// * ����豸�ҵ����ʹ򿪲�������һ��
	// */
	if(result)
	{
		result->iOpen();
	}	
	return result;
}


/**
 * ������putCardDevice
 * ������
 * @device: ���豸
 *
 * ����ֵ��
 * �ɹ��� 0
 * ʧ�ܣ� ����
 */
int putCardDevice(struct CardDevice *device)
{
	int result = 0;

	if(device != NULL)
	{
		device->iClose();
		FreeLibrary(device->hInstLibrary);
		free(device);
	}

	return result;
}


int authUDev(const char *System)
{
	struct CardAuth *pAuth = NULL;
	int status = 0;
	pAuth = InitAuthDev(System);

	if (pAuth == NULL || !pAuth->iAuthUDev()) {
		status = -1;
	}

	if (pAuth) {
		FreeLibrary(pAuth->hAuthLibrary);
		free(pAuth);
	}

	return status;
}
