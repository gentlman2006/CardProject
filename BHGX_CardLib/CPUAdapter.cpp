#include <memory>
#include <map>
#include <vector>
#include "CPUAdapter.h"
#include "CPUCard.h"
#include "device.h"
#include "resource.h"
#include "StringUtil.h"
#include "SegmentHelper.h"
#include "public/Markup.h"
#include "public/debug.h"
#include "public/liberr.h"
#include "public/TimeUtil.h"
#include "public/XmlUtil.h"
#include "public/ConvertUtil.h"
#include "public/algorithm.h"

#pragma comment(lib, "tinyxml/libs/tinyxmld.lib")

using namespace std;

CardOps g_CpuCardOps;
static CSegmentHelper *g_SegHelper = NULL;

extern "C" {
	struct RecFolder g_recIndex[30];
}


#define  INSERT_SEGS(list, seg, id, name)											\
	memcpy(g_recIndex[id-1].section, g_recIndex[seg->ID-1].section, 10);            \
	memcpy(g_recIndex[id-1].subSection, g_recIndex[seg->ID-1].subSection, 10);      \
	memcpy(g_recIndex[id-1].fileName, name, strlen(name));							\
	seg->ID = id;																	\
	list->SegTailer->Next = seg;													\
	list->SegTailer = seg;															\

struct XmlProgramS *g_CpuXmlListHead = NULL;


typedef  std::map<std::string, struct XmlColumnS *> XmlColumnMapT;
XmlColumnMapT XmlColumnMap; 

static int InitionCpuGList(char *xmlstr);

/**
*
*/
static int InitCpuGlobalList() 
{
	if (g_CpuXmlListHead) {
		return 0;
	}

	// ����Դ�ļ������ȡXML�ļ����ҳ�ʼ����
	HINSTANCE hInstance = ::LoadLibrary("BHGX_CardLib.dll");
	int res_id = IDR_XML4;
	if ((CPU_M1 | CPU_ONLY) == 1) {
		res_id = IDR_XML6;
	} else if ((CPU_8K | CPU_8K_TEST | CPU_8K_ONLY) == 1) {
		res_id = IDR_XML5;
	} else if ( CPU_16K == 1) {
		res_id = IDR_XML7;
	}
	
	HRSRC hResInfo = ::FindResource(hInstance, MAKEINTRESOURCE(res_id), "XML");
	HGLOBAL hgRes = ::LoadResource(hInstance, hResInfo);
	DWORD cbRes = ::SizeofResource(hInstance, hResInfo);
	char *pvRes = (char *)::LockResource(hgRes);
	if(!pvRes){	
		LogPrinter("����xml�ļ�����\n");
		return CardInitErr;
	}

	// ����Ѿ�����������
	if(g_CpuXmlListHead)
		return -2;

	// ��ʼ��ȫ���б�
	InitionCpuGList(pvRes);
	return 0;
}

static int InitionCpuGList(char *xmlstr)
{
	if (g_CpuXmlListHead == NULL){
		g_CpuXmlListHead = (struct XmlProgramS*)malloc(sizeof(struct XmlProgramS));
		g_CpuXmlListHead->SegHeader = g_CpuXmlListHead->SegTailer = NULL;
	}

	struct ColCell 
	{
		std::string Source;
		int itemtype;
		int nByte;
	};
	struct XmlSegmentS *pTmp = NULL;
	CMarkup xml;
	int nSegID = 0;
	xml.SetDoc(xmlstr);
	if (!xml.FindElem("PROGRAMS")){
		return -1;
	}
	xml.IntoElem();
	if (! xml.FindElem("PROGRAM")){
		return -1;
	}
	std::string AppContext = xml.GetAttrib("Context");

	xml.IntoElem();
	while (xml.FindElem("SEGMENT")) {
		std::string szTmp;
		int RecFlag = atoi(xml.GetAttrib("REC").c_str());
		struct XmlSegmentS *pSeg = (struct XmlSegmentS*)malloc(sizeof(struct XmlSegmentS));
		pSeg->ColumnHeader = pSeg->ColumnTailer = NULL;
		pSeg->Next = NULL;
		pSeg->datatype = (eFileType)RecFlag;
		pSeg->offset = 0;
		pSeg->ID = atoi(xml.GetAttrib("ID").c_str());
		nSegID = pSeg->ID;
		szTmp = xml.GetAttrib("SECTION").c_str();
		memcpy(g_recIndex[nSegID-1].section, szTmp.c_str(),szTmp.size());

		szTmp = xml.GetAttrib("SUBSECTION").c_str();
		memcpy(g_recIndex[nSegID-1].subSection, szTmp.c_str(), szTmp.size());
		memcpy(pSeg->Target, xml.GetAttrib("SOURCE").c_str(),sizeof(pSeg->Target));

		std::string cFileType = xml.GetAttrib("FILE");

		xml.IntoElem();
		int nIDCounts = 0, subflag=0;
		int nOffset = 0;
		XmlColumnS *pColTmp = NULL;
		while (xml.FindElem("COLUMN")) {
			++nIDCounts;
			XmlColumnS *pColumnS = (XmlColumnS*)malloc(sizeof(XmlColumnS));
			pColumnS->Next = NULL;
			pColumnS->parent = pSeg;
			strcpy(pColumnS->Source, xml.GetAttrib("SOURCE").c_str());
			if (nSegID < 11) 
				pColumnS->ID = atoi(xml.GetAttrib("ID").c_str());
			else
				pColumnS->ID = nIDCounts;

			pColumnS->CheckInfo.CpuInfo.ColumnByte = atoi(xml.GetAttrib("OCCUPYBYTE").c_str());
			pColumnS->Offset = nOffset;
			pColumnS->CheckInfo.CpuInfo.itemtype = (eItemType)atoi(xml.GetAttrib("TYPE").c_str());
			nOffset += pColumnS->CheckInfo.CpuInfo.ColumnByte;
			pColumnS->Value = NULL;

			//���뵽Segment��
			if (pSeg->ColumnHeader == NULL) {
				pSeg->ColumnHeader = pColumnS;
				pSeg->ColumnTailer = pSeg->ColumnHeader;

			}else {
				pSeg->ColumnTailer->Next = pColumnS;
				pSeg->ColumnTailer = pColumnS;
			}

			//���������У������м��뵽������
			subflag = atoi(xml.GetAttrib("SUB").c_str());
			if (subflag == 1) {
				xml.IntoElem();
				while (xml.FindElem("SUBCOL"))
				{
					int counts = atoi(xml.GetAttrib("COUNT").c_str());
					std::vector<ColCell*> vecCell;
					xml.IntoElem();
					while (xml.FindElem("SUBCOLUMN"))
					{
						ColCell *pCell = new ColCell;
						pCell->nByte = atoi(xml.GetAttrib("OCCUPYBYTE").c_str());
						pCell->itemtype = atoi(xml.GetAttrib("TYPE").c_str());
						pCell->Source = xml.GetAttrib("SOURCE");
						vecCell.push_back(pCell);
					}
					xml.OutOfElem();

					char SourceName[30];
					memset(SourceName, 0, sizeof(SourceName));
					for (int Index = 0; Index < counts; ++Index)
					{
						for (size_t i=0; i<vecCell.size(); ++i)
						{
							ColCell *pCell = vecCell[i];
							++nIDCounts;
							XmlColumnS *pColumnS = (XmlColumnS*)malloc(sizeof(XmlColumnS));
							pColumnS->Next = NULL;
							sprintf(SourceName, "%s%d", pCell->Source.c_str(), Index+1);
							strcpy(pColumnS->Source, SourceName);
							pColumnS->ID = nIDCounts;
							pColumnS->CheckInfo.CpuInfo.ColumnByte = pCell->nByte;
							pColumnS->Offset = nOffset;
							pColumnS->CheckInfo.CpuInfo.itemtype = (eItemType)pCell->itemtype;
							pColumnS->parent = pSeg;
							nOffset += pColumnS->CheckInfo.CpuInfo.ColumnByte;
							pColumnS->Value = NULL;

							pSeg->ColumnTailer->Next = pColumnS;
							pSeg->ColumnTailer = pColumnS;
						}
					}

					for (size_t i=0; i<vecCell.size(); ++i)
					{
						ColCell *pCell = vecCell[i];
						delete pCell;
					}
					vecCell.clear();
				}
				xml.OutOfElem();
			}
		}
		xml.OutOfElem();

		if (g_CpuXmlListHead->SegHeader == NULL) {
			g_CpuXmlListHead->SegHeader = pSeg;
			g_CpuXmlListHead->SegTailer = pSeg;
			memcpy(g_recIndex[nSegID-1].fileName, cFileType.c_str(), cFileType.size());
		} else {

			if (cFileType == std::string("EE01--03")) {
				INSERT_SEGS(g_CpuXmlListHead, pSeg, nSegID, "EE01");
				pTmp = g_SegHelper->CloneSegment(pSeg, 0);
				nSegID++;
				INSERT_SEGS(g_CpuXmlListHead, pTmp, nSegID, "EE02")
					nSegID++;
				pTmp = g_SegHelper->CloneSegment(pSeg, 0);
				INSERT_SEGS(g_CpuXmlListHead, pTmp, nSegID, "EE03")
			} else if (cFileType == std::string("ED01--05")) {

				INSERT_SEGS(g_CpuXmlListHead, pSeg, nSegID, "ED01")
					pTmp = g_SegHelper->CloneSegment(pSeg, 0);
				nSegID++;
				INSERT_SEGS(g_CpuXmlListHead, pTmp, nSegID, "ED02")
					nSegID++;
				pTmp = g_SegHelper->CloneSegment(pSeg, 0);
				INSERT_SEGS(g_CpuXmlListHead, pTmp, nSegID, "ED03")
					nSegID++;
				pTmp = g_SegHelper->CloneSegment(pSeg, 0);
				INSERT_SEGS(g_CpuXmlListHead, pTmp, nSegID, "ED04")
					nSegID++;
				pTmp = g_SegHelper->CloneSegment(pSeg, 0);
				INSERT_SEGS(g_CpuXmlListHead, pTmp, nSegID, "ED05")
			} else {

				memcpy(g_recIndex[nSegID-1].fileName, cFileType.c_str(), cFileType.size());
				g_CpuXmlListHead->SegTailer->Next = pSeg;
				g_CpuXmlListHead->SegTailer = pSeg;
			}
		}

	}
	xml.OutOfElem();
	xml.OutOfElem();

	return 0;
}

static int  CpuConvertXmlByList(struct XmlSegmentS *listHead, 
								char *xml, int *length)
{
	struct XmlSegmentS *SegmentElement = NULL;
	struct XmlColumnS *ColumnElement = NULL;

	TiXmlDocument *XmlDoc;
	TiXmlElement *RootElement;
	TiXmlDeclaration HeadDec;
	TiXmlElement *Segment;
	TiXmlElement *Cloumn;
	TiXmlPrinter Printer;

	char buf[10];

	// ����XML�ĵ�
	XmlDoc = new TiXmlDocument();

	// ����XML��ͷ��˵��
	HeadDec.Parse("<?xml version=\"1.0\" encoding=\"gb2312\" ?>", 0, TIXML_ENCODING_UNKNOWN);
	XmlDoc->LinkEndChild(&HeadDec);

	// ����TiXMLDoc�ĵ�
	RootElement = new TiXmlElement("SEGMENTS");
	RootElement->SetAttribute("PROGRAMID", "001");

	for(SegmentElement = listHead; SegmentElement; 
		SegmentElement = SegmentElement->Next){
			memset(buf, 0, 10);
			sprintf_s(buf, 10, "%d", SegmentElement->ID);

			Segment = new TiXmlElement("SEGMENT");
			Segment->SetAttribute("ID",buf);

			int nNameSeg = SegmentElement->ID;

			for(ColumnElement = SegmentElement->ColumnHeader; ColumnElement; 
				ColumnElement = ColumnElement->Next){
					memset(buf, 0, 10);
					sprintf_s(buf, 10, "%d", ColumnElement->ID);

					Cloumn = new TiXmlElement("COLUMN");
					Cloumn->SetAttribute("ID", buf);
					Cloumn->SetAttribute("SOURCE", ColumnElement->Source);

					Cloumn->SetAttribute("VALUE", ColumnElement->Value);

					Segment->LinkEndChild(Cloumn);   
			}
			RootElement->LinkEndChild(Segment);
	}
	XmlDoc->LinkEndChild(RootElement);

	// ��XML�ĵ������ݴ����ϲ�
	XmlDoc->Accept(&Printer);
	*length = (int)Printer.Size();
	memcpy(xml, Printer.CStr(), *length);

	return 0;
}


/**
* CloneColmn - ��¡XmlColmnԪ�ص����ݽṹ
* @ ClomnElement ����¡��Ԫ��
*/

static void CpuCallocForColmn(struct XmlColumnS *result) 
{
	int length = result->CheckInfo.CpuInfo.ColumnByte; 
	int padding = 0;
	struct XmlSegmentS *pSeg = result->parent;

	if (pSeg->datatype == eRecType) {
		// rec file dataformat: TLV type:length:value
		padding = 2;
	}

	if (result->CheckInfo.CpuInfo.itemtype != eAnsType) {
		length *= 2; 
	}
	length += padding + 1;
	result->Value = (char*)malloc(length);
	memset(result->Value, 0, length);
}


static struct XmlSegmentS*  CpuConvertXmltoList(char *xml)
{
	struct XmlSegmentS *SegmentElement = NULL;
	struct XmlSegmentS *CurrSegmentElement = NULL;
	struct XmlSegmentS *TempSegmentElement = NULL;
	struct XmlSegmentS *result = NULL;

	struct XmlColumnS  *ColumnElement = NULL;
	struct XmlColumnS  *CurrColumnElement = NULL;
	struct XmlColumnS  *TempColumnElement = NULL;

	TiXmlDocument XmlDoc;
	TiXmlElement  *RootElement;
	TiXmlElement  *Segment;
	TiXmlElement  *Colum;
	char *HexString = NULL;
	char *tmpString = NULL;
	int ElemLen = 0, padding = 0;
	int realLen = 0;

	struct XmlSegmentS *XmlListHead = g_CpuXmlListHead->SegHeader;

	// ����XML���
	XmlDoc.Parse(xml);
	RootElement = XmlDoc.RootElement();

	Segment = RootElement->FirstChildElement();
	while(Segment){
		int ID = atoi(Segment->Attribute("ID"));
		SegmentElement = g_SegHelper->FindSegmentByID(XmlListHead, ID);
		if (NULL == SegmentElement)
			break;

		TempSegmentElement = (struct XmlSegmentS *)malloc(sizeof(struct XmlSegmentS));
		memcpy(TempSegmentElement, SegmentElement, sizeof(struct XmlSegmentS));
		TempSegmentElement->ColumnHeader = NULL;
		TempSegmentElement->ColumnTailer = NULL;
		TempSegmentElement->Next = NULL;

		// �����µ�Ԫ��
		if(result){
			CurrSegmentElement->Next = TempSegmentElement;
			CurrSegmentElement = TempSegmentElement;
		} else{
			CurrSegmentElement = TempSegmentElement;
			result = CurrSegmentElement;
		}

		padding = 0;
		Colum = Segment->FirstChildElement();
		while(Colum){
			int nColumnID = atoi(Colum->Attribute("ID"));
			ColumnElement = g_SegHelper->FindColumnByID(SegmentElement->ColumnHeader, nColumnID);

			if (NULL == ColumnElement) {
				Colum = Colum->NextSiblingElement();
				continue;  //search for the next column node
			}

			TempColumnElement = g_SegHelper->CloneColmn(ColumnElement, 1);
			if (TempColumnElement == NULL) {
				Colum = Colum->NextSiblingElement();
				continue;  //search for the next column node
			}

			//Ϊ�˽�ʡ�ռ�
			if (TempSegmentElement->datatype == eRecType) {
				padding = 2; //��¼�ļ���Ҫ���2���ֽ�
			}
			std::string strColum = Colum->Attribute("VALUE");
			ElemLen = (int)strColum.length();

			//����ÿ���ֶ��ַ�ת������ΪAns����ʱ����Ҫת��
			CpuInfo info = TempColumnElement->CheckInfo.CpuInfo;

			//��������ת��ans cn b����
			if (info.itemtype != eAnsType) {
				tmpString = NULL;
				char tmpArray[64];
				memset(tmpArray, 0, sizeof(tmpArray));
				HexString = TempColumnElement->Value;
				memset(TempColumnElement->Value, 0xFF, info.ColumnByte * 2);
				ElemLen = ElemLen < info.ColumnByte * 2 ? ElemLen : info.ColumnByte * 2;

				if (ElemLen > sizeof(tmpArray)) {
					tmpString = (char*)malloc(info.ColumnByte * 2 + 1);
					memset(tmpString, 0, info.ColumnByte * 2 + 1);
				} else {
					tmpString = &tmpArray[0];
				}

				if (ElemLen % 2) { //����2�ı���ʱ�����䡮f��
					tmpString[ElemLen] = 'f';
				}
				memcpy(tmpString, strColum.c_str(), ElemLen);

				//��С����ת��Ϊ'a'
				if (info.itemtype == eCnType) {
					size_t pos = strColum.find('.');
					if (pos != -1) {
						*(tmpString + pos) = 'a';
					}
				}

				HexstrToBin((BYTE*)HexString+padding, (BYTE*)tmpString, ElemLen);

				if (ElemLen > sizeof(tmpArray)) {
					SAFE_DELETE_C(tmpString);
				}
			} else {
				HexString = TempColumnElement->Value;
				realLen = info.ColumnByte < ElemLen ? info.ColumnByte : ElemLen;
				memcpy(HexString+padding, strColum.c_str(), realLen);
			}

			// ��������
			if(CurrSegmentElement->ColumnHeader){
				CurrSegmentElement->ColumnTailer->Next = TempColumnElement;
				CurrSegmentElement->ColumnTailer = TempColumnElement;
			} else{
				CurrSegmentElement->ColumnHeader = TempColumnElement;
				CurrSegmentElement->ColumnTailer = TempColumnElement;
			}

			// ������
			Colum = Colum->NextSiblingElement();
		}

		// ������
		Segment = Segment->NextSiblingElement();
	}

	// ���ؽ��
	return result;
}


CardOps * __stdcall InitCpuCardOps()
{
	g_CpuCardOps.cardAdapter = InitCpuAdapter();
	g_CpuCardOps.iCallocForColmn = CpuCallocForColmn;
	g_CpuCardOps.iConvertXmlByList = CpuConvertXmlByList;
	g_CpuCardOps.iConvertXmltoList = CpuConvertXmltoList;
	g_CpuCardOps.iInitGList = InitCpuGlobalList;

	g_CpuCardOps.iInitGList();
	g_CpuCardOps.programXmlList = g_CpuXmlListHead;

	if (g_SegHelper == NULL) {
		g_SegHelper = new CSegmentHelper(&g_CpuCardOps);
	}
	return &g_CpuCardOps;
}

void __stdcall CPUClear()
{
	SAFE_DELETE(g_SegHelper);
	SAFE_DELETE_C(g_CpuXmlListHead);
}