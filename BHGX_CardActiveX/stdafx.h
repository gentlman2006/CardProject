// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�

#pragma once

#ifndef STRICT
#define STRICT
#endif

//#ifndef WINVER // Allow use of features specific to Windows 95 and Windows NT 4 or later.
//#define WINVER 0x0501 // Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
//#endif
//
//#ifndef _WIN32_WINNT // Allow use of features specific to Windows NT 4 or later.
//#define _WIN32_WINNT 0x0501 // Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
//#endif
//
//#ifndef _WIN32_WINDOWS // Allow use of features specific to Windows 98 or later.
//#define _WIN32_WINDOWS 0x0501 // Change this to the appropriate value to target Windows Me or later.
//#endif
//
//#ifndef _WIN32_IE // Allow use of features specific to IE 4.0 or later.
//#define _WIN32_IE 0x0601 // Change this to the appropriate value to target IE 5.0 or later.
//#endif

// ������뽫λ������ָ��ƽ̨֮ǰ��ƽ̨��ΪĿ�꣬���޸����ж��塣
// �йز�ͬƽ̨��Ӧֵ��������Ϣ����ο� MSDN��
#ifndef WINVER				// ����ʹ���ض��� Windows XP ����߰汾�Ĺ��ܡ�
#define WINVER 0x0501		// ����ֵ����Ϊ��Ӧ��ֵ���������� Windows �������汾��
#endif

#ifndef _WIN32_WINNT		// ����ʹ���ض��� Windows XP ����߰汾�Ĺ��ܡ�
#define _WIN32_WINNT 0x0501	// ����ֵ����Ϊ��Ӧ��ֵ���������� Windows �������汾��
#endif						

#ifndef _WIN32_WINDOWS		// ����ʹ���ض��� Windows 98 ����߰汾�Ĺ��ܡ�
#define _WIN32_WINDOWS 0x0410 // ����ֵ����Ϊ�ʵ���ֵ����ָ���� Windows Me ����߰汾��ΪĿ�ꡣ
#endif

#ifndef _WIN32_IE			// ����ʹ���ض��� IE 6.0 ����߰汾�Ĺ��ܡ�
#define _WIN32_IE 0x0600	// ����ֵ����Ϊ��Ӧ��ֵ���������� IE �������汾��
#endif

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// ĳЩ CString ���캯��������ʽ��


#include "resource.h"
#include <atlbase.h>
#include <atlcom.h>

using namespace ATL;