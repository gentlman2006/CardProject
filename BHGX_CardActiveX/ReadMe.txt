========================================================================
    �ģ��⣺BHGX_CardActiveX ��Ŀ����
========================================================================

Ӧ�ó�������Ϊ�������˴� BHGX_CardActiveX ��Ŀ���������ڴ˻���֮��
��д�Լ��Ķ�̬���ӿ� (DLL)��


���ļ������������Ŀ��ÿ���ļ�
�����ݡ�

BHGX_CardActiveX.vcproj
    ����ʹ��Ӧ�ó��������ɵ� VC++ ��Ŀ������Ŀ�ļ��� 
    �������й����ɸ��ļ���ʹ�õ� Visual C++ �汾����Ϣ���Լ�
    �й���Ӧ�ó�������ѡ���ƽ̨�����ú���Ŀ����
    ����Ϣ��

BHGX_CardActiveX.idl
    ���ļ�������������Ŀ�ж�������Ϳ⡢�ӿ�
    �� coclass �� IDL ���塣
    ���ļ����� MIDL ���������������ɣ�
        C++ �ӿڶ���� GUID ����                 (BHGX_CardActiveX.h)
        GUID ����                                (BHGX_CardActiveX_i.c)
        һ�����Ϳ�                               (BHGX_CardActiveX.tlb)
        ���ʹ���                                 (BHGX_CardActiveX_p.c �� dlldata.c)

BHGX_CardActiveX.h
    ���ļ������� BHGX_CardActiveX.idl �ж������� C++ �ӿڶ���
    �� GUID �����������ڱ���������� MIDL �������ɡ�

BHGX_CardActiveX.cpp
    ���ļ���������ӳ��� DLL ������ʵ�֡�

BHGX_CardActiveX.rc
    ���ǳ���ʹ�õ����� Microsoft Windows ��Դ����
    ����

BHGX_CardActiveX.def
    ��ģ�鶨���ļ�Ϊ�������ṩ�й� DLL ��Ҫ���
    ��������Ϣ�������������������ݵĵ�����
        DllGetClassObject  
        DllCanUnloadNow    
        GetProxyDllInfo    
        DllRegisterServer	
        DllUnregisterServer

/////////////////////////////////////////////////////////////////////////////
������׼�ļ���

StdAfx.h��StdAfx.cpp
    ��Щ�ļ�����������Ϊ BHGX_CardActiveX.pch ��Ԥ����ͷ (PCH) �ļ�
    ����Ϊ StdAfx.obj ��Ԥ���������ļ���

Resource.h
    �������ڶ�����Դ ID �ı�׼ͷ�ļ���

/////////////////////////////////////////////////////////////////////////////
����/��� (stub) DLL ��Ŀ��ģ�鶨���ļ���

BHGX_CardActiveXps.vcproj
    ���ļ����������ɴ���/��� (stub) DLL ����Ŀ�ļ������б�Ҫ����
	����Ŀ�е� IDL �ļ��������ٰ���һ���ӿڣ�����
	�����ɴ���/��� (stub) DLL ֮ǰ�����ȱ��� IDL �ļ���	�˽�������
	dlldata.c��BHGX_CardActiveX_i.c �� BHGX_CardActiveX_p.c�����ǡ�
	�����ɴ���/��� (stub) DLL ������ġ�

BHGX_CardActiveXps.def
    ��ģ�鶨���ļ�Ϊ�������ṩ�йش���/��� (stub) ��Ҫ���
    ��������Ϣ��

/////////////////////////////////////////////////////////////////////////////
