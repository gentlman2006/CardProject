BHGX_CreateCardData.dll 1.0.0.7
ʱ�䣺2013-08-8 16:36
�޸Ĺ��ܣ�
1���޸�Ϊ�Զ���License

BHGX_CreateCardData.dll 1.0.0.6
ʱ�䣺2012-09-25 16:36
�޸Ĺ��ܣ�
1�������ƿ�����1000��

BHGX_CreateCardData.dll 1.0.0.5
ʱ�䣺2011-05-07 16:00
�޸Ĺ��ܣ�
1���ƿ���������Ӧũ�Ϻ͹���

BHGX_CreateCardData.dll 1.0.0.4
ʱ�䣺2011-04-20 11:30
�޸Ĺ��ܣ�
1���޸������ַ�����

BHGX_CreateCardData.dll 1.0.0.2
ʱ�䣺2011-11-16 11:30
�޸Ĺ��ܣ�
1�������ƿ����ݽ��ܹ���

BHGX_CreateCardData.dll 1.0.0.1
ʱ�䣺2011-10-15 16:30
�޸Ĺ��ܣ�
1�������ƿ����ݳɹ�.data�ļ�

	/*******************************************************************************
	* �������ƣ�	iCreateCardData  ũ���ƿ��ӿ�	
	* ����������	filename	�ƿ���Ϣ�ı��ļ�
	*				license		����Ϊ"��������.license"
					datafile	���ɿ����� Ĭ��"card.data"
					infofile	���࿨����
	* ���������	
	* ���������	
	* �� �� ֵ��	bool �ɹ� �� false ʧ��	
	*******************************************************************************/
	DLL_EXPORT int __stdcall iCreateCardData( 
						const char *filename, 
						const char *license,
						const char *datafile = "card.data", 
						const char *infofile = "cardpage.data" 
						);

