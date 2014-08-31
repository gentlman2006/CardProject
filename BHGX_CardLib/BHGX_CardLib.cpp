#include <vector>
#include <stdio.h>
#include <time.h>
#include <string>
#include <map>

#include "BHGX_CardLib.h"
#include "ns_pipeClient/n_USCOREapiSoap.nsmap"
#include "ns_pipeClient/soapn_USCOREapiSoapProxy.h"
#include "public/debug.h"
#include "public/liberr.h"
#include "tinyxml/headers/tinyxml.h"

#include "Card.h"
#include "CPUCard.h"
#include "M1Card.h"
#include "adapter.h"
#include "CPUAdapter.h"
#include "public/algorithm.h"
#include "public/Markup.h"
#include "Encry/DESEncry.h"
#include "public/Authority.h"
#include "public/TimeUtil.h"
#include "public/XmlUtil.h"
#include "public/ExceptionCheck.h"
#include "SegmentHelper.h"
#include "resource.h"
#include "BHGX_Printer.h"
#include "StringUtil.h"
#include "public/ConvertUtil.h"
#include "WebServiceAssist.h"

using namespace std;
#pragma warning (disable : 4996)
#pragma warning (disable : 4267)
#pragma warning (disable : 4020)
#pragma comment(lib, "tinyxml/libs/tinyxmld.lib")

#define DBGCore(format, ...) LogWithTime(0, format)

#define SAFE_DELETE(a)  if (a != NULL) { delete(a);a = NULL;}
#define SAFE_DELETE_C(a)  if (a != NULL) { free(a);a = NULL;}

#define NR_MASK(nr) (1 << nr)
#define NOT_NR_MASK(nr) ~(1 << nr)

#define SETBIT(byte, nr) byte |= NR_MASK(nr)
#define CLRBIT(byte, nr) byte &= NOT_NR_MASK(nr)

#define SAFEARRAY_DELETE(a)  if (a != NULL) { delete [] a ;a = NULL;}

#define ISSCANCARD { if (iScanCard() != 0) return CardScanErr;}
											
#define ISGWCARD(a) ((a[0]) == '1')

#define SCANCARD_XML(xml, ret)					\
	if (iScanCard() != 0) {						\
		CreateResponXML(3, "Ѱ��ʧ��", ret);	\
		strcpy(xml, ret);						\
		return CardScanErr;						\
	}											\



#define TIMEOUT		15000
#define SEGBASE		100
#define ASSERT_INIT(a)					\
	if (a != TRUE){						\
		return CardInitErr;				\
	}									\


#define ASSERT_OPEN(a)					\
	if (a != TRUE)   {					\
		return CardNotOpen;				\
	}									\

struct QueryColum
{
	int nSegID;
	int nID;
	std::string szSource;
	std::string szValue;
	QueryColum()
		:nSegID(-1)
		,nID(-1)
		,szSource("unknown")
		,szValue("unknown")
	{}
	QueryColum(int segId, int id, char *source, char *value)
		:nSegID(segId)
		,nID(id)
		,szSource(source)
		,szValue(value)
	{}

	QueryColum(const QueryColum &item)
		:nID(item.nID),
		nSegID(item.nSegID),
		szSource(item.szSource),
		szValue(item.szValue)
	{}

	QueryColum & operator=(const QueryColum &item) {
		if (this != &item) {
			this->nID = item.nID;
			this->nSegID = item.nSegID;
			this->szSource = item.szSource;
			this->szValue = item.szValue;
		}
		return *this;
	}
	bool operator < (struct QueryColum &item) {
		return nID < item.nID;
	}
};


//cpu �� M1�໥ת��ʱ�Ķ�Ӧ��ϵ
struct dataItem 
{
	int nId;	
	int nCpuId;
	std::string source;    //M1
	std::string value;
	dataItem ()
		:nId(-1)
		,nCpuId(-1)
	{}
	dataItem(const dataItem &item)
		:nId(item.nId),
		nCpuId(item.nCpuId),
		source(item.source),
		value(item.value)
	{}

	dataItem & operator=(const dataItem &item) {
		if (this != &item) {
			this->nId = item.nId;
			this->nCpuId = item.nCpuId;
			this->source = item.source;
			this->value = item.value;
		}
		return *this;
	}
	bool operator < (struct dataItem &item) {
		return nId < item.nId;
	}
};


//source �� value��Ĭ�϶�Ӧֵ
std::map<string, QueryColum> g_sourceValueMap;
std::map<string, int> g_segMap;


CBHGX_Printer *m_pBHPrinter = NULL;
BOOL g_bPreLoad = FALSE;
BOOL g_bCardOpen = FALSE;

static CardOps *g_CardOps = NULL;
static CardOps *g_CpuCardOps = NULL;
static CardOps *g_M1CardOps = NULL;
static CSegmentHelper *g_SegHelper = NULL;

/**
 * ȫ�ֵ����ݽṹ
 */
struct XmlProgramS *g_XmlListHead = NULL;

//��CPU��ID��ӳ�䵽M1�У����н����������Ŵ����0, 
//CPU���з�����������,������������ֱ�����2,5��
int cpuInM1Index[29] = {839,108, 103, -1, -1, 104, -1, 840,
						-1, 211, 215, 214, 212, -1, -1, -1,
						-1, 213, -1, 735, -1, -1,416, -1,
						625, 627, -1, -1, -1,};
#define  CPU_WRITE_DOWN  16

char *M1Reserver[29] = {
	"","","","000000","000000","","","",
	"","","","","","","","",
	"","","","","","","","",
	"","","","","",
};

char *M1SourceReserver[29] = {
	"HEALTHRECORDNO","CARDNO","RESERVE1","PASSWORD","STAGENO",
	"RESERVE2","ORGANIZATIONNO","MEDICARECERTIFICATENO",
	"LASTUPDATEDATE","NAME","IDNUMBER","BIRTHDAY","GENDER",
	"HOUSEHOLDERRELATION","PEOPLETYPE","HOUSEHOLDTYPE",
	"CIVILAFFAIRSFLAG","NATIONALITY","HEALTHSTATE","CIVILSTATE",
	"MARRYRELATIVESFLAG","FIRSTPARTAKEDATE","PHONENO","RESERVE4",
	"LINKMANNAME","LINKMANPHONENO","CHRONICCODE1","CHRONICCODE2","RESERVE5",
};

static void 
DestroyList(struct XmlSegmentS *listHead, int mode);

static void 
ReadConfigFromReg(char *name);

static int
QueryItem(CardType type, const char *name, char *xml,
		   int &nLen, QueryColum &stQuery);

static int
QueryItem(CardType type, const char *name, char *xml, int &nLen);

static int 
ParseValueQuery(char *source, std::string &szResult);

static struct XmlColumnS * 
M1QueryItem(const char *name, char *xml, int &nLen);

static struct XmlColumnS * 
CpuQueryItem(const char *name, char *xml, int &nLen);

static int 
iCreateXmlByVector(const vector<QueryColum>&  v, char *xml, int *length);

static int InitCardOps() 
{
	return 0;
}
//*************************************

static BOOL isWriteable(int cpuIndex) 
{
	return cpuIndex < CPU_WRITE_DOWN;
}

static int M1ConvertXmlByArray(const std::vector<struct dataItem> &vecItem, int segId, char *xml, int *length) 
{
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

	memset(buf, 0, 10);
	sprintf_s(buf, 10, "%d", segId);

	Segment = new TiXmlElement("SEGMENT");
	Segment->SetAttribute("ID",buf);

	for (size_t i=1; i<vecItem.size(); i++) {
		struct dataItem  item = vecItem[i];
		memset(buf, 0, 10);
		sprintf_s(buf, 10, "%d", item.nId);

		Cloumn = new TiXmlElement("COLUMN");
		Cloumn->SetAttribute("ID", buf);
		//Cloumn->SetAttribute("SOURCE", item.source.c_str());
		Cloumn->SetAttribute("VALUE", item.value.c_str());

		Segment->LinkEndChild(Cloumn); 
	}
	RootElement->LinkEndChild(Segment);
	if (vecItem[1].value.size() > 0 ) {
		memset(buf, 0, 10);
		sprintf_s(buf, 10, "%d", 5);

		Segment = new TiXmlElement("SEGMENT");
		Segment->SetAttribute("ID",buf);
		Cloumn = new TiXmlElement("COLUMN");

		sprintf_s(buf, 10, "%d", 1);
		Cloumn->SetAttribute("ID", buf);
		Cloumn->SetAttribute("VALUE", vecItem[0].value.c_str());
		Segment->LinkEndChild(Cloumn); 

		RootElement->LinkEndChild(Segment);
	}


	XmlDoc->LinkEndChild(RootElement);
	  
	// ��XML�ĵ������ݴ����ϲ�
	XmlDoc->Accept(&Printer);
	*length = (int)Printer.Size();
	memcpy(xml, Printer.CStr(), *length);
	return 0;

}




static int GetCpuReadFlag(const std::map<int, dataItem> &mapInfo) 
{
	std::map<int, dataItem>::const_iterator iter = mapInfo.begin();
	int offset = 0, flag = 0;
	for (; iter != mapInfo.end(); iter++)
	{
		int id = iter->first;
		if (id < 16 && id != 7) {
			continue;
		} else if (id < 21) {
			offset = 3;
		} else if (id < 25) {
			offset = 4;
		} else if (id < 34) {
			offset = 5;
		} else if (id < 37) {
			offset = 6;
		} else if (id < 41) {
			offset = 7;
		} else if (id < 56) {
			offset = 8;
		} 
		SETBIT(flag, offset);
	}
	return flag;
}

static void CpuConvertRetPos(std::vector<QueryColum> &vec) 
{
	std::map<string, int>::iterator iter = g_segMap.begin();
	for (size_t i=0; i<vec.size(); i++) 
	{
		QueryColum &query = vec[i];
		iter = g_segMap.find(query.szSource);
		if (iter != g_segMap.end()) {
			query.nSegID = iter->second / SEGBASE;
			query.nID = iter->second % SEGBASE;
		}
	}
}

static void xml2Map(char *src, std::map<int, dataItem> &mapInfo, CardType type, bool isRead = false)
{
	TiXmlDocument XmlDoc;
	TiXmlElement  *RootElement;
	TiXmlElement  *Segment;
	TiXmlElement  *Colum;

	XmlDoc.Parse(src);
	RootElement = XmlDoc.RootElement();
	if (RootElement == NULL){
		return;
	}
	Segment = RootElement->FirstChildElement();

	while (Segment)
	{
		int nSeg = atoi(Segment->Attribute("ID"));
		Colum = Segment->FirstChildElement();
		while (Colum) {
			int nColumID = atoi(Colum->Attribute("ID"));
			std::string strColum = Colum->Attribute("VALUE");
			struct dataItem item;
			item.nId = nColumID;
			item.value = strColum;
			if (isRead) {
				item.source = Colum->Attribute("SOURCE");
			}
			
			if (type == eCPUCard) {
				mapInfo[nColumID] = item;
			} else {
				mapInfo[nColumID + nSeg * SEGBASE] = item;
			}
			Colum = Colum->NextSiblingElement();
		}
		Segment = Segment->NextSiblingElement();
	}
}

static int ParseValueQuery(char *source, std::string &szResult)
{
	std::string strXML;
	char szQuery[1024];
	memset(szQuery, 0, sizeof(szQuery));

	int n = iQueryInfo(source,  szQuery);
	if (n != 0){
		return n;
	}
	GetQueryInfoForOne(szQuery, szResult);
	return 0;
}

void M12CpuMap(const std::map<int, dataItem> &mapInfo, 
			   std::map<int, dataItem> &mapCpuInfo)
{
	std::map<int, dataItem>::const_iterator iter = mapInfo.begin();
	for (; iter != mapInfo.end(); iter++) {
		int id = iter->first;
		dataItem item = iter->second;
		int SegId = 0, ColmnId = 0;
		SegId = id / SEGBASE;
		ColmnId = id % SEGBASE;
		if (SegId == 5) {
			if (cpuInM1Index[0] != -1) {
				item.nCpuId = cpuInM1Index[0] % SEGBASE;
				mapCpuInfo[item.nCpuId] = item;
			} 

		} else if (SegId == 2){
			if (cpuInM1Index[ColmnId] != -1) {
				item.nCpuId = cpuInM1Index[ColmnId] % SEGBASE;
				mapCpuInfo[item.nCpuId] = item;
			}
		}
	}
}

static int Cpu2M1Xml(char *src, char *dest, int *length) 
{
	std::map<int, dataItem> mapCpuInfo;
	xml2Map(src, mapCpuInfo, eM1Card, false);
	std::vector<struct dataItem> vecData;
	for (int i=0; i<29; i++) {
		struct dataItem item;
		item.nId = i;
		std::map<int, dataItem>::iterator iter = mapCpuInfo.end();
		int cpuId = cpuInM1Index[i];
		item.nCpuId = cpuId;
		if (cpuId == -1) {
			item.value = M1Reserver[i];
			//item.source = M1SourceReserver[i];
		} else {
			iter = mapCpuInfo.find(cpuId);
			if (iter != mapCpuInfo.end()) {
				item.value = iter->second.value;
				//item.source = iter->second.source;
			} else {
				item.value = M1Reserver[i];
				//item.source = M1SourceReserver[i];
			}
		}
		vecData.push_back(item);
	}
	return M1ConvertXmlByArray(vecData, 2, dest, length);
}

static int M12CpuXml(char *xml, const std::map<int, dataItem> &mapInfo) 
{
	TiXmlDocument XmlDoc;
	TiXmlElement  *RootElement;
	TiXmlElement  *Segment;
	TiXmlElement  *Colum;
	TiXmlPrinter Printer;
	std::map<int, dataItem>::const_iterator iter = mapInfo.begin();

	XmlDoc.Parse(xml);
	RootElement = XmlDoc.RootElement();
	if (RootElement == NULL){
		return -1;
	}
	Segment = RootElement->FirstChildElement();

	while (Segment)
	{
		int nSeg = atoi(Segment->Attribute("ID"));
		Colum = Segment->FirstChildElement();
		while (Colum)
		{
			int nColumID = atoi(Colum->Attribute("ID"));
			iter = mapInfo.find(nColumID);
			if (iter != mapInfo.end()) {
				const dataItem &item = iter->second;
				Colum->SetAttribute("VALUE", item.value.c_str());
			}
			Colum = Colum->NextSiblingElement();
		}
		Segment = Segment->NextSiblingElement();
	}
	XmlDoc.Accept(&Printer);
	strcpy(xml, Printer.CStr());
	return 0;
}

/**
 *
 */
static void DestroyList(struct XmlSegmentS *listHead, int mode)
{
	struct XmlSegmentS	*CurrSegmentElement = NULL;
	struct XmlSegmentS	*TempSegmentElement = NULL;
	struct XmlColumnS	*CurrColumnElement	= NULL;
	struct XmlColumnS	*TempColumnElement	= NULL;

	CurrSegmentElement = listHead;
	while(CurrSegmentElement)
	{
		CurrColumnElement = CurrSegmentElement->ColumnHeader;
		while(CurrColumnElement)
		{
			TempColumnElement = CurrColumnElement;
			CurrColumnElement = CurrColumnElement->Next;
			if (mode) {
				SAFE_DELETE_C(TempColumnElement->Value);
			}
			SAFE_DELETE_C(TempColumnElement);
		}
		
		TempSegmentElement = CurrSegmentElement;
		CurrSegmentElement = CurrSegmentElement->Next;

		SAFE_DELETE_C(TempSegmentElement);
	}

	return;
}



static void ReadConfigFromReg(char *name)
{
	HKEY RootKey;
	HKEY hKey;
	char szValue[256];
	DWORD dwLen;

	RootKey = HKEY_CURRENT_USER;
	DWORD dwDesc;
	DWORD dwType = REG_SZ;
	if (ERROR_SUCCESS != RegOpenKeyEx(RootKey, (LPCSTR)(MainKey), 
		(DWORD)strlen(MainKey), KEY_READ | KEY_WRITE, &hKey))
	{
		if (ERROR_SUCCESS != RegCreateKeyEx(RootKey, MainKey, 0, MainKey, 0,
			KEY_READ | KEY_WRITE, NULL, &hKey, &dwDesc)){
			RegCloseKey(hKey);
			return;
		}

		if (ERROR_SUCCESS != RegSetValueEx(hKey, "Config", NULL, dwType, 
										  (PBYTE)CONFIG, (DWORD)strlen(CONFIG)))
		{
			RegCloseKey(hKey);
			return;
		}
	}

	dwLen = 256;
	if (ERROR_SUCCESS != RegQueryValueEx(hKey, "Config", NULL, 
										&dwType, (PBYTE)szValue,&dwLen))
	{
		RegCloseKey(hKey);
		return;
	}
	szValue[dwLen] = 0;
	RegCloseKey(hKey);
	strcpy_s(name, NAME_MAX_LEN, szValue);
}


static int QueryItem(CardType type, const char *name, char *xml, 
					  int &nLen, QueryColum &stQuery)
{
	struct XmlColumnS *pColum = NULL;
	if (type == eCPUCard) {
		pColum = CpuQueryItem(name, xml, nLen);
	} else {
		pColum = M1QueryItem(name, xml, nLen);
	}
	if (pColum == NULL) {
		std::map<string, QueryColum>::iterator iter = 
			g_sourceValueMap.find(name);
		if (iter != g_sourceValueMap.end()) {
			stQuery = iter->second;
		} else {
			stQuery = QueryColum();
		}
		return 0;
	}
	stQuery.nSegID = pColum->parent->ID;
	stQuery.nID = pColum->ID;
	stQuery.szSource = pColum->Source;
	stQuery.szValue = xml;
	if (type == eCPUCard) {
		SAFE_DELETE_C(pColum->Value);
		SAFE_DELETE_C(pColum);
	}
	return 0;
}

static int QueryItem(CardType type, const char *name, char *xml, int &len) 
{
	if (type == eCPUCard) {
		CpuQueryItem(name, xml, len);
	} else {
		M1QueryItem(name, xml, len);
	}
	return 0;
}

static struct XmlColumnS* CpuQueryItem(const char *name, char *xml, int &nLen)
{
	struct RWRequestS	*RequestList = NULL;
	struct XmlColumnS * queryItem = NULL;
	struct XmlColumnS *pBinColum = NULL;
	int status = -1;

	//find segId from column name
	struct XmlSegmentS *pFindSeg = g_SegHelper->getSegmentByColumName(g_XmlListHead->SegHeader, name);
	if (pFindSeg == NULL) {
		nLen = -1;
		return NULL;   
	}

	//bin file can read by random , while others must read by one section and extract own name. 
	if (pFindSeg->datatype == eBinType) {
		pBinColum = g_SegHelper->FindColumByColumName(pFindSeg, name);
		assert(pBinColum);

		RequestList = (struct RWRequestS *)malloc(sizeof(struct RWRequestS));
		memset(RequestList, 0, sizeof(struct RWRequestS));
		RequestList->datatype = pFindSeg->datatype;
		RequestList->itemtype = pBinColum->CheckInfo.CpuInfo.itemtype;
		RequestList->length = pBinColum->CheckInfo.CpuInfo.ColumnByte;
		RequestList->nColumID = pBinColum->ID;
		RequestList->nID = pFindSeg->ID;
		RequestList->offset = pBinColum->Offset;
		RequestList->value = (BYTE*)pBinColum->Value;
	} else {
		assert(pFindSeg);

		// ��ȡ��д����
		RequestList = apt_CreateRWRequest(pFindSeg , 0, g_CardOps->cardAdapter->type);
	}

	// �豸����ʵ��ȡ
	int res = g_CardOps->cardAdapter->iReadCard(RequestList, g_CardOps->cardAdapter);

	XmlSegmentS *pSeg = pFindSeg;
	
	while(pSeg)
	{
		int nSeg = pSeg->ID;
		XmlColumnS *pColum = pSeg->ColumnHeader;
		while(pColum)
		{
			if (strcmp(name, pColum->Source) == 0) {
				nLen = strlen(pColum->Value);
				memcpy(xml, pColum->Value, nLen);

				queryItem = g_SegHelper->CloneColmn(pColum, 1);
				memcpy(queryItem->Value, pColum->Value, nLen);
				goto done;
			}
			pColum = pColum->Next;
		}
		pSeg = pSeg->Next;
	}

done:
	// ���ٶ�д��������
	apt_DestroyRWRequest(RequestList, 0);
	DestroyList(pFindSeg, 1);
	return queryItem;
}

static struct XmlColumnS*   M1QueryItem(const char *name, char *xml, int &nLen)
{
	struct RWRequestS	*RequestList = NULL;
	struct XmlColumnS   *queryItem = NULL;
	RequestList = (struct RWRequestS *)malloc(sizeof(struct RWRequestS));
	memset(RequestList, 0, sizeof(struct RWRequestS));
	unsigned char buf[160];
	XmlSegmentS *pSeg = g_XmlListHead->SegHeader;
	int nRet = -1;

	while(pSeg)
	{
		int nSeg = pSeg->ID;
		XmlColumnS *pColum = pSeg->ColumnHeader;
		while(pColum)
		{
			if (strcmp(name, pColum->Source) == 0){
				RequestList->offset = pColum->Offset;
				RequestList->length = pColum->CheckInfo.M1Info.ColumnBit;
				RequestList->value = buf;
				memset(buf, 0, 160);

				if (RequestList->length > 0){
					nRet = g_CardOps->cardAdapter->iReadCard(RequestList, g_CardOps->cardAdapter);
					if (nRet != 0){
						return NULL;
					}
					if (std::string(pColum->Source) == std::string("NAME") || 
						std::string(pColum->Source) == std::string("LINKMANNAME")){
						std::string szName;
						char strName[160];
						memset(strName, 0, sizeof(strName));
						int nLen = 0;
						memcpy(strName, buf, strlen((char*)buf) +1);
						if (Is_IntName(strName)){
							szName = CConvertUtil::uf_gbk_int_covers(strName, "togbk");
						} else {
							szName = strName;
						}
						memcpy(buf, szName.c_str(), szName.size());
						buf[szName.size()] = 0;
					}
					memcpy(xml, buf, strlen((char*)buf));
					xml[strlen((char*)buf)] = 0;
					nLen = (int)strlen(xml);
					queryItem = pColum;
					goto done;
				}
			}
			pColum = pColum->Next;
		}
		pSeg = pSeg->Next;
	}

done:
	SAFE_DELETE_C(RequestList);
	return queryItem;
}

/**
*
*/
static int iCreateXmlByVector(const vector<QueryColum>&  v, char *xml, int *length)
{
	TiXmlDocument *XmlDoc;
	TiXmlElement *RootElement;
	TiXmlDeclaration HeadDec;
	TiXmlElement *Segment;
	TiXmlElement *Cloumn;
	TiXmlPrinter Printer;
	std::map<int,std::vector<QueryColum> > mapSeg;
	for (size_t i=0; i<v.size(); ++i){
		QueryColum stQuery = v[i];
		mapSeg[stQuery.nSegID].push_back(stQuery);
	}

	int result = 0;
	// ����XML�ĵ�
	XmlDoc = new TiXmlDocument();

	// ����XML��ͷ��˵��
	HeadDec.Parse("<?xml version=\"1.0\" encoding=\"gb2312\" ?>", 0, TIXML_ENCODING_UNKNOWN);
	XmlDoc->LinkEndChild(&HeadDec);

	// ����TiXMLDoc�ĵ�
	RootElement = new TiXmlElement("SEGMENTS");
	RootElement->SetAttribute("PROGRAMID", "001");
	std::map<int,std::vector<QueryColum> >::iterator iter= mapSeg.begin();
	for (; iter!=mapSeg.end(); ++iter)
	{
		int nSeg = iter->first;
		std::vector<QueryColum> &vecQuery = iter->second;
		Segment = new TiXmlElement("SEGMENT");
		Segment->SetAttribute("ID",nSeg);

		for(size_t i = 0; i < vecQuery.size(); i++)
		{
			QueryColum &stQuery = vecQuery[i];
			Cloumn = new TiXmlElement("COLUMN");
			Cloumn->SetAttribute("ID", stQuery.nID);
			Cloumn->SetAttribute("SOURCE", stQuery.szSource.c_str());
			Cloumn->SetAttribute("VALUE", stQuery.szValue.c_str());

			Segment->LinkEndChild(Cloumn);
		}
		RootElement->LinkEndChild(Segment);
	}

	XmlDoc->LinkEndChild(RootElement);

	// ��XML�ĵ������ݴ����ϲ�
	XmlDoc->Accept(&Printer);
	*length = (int)Printer.Size();
	memcpy(xml, Printer.CStr(), *length);

	return result;
}


/**
 *
 */
int __stdcall iGetCardVersion(char *pszVersion)
{
	int res = 0;

	memcpy(pszVersion, "1.0", sizeof("1.0"));

	return res;
}

int __stdcall iCardCompany(char *szCompanyXml)
{
	return 0;
}

/**
 *
 */
int __stdcall iCardInit(char *xml)
{
	char szSystem[NAME_MAX_LEN];
	ZeroMemory(szSystem, sizeof(szSystem));

	ReadConfigFromReg(szSystem);

	// ���豸���г�ʼ��
	g_bPreLoad = (apt_InitCoreDevice(szSystem)==0);
	g_bCardOpen = g_bPreLoad;

	InitCardOps();
	return g_bPreLoad==TRUE ? 0:-1;
}

int __stdcall iCardDeinit()
{
	if (g_CpuCardOps) {
		DestroyList(g_CpuCardOps->programXmlList->SegHeader, 0);
		CPUClear();
	}

	if (g_M1CardOps) {
		DestroyList(g_M1CardOps->programXmlList->SegHeader, 0);
		M1clear();
	}
	SAFE_DELETE(g_SegHelper);

	g_XmlListHead = NULL;
	g_bPreLoad = FALSE;
	g_bCardOpen = FALSE;
	return apt_CloseCoreDevice();
}

int __stdcall iCardOpen()
{
   ASSERT_INIT(g_bPreLoad);
   if (g_bCardOpen) {
	   return 0;
   }

   int ret =  apt_OpenDev();
   if (!ret) {
	   g_bCardOpen = TRUE;
   }
   return ret;
}

int __stdcall iCardClose()
{
	ASSERT_OPEN(g_bCardOpen);
	int ret =  apt_CloseDev();
	if (!ret) {
		g_bCardOpen = FALSE;
	}
	return ret;
}


int __stdcall iScanCard()
{
	ASSERT_OPEN(g_bCardOpen);
	return apt_ScanCard();
}
/**
*
*/
char* __stdcall err(int errcode)
{
	if(errcode < 0)
		errcode = -errcode;

	return _err(errcode);
}

int __stdcall iCardIsEmpty()
{
	ASSERT_OPEN(g_bCardOpen);

	ISSCANCARD;

	char strCardNo[20];
	memset(strCardNo, 0, sizeof(strCardNo));
	int nLen = 0;
	QueryItem(g_CardOps->cardAdapter->type, "CARDNO", strCardNo, nLen);

	int status = IsAllTheSameFlag((unsigned char*)strCardNo, nLen, '0');
	strlwr(strCardNo);
	int stat = IsAllTheSameFlag((unsigned char*)strCardNo, nLen, 'f');
	return (status & stat) == 0 ? 0 : CardIsNotEmpty;
}

/**
 *
 */

static int _iReadInfo(int flag, char *xml) 
{
	struct XmlSegmentS	*list = NULL;
	struct RWRequestS	*RequestList = NULL;
	int length;
	int status = CardProcSuccess;
	
	list = g_SegHelper->GetXmlSegmentByFlag(flag);
	if (list == NULL) {
		CreateResponXML(CardXmlErr, err(CardXmlErr), xml);
		return CardXmlErr;
	}

	// ��ȡ��д����
	RequestList = apt_CreateRWRequest(list, 0, g_CardOps->cardAdapter->type);
	if (RequestList == NULL) {
		status = CardMallocFailed;
		CreateResponXML(status, err(status), xml);
		goto done;
	}

	// �豸����ʵ��ȡ
	status = g_CardOps->cardAdapter->iReadCard(RequestList, g_CardOps->cardAdapter);

	// ͨ���������XML�ַ���
	g_CardOps->iConvertXmlByList(list, xml, &length);

	// ���ٶ�д��������
	apt_DestroyRWRequest(RequestList, 0);

done:
	DestroyList(list, 1);
	return status;
}

int __stdcall iReadInfo(int flag, char *xml)
{
	ASSERT_OPEN(g_bCardOpen);
	if (xml == NULL) {
		return CardReadErr;
	}
	
	ISSCANCARD;
	int bNHInfoRead = 0;
	if (g_CardOps->cardAdapter->type == eM1Card ) {

		// ����Flag����List,�����������ڵ�����
		if ((flag & 0x2) && !(flag & 0x10)){
			flag = flag | 0x10;
		}
	} else {
		if ((flag & 0x2) == 0x2) {
			bNHInfoRead = 1;
			flag = 0x1 | 0x2 | 0x8 | 32 | 64 | 128;
		}
		
	}

	char readxml[1024*12];
	ZeroMemory(readxml, sizeof(readxml));

	int status = _iReadInfo(flag, readxml);

	//convert cpu xml to m1 xml;
	if (g_CardOps->cardAdapter->type == eM1Card) {
		strcpy(xml, readxml);
	} else if (g_CardOps->cardAdapter->type == eCPUCard) {
		if (bNHInfoRead == 1) {
			int length = 0;
			Cpu2M1Xml(readxml, xml, &length);
		} else {
			strcpy(xml, readxml);
		}
	} else {}
	return status == 0 ? CardProcSuccess : CardReadErr;
}

DLL_EXPORT int __stdcall iReadAnyInfo(int flag, char *xml, char *name)
{
	ASSERT_OPEN(g_bCardOpen);
	ISSCANCARD;

	int status = -1;
	if (flag > 0){
		status = iReadInfo(flag, xml);
	}

	if (flag<=0 && strcmp(name, "") != 0){
		status = iQueryInfo(name, xml);
	}
	return status;
}


/**
 *
 */
static int _iWriteInfo(char *xml) 
{
	struct XmlSegmentS *XmlList;
	struct RWRequestS	*RequestList = NULL;
	int status = CardProcSuccess;
		// ����List
	XmlList = g_CardOps->iConvertXmltoList(xml);

	if (XmlList == NULL){
		return CardXmlErr;
	}

	// ������д����
	RequestList = apt_CreateRWRequest(XmlList, 0, eCPUCard);
	if (RequestList == NULL) {
		status = CardMallocFailed;
		goto done;
	}

	// ���豸������ʵ��д
	status = g_CardOps->cardAdapter->iWriteCard(RequestList, g_CardOps->cardAdapter);

	// ���ٶ�д����
	apt_DestroyRWRequest(RequestList, 0);

	// ����XML����
done:
	DestroyList(XmlList, 1);
	return status;
}

//�����Ƿ�CPU�ɻ�д�� vecFlag Ϊ��¼�ļ�  vecBinΪbin�ļ�
static int checkCpuWriteXml(char *xmlStr, 
							std::vector<int> &vecFlag,
							std::vector<int> &vecBin)
{
	CMarkup xml;

	xml.SetDoc(xmlStr);
	if (!xml.FindElem("SEGMENTS")){
		return -1;
	}
	xml.IntoElem();
	while (xml.FindElem("SEGMENT")){
		int id = atoi(xml.GetAttrib("ID").c_str());
		if (id <= 3) {
			return 0;
		} else if (id < 11) {
			vecFlag.push_back(id);
		} else {
			vecBin.push_back(id);
		}
	}
	xml.OutOfElem();
	return 1;
}

static int WriteFile(char *filename,const char *xml)
{
	FILE *handle;

	fopen_s(&handle, filename, "w");
	fwrite(xml, strlen(xml), 1, handle);
	fclose(handle);

	return 0;
}

int __stdcall iWriteInfo(char *xml)
{
	ASSERT_OPEN(g_bCardOpen);
	int len = strlen(xml) + 1;
	int status = 0;
	
	std::string xmlStr(xml);
	if (len == 1 || CXmlUtil::CheckCardXMLValid(xmlStr) < 0){
		LogPrinter("CardXML:Check Error\n");
		return CardXmlErr;
	}
	
	ISSCANCARD;

	if (g_CardOps->cardAdapter->type == eM1Card) {
		status =  _iWriteInfo((char*)xmlStr.c_str());
		goto done;
	}

	if (g_CardOps->cardAdapter->type == eCPUCard) {

		std::map<int, dataItem> mapInfo, mapCpuInfo;
		int flag = 0, isCpuWrite = 0, isRec = 1;

		//add verify cpu write xml, return vector segflag
		std::vector<int> vecRecFlag;
		std::vector<int> vecBinFlag;
		isCpuWrite = checkCpuWriteXml((char*)xmlStr.c_str(), vecRecFlag, vecBinFlag);
		if (isCpuWrite == 1) {
			if (vecBinFlag.size() > 0) {
				isRec = 0;
			}

			if (isRec) {   //��¼�ļ�����¼Ҫ��д�Ŀ�
				for (size_t i=0; i<vecRecFlag.size(); i++) {
					if (vecRecFlag[i] > 3) {
						SETBIT(flag, (vecRecFlag[i]-1));
					}	
				}
				xml2Map((char*)xmlStr.c_str(), mapCpuInfo, eCPUCard, false);
			}
		} else {
			if (vecRecFlag.size() > 0 || vecBinFlag.size() > 0) {

				DBGCore("CPU���޷���д��2�����M1����");
				return CardWriteErr;
			}

			xml2Map((char*)xmlStr.c_str(), mapInfo, eM1Card, false);

			if (mapInfo.size() > 0) {
				M12CpuMap(mapInfo, mapCpuInfo);
			}
			flag = GetCpuReadFlag(mapCpuInfo);

			//û���ҵ�����д���ֶ�
			if (flag == 1 || flag == 0) {
				return CardProcSuccess;
			}
		}

		//status =  _iWriteInfo(xml);

		if (isRec) {
			char convertXml[2048];
			ZeroMemory(convertXml, sizeof(convertXml));
			status = _iReadInfo(flag, convertXml);
			if (status) {
				return CardReadErr;
			}

			if (mapCpuInfo.size() > 0) {
				M12CpuXml(convertXml, mapCpuInfo);
			}
			ISSCANCARD;

			status =  _iWriteInfo(convertXml);
		} else {  //cpu bin write
			status =  _iWriteInfo((char*)xmlStr.c_str());
		}
	} 

done:
	return status == 0 ? 0 : CardWriteErr;
}


/**
*
*/
int __stdcall iQueryInfo(char *name, char *xml)
{
	ASSERT_OPEN(g_bCardOpen);
	ISSCANCARD;

	char readbuf[256];
	int	nLen = 0;
	std::vector<std::string> vecQuery;
	std::vector<QueryColum> vecResult;
	struct RWRequestS	*RequestList = NULL;

	vecQuery = split(name, "|");
	for (size_t i=0; i<vecQuery.size(); ++i){
		std::string &strColum = vecQuery[i];

		QueryColum stQuery;
		ZeroMemory(readbuf, sizeof(readbuf));
		if (QueryItem(g_CardOps->cardAdapter->type, 
			strColum.c_str(), readbuf, nLen, stQuery) == 0){
			vecResult.push_back(stQuery);
		}
	}
	if (vecResult.size() == 0){
		return CardReadErr;
	}
	if (g_CardOps->cardAdapter->type == eCPUCard) {
		CpuConvertRetPos(vecResult);
	}
	iCreateXmlByVector(vecResult, xml, &nLen);
	return 0;
}


int __stdcall iPatchCard(
						char *pszCardDataXml,
						char *pszCardCoverDataXml,
						char *pszPrinterType,
						char *pszCardCoverXml 
						)
{
	ASSERT_OPEN(g_bCardOpen);
	
	if (m_pBHPrinter == NULL){
		m_pBHPrinter = new CBHGX_Printer;
		if (m_pBHPrinter->Init(pszPrinterType) != 0){
			SAFE_DELETE(m_pBHPrinter);
			return CardDLLLoadErr;
		}
	}

	m_pBHPrinter->FeedCard();

	Sleep(1000);
	int nTimeOut = 1000;
	int type = 0;
	while (iScanCard() != 0)
	{
		if (nTimeOut >= TIMEOUT){
			cout<<"Ѱ��ʧ��"<<endl;
			m_pBHPrinter->DeInitPrinter();
			SAFE_DELETE(m_pBHPrinter);
			return FeedCardError;
		}
		Sleep(1000);
		nTimeOut += 1000;
	}

	int nret = iCreateCard(pszCardDataXml);
	if (nret != 0){
		m_pBHPrinter->DeInitPrinter();
		SAFE_DELETE(m_pBHPrinter);
		return CardCreateErr;
	}
   
	m_pBHPrinter->BackToPrintHeader();
	nret = iPrintCard(pszPrinterType, pszCardCoverDataXml, pszCardCoverXml);
	if (nret != 0){
		SAFE_DELETE(m_pBHPrinter);
		return CardCoverPrintErr;
	}
	SAFE_DELETE(m_pBHPrinter);

	return 0;
}

//��ȡ��ӡ���б�����
int __stdcall iGetPrinterList(char *PrinterXML)
{
	if (PrinterXML == NULL || strlen(PrinterXML) == 0){
		return -1;
	}
	std::vector<std::string> vecPrinter;
	CBHGX_Printer::GetPrinterList(vecPrinter);
	int nSize = (int)vecPrinter.size();
	TiXmlDocument *XmlDoc = new TiXmlDocument;
	TiXmlElement *RootElement = NULL;
	TiXmlDeclaration HeadDec;
	TiXmlElement *PrintSeg = NULL;
	TiXmlPrinter Printer;

	HeadDec.Parse("<?xml version=\"1.0\" encoding=\"gb2312\"?>", 0, TIXML_ENCODING_UNKNOWN);
	XmlDoc->LinkEndChild(&HeadDec);

	RootElement = new TiXmlElement("PrinterList");
	RootElement->SetAttribute("NUM", (int)vecPrinter.size());
	for (size_t i=0; i<vecPrinter.size(); ++i)
	{
		PrintSeg = new TiXmlElement("PrinterName");
		PrintSeg->SetAttribute("NAME", vecPrinter[i].c_str());
		RootElement->LinkEndChild(PrintSeg);
	}

	XmlDoc->LinkEndChild(RootElement);
	XmlDoc->Accept(&Printer);
	strcpy(PrinterXML, Printer.CStr());
	return nSize;
}

int __stdcall iPrintCard(
						char *pszPrinterType,
						char *pszCardCoverDataXml,
						char *pszCardCoverXml 
						)
{
	int nPrint = 0;
	if (m_pBHPrinter == NULL){
		m_pBHPrinter = new CBHGX_Printer;
		if (m_pBHPrinter->Init(pszPrinterType) != 0) {
			SAFE_DELETE(m_pBHPrinter);
			return CardDLLLoadErr;
		}
	}

	if (m_pBHPrinter->InitPrinter(pszCardCoverDataXml, pszCardCoverXml)!= 0){
		SAFE_DELETE(m_pBHPrinter);
		return CardXmlErr;
	}

	if (!m_pBHPrinter->StartPrint()){
		m_pBHPrinter->DeInitPrinter();
		nPrint = -1;
	}
	
	SAFE_DELETE(m_pBHPrinter);
	return nPrint;
}


int __stdcall iCreateCard(char *pszCardDataXml)
{
	ASSERT_OPEN(g_bCardOpen);
	int result = 0;
	ISSCANCARD;

	std::string xmlStr(pszCardDataXml);
	if (CXmlUtil::CheckCardXMLValid(xmlStr) < 0){
		LogPrinter("CardXML:Check Error\n");
		//DBGCore( "CardXML Check Error\n");
		return CardXmlErr;
	}

	if (g_CardOps->cardAdapter->type == eCPUCard) {
		LogPrinter("CPU�ݲ�֧��\n");
		return CardNoSupport;
	}

	//M1�ƿ�
	XmlSegmentS *seg = g_CardOps->iConvertXmltoList((char*)xmlStr.c_str());
	seg = g_SegHelper->FindSegmentByID(seg, 2);
	int nRet = 0;
	if (seg != NULL){
		unsigned char KeyB[6];
		XmlColumnS *stColumn = g_SegHelper->FindColumnByID(seg->ColumnHeader, 1);
		if (seg->ColumnHeader->Value[0] == '0'){
			stColumn = g_SegHelper->FindColumnByID(seg->ColumnHeader, 7);
		}
		iGetKeyBySeed((unsigned char *)stColumn->Value, KeyB);

		nRet = iWriteInfo((char*)xmlStr.c_str());
		DBGCore( "д�����ݽ��:%d\n", nRet);
		LogPrinter("��д���ݣ�%d\n", nRet);

		nRet = InitPwd(KeyB);
		DBGCore( "����������%d\n", nRet);
		LogPrinter( "����������%d\n", nRet);

	} else {
		return CardCreateErr;
	}
	return nRet != 0? CardCreateErr:0;
}

int __stdcall iFormatCard()
{
	int status = 0;
	ASSERT_OPEN(g_bCardOpen);
	ISSCANCARD;

	if (g_CardOps->cardAdapter->type == eCPUCard) {
		status = FormatCpuCard(0xff);
	} else {
		status = aFormatCard(0xff);
	}
	return status == 0 ? CardProcSuccess : CardFormatErr;
}


int __stdcall iCardCtlCard(int cmd, void *data)
{
	ASSERT_OPEN(g_bCardOpen);
	return apt_IOCtl(cmd, data);
}

int __stdcall iCheckMsgForNH(char *pszCardCheckWSDL,char *pszCardServerURL,char* pszXml)
{
	ASSERT_OPEN(g_bCardOpen);
	std::string strCheckWSDL = pszCardCheckWSDL;
	std::string strServerURL = pszCardServerURL;

	char strResult[4096];
	memset(strResult, 0, sizeof(strResult));
	SCANCARD_XML(pszXml, strResult);

	char szQuery[1024];
	memset(szQuery, 0, sizeof(szQuery));

	std::string strCardNO;
	if (iQueryInfo("CARDNO", szQuery) != 0){
		CreateResponXML(3, "��ȡ����ʧ��", strResult);
		strcpy(pszXml, strResult);
		return CardReadErr;
	}
	GetQueryInfoForOne(szQuery, strCardNO);
	
	int nCheckCode = CardProcSuccess;

	bool bSuccessed = true;
	n_USCOREapiSoap m_CardObj;
	m_CardObj.endpoint = strServerURL.c_str();
	soap_init(m_CardObj.soap);
	soap_set_mode(m_CardObj.soap,SOAP_C_UTFSTRING);

	char *strCheckParams = (char*)malloc(sizeof(char)*2048);
	memset(strCheckParams, 0, 1024*2);
	CreateCheckWsdlParams(strCardNO.c_str(), strCheckWSDL.c_str(), strCheckParams);

	_ns1__nh_USCOREpipe pCheck;
	pCheck.parms = strCheckParams;

	_ns1__nh_USCOREpipeResponse pReturn;// = new _ns1__nh_USCOREpipeResponse;

	pReturn.nh_USCOREpipeResult = (char*)malloc(sizeof(char)*1024);

	m_CardObj.__ns2__nh_USCOREpipe(&pCheck, &pReturn);
	if(m_CardObj.soap->error) {   
		bSuccessed = false;
		CreateResponXML(3, "�����������ʧ��", strResult);
		strcpy(pszXml, strResult);
	} else {
		std::string strRetCode, strStatus;
		std::string strXML = pReturn.nh_USCOREpipeResult;
		GetCheckState(strXML, strRetCode, strStatus);

		std::string strCheckDesc;
		if (GetCheckRetDesc(strRetCode, strCheckDesc) == 0) {
			bSuccessed = false;
			CreateResponXML(1, strCheckDesc.c_str(), strResult);
			strcpy(pszXml, strResult);
		} else {
			nCheckCode = atoi(strStatus.c_str());
			strCheckDesc.clear();
			if (GetCardStatus(nCheckCode, strCheckDesc) == 0){
				bSuccessed = false;
				CreateResponXML(1, strCheckDesc.c_str(), strResult);
				strcpy(pszXml, strResult);
			}
		}
	}
	SAFE_DELETE_C(strCheckParams);
	SAFE_DELETE_C(pReturn.nh_USCOREpipeResult);

	if (bSuccessed){
		char szRead[4096];
		memset(szRead, 0, sizeof(szRead));
		iReadInfo(2, szRead);
		strcpy(pszXml, szRead);
	}

	soap_end(m_CardObj.soap);   
	soap_done(m_CardObj.soap); 
	return bSuccessed ? 0 : CardCheckError;
}

int __stdcall iReadConfigMsg(char *pszConfigXML,char *pszReadXML)
{
	ASSERT_OPEN(g_bCardOpen);
	std::string strQuery("");
	std::vector<std::string> vecQuery;
	vecQuery = split(std::string(pszConfigXML), "|");
	for (size_t i=0; i<vecQuery.size(); ++i)
	{
		int nCode = atoi(vecQuery[i].c_str());
		switch (nCode)
		{
		case 1:
			strQuery += "NAME|";
			break;
		case 2:
			strQuery += "IDNUMBER|";
			break;
		case 3:
			strQuery += "BIRTHDAY|";
			break;
		case 4:
			strQuery += "GENDER|";
			break;
		default:
			strQuery = "";
			break;
		}
	}
	strQuery = strQuery.substr(0, strQuery.size()-1);
	
	return iQueryInfo((char*)strQuery.c_str(), pszReadXML);
}



int __stdcall iRegMsgForNH(char *pszCardServerURL,char* pszXml)
{
	ASSERT_OPEN(g_bCardOpen);
	if (g_CardOps->cardAdapter->type != eM1Card) {
		return CardNoSupport;
	}
	std::string strServerURL = pszCardServerURL;

	std::string strXML;

	char strResult[4096];
	memset(strResult, 0, sizeof(strResult));
	SCANCARD_XML(pszXml, strResult);

	char szQuery[1024];
	memset(szQuery, 0, sizeof(szQuery));
	std::string strCardNO;
	int n = iQueryInfo("CARDNO", szQuery);
	if (n != 0){
		CreateResponXML(3, "��ȡ����ʧ��", strResult);
		strcpy(pszXml, strResult);
		return CardReadErr;
	}
	GetQueryInfoForOne(szQuery, strCardNO);
	
	bool bSuccessed = true;
	n_USCOREapiSoap m_CardObj;
	m_CardObj.endpoint = strServerURL.c_str();
	soap_init(m_CardObj.soap);
	soap_set_mode(m_CardObj.soap,SOAP_C_UTFSTRING);

	char* strRegParams = new char[1024];
	memset(strRegParams, 0, 1024);
	CreateRegWsdlParams(strCardNO.c_str(), strRegParams); 

	_ns1__nh_USCOREpipe pCheck;
	pCheck.parms = strRegParams;

	_ns1__nh_USCOREpipeResponse pReturn;

	pReturn.nh_USCOREpipeResult = new char[4096];

	m_CardObj.__ns2__nh_USCOREpipe(&pCheck, &pReturn);

	if(m_CardObj.soap->error) {   
		bSuccessed = false;
		CreateResponXML(3, "�����������ʧ��", strResult); 
		strcpy(pszXml, strResult);
	} else {
		std::string strRetCode, strStatus;
		strXML = pReturn.nh_USCOREpipeResult;

		GetCheckState(strXML, strRetCode, strStatus);
		std::string strCheckDesc;
		if (GetCheckRetDesc(strRetCode, strCheckDesc) == 0) {
			bSuccessed = false;
			CreateResponXML(3, strCheckDesc.c_str(), strResult);
			strcpy(pszXml, strResult);
		} else{
			if (strStatus.size() > 0 && CXmlUtil::CheckCardXMLValid(strStatus) == 0){ 
				FormatWriteInfo(strStatus.c_str(), strResult);
				int nState = iWriteInfo(strResult);
				if (nState != 0){
					bSuccessed = false;
					memset(strResult, 0, sizeof(strResult));
					CreateResponXML(2, "����дʧ��", strResult);
					strcpy(pszXml, strResult);
				}
			}
		}
	}
	delete [] strRegParams;
	delete [] pReturn.nh_USCOREpipeResult;

	if (bSuccessed){
		memset(strResult, 0, sizeof(strResult));
		iReadInfo(2 , strResult);
		strcpy(pszXml, strResult);
	}
	soap_end(m_CardObj.soap);   
	soap_done(m_CardObj.soap); 
	return bSuccessed ? 0 : CardRegError;
}

std::map<int, std::map<int, std::string> > mapLogConfig; 
int g_rwFlag = 0;
char g_processName[20];


void geneHISLog(const char *pszContent, std::map<int, std::string> &mapXmlInfo)
{
	TiXmlDocument XmlDoc;
	TiXmlElement  *RootElement;
	TiXmlElement  *Segment;
	XmlDoc.Parse(pszContent);
	RootElement = XmlDoc.RootElement();
	Segment = RootElement->FirstChildElement();

	std::vector<TiXmlElement*> vtcSeg;
	while (Segment) {
		vtcSeg.push_back(Segment);
		Segment = Segment->NextSiblingElement();
	}

	TiXmlDocument *XmlDocLog = NULL;
	TiXmlElement *LogElement = NULL;
	TiXmlElement *InfoSegment = NULL;
	TiXmlElement *LogSegment = NULL;
	TiXmlPrinter Printer;
	TiXmlDeclaration HeadDec;

	// ����XML�ĵ�
	XmlDocLog = new TiXmlDocument();

	// ����XML��ͷ��˵��
	HeadDec.Parse("<?xml version=\"1.0\" encoding=\"gb2312\" ?>", 0, TIXML_ENCODING_UNKNOWN);
	XmlDocLog->LinkEndChild(&HeadDec);

	LogElement = new TiXmlElement("LogInfo");
	LogElement->SetAttribute("PROGRAMID", "001");

	InfoSegment = new TiXmlElement("Info");
	InfoSegment->SetAttribute("ID", 1);
	InfoSegment->SetAttribute("DESC", "base info");

	LogSegment = new TiXmlElement("SEGMENT");
	LogSegment->SetAttribute("ID", 1);

	//insert base info column
	char timeStr[64];
	CTimeUtil::getCurrentTime(timeStr);
	std::map<int, std::string> contentMap = mapLogConfig[2];
	contentMap[-1] = mapXmlInfo[2];
	contentMap[0] = mapXmlInfo[5];
	contentMap[8] = mapXmlInfo[1];
	contentMap[9] = mapXmlInfo[10];
	contentMap[10] = mapXmlInfo[9];
	contentMap[11] = timeStr;
	if (g_rwFlag == 0)  {
		contentMap[12] = "0";
	} else {
		contentMap[12] = "1";
	}
	contentMap[13] = g_processName;
	std::map<int, std::string>::iterator mapIter = contentMap.begin();
	for (; mapIter != contentMap.end(); mapIter++) {
		TiXmlElement *pColumn = new TiXmlElement("COLUMN");
		pColumn->SetAttribute("ID", mapIter->first + 2);
		pColumn->SetAttribute("VALUE", mapIter->second.c_str());
		LogSegment->LinkEndChild(pColumn);
	}
	InfoSegment->LinkEndChild(LogSegment);
	LogElement->LinkEndChild(InfoSegment);

	TiXmlElement *pCtInfoSegment = new TiXmlElement("Info");
	pCtInfoSegment->SetAttribute("ID", 2);
	pCtInfoSegment->SetAttribute("VALUE", "content");

	for (int i=0; i<vtcSeg.size(); i++) {
		pCtInfoSegment->InsertEndChild(*vtcSeg[i]);
	}
	LogElement->LinkEndChild(pCtInfoSegment);

	XmlDocLog->LinkEndChild(LogElement);
	XmlDocLog->Accept(&Printer);

	std::map<int, std::string> configMap = mapLogConfig[1];
	std::string strFilePath(configMap[1]);
	//strFilePath += strcat((char*)contentMap[2].c_str(), "_");
	strFilePath += strcat(CTimeUtil::getCurrentDay(timeStr), ".log");
	FILE *fp = fopen(strFilePath.c_str(), "a+");
	fwrite(Printer.CStr(), strlen(Printer.CStr()), 1, fp);
	fclose(fp);
}

int __stdcall iRegMsgForNHLog(char *pszCardServerURL, char* pszLogXml, char* pszXml)
{
	int status = iRegMsgForNH(pszCardServerURL, pszLogXml);
	if (status != CardProcSuccess) {
		return CardRegError;
	}
	CXmlUtil::paserLogXml(pszLogXml, mapLogConfig);

	std::map<int, std::string> mapReaderInfo;
	CXmlUtil::parseHISXml(pszXml, mapReaderInfo);

	g_rwFlag = 0;
	strcpy(g_processName, "iRegMsgForNHLog");
	geneHISLog(pszXml, mapReaderInfo);
	return CardProcSuccess;
}


int __stdcall iReadCardMessageForNHLocal(char* pszLogXml, char* pszXml)
{
	std::string strMedicalID;
	int n = ParseValueQuery("MEDICARECERTIFICATENO", strMedicalID);
	if (n != 0) {
		CreateResponXML(3, "��ȡ�κϺ�ʧ��", pszXml);
		return 3;
	}
	return iCheckMsgForNHLocal(pszLogXml, pszXml);
}

//��У�� ������У��
int __stdcall iCheckMsgForNHLocal(char* pszLogXml, char* pszXml)
{
	int status = iCheckException(pszLogXml, pszXml);
	if (status != CardProcSuccess) {
		return status;
	}

	if (CardProcSuccess != iReadInfo(2, pszXml)) {
		return CardReadErr;
	}
	return CardProcSuccess;
}

int __stdcall iReadCardMessageForNHLog(char *pszCardCheckWSDL, 
									char *pszCardServerURL, 
									char* pszLogXml,
									char* pszXml)
{
	int status = iReadCardMessageForNH(pszCardCheckWSDL, pszCardServerURL, pszXml);
	if (status != CardProcSuccess) {
		return CardCheckError;
	}
	CXmlUtil::paserLogXml(pszLogXml, mapLogConfig);

	std::map<int, std::string> mapReaderInfo;
	CXmlUtil::parseHISXml(pszXml, mapReaderInfo);

	g_rwFlag = 0;
	strcpy(g_processName, "iReadCardMessageForNHLog");
	geneHISLog(pszXml, mapReaderInfo);
	return CardProcSuccess;
}

int __stdcall iReadOnlyCardMessageForNH(char* pszXml)
{
	return iReadInfo(2, pszXml);
}

int __stdcall iReadCardMessageForNH(char *pszCardCheckWSDL, char *pszCardServerURL, char* pszXml)
{
	std::string strCheckWSDL = pszCardCheckWSDL;
	std::string strServerURL = pszCardServerURL;

	std::string strXML;
	char szQuery[1024];
	memset(szQuery, 0, sizeof(szQuery));
	char strResult[4096];
	memset(strResult, 0, sizeof(strResult));
	SCANCARD_XML(pszXml, strResult);

	std::string strMedicalID;
	int n = ParseValueQuery("MEDICARECERTIFICATENO", strMedicalID);
	if (n != 0) {
		CreateResponXML(3, "��ȡ�κϺ�ʧ��", strResult);
		strcpy(pszXml, strResult);
		return CardReadErr;
	}
	if (strMedicalID.size() == 0) {
		CreateResponXML(3, "�κϺ�Ϊ��", strResult);
		strcpy(pszXml, strResult);
		return CardMedicalFailed;
	}

	std::string strCardNO;
	n = ParseValueQuery("CARDNO", strCardNO);
	if (n != 0 || strCardNO.size() == 0){
		CreateResponXML(3, "��ȡ����ʧ�ܻ��߿���Ϊ��", strResult);
		strcpy(pszXml, strResult);
		return 3;
	}

	bool bSuccessed = true;
	n_USCOREapiSoap m_CardObj;
	m_CardObj.endpoint = strServerURL.c_str();
	soap_init(m_CardObj.soap);
	soap_set_mode(m_CardObj.soap,SOAP_C_UTFSTRING);

	if (IsMedicalID(strMedicalID)){
		char *strCheckParams = new char[1024];
		memset(strCheckParams, 0, 1024);
		CreateCheckWsdlParams(strCardNO.c_str(), strCheckWSDL.c_str(), strCheckParams);

		_ns1__nh_USCOREpipe pCheck;
		pCheck.parms = strCheckParams;

		_ns1__nh_USCOREpipeResponse pReturn;// = new _ns1__nh_USCOREpipeResponse;

		pReturn.nh_USCOREpipeResult = new char[1024];

		m_CardObj.__ns2__nh_USCOREpipe(&pCheck, &pReturn);
		if(m_CardObj.soap->error) {   
			bSuccessed = false;
			CreateResponXML(3, "�����������ʧ��", strResult); 
			strcpy(pszXml, strResult);
		} else {
			std::string strRetCode, strStatus;
			strXML = pReturn.nh_USCOREpipeResult;
			GetCheckState(strXML, strRetCode, strStatus);

			std::string strCheckDesc;
			if (GetCheckRetDesc(strRetCode, strCheckDesc) == 0) {
				bSuccessed = false;
				CreateResponXML(1, strCheckDesc.c_str(), strResult);
				strcpy(pszXml, strResult);
			} else{
				int nCardStatus = atoi(strStatus.c_str());
				strCheckDesc.clear();

				if (GetCardStatus(nCardStatus, strCheckDesc) == 0){
					bSuccessed = false;
					CreateResponXML(1, strCheckDesc.c_str(), strResult);
					strcpy(pszXml, strResult);
				}
			}
		}
		SAFEARRAY_DELETE(strCheckParams);
		SAFEARRAY_DELETE(pReturn.nh_USCOREpipeResult);

		if (!bSuccessed){
			soap_end(m_CardObj.soap);   
			soap_done(m_CardObj.soap); 
			return 1;
		}
	}

	if (bSuccessed && g_CardOps->cardAdapter->type == eM1Card){
		char* strRegParams = new char[1024];
		memset(strRegParams, 0, 1024);
		CreateRegWsdlParams(strCardNO.c_str(), strRegParams); 

		_ns1__nh_USCOREpipe pCheck;
		pCheck.parms = strRegParams;

		_ns1__nh_USCOREpipeResponse pReturn;// = new _ns1__nh_USCOREpipeResponse;

		pReturn.nh_USCOREpipeResult = new char[4096];

		m_CardObj.__ns2__nh_USCOREpipe(&pCheck, &pReturn);

		if(m_CardObj.soap->error){   
			bSuccessed = false;
			CreateResponXML(3, "�����������ʧ��", strResult); 
			strcpy(pszXml, strResult);
		}  else {

			std::string strRetCode, strStatus;
			strXML = pReturn.nh_USCOREpipeResult;

			GetCheckState(strXML, strRetCode, strStatus);

			std::string strCheckDesc;
			if (GetCheckRetDesc(strRetCode, strCheckDesc) == 0) {
				bSuccessed = false;
				CreateResponXML(3, strCheckDesc.c_str(), strResult);
				strcpy(pszXml, strResult);
			} else {
				FormatWriteInfo(strStatus.c_str(), strResult);
				int nState = iWriteInfo(strResult);
				if (nState != 0){
					bSuccessed = false;
					memset(strResult, 0, sizeof(strResult));
					CreateResponXML(2, "����дʧ��", strResult);
					strcpy(pszXml, strResult);
				}
			}
		}
		SAFEARRAY_DELETE(strRegParams);
		SAFEARRAY_DELETE(pReturn.nh_USCOREpipeResult);
	}

	if (bSuccessed){
		memset(strResult, 0, sizeof(strResult));
		iReadInfo(2, strResult);
		strcpy(pszXml, strResult);
	}
	soap_end(m_CardObj.soap);   
	soap_done(m_CardObj.soap); 
	return bSuccessed==true ? 0 : 2;
}


int __stdcall iEncryFile(char *filename)
{
	CDESEncry encry;
	bool bSuccess = encry.EncryFile(filename);
	return bSuccess ? 0 : EncryFileError;
}


int __stdcall iCreateLicense(char *filename, char *timeStr)
{
	if (timeStr == NULL || strlen(timeStr) == 0) {
		return InitFullLicense(filename);
	}else {
		return InitTimeLicense(filename, timeStr);
	}
	return 0;
}

int __stdcall iCheckLicense(char *filename,int type)
{
	int status = 0;
	if (type == 0) {
		status = CheckTimeLicense(filename);
		status = (status == 0 ? 0 : CardAuthExpired);
	} else {
		status = CheckFullLicense(filename);
		status = (status == 0 ? 0 : CardNoAuthority);
	}
	return status;
}

int __stdcall iCheckException(char *pszLogXml,char *pszXml)
{
	CExceptionCheck check(pszLogXml);
	int status = check.filterForbidden(pszXml);
	if (status != CardProcSuccess) {
		return status;
	} 
	status = check.filterWarnning(pszXml);
	if (status != CardProcSuccess) {
		return status;
	}
	return CardProcSuccess;
}

int __stdcall apt_InitGList(CardType eType)
{
	if (eType == eCPUCard) {
		g_CpuCardOps = InitCpuCardOps();
		g_XmlListHead = g_CpuCardOps->programXmlList;
		g_CardOps = g_CpuCardOps;
	} else {
		g_M1CardOps = InitM1CardOps();
		g_XmlListHead = g_CardOps->programXmlList;
		g_CardOps = g_M1CardOps;
	}
	g_SegHelper = new CSegmentHelper(g_XmlListHead, g_CardOps); 
	return 0;
}
