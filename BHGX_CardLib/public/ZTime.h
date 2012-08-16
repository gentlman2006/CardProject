
#ifndef	_TIME_H_785432543
#define	_TIME_H_785432543


#include "ZTimeSpan.h"
#include <string>

using   namespace   std; 


class CZTime
{
public:
	//���캯��
	CZTime();

	//�������캯��
	CZTime(const CZTime &time);

	//���캯��
	CZTime(
		int nYear,	//��Ԫ������� 1900-2100
		int nMonth,		//��Ԫ������� 1-12
		int nDay,		//��Ԫ������� 1-31
		int nHour,		//ʱ0-23
		int nMin,		//��0-59
		int nSec);		//��0-59
	
	//��������
	~CZTime();

	//��ȡϵͳ��ǰʱ��
	static CZTime GetZCurrentTime();


	// < ��������� 
	bool operator < (const CZTime &time) const;

	// == ��������� 
	bool operator == (const CZTime &time) const;

	// != ��������� 
	bool operator != (const CZTime &time) const;

	// <= ��������� 
	bool operator <= (const CZTime &time) const;

	// >= ��������� 
	bool operator >= (const CZTime &time) const;

	// > ��������� 
	bool operator > (const CZTime &time) const;

	// + ��������� 
	CZTime operator + (const CZTimeSpan &timeSpan) const;

	// - ��������� 
	CZTime operator - (const CZTimeSpan &timeSpan) const;

	// - ��������� 
	CZTimeSpan operator - (const CZTime &time) const;

	// = ��������� 
	CZTime& operator = (const CZTime &time);	

	//��ȡ���1900��1��1�յ�ʱ������
	long GetTotalDays() const;

	//��ȡ���00��00��00 ��ʱ������
	long GetTotalSeconds() const;

protected:

	//ʱ��ֵ���1900��1��1�յ�������
	long m_nDays;

	//ʱ��ֵ���00��00��00 ��������
	long m_nSeconds;

	//����ʱ��ֵ
	bool _SetDate(
		int nYear,				//��Ԫ������� 1900-2100
		int nMonth,				//��Ԫ������� 1-12
		int nDay,				//��Ԫ������� 1-31
		int nHour,				//ʱ0-23
		int nMin,				//��0-59
		int nSec);				//��0-59

	//����ʱ��ֵ
	bool _SetDate(
		int nDays,				//���1900��1��1�յ�������
		int nSeconds);			//���00��00��00 ��������

	//��ȡ���� 
	bool _GetDate(
		int &nYear,				//��Ԫ�������
		int &nMonth,			//��Ԫ�������
		int &nDay				//��Ԫ�������
		) const;	

	//��ȡʱ��
	bool _GetTime(
		int &nHour,				//ʱ
		int &nMin,				//��
		int &nSec				//��
		) const;

	//���ݸ�ʽ�ַ���ȡ��ʽֵ
	string _GetValue(char cFormat) const;

	//�������ڼ���ȡ����ȫ��
	string _GetDayofWeekFullName(int nDayofWeek) const;

	//���ݼ��»�ȡ����д
	string _GetMonthAbbrevName(int nMonth) const;

	//���ݼ��»�ȡ��ȫ��
	string _GetMonthFullName(int nMonth)const;

};



#endif





