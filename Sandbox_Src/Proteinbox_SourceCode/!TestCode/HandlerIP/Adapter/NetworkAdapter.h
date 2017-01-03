//////////////////////////////////////////////////////////////////////////
// �ļ�����NetworkAdapter.h
// ���������������������ӿ�
// ���ߣ�hmf3000
// ��Ȩ�����������������ã������ô���ĳ���Ա�ܹ����ļ���ͷ��
//             ����һ��ע�ͣ�����лhmf3000�Դ����׫д�� ���ԡ�

#pragma once

class CNetworkAdapter
{
public:
	explicit CNetworkAdapter(void);
	~CNetworkAdapter(void);

public:

// Operation
    BOOL MakeChange(int n);                      // ʹ��ǰ������Ч

// Property
    CString GetIPAddress(int n) const;           // ��ȡIP��ַ
    CString GetMACAddress(int n) const;          // ��ȡMAC��ַ
	CString GetDescription(int n) const;          // ��ȡMAC��ַ
	ULONG GetDescriptionCounts() const;          // ��ȡ����
    void SetIPAddress( CString strIP, int n );
    void SetMACAddress( CString strMAC, int n );
	void InitLocalAdapterInfo();

private:
    
// Implementation
    BOOL RegSetIP( CString strAdapterName, CString strIPAddress, CString strNetMask );
    BOOL RegSetMAC( CString strAdapterName, CString strMACAddress );

private:
    struct ADAPTER_INFO
    {
        CString     m_strIP;
        CString     m_strMAC;
        CString     m_strNetMask;
        CString     m_strAdapterName;
		CString     m_strDescription ;
    };

	ULONG m_nCounts ;

    // һ������µ�һ��AdapterΪ��ǰ����ʹ�õ�Adaper
//    vector<ADAPTER_INFO>    m_vecAdapterInfo;               // ������������
	ADAPTER_INFO   m_vecAdapterInfo[5];               // ������������
};
