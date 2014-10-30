// CardProcess.h : CCardProcess ������

#pragma once
#include "resource.h"       // ������

#include "BHGX_CardActiveX.h"
#include <string>
#include <map>
using namespace std;


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Windows CE ƽ̨(�粻�ṩ��ȫ DCOM ֧�ֵ� Windows Mobile ƽ̨)���޷���ȷ֧�ֵ��߳� COM ���󡣶��� _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA ��ǿ�� ATL ֧�ִ������߳� COM ����ʵ�ֲ�����ʹ���䵥�߳� COM ����ʵ�֡�rgs �ļ��е��߳�ģ���ѱ�����Ϊ��Free����ԭ���Ǹ�ģ���Ƿ� DCOM Windows CE ƽ̨֧�ֵ�Ψһ�߳�ģ�͡�"
#endif



// CCardProcess

class ATL_NO_VTABLE CCardProcess :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CCardProcess, &CLSID_CardProcess>,
	public ISupportErrorInfo,
	public IObjectWithSiteImpl<CCardProcess>,
	public IDispatchImpl<ICardProcess, &IID_ICardProcess, &LIBID_BHGX_CardActiveXLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CCardProcess()
	{
		strcpy_s(m_strErr, sizeof(m_strErr), "�������ɹ�");
	}

DECLARE_REGISTRY_RESOURCEID(IDR_CARDPROCESS)


BEGIN_COM_MAP(CCardProcess)
	COM_INTERFACE_ENTRY(ICardProcess)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(IObjectWithSite)
END_COM_MAP()

// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}
public:
	STDMETHOD(iATLGetCardVersion)(BSTR* szVersion);
	STDMETHOD(iATLCardInit)(LONG* nRet);
	STDMETHOD(iATLReadInfo)(LONG nFlag, BSTR* szReadXML);
	STDMETHOD(iATLWriteInfo)(BSTR szXML, LONG* nRet);
	STDMETHOD(iATLQueryInfo)(BSTR szQuerySource, BSTR* szResult);
	STDMETHOD(iATLerr)(BSTR* szError);
	STDMETHOD(iATLFormatCard)(LONG* nRet);
	STDMETHOD(iATLCreateCard)(BSTR szCardXML, LONG* nRet);
	STDMETHOD(iATLPrintCard)(BSTR pszPrinterType, BSTR pszCardCoverDataXml, BSTR pszCardCoverXml, LONG* nRet);
	STDMETHOD(iATLPatchCard)(BSTR szCardXML, BSTR pszCardCoverDataXml, BSTR pszPrinterType, BSTR pszCardCoverXml, LONG* nRet);
	STDMETHOD(iATLCreateCardData)(BSTR szCreateData, BSTR szLicense);
	STDMETHOD(iATLScanCard)(LONG* nRet);
	STDMETHOD(iATLCardClose)(LONG* nRet);
	STDMETHOD(iATLReadCardMessageForNH)(BSTR pszCardCheckWSDL, BSTR pszCardRewritePackageWSDL, BSTR* pszXml);
	STDMETHOD(iATLCardIsEmpty)(LONG* bEmpty);
	STDMETHOD(iATLCheckMsgForNH)(BSTR bstrCheckWSDL, BSTR bstrServerURL, BSTR* strCheckRet);
	STDMETHOD(iATLReadConfigMsg)(BSTR bstrConfigInfo, BSTR* bstrReadXML);
	STDMETHOD(iATLRegMsgForNH)(BSTR bstrServerURL, BSTR* bstrReadXML);
	STDMETHOD(iATLEncryFile)(BSTR bstrfilename, LONG* nProCode);
	STDMETHOD(iATLGetPrinterList)(BSTR* bstrPrinterXML);
	STDMETHOD(iATLReadHISInfo)(BSTR pszCardCheckWSDL, BSTR pszCardRewritePackageWSDL,BSTR* szXML);
	STDMETHOD(iATLReadInfoForXJ)(BSTR pszCardCheckWSDL, BSTR pszCardRewritePackageWSDL,BSTR* szXML);
	STDMETHOD(iATLReadOnlyHIS)(BSTR* bstrHISInfo);
	STDMETHOD(iATLCardOpen)(LONG* Ret);
	STDMETHOD(iATLCardDeinit)(LONG* Ret);
	STDMETHOD(iATLCreateLicense)(BSTR timeFMT, LONG* ret);
	STDMETHOD(iATLFormatHospInfo)(LONG* pRet);
	STDMETHOD(iATLWriteHospInfo)(BSTR xml, LONG* pRet);
	STDMETHOD(iATLReadClinicInfo)(BSTR pszCode, BSTR* readXML);
	STDMETHOD(iATLReadMedicalInfo)(BSTR pszCode, BSTR* readXML);
	STDMETHOD(iATLReadFeeInfo)(BSTR pszCode, BSTR* readXML);
	STDMETHOD(iATLReadOnlyHISLog)(BSTR logConfXml, BSTR* bstrHISInfo);
	STDMETHOD(iATLReadInfoForXJLog)(BSTR pszCardCheckWSDL, BSTR pszCardServerURL, BSTR pszLogXml, BSTR* xml);
	STDMETHOD(iATLReadHISInfoLog)(BSTR pszCardCheckWSDL, BSTR pszCardServerURL, BSTR pszLogXml, BSTR* xml);
	STDMETHOD(iATLReadMedicalInfoLog)(BSTR pszCode, BSTR pszLogXml, BSTR* xml);
	STDMETHOD(iATLReadFeeInfoLog)(BSTR pszCode, BSTR pszLogXml, BSTR* xml);
	STDMETHOD(iATLWriteHospInfoLog)(BSTR xml, BSTR pszLogXml, LONG* pRet);
	STDMETHOD(iATLReadClinicInfoLog)(BSTR pszCode, BSTR pszLogXml, BSTR* xml);
	STDMETHOD(iATLReadOnlyHISLocal)(BSTR pszLogXml, BSTR* xml);
	STDMETHOD(iATLReadInfoForXJLocal)(BSTR pszCardCheckWSDL, BSTR pszCardServerURL, BSTR pszLogXml, BSTR* xml);
	STDMETHOD(iATLReadHISInfoLocal)(BSTR pszCardCheckWSDL, BSTR pszCardServerURL, BSTR pszLogXml, BSTR* xml);
	STDMETHOD(iATLReadMedicalInfoLocal)(BSTR pszCode, BSTR pszLogXml, BSTR* xml);
	STDMETHOD(iATLReadFeeInfoLocal)(BSTR pszCode, BSTR pszLogXml, BSTR* xml);
	STDMETHOD(iATLReadClinicInfoLocal)(BSTR pszCode, BSTR pszLogXml, BSTR* xml);
	STDMETHOD(iATLWriteHospInfoLocal)(BSTR xml, BSTR pszLogXml, LONG* pRet);
	STDMETHOD(iATLRegMsgForNHLog)(BSTR bstrServerURL, BSTR pszLogXml, BSTR* bstrReadXML);
	STDMETHOD(iATLReadCardMessageForNHLog)(BSTR pszCardCheckWSDL, BSTR pszCardRewritePackageWSDL, BSTR pszLogXml, BSTR* pszXml);
	STDMETHOD(iATLWriteHospInfoOnlyLog)(BSTR xml, BSTR pszLogXml, LONG* pRet);
	STDMETHOD(iATLReadClinicInfoOnlyLog)(BSTR pszCode, BSTR pszLogXml, BSTR* xml);
	STDMETHOD(iATLReadFeeInfoOnlyLog)(BSTR pszCode, BSTR pszLogXml, BSTR* xml);
	STDMETHOD(iATLReadMedicalInfoOnlyLog)(BSTR pszCode, BSTR pszLogXml, BSTR* xml);
	STDMETHOD(iATLReadCardMessageForNHLocal)(BSTR pszLogXml, BSTR* pszXml);
	STDMETHOD(iATLCheckMsgForNHLocal)(BSTR pszLogXml, BSTR* pszXml);
	STDMETHOD(iATLReadOnlyCardMessageForNH)(BSTR* pszXml);
	STDMETHOD(iATLReadAll)(BSTR* xml);
	STDMETHOD(iATLRWRecycle)(BSTR pszCardCorp, BSTR pszXinCorp, LONG counts, BSTR write_xml, BSTR* pszXml);

protected:
	void GetErrInfo(int nProcRet, LONG &nRet);

	char *GetErrInfo(int nProcRet);

	void CreateResponXML(int nID, const char *szResult, char *RetXML);

protected:
	char m_strErr[1024];
	
};

OBJECT_ENTRY_AUTO(__uuidof(CardProcess), CCardProcess)
