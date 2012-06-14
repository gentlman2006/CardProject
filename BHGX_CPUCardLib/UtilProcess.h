#ifndef _UTIL_H
#define _UTIL_H
#pragma once

typedef unsigned char BYTE;

typedef int (*ReadCard)();
typedef int (*WriteCard)();
enum CardDataType
{
	eStart = 0,
	eProviderInfo,
	eCardHolderInfo,
	eAddr,
	eContacter,
	eSocialInfo,
	eCertificat,
	eClinical,
	eSpecialInfo,
	eAllergyInfo,
	eImmuneInfo,
	eHosindex,
	eServiceIndex,

	eSegmentHosServ,
};
#pragma  pack(push, 1)

//����������Ϣ
struct ProviderInfo 
{
	BYTE cardType;		//������
	BYTE version[4];	//�淶�汾
	BYTE provider[30];
	BYTE provCode[11];
	BYTE provCert[180];
	int  sendTime;
	int  holdlast;
	BYTE cardno[18];
	BYTE safecode[3];
	BYTE chipseq[10];
};

//�ֿ�����Ϣ
struct CardHolderInfo
{
	BYTE name[30];
	BYTE sex;
	BYTE nationcode;
	int  birthday;
	BYTE identity[18];
	BYTE phone1[20];
	BYTE phone2[20];
	BYTE feecharge[3];
};

//����2����ַ
struct Addr
{
	BYTE addrType;
	BYTE addrInfo[100];
};

//3����ϵ��
struct Contacter
{
	BYTE name[30];
	BYTE relation;
	BYTE phone[20];
};

//ְҵ��������Ϣ�ļ�
struct SocialInfo
{
	BYTE degree;
	BYTE married;
	BYTE employecode[3];
};

//֤����¼��Ϣ
struct Certificat
{
	BYTE type;
	BYTE certno[18];
	BYTE healthno[17];
	BYTE farmno[18];
};

//�ٴ���������
struct Clinical
{
	BYTE ABOCode;
	BYTE RHCode;
	BYTE xiaoCode;
	BYTE heartCode;
	BYTE headCode;
	BYTE dianCode;
	BYTE bloodCode;
	BYTE sweetCode;
	BYTE greeneyeCode;
	BYTE touxiCode;
	BYTE organtransCode;
	BYTE organlackCode;
	BYTE loadCode;
	BYTE heartpaceCode;
	BYTE others[40];
};

struct SpecialInfo
{
	BYTE mentalflag;
};

//ѭ��3��
struct AllergyInfo
{
	BYTE name[20];
	BYTE Desc[100];
};

//ѭ��10��
struct ImmuneInfo
{
	BYTE name[20];
	int  time;
};

//����5��¼
struct Hosindex
{
	BYTE valideflag;
};

//����5��¼
struct ServiceIndex
{
	BYTE valideflag;
};

struct XmlColumnS
{
	int		ID;
	char	Source[30];

	int		Offset;
	int		ColumnByte;
	BYTE	*value;

	struct XmlColumnS	*Next;
};

struct XmlSegmentS
{
	int		ID;
	int		datatype;	//1:��¼�ļ� 0:�������ļ�
	int		offset;
	BYTE	Target[30];

	struct XmlColumnS	*ColumnHeader;
	struct XmlColumnS   *ColumnTailer;
	struct XmlSegmentS	*Next;
};

struct XmlProgramS
{
	struct XmlSegmentS *SegHeader;
	struct XmlSegmentS *SegTailer;
};

struct SegmentHosServ
{
	int		ID;
	char	Source[30];

	struct XmlColumnS	*ColumnHeader;
	struct XmlColumnS   *ColumnTailer;
	struct SegmentHosServ	*Next;
};






struct RWRequestS
{
	int	mode;	// ��д�����־]
	int nID;
	int	datatype;	//1:��¼�ļ� 0:�������ļ�

	int offset;	// ���Ե�ַƫ����
	int	length;	// ��Ԫ�صĳ���
	unsigned char *value;

	struct RWRequestS *agent;	// ��ʵ�Ķ�д����

	void *pri;	

	// �Ը���������ݽ��мӽ���
	//void encode(struct RWRequest *);
	//void decode(struct RWRequest *);

	// ��һ��Ԫ��
	struct RWRequestS	*Next;
};

//���ڼ�¼�ļ�������
struct RecFolder{
	BYTE	section[10];
	BYTE    subSection[10];
	BYTE	fileName[10];
};

#pragma pack(pop)

#ifdef  __cplusplus
extern "C" {
#endif

int __stdcall CreateCPUData(char *configXML);
char* ReadConfigFromReg();

struct XmlSegmentS *CloneSegment(struct XmlSegmentS *SegmentElement);

struct XmlSegmentS *GetXmlSegmentByFlag(int nID);

void DestroyList(struct XmlSegmentS *node);

struct RWRequestS* __stdcall CreateRequest(struct XmlSegmentS *listHead, int mode);
struct RWRequestS* __stdcall CreateRWRequest(struct XmlColumnS *listHead, int mode);

void __stdcall DestroyRequest(struct RWRequestS *list, int flag);
void __stdcall DestroyRWRequest(struct RWRequestS *list, int flag);

int CheckCardXMLValid(char *pszCardXml);

int iConvertXmlByList(struct XmlSegmentS*, char *, int *);
struct XmlSegmentS* ConvertXmltoList(char *xml);

#ifdef  __cplusplus
};
#endif

#endif