///////////////////////////////////////////////////
// Driver.h�ļ�

#ifndef __DRIVER_H__
#define __DRIVER_H__

#include <stdlib.h>
#include <stdio.h>
#include <wtypes.h>
#include <windows.h>
#include <Winsvc.h>	// Ϊ��ʹ��SCM����


//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif


class CDriver
{

public:
// ���캯������������
	// ���캯����pszDriverPathΪ��������Ŀ¼��pszLinkNameΪ������������
	// ����Ĺ��캯���У�����ͼ������򿪷���
	CDriver(LPCTSTR pszDriverPath, LPCTSTR pszLinkName);
	// �����������������ֹͣ����
	virtual ~CDriver();

// ����
	// �������Ƿ����
	virtual BOOL IsValid() { return (m_hSCM != NULL && m_hService != NULL); }

// ����
	// ��������Ҳ����˵������DriverEntry������������
	virtual BOOL StartDriver();
	// �������񡣼����������DriverUnload���̽�������
	virtual BOOL StopDriver();
	
	// ���豸����ȡ�õ���������һ�����
	virtual BOOL OpenDevice();



	// ���豸���Ϳ��ƴ���
	virtual DWORD IoControl(DWORD nCode, PVOID pInBuffer, 
			DWORD nInCount, PVOID pOutBuffer, DWORD nOutCount);


	BOOL MByteToWChar(LPCSTR lpcszStr, LPWSTR lpwszStr, DWORD dwSize);
	BOOL WCharToMByte(LPCWSTR lpcwszStr, LPSTR lpszStr, DWORD dwSize);
// ʵ��
protected:
	char m_szLinkName[56];	// ������������
	char m_szSysName[64];

	BOOL m_bStarted;	// ָ�������Ƿ�����
	BOOL m_bCreateService;	// ָ���Ƿ񴴽��˷���

	HANDLE m_hSCM;		// SCM���ݿ���
	HANDLE m_hService;	// ������
	HANDLE m_hDriver;	// �豸���
};


//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
#endif // __DRIVER_H__