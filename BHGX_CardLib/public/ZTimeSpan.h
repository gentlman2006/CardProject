
#ifndef _TIME_SPAN_H_78R789493289148932819452839KFLDKL
#define _TIME_SPAN_H_78R789493289148932819452839KFLDKL


class CZTimeSpan
{
public:

	//���캯��
	CZTimeSpan();

	//�������캯��
	CZTimeSpan(const CZTimeSpan &timespan);

	//���캯��
	CZTimeSpan(long nDays, long nSecs);

	//���캯������������Ϊʱ�������ա�ʱ���֡���ֵ
	CZTimeSpan(long lDays,	//ʱ������ֵ 0-24800(Լ)
		int nHours,			//ʱ����ʱֵ 0-23
		int nMins,			//ʱ������ֵ 0-59
		int nSecs);			//ʱ������ֵ 0-59

	//��������
	~CZTimeSpan();
	
	//��ȡʱ����ռ���
	long GetDays() const;

	//��ȡʱ���ʱ����
	long GetHours() const;
	
	//��ȡʱ��������ʱ��
	long GetTotalHours() const;

	//��ȡʱ��ķּ���
	long GetMinutes() const;

	//��ȡʱ�������ܷ���
	long GetTotalMinutes() const;

	//��ȡʱ��������
	long GetSeconds() const;

	//��ȡʱ������������
	long GetTotalSeconds() const;
	
	// == ��������� 
	bool operator == (const CZTimeSpan &span) const ;

	// != ��������� 
	bool operator != (const CZTimeSpan &span) const ;

	// < ��������� 
	bool operator < (const CZTimeSpan &span) const ;

	// > ��������� 
	bool operator > (const CZTimeSpan &span) const ;

	// <= ��������� 
	bool operator <= (const CZTimeSpan &span) const ;

	// >= ��������� 
	bool operator >= (const CZTimeSpan &span) const ;

	// = ��������� 
	CZTimeSpan& operator = (const CZTimeSpan &timespan);

	//��ȡʱ��������ֵ
	long GetTimeSpanSeconds() const;

	//����ʱ����
	bool SetTimeSpan(long nDays, long nSeconds);

protected:
	//ʱ�����������
	long m_nSeconds;

	//ʱ�������ռ���
	long m_nDays;

	//����ʱ��������������Ϊʱ�������ա�ʱ���֡���ֵ
	bool _SetTimeSpan(long lDays, int nHours, int nMins, int nSecs);

	
};


#endif

