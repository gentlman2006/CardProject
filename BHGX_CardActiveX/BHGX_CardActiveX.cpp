// BHGX_CardActiveX.cpp : DLL ������ʵ�֡�


#include "stdafx.h"
#include "resource.h"
#include "BHGX_CardActiveX.h"


class CBHGX_CardActiveXModule : public CAtlDllModuleT< CBHGX_CardActiveXModule >
{
public :
	DECLARE_LIBID(LIBID_BHGX_CardActiveXLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_BHGX_CARDACTIVEX, "{98A47605-C66D-4EA0-BD73-FF5C48FC4C41}")
};

CBHGX_CardActiveXModule _AtlModule;


#ifdef _MANAGED
#pragma managed(push, off)
#endif

// DLL ��ڵ�
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	hInstance;
    return _AtlModule.DllMain(dwReason, lpReserved); 
}

#ifdef _MANAGED
#pragma managed(pop)
#endif




// ����ȷ�� DLL �Ƿ���� OLE ж��
STDAPI DllCanUnloadNow(void)
{
    return _AtlModule.DllCanUnloadNow();
}


// ����һ���๤���Դ������������͵Ķ���
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}


// DllRegisterServer - ������ӵ�ϵͳע���
STDAPI DllRegisterServer(void)
{
    // ע��������Ϳ�����Ϳ��е����нӿ�
    HRESULT hr = _AtlModule.DllRegisterServer();
	return hr;
}


// DllUnregisterServer - �����ϵͳע������Ƴ�
STDAPI DllUnregisterServer(void)
{
	HRESULT hr = _AtlModule.DllUnregisterServer();
	return hr;
}

