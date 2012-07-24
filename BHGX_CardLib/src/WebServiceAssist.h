#ifndef _WEBASSIST_H
#define _WEBASSIST_H

#include <map>
#include <string>

#pragma warning (disable : 4996)

//#include "Markup.h"
#include "../tinyxml/headers/tinyxml.h"

#pragma comment(lib, "tinyxml/libs/tinyxmld.lib")

std::map<std::string, std::string> m_mapCodeDesc;
std::map<int, std::string> m_mapCardStatus;

using namespace std;

void CreateResponXML(int nID, const char *szResult, char *RetXML)
{
	TiXmlDocument *XmlDoc;
	TiXmlElement *RootElement;
	TiXmlElement *Segment;
	TiXmlDeclaration HeadDec;
	TiXmlPrinter Printer;

	// ����XML�ĵ�
	XmlDoc = new TiXmlDocument();

	// ����XML��ͷ��˵��
	HeadDec.Parse("<?xml version=\"1.0\" encoding=\"gb2312\" ?>", 0, TIXML_ENCODING_UNKNOWN);
	XmlDoc->LinkEndChild(&HeadDec);

	RootElement = new TiXmlElement("CardProcess");
	Segment = new TiXmlElement("ReturnInfo");
	Segment->SetAttribute("ID", nID);
	Segment->SetAttribute("Desc", szResult);

	RootElement->LinkEndChild(Segment);
	XmlDoc->LinkEndChild(RootElement);

	XmlDoc->Accept(&Printer);
	strcpy(RetXML, Printer.CStr());
}

int GetCheckState(const std::string &strXML, std::string &strRetCode, std::string &strSec)
{
	size_t nPos = strXML.find("\t");
	if (nPos != -1)
	{
		strRetCode = strXML.substr(0, nPos);
		strSec = strXML.substr(nPos+1, strXML.size());
		return 0;
	}
	else
	{
		strRetCode = strXML;
	}
	return -1;
}

static int _FormatWrite(std::map<int, std::string> &mapAll, char *strFMTWrite)
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

int FormatWriteInfo(const char *strWrite, char *strFMTWrite)
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
	if(Segment)
	{
		Colum = Segment->FirstChildElement();
		std::string strValue;
		while (Colum)
		{
			int nID = atoi(Colum->Attribute("ID"));
			strValue = Colum->Attribute("VALUE");
			mapAll[nID] = strValue;
			Colum = Colum->NextSiblingElement();
		}
	}

	_FormatWrite(mapAll, strFMTWrite);
	return 0;
}

int GetQueryMapInfo(char *QueryXML, std::map<std::string,std::string> &mapQuery)
{
	TiXmlDocument XmlDoc;

	TiXmlElement  *RootElement;
	TiXmlElement  *Segment;
	TiXmlElement  *Colum;
	XmlDoc.Parse(QueryXML);
	RootElement = XmlDoc.RootElement();
	Segment = RootElement->FirstChildElement();
	if(Segment)
	{
		Colum = Segment->FirstChildElement();
		if (Colum)
		{
			std::string szValue = Colum->Attribute("VALUE");
			std::string szName = Colum->Attribute("Source");
			mapQuery[szName] = szValue;
		}
	}
	return mapQuery.size() > 0;
}

int GetQueryInfo(char *QueryXML, std::string &szCardNO)
{
	TiXmlDocument XmlDoc;

	TiXmlElement  *RootElement;
	TiXmlElement  *Segment;
	TiXmlElement  *Colum;
	XmlDoc.Parse(QueryXML);
	RootElement = XmlDoc.RootElement();
	Segment = RootElement->FirstChildElement();
	if(Segment)
	{
		Colum = Segment->FirstChildElement();
		if (Colum)
		{
			szCardNO = Colum->Attribute("VALUE");
			return 0;
		}
	}
	return -1;;
}

int CreateCheckWsdlParams(const char *CardID, const char *pszCardCheckWSDL, char *strParams)
{
	sprintf(strParams, "<root><system><parm name=\"functionName\" value=\" CardCheckForNH\"/>"
		"<parm name=\"divisionCode\" value=\"\"/></system><parms><parm name=\"cardID \" value=\"%s\"/>"
		"<parm name=\" pszCardCheckWSDL \" value=\"%s\"/></parms></root>", CardID, pszCardCheckWSDL);
	strParams[strlen(strParams)] = 0;
	return 0;
}

int CreateRegWsdlParams(const char *CardID, char *strParams)
{
	sprintf(strParams, "<root><system><parm name=\"functionName\" value=\" GetRewritePackage\"/>"
		"<parm name=\"divisionCode\" value=\"\"/></system><parms><parm name=\"cardID  \" value=\"%s\"/>"
		"</parms></root>", CardID);
	strParams[strlen(strParams)] = 0;
	return 0;
}

int GetCheckRetDesc(const std::string &strStatus, std::string &strDesc)
{
	if (m_mapCodeDesc.size() == 0)
	{
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
	if (iter != m_mapCodeDesc.end())
	{
		strDesc = iter->second;
	}
	else
	{
		strDesc = strStatus;
	}
	return (strStatus == std::string("I710000") || strStatus == std::string("I720000"));
}

int GetCardStatus(int nStatus, std::string &strStatus)
{
	
	if (m_mapCardStatus.size() == 0)
	{
		m_mapCardStatus[0] = "������";
		m_mapCardStatus[1] = "���ڿ��𻵻��Ѳ����¿����˿��ѱ�ע��";
		m_mapCardStatus[2] = "�˿��Ѱ����ʧҵ���ݲ���ʹ��";
		m_mapCardStatus[3] = "����Ч";
	}
	std::map<int,std::string>::iterator iter = m_mapCardStatus.begin();
	iter = m_mapCardStatus.find(nStatus);
	if (iter != m_mapCardStatus.end())
	{
		strStatus = iter->second;
	}
	else
	{
		strStatus = "δ֪����";
	}
	return (nStatus == 0);
}

bool IsMedicalID(const std::string &strID)
{
	for (size_t i=0; i<strID.size(); ++i)
	{
		char ID = strID[i];
		if (ID != 0x30)
		{
			return true;
		}
	}
	return false;
}

#endif
