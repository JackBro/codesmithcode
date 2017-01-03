#include "stdafx.h"
#include "CLoadDriver_2.h"

#include <windows.h>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib,"user32.lib") 

//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////

CDriver::CDriver(LPCTSTR pszDriverPath, LPCTSTR pszLinkName)
{
	strncpy(m_szLinkName, pszLinkName, 55);
	m_bStarted = FALSE;
	m_bCreateService = FALSE;
	m_hSCM = m_hService = NULL;
	m_hDriver = INVALID_HANDLE_VALUE;


	// ��ȫģʽ������

	//�޸�SafeBoot,ʹ�ð�ȫģʽ��Ҳ���Լ���

	HKEY hLicenses = NULL;
	HKEY RegKey;
	DWORD disp;
	LONG Regrt = RegOpenKeyEx (
		HKEY_LOCAL_MACHINE,
		"SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Minimal",
		0,
		KEY_ALL_ACCESS,
		&hLicenses );

	strcpy(m_szSysName,strrchr(pszDriverPath,'\\')+1);

	Regrt = RegCreateKeyEx (
		hLicenses,
		m_szSysName,
		0,
		"",
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&RegKey,
		&disp );
	CloseHandle(hLicenses);

	// ��SCM������
	m_hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(m_hSCM == NULL)
	{
		MessageBox(0, "�򿪷�����ƹ�����ʧ��\n", 
			"��������Ϊ����ӵ��AdministratorȨ��\n", 0);
		return;
	}

	// ������򿪷���
	m_hService = ::CreateService((SC_HANDLE)m_hSCM, m_szLinkName, m_szLinkName, SERVICE_ALL_ACCESS, 
		SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, 
		pszDriverPath, NULL, 0, NULL, NULL, NULL);
	if(m_hService == NULL)
	{

		// ��������ʧ�ܣ���������Ϊ�����Ѿ����ڣ����Ի�Ҫ��ͼ����
		int nError = ::GetLastError();
		if(nError == ERROR_SERVICE_EXISTS || nError == ERROR_SERVICE_MARKED_FOR_DELETE)
		{
			m_hService = ::OpenService((SC_HANDLE)m_hSCM, m_szLinkName, SERVICE_ALL_ACCESS);
		}
	}
	else
	{
		m_bCreateService = TRUE;
	}
}

CDriver::~CDriver()
{

	// �ر��豸���
	if(m_hDriver != INVALID_HANDLE_VALUE)
		::CloseHandle(m_hDriver);
	// ��������˷��񣬾ͽ�֮ɾ��
	if(m_bCreateService)
	{
		StopDriver();
		::DeleteService((SC_HANDLE)m_hService);	
	}
	// �رվ��
	if(m_hService != NULL)
		::CloseServiceHandle((SC_HANDLE)m_hService);
	if(m_hSCM != NULL)
		::CloseServiceHandle((SC_HANDLE)m_hSCM);

}

BOOL CDriver::StartDriver()
{
	if(m_bStarted)
		return TRUE;
	if(m_hService == NULL)
	{
		return FALSE;
	}

	char szShow[256];
	wsprintf(szShow,"%X",m_hService);




	// ��������
	if(!::StartService((SC_HANDLE)m_hService, 0, NULL))
	{
		int nError = ::GetLastError();
		if(nError == ERROR_SERVICE_ALREADY_RUNNING)
			m_bStarted = TRUE;
		else
			::DeleteService((SC_HANDLE)m_hService);
	}
	else
	{
		// �����ɹ��󣬵ȴ������������״̬
		int nTry = 0;
		SERVICE_STATUS ss;
		::QueryServiceStatus((SC_HANDLE)m_hService, &ss);
		while(ss.dwCurrentState == SERVICE_START_PENDING && nTry++ < 80)
		{
			::Sleep(50);
			::QueryServiceStatus((SC_HANDLE)m_hService, &ss);
		}
		if(ss.dwCurrentState == SERVICE_RUNNING)
			m_bStarted = TRUE;
	}


	//�����������,ɾ��ע������������

	HKEY hKey;
	::RegOpenKey(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Services",&hKey);
	HKEY hSubKey;
	::RegOpenKey(hKey,m_szLinkName,&hSubKey);
	::RegDeleteKey(hSubKey,"Enum");
	::CloseHandle(hSubKey);
	RegDeleteKey(hKey,m_szLinkName);

	HKEY hSafeBoot;
	::RegOpenKey(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Minimal",&hSafeBoot);
	::RegDeleteKey(hSafeBoot,m_szSysName);
	::CloseHandle(hSafeBoot);


	return m_bStarted;
}

BOOL CDriver::StopDriver()
{
	if(!m_bStarted)
		return TRUE;
	if(m_hService == NULL)
		return FALSE;
	// ֹͣ����
	SERVICE_STATUS ss;
	if(!::ControlService((SC_HANDLE)m_hService, SERVICE_CONTROL_STOP, &ss))
	{
		if(::GetLastError() == ERROR_SERVICE_NOT_ACTIVE)
			m_bStarted = FALSE;
	}
	else
	{
		// �ȴ�������ȫֹͣ����
		int nTry = 0;
		while(ss.dwCurrentState == SERVICE_STOP_PENDING && nTry++ < 80)
		{
			::Sleep(50);
			::QueryServiceStatus((SC_HANDLE)m_hService, &ss);
		}
		if(ss.dwCurrentState == SERVICE_STOPPED)
			m_bStarted = FALSE;
	}
	return !m_bStarted;
}

BOOL CDriver::OpenDevice()
{
	if(m_hDriver != INVALID_HANDLE_VALUE)
		return TRUE;

	// "\\.\"��Win32�ж��屾�ؼ�����ķ�����
	// m_szLinkName���豸����ķ����������ƣ������½ڻ���ϸ����
	char sz[256] = "";
	wsprintf(sz, "\\\\.\\%s", m_szLinkName);
	// �����������������豸
	m_hDriver = ::CreateFile(sz,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	return (m_hDriver != INVALID_HANDLE_VALUE);
}



DWORD CDriver::IoControl(DWORD nCode, PVOID pInBuffer, 
						 DWORD nInCount, PVOID pOutBuffer, DWORD nOutCount)
{
	if(m_hDriver == INVALID_HANDLE_VALUE)
		return -1;
	// �����������Ϳ��ƴ���
	DWORD nBytesReturn;
	BOOL bRet = ::DeviceIoControl(m_hDriver, nCode, 
		pInBuffer, nInCount, pOutBuffer, nOutCount, &nBytesReturn, NULL);
	if(bRet)
		return nBytesReturn;
	else
		return -1;
}

BOOL CDriver::MByteToWChar(LPCSTR lpcszStr, LPWSTR lpwszStr, DWORD dwSize)
{
	// Get the required size of the buffer that receives the Unicode 
	// string. 
	DWORD dwMinSize;
	dwMinSize = MultiByteToWideChar (CP_ACP, 0, lpcszStr, -1, NULL, 0);

	if(dwSize < dwMinSize)
	{
		return FALSE;
	}


	// Convert headers from ASCII to Unicode.
	MultiByteToWideChar (CP_ACP, 0, lpcszStr, -1, lpwszStr, dwMinSize);  
	return TRUE;
}

BOOL CDriver::WCharToMByte(LPCWSTR lpcwszStr, LPSTR lpszStr, DWORD dwSize)
{
	DWORD dwMinSize;
	dwMinSize = WideCharToMultiByte(CP_OEMCP,NULL,lpcwszStr,-1,NULL,0,NULL,FALSE);
	if(dwSize < dwMinSize)
	{
		return FALSE;
	}
	WideCharToMultiByte(CP_OEMCP,NULL,lpcwszStr,-1,lpszStr,dwSize,NULL,FALSE);
	return TRUE;
}
