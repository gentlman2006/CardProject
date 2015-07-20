#include "Card.h"
#include "public/XmlUtil.h"
#include "public/liberr.h"
#include "WebServiceAssist.h"
#include "BHGX_CardLib.h"
#include "ns_pipeClient/n_USCOREapiSoap.nsmap"
#include "ns_pipeClient/soapn_USCOREapiSoapProxy.h"

#pragma warning (disable : 4996)

#include "tinyxml/headers/tinyxml.h"
#pragma comment(lib, "tinyxml/libs/tinyxmld.lib")
using namespace std;

WebServiceUtil::WebServiceUtil()
{
}

WebServiceUtil::~WebServiceUtil()
{
}

WebServiceUtil::WebServiceUtil(char * strCheckWSDL, char * strServerURL):
m_strCheckWSDL(strCheckWSDL),
m_strServerURL(strServerURL)
{

}

bool WebServiceUtil::IsMedicalID(const std::string &strID)
{
	for (size_t i=0; i<strID.size(); ++i){
		char ID = strID[i];
		if (ID != 0x30){
			return true;
		}
	}
	return false;
}

int WebServiceUtil::NHCheckValid(std::string strCardNO, char *pszXml)
{
	int status = CardProcSuccess;
	if (!isValidUrl((char*)m_strServerURL.c_str()) || !isValidUrl((char*)m_strCheckWSDL.c_str())) {
		return CardInputParamError;
	}

	n_USCOREapiSoap soapUtil;
	soapUtil.endpoint = m_strServerURL.c_str();
	soap_init(soapUtil.soap);
	soap_set_mode(soapUtil.soap,SOAP_C_UTFSTRING);

	char *strCheckParams = new char[1024];
	memset(strCheckParams, 0, 1024);
	CreateCheckWsdlParams(strCardNO.c_str(), m_strCheckWSDL.c_str(), strCheckParams);

	_ns1__nh_USCOREpipe pCheck;
	pCheck.parms = strCheckParams;
	_ns1__nh_USCOREpipeResponse pReturn;

	pReturn.nh_USCOREpipeResult = new char[1024];

	soapUtil.__ns2__nh_USCOREpipe(&pCheck, &pReturn);
	if(soapUtil.soap->error) {   
		status = ConnectWebServerFailed;
		CXmlUtil::CreateResponXML(status, "�����������ʧ��", pszXml);
	} else {
		std::string strRetCode, strStatus;
		std::string strXML = pReturn.nh_USCOREpipeResult;
		GetCheckState(strXML, strRetCode, strStatus);

		std::string strCheckDesc;
		if (GetCheckRetDesc(strRetCode, strCheckDesc) == 0) {
			status = CardCheckError;
			CXmlUtil::CreateResponXML(status, strCheckDesc.c_str(), pszXml);
		} else{
			int nCardStatus = atoi(strStatus.c_str());
			strCheckDesc.clear();

			if (GetCardStatus(nCardStatus, strCheckDesc) == 0){
				status = CardCheckError;
				CXmlUtil::CreateResponXML(status, strCheckDesc.c_str(), pszXml);
			}
		}
	}
	SAFEARRAY_DELETE(strCheckParams);
	SAFEARRAY_DELETE(pReturn.nh_USCOREpipeResult);

	soap_end(soapUtil.soap);   
	soap_done(soapUtil.soap); 
	return status;
}

int WebServiceUtil::NHRegCard(std::string strCardNO, char *pszXml)
{
	int status = CardProcSuccess;
	if (!isValidUrl((char*)m_strServerURL.c_str())) {
		return CardInputParamError;
	}

	n_USCOREapiSoap soapUtil;
	soapUtil.endpoint = m_strServerURL.c_str();
	soap_init(soapUtil.soap);
	soap_set_mode(soapUtil.soap,SOAP_C_UTFSTRING);

	char* strRegParams = new char[1024];
	memset(strRegParams, 0, 1024);
	CreateRegWsdlParams(strCardNO.c_str(), strRegParams); 

	_ns1__nh_USCOREpipe pCheck;
	pCheck.parms = strRegParams;

	_ns1__nh_USCOREpipeResponse pReturn;
	pReturn.nh_USCOREpipeResult = new char[4096];

	soapUtil.__ns2__nh_USCOREpipe(&pCheck, &pReturn);

	if(soapUtil.soap->error){   
		status = ConnectWebServerFailed;
		CXmlUtil::CreateResponXML(status, "�����������ʧ��", pszXml); 
	}  else {
		std::string strRetCode, strStatus;
		std::string strXML = pReturn.nh_USCOREpipeResult;
		GetCheckState(strXML, strRetCode, strStatus);

		std::string strCheckDesc;
		if (GetCheckRetDesc(strRetCode, strCheckDesc) == 0) {
			status = CardCheckError;
			CXmlUtil::CreateResponXML(status, strCheckDesc.c_str(), pszXml);
		} else {
			FormatWriteInfo(strStatus.c_str(), pszXml);
			status = iWriteInfo(pszXml);
			if (status != CardProcSuccess) {
				CXmlUtil::CreateResponXML(CardWriteErr, "����дʧ��", pszXml);
			}
		}
	}
	SAFEARRAY_DELETE(strRegParams);
	SAFEARRAY_DELETE(pReturn.nh_USCOREpipeResult);
	soap_end(soapUtil.soap);   
	soap_done(soapUtil.soap); 
	return status;
}

int WebServiceUtil::GetCheckState(const std::string &strXML, std::string &strRetCode, std::string &strSec)
{
	size_t nPos = strXML.find("\t");
	if (nPos != -1){
		strRetCode = strXML.substr(0, nPos);
		strSec = strXML.substr(nPos+1, strXML.size());
		return 0;
	} else {
		strRetCode = strXML;
	}
	return -1;
}

int WebServiceUtil::_FormatWrite(std::map<int, std::string> &mapAll, char *strFMTWrite)
{
	TiXmlDocument *XmlDoc;

	TiXmlElement  *RootElement;
	TiXmlElement  *Segment;
	TiXmlElement  *Colum;
	TiXmlDeclaration HeadDec;
	TiXmlPrinter Printer;

	// ����XML��ͷ��˵��
	XmlDoc = new TiXmlDocument();

	HeadDec.Parse("<?xml version=\"1.0\" encoding=\"gb2312\" ?>", 0, TIXML_ENCODING_UNKNOWN);
	XmlDoc->LinkEndChild(&HeadDec);

	RootElement = new TiXmlElement("SEGMENTS");
	RootElement->SetAttribute("PROGRAMID", "001");
	Segment = new TiXmlElement("SEGMENT");
	Segment->SetAttribute("ID", 2);
	std::map<int, std::string>::iterator iter = mapAll.begin();
	for (; iter != mapAll.end(); ++iter)
	{
		Colum = new TiXmlElement("COLUMN");
		Colum->SetAttribute("ID", iter->first);
		Colum->SetAttribute("VALUE", iter->second.c_str());
		Segment->LinkEndChild(Colum);
	}

	RootElement->LinkEndChild(Segment);
	XmlDoc->LinkEndChild(RootElement);

	XmlDoc->Accept(&Printer);
	strcpy(strFMTWrite, Printer.CStr());
	return 0;
}

int WebServiceUtil::FormatWriteInfo(const char *strWrite, char *strFMTWrite)
{
	std::map<int, std::string> mapAll;
	TiXmlDocument XmlDoc;

	TiXmlElement  *RootElement;
	TiXmlElement  *Segment;
	TiXmlElement  *Colum;
	XmlDoc.Parse(strWrite);
	RootElement = XmlDoc.RootElement();
	Segment = RootElement->FirstChildElement();
	int nSegID = atoi(Segment->Attribute("ID"));
	if(Segment){
		Colum = Segment->FirstChildElement();
		std::string strValue;
		while (Colum){
			int nID = atoi(Colum->Attribute("ID"));
			strValue = Colum->Attribute("VALUE");
			mapAll[nID] = strValue;
			Colum = Colum->NextSiblingElement();
		}
	}

	_FormatWrite(mapAll, strFMTWrite);
	return 0;
}



int WebServiceUtil::CreateCheckWsdlParams(const char *CardID, const char *pszCardCheckWSDL, char *strParams)
{
	sprintf(strParams, "<root><system><parm name=\"functionName\" value=\" CardCheckForNH\"/>"
		"<parm name=\"divisionCode\" value=\"\"/></system><parms><parm name=\"cardID \" value=\"%s\"/>"
		"<parm name=\" pszCardCheckWSDL \" value=\"%s\"/></parms></root>", CardID, pszCardCheckWSDL);
	strParams[strlen(strParams)] = 0;
	return 0;
}

int WebServiceUtil::CreateRegWsdlParams(const char *CardID, char *strParams)
{
	sprintf(strParams, "<root><system><parm name=\"functionName\" value=\" GetRewritePackage\"/>"
		"<parm name=\"divisionCode\" value=\"\"/></system><parms><parm name=\"cardID  \" value=\"%s\"/>"
		"</parms></root>", CardID);
	strParams[strlen(strParams)] = 0;
	return 0;
}

int WebServiceUtil::GetCheckRetDesc(const std::string &strStatus, std::string &strDesc)
{
	if (m_mapCodeDesc.size() == 0){
		m_mapCodeDesc["I710000"] = "��У��ɹ�";
		m_mapCodeDesc["E710001"] = "���Ų���Ϊ��";
		m_mapCodeDesc["E710002"] = "WSDL��ַ����Ϊ��";
		m_mapCodeDesc["E710003"] = "��״̬���ʧ��";
		m_mapCodeDesc["E710004"] = "���ӿ�У�����ʧ��";
		m_mapCodeDesc["I720000"] = "��ע��ɹ�";
		m_mapCodeDesc["E720001"] = "ҽ��֤��Ų���Ϊ��";
		m_mapCodeDesc["E720002"] = "ҽ��֤��ų��Ȳ���ȷ��ʵ�ʳ���Ӧ��Ϊ12λ";
		m_mapCodeDesc["E720003"] = "δ�Ҵ˲κ���Ա�ĵ�����Ϣ";
		m_mapCodeDesc["E720004"] = "�˻��߻�д����ϢΪ��";
		m_mapCodeDesc["E720005"] = "��д��ʶ����ʧ��";
		m_mapCodeDesc["E720006"] = "��ȡbasicinfo.xml���ò���ʧ��";
		m_mapCodeDesc["E720007"] = "��ȡprogram.xml���ò���ʧ��";
		m_mapCodeDesc["E720008"] = "XML��д����ʽ����ȷ";
	}

	std::map<std::string, std::string>::iterator iter = m_mapCodeDesc.begin();
	iter = m_mapCodeDesc.find(strStatus);
	if (iter != m_mapCodeDesc.end()){
		strDesc = iter->second;
	} else {
		strDesc = strStatus;
	}
	return (strStatus == std::string("I710000") || strStatus == std::string("I720000"));
}

int WebServiceUtil::GetCardStatus(int nStatus, std::string &strStatus)
{

	if (m_mapCardStatus.size() == 0) {
		m_mapCardStatus[0] = "������";
		m_mapCardStatus[1] = "���ڿ��𻵻��Ѳ����¿����˿��ѱ�ע��";
		m_mapCardStatus[2] = "�˿��Ѱ����ʧҵ���ݲ���ʹ��";
		m_mapCardStatus[3] = "����Ч";
	}
	std::map<int,std::string>::iterator iter = m_mapCardStatus.begin();
	iter = m_mapCardStatus.find(nStatus);
	if (iter != m_mapCardStatus.end()){
		strStatus = iter->second;
	}else{
		strStatus = "δ֪����";
	}
	return (nStatus == 0);
}