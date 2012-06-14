#include <string>
#include <windows.h>

#if !defined(_SQ_TEXTCONVERT_H_INCLUDED_)
#define _SQ_TEXTCONVERT_H_INCLUDED_

//���ֽ��ַ���ת��Ϊ���ֽ��ַ���
// CodePage:[in]����
// lpcwszText:[in]���ֽ��ַ���
// strVal:[out]���ֽ��ַ���
//����ֵ:TRUE��ʾ�ɹ�,������ʾʧ��
inline BOOL WcharToChar(UINT CodePage,const wchar_t *lpcwszText,std::string &strVal)
{
	CHAR *lpszVal( NULL );
	int nLength(0);
	size_t dwSize(0);

	nLength = ::WideCharToMultiByte( CodePage,0,lpcwszText,-1,NULL,0,NULL,NULL);
	if( nLength <= 0 )
		return FALSE;

	dwSize = nLength * sizeof(CHAR);
	lpszVal = (CHAR *)malloc( dwSize );
	if( NULL == lpszVal )
		return FALSE;

	memset( lpszVal,0x0,dwSize);
	nLength = WideCharToMultiByte( CodePage, 0, lpcwszText,-1,lpszVal, nLength, NULL, NULL);
	if( nLength <= 0 )
	{
		free(lpszVal);
		lpszVal = NULL;
		return FALSE;
	}

	strVal.assign(lpszVal);
	free(lpszVal);
	lpszVal = NULL;
	return TRUE;
}

//���ֽ��ַ���ת��Ϊ���ֽ��ַ���
// CodePage:[in]����
// lpcwszText:[in]���ֽ��ַ���
// lppszVal:[out]���ֽ��ַ���
//����ֵ:TRUE��ʾ�ɹ�,������ʾʧ��
inline BOOL WcharFromChar(UINT CodePage,const char *lpcszText,std::wstring &wstrVal)
{
	WCHAR *lpwszVal( NULL );
	int nLength(0);
	size_t dwSize(0);
	nLength = ::MultiByteToWideChar( CodePage, 0, lpcszText, -1 , NULL, 0); 
	if( nLength <= 0 )
		return FALSE;

	dwSize = nLength * sizeof(WCHAR);
	lpwszVal = (WCHAR *)malloc( dwSize );
	if( NULL == lpwszVal )
		return FALSE;

	memset(lpwszVal,0x0,dwSize);
	nLength = ::MultiByteToWideChar( CodePage, 0, lpcszText, -1 , lpwszVal, nLength ); 
	if( nLength <= 0 )
	{
		free(lpwszVal);
		lpwszVal = NULL;
		return FALSE;
	}
	wstrVal.assign( lpwszVal );
	free(lpwszVal);
	lpwszVal = NULL;
	return TRUE;
}

//���ֽ��ַ���ת��Ϊ���ֽ��ַ���
// lpcwszText:[in]���ֽ��ַ���
// strVal:[out]���ֽ��ַ���
//����ֵ:TRUE��ʾ�ɹ�,������ʾʧ��
inline BOOL WToA(const wchar_t *lpcwszText,std::string &strVal)
{
	return WcharToChar(CP_ACP,lpcwszText,strVal);
}

//���ֽ��ַ���ת��Ϊ���ֽ��ַ���
// lpcwszText:[in]���ֽ��ַ���
// lppszVal:[out]���ֽ��ַ���
//����ֵ:TRUE��ʾ�ɹ�,������ʾʧ��
inline BOOL WFromA(const char *lpcszText,std::wstring &wstrVal )
{
	return WcharFromChar(CP_ACP,lpcszText,wstrVal);
}

//���ֽ��ַ���ת��ΪUTF-8�ַ���
// lpcwszText:[in]���ֽ��ַ���
// strVal:[out]UTF-8�ַ���
//����ֵ:TRUE��ʾ�ɹ�,������ʾʧ��
inline BOOL WToUTF8(const wchar_t *lpcwszText,std::string &strVal)
{
	return WcharToChar(CP_UTF8,lpcwszText,strVal);
}


//UTF-8�ַ���ת��Ϊ���ֽ��ַ���
// lpcwszText:[in]UTF-8�ַ���
// lppszVal:[out]���ֽ��ַ���
//����ֵ:TRUE��ʾ�ɹ�,������ʾʧ��
inline BOOL WFromUTF8(const char *lpcszText,std::wstring &wstrVal )
{
	return WcharFromChar(CP_UTF8,lpcszText,wstrVal);
}