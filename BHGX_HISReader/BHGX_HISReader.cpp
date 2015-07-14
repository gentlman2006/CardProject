#include <map>
#include <vector>
#include <windows.h>
#include <stdlib.h>
#include "resource.h"
#include "Markup.h"
#include "XmlUtil.h"
#include "BHGX_HISReader.h"
#include "../BHGX_CardLib/BHGX_CardLib.h"
#include "../BHGX_CardLib/public/liberr.h"
#include "tinyxml/headers/tinyxml.h"

using namespace std;
#pragma comment(lib, "tinyxml/libs/tinyxmld.lib")

#ifdef _DEBUG
#pragma comment(lib, "../Debug/BHGX_CardLib.lib")
#else
#pragma comment(lib, "../Release/BHGX_CardLib.lib")
#endif

#pragma warning (disable : 4996)
#define LICENSEFILE	"��������HIS.license"

#define SAFE_DELETE(a)  if (a != NULL) { delete(a);a = NULL;}

#define HIS_FLAG    0
#define XJ_FALG		1
#define BUFF_SIZE	2048

class HISProcess
{
public:
	HISProcess(int flag)
	{
		m_pszLogXml = NULL;
		if (flag == HIS_FLAG) {
			m_MapColumInfo["CARDNO"].strDesc = "����";
			m_MapColumInfo["MEDICARECERTIFICATENO"].strDesc = "���˲κϺ�";
			m_MapColumInfo["NAME"].strDesc = "����";
			m_MapColumInfo["IDNUMBER"].strDesc = "���֤��";
			m_MapColumInfo["GENDER"].strDesc = "�Ա�";
			m_MapColumInfo["BIRTHDAY"].strDesc = "��������";
			m_MapColumInfo["PEOPLETYPE"].strDesc = "��Ա����";
			m_MapColumInfo["HOUSEHOLDERRELATION"].strDesc = "�뻧����ϵ";
			m_MapColumInfo["FIRSTPARTAKEDATE"].strDesc = "�״βκ�ʱ��";
			m_MapColumInfo["HEALTHSTATE"].strDesc = "����״��";
			m_MapColumInfo["PHONENO"].strDesc = "�绰����";
			m_MapColumInfo["HEALTHRECORDNO"].strDesc = "����������";
		}

		if (flag == XJ_FALG) {
			m_MapColumInfo["MEDICARECERTIFICATENO"].strDesc = "ҽ��֤��";
			m_MapColumInfo["NAME"].strDesc = "����";
			m_MapColumInfo["IDNUMBER"].strDesc = "���֤��";
			m_MapColumInfo["GENDER"].strDesc = "�Ա�";
			m_MapColumInfo["BIRTHDAY"].strDesc = "��������";
			m_MapColumInfo["NATIONALITY"].strDesc = "����";
			m_MapColumInfo["HEALTHRECORDNO"].strDesc = "����������";
		}
	}

	~HISProcess(){
	}

	void setLogParams(int rwFlag, char *processName)
	{
		m_rwFlag = rwFlag;
		m_funcName = processName;
	}

	char * geneQuerys(std::string &strQuerys) {
		std::map<std::string, ColumInfo>::iterator iter = m_MapColumInfo.begin();
		strQuerys = iter->first;
		for (; iter != m_MapColumInfo.end(); iter++) {
			strQuerys += iter->first;
			strQuerys += "|";
		}
		strQuerys = strQuerys.substr(0, strQuerys.size()-1);
		return (char*)strQuerys.c_str();
	}

	void fillHISMapInfo(std::map<std::string, ColumInfo> &mapHISInfo)
	{	
		if (mapHISInfo.size() > 0){
			std::map<std::string, ColumInfo>::iterator iter = m_MapColumInfo.begin();
			for (; iter != m_MapColumInfo.end(); ++iter){
				std::string szSource = iter->first;
				ColumInfo &stHIS = iter->second;
				std::map<string, ColumInfo>::iterator iter = mapHISInfo.find(szSource);
				if (iter == mapHISInfo.end()) {
					stHIS.ID = -1;
				} else {
					stHIS.ID = mapHISInfo[szSource].ID;
					stHIS.strValue = mapHISInfo[szSource].strValue;
					stHIS.strSource = mapHISInfo[szSource].strSource;
				}

			}
		}
	}

	int getHISInfo(const char *szReader, char *strColumInfo, bool bLog)
	{
		strcpy(strColumInfo, szReader);
		if (bLog) {
			iGeneLog(m_pszLogXml, m_rwFlag, (char*)m_funcName.c_str(), (char*)szReader);
		}
		return 0;
	}

	int readHISBaseInfo(
		char *pszCardCheckWSDL, 
		char *pszCardServerURL, 
		char *pszLogXml, 
		char *xml, 
		bool bNet, 
		bool bLocal)
	{
		if (bNet) {
			if (pszCardCheckWSDL == NULL || strlen(pszCardCheckWSDL) == 0 || 
				pszCardServerURL == NULL || strlen(pszCardServerURL) == 0) {
					return CardInputParamError;
			}
		}
	
		int nInit = iCardInit();
		if (nInit != 0){
			CXmlUtil::CreateResponXML(CardInitErr, err(CardInitErr), xml);
			return CardInitErr;
		}

		if (pszLogXml) {
			m_pszLogXml = pszLogXml;
		}

		if (bLocal) {
			int status = iCheckException(pszLogXml, xml);
			if (status != CardProcSuccess) {
				return status;
			}
			
			std::string strQuery;
			geneQuerys(strQuery);
			if (CardProcSuccess != iQueryInfo((char*)strQuery.c_str(), xml)) {
				return CardReadErr;
			}

		} else {
			int status = 0;
			if (bNet) {
				status = iCheckMsgForNH(pszCardCheckWSDL, pszCardServerURL, xml);
				if (status != CardProcSuccess){
					return status;
				}
			} else {
				std::string strQuery;
				geneQuerys(strQuery);
				if (CardProcSuccess != iQueryInfo((char*)strQuery.c_str(), xml)) {
					return CardReadErr;
				}
			}
		}
		iCardDeinit();
		return CardProcSuccess;
	}

	void createColumInfoXML(char *strColumInfo)
	{
		TiXmlDocument *XmlDoc = NULL;
		TiXmlElement *RootElement = NULL;
		TiXmlElement *Segment = NULL;
		TiXmlElement *Column = NULL;
		TiXmlDeclaration HeadDec;
		TiXmlPrinter Printer;

		// ����XML�ĵ�
		XmlDoc = new TiXmlDocument();

		// ����XML��ͷ��˵��
		HeadDec.Parse("<?xml version=\"1.0\" encoding=\"gb2312\" ?>", 0, TIXML_ENCODING_UNKNOWN);
		XmlDoc->LinkEndChild(&HeadDec);

		RootElement = new TiXmlElement("SEGMENTS");
		RootElement->SetAttribute("PROGRAMID", "001");
		Segment = new TiXmlElement("SEGMENT");
		Segment->SetAttribute("ID", 2);

		std::map<string, ColumInfo>::const_iterator iter = m_MapColumInfo.begin();
		for (; iter != m_MapColumInfo.end(); ++iter) {
			const ColumInfo &stHIS = iter->second;
			if (stHIS.ID == -1) {
				continue;
			}
			Column = new TiXmlElement("COLUMN");
			Column->SetAttribute("ID", stHIS.ID);
			Column->SetAttribute("VALUE", stHIS.strValue.c_str());
			Segment->LinkEndChild(Column);
		}

		RootElement->LinkEndChild(Segment);
		XmlDoc->LinkEndChild(RootElement);

		XmlDoc->Accept(&Printer);
		strcpy(strColumInfo, Printer.CStr());
	}

private:
	int m_rwFlag;
	std::string m_funcName;
	char * m_pszLogXml;
	std::map<std::string, ColumInfo> m_MapColumInfo;
};


int __stdcall iReadHISInfo(char *pszCardCheckWSDL, char *pszCardServerURL, char *xml)
{
	char szRead[BUFF_SIZE];
	memset(szRead, 0, sizeof(szRead));

	HISProcess his(HIS_FLAG);
	int status = his.readHISBaseInfo(pszCardCheckWSDL,pszCardServerURL, NULL, szRead, true, false);
	if (status != CardProcSuccess) {
		strcpy(xml, szRead);
		return status;
	}
	status = his.getHISInfo(szRead, xml, false);
	return status;
}

int __stdcall iReadOnlyHIS(char *xml)
{
	char szRead[BUFF_SIZE];
	memset(szRead, 0, sizeof(szRead));
	isCardAuth();

	HISProcess his(HIS_FLAG);
	int status = his.readHISBaseInfo("", "", NULL, szRead, false, false);
	if (status != CardProcSuccess) {
		strcpy(xml, szRead);
		return status;
	}
	status = his.getHISInfo(szRead, xml, false);
	return status;
}

int __stdcall iReadInfoForXJ(char *pszCardCheckWSDL, char *pszCardServerURL, char *xml)
{
	char szRead[BUFF_SIZE];
	memset(szRead, 0, sizeof(szRead));

	HISProcess his(XJ_FALG);
	int status = his.readHISBaseInfo(pszCardCheckWSDL,pszCardServerURL, NULL,  szRead, true, false);
	if (status != CardProcSuccess) {
		strcpy(xml, szRead);
		return status;
	}
	return his.getHISInfo(szRead, xml, true);
}

int __stdcall iReadHISInfoLog(char *pszCardCheckWSDL, char *pszCardServerURL, char *pszLogXml, char *xml)
{
	char szRead[BUFF_SIZE];
	memset(szRead, 0, sizeof(szRead));
	HISProcess his(HIS_FLAG);
	int status = his.readHISBaseInfo(pszCardCheckWSDL,pszCardServerURL, pszLogXml, szRead, true, false);
	if (status != CardProcSuccess) {
		strcpy(xml, szRead);
		return status;
	}
	his.setLogParams(0, "iReadHISInfoLog");
	return his.getHISInfo(szRead, xml, true);
}

int __stdcall iReadOnlyHISLog(char *pszLogXml, char *xml)
{
	char szRead[BUFF_SIZE];
	memset(szRead, 0, sizeof(szRead));
	HISProcess his(HIS_FLAG);
	int status = his.readHISBaseInfo("", "", pszLogXml,  szRead, false, false);
	if (status != CardProcSuccess) {
		strcpy(xml, szRead);
		return status;
	}
	his.setLogParams(0, "iReadOnlyHISLog");
	return his.getHISInfo(szRead, xml, true);
}


int __stdcall iReadInfoForXJLog(char *pszCardCheckWSDL, char *pszCardServerURL, char *pszLogXml, char *xml) 
{
	char szRead[BUFF_SIZE];
	memset(szRead, 0, sizeof(szRead));
	HISProcess his(XJ_FALG);
	int status = his.readHISBaseInfo(pszCardCheckWSDL, pszCardServerURL, pszLogXml, szRead, true, false);
	if (status != CardProcSuccess) {
		strcpy(xml, szRead);
		return status;
	}
	his.setLogParams(0, "iReadInfoForXJLog");
	return his.getHISInfo(szRead, xml, true);
}

int __stdcall iReadHISInfoLocal(char *pszLogXml, char *xml)
{
	char szRead[BUFF_SIZE];
	memset(szRead, 0, sizeof(szRead));
	HISProcess his(HIS_FLAG);
	int status = his.readHISBaseInfo("", "", pszLogXml, szRead, true, true);
	if (status != CardProcSuccess) {
		strcpy(xml, szRead);
		return status;
	}
	his.setLogParams(0, "iReadHISInfoLocal");
	return his.getHISInfo(szRead, xml, true);
}

int __stdcall iReadInfoForXJLocal(char *pszLogXml, char *xml) 
{
	char szRead[BUFF_SIZE];
	memset(szRead, 0, sizeof(szRead));

	HISProcess his(XJ_FALG);
	int status = his.readHISBaseInfo("", "", pszLogXml, szRead, true, true);
	if (status != CardProcSuccess) {
		strcpy(xml, szRead);
		return status;
	}
	his.setLogParams(0, "iReadInfoForXJLocal");
	return his.getHISInfo(szRead, xml, true);
}

int __stdcall iReadOnlyHISLocal(char *pszLogXml, char *xml)
{
	char szRead[BUFF_SIZE];
	memset(szRead, 0, sizeof(szRead));
	HISProcess his(HIS_FLAG);
	int status = his.readHISBaseInfo("" , "", pszLogXml, szRead, false, true);
	if (status != CardProcSuccess) {
		strcpy(xml, szRead);
		return status;
	}
	his.setLogParams(0, "iReadOnlyHISLocal");
	return his.getHISInfo(szRead, xml, true);
}
