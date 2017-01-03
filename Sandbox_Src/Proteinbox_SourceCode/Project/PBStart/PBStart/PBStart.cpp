/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2011/07/07 [7:7:2011 - 14:03]
* MODULE : \PBStart\PBStart.cpp
* 
* Description:
*
*   start.exe����ͨ�������д��η�ʽ����"��ɳ��"�Ľ���
*                        
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "stdafx.h"
#include "ParseCMD.h"
#include "PBStart.h"

//////////////////////////////////////////////////////////////////////////

int APIENTRY 
WinMain (
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nCmdShow
	)
{
	int ExitCode = 1 ;
	BOOL bRet = FALSE ;
	CPBStart CMain ;

	// 1. ����������
	bRet = CParseCMD::GetInstance().ParaseCommandLine();
	if ( FALSE == bRet ) { return CMain.KillProcess( 1 ); }

	// 2. ��������PBCtrl.exe
	bRet = CParseCMD::GetInstance().StartPBCtrl();
	if ( FALSE == bRet ) { return CMain.KillProcess( 1 ); }

	// 3. Vista����AdminȨ����������
	CParseCMD::GetInstance().INIGetConf( L"StartServices" ); // ����PBSrv.exe����
	CParseCMD::GetInstance().VistaRunAsAdmin( TRUE );

	// 4. ��������,start.exeĸ���Ѿ�����,�����¼�����̽ڵ���
	bRet = CParseCMD::GetInstance().InitProcess();
	if ( FALSE == bRet ) { return CMain.KillProcess( 1 ); }

	// 5. �Ƿ���Ҫ����COM��ط���
	if ( CParseCMD::GetInstance().m_bNeedStartCOM )
	{
		PB_StartCOM();
	}

	// 6. �ع��������ַ���,����"��ɳ��"�Ľ���
	if ( g_CommandLine_strings && CMain.GetFullPath(g_CommandLine_strings) )
	{
		ExitCode = CMain.RunSandboxedProc();
	}

	return CMain.KillProcess( ExitCode );
}


//////////////////////////////////////////////////////////////////////////

CPBStart::CPBStart()
{
	
}

CPBStart::~CPBStart()
{
	
}


int CPBStart::KillProcess( int ExitCode )
{
	ZwTerminateProcess( (HANDLE)0xFFFFFFFF, ExitCode );
	return ExitCode;
}


BOOL CPBStart::GetFullPath( IN OUT LPWSTR lpData )
{
	LPWSTR FilePart = NULL, lpFileName = NULL, lpBuffer = NULL ;

	// 1. ȥ��·���е�""; ���紫������·��Ϊ"d:\1.exe",�����Ϊd:\1.exe
	lpFileName = (LPWSTR) HeapAlloc( GetProcessHeap(), 4, 0x5000 );
	if ( '\"' == *lpData )
	{
		wcscpy( lpFileName, lpData + 1 );
		if ( lpFileName[wcslen(lpFileName) - 1] == '\"' )
		{
			lpFileName[wcslen(lpFileName) - 1] = 0 ;
		}
	} 
	else
	{
		wcscpy( lpFileName, lpData );
	}

	// 2. �õ���Ӧ��ȫ·����
	lpBuffer = (LPWSTR) HeapAlloc( GetProcessHeap(), 4, 0x5000 );

	if ( lpData[0] != '.' || lpData[1] )
		GetFullPathNameW( lpFileName, 0x27FC, lpBuffer, &FilePart );
	else
		SHGetFolderPathW( 0, 5, 0, 0, lpBuffer );

	// 3. �ж��Ƿ��������ļ���.����,����Ӳ�������֮
	if ( PB_IsDirectory( lpBuffer ) ) 
	{
		*lpData = '\"';
		GetSystemWindowsDirectoryW( lpData + 1, 0x104 );
		wcscat( lpData, L"\\explorer.exe\" /e,\"" );
		wcscat( lpData, lpBuffer );
		wcscat( lpData, L"\"" );
	}

	// ��������������"d:\" ����Ҫ����Ŀ¼.��ת��Ϊ""C:\WINDOWS\explorer.exe" /e,"d:\""
	HeapFree( GetProcessHeap(), 4, lpBuffer );
	HeapFree( GetProcessHeap(), 4, lpFileName );
	return TRUE;
}


int CPBStart::RunSandboxedProc()
{
	BOOL bRet = FALSE ;
	DWORD dwErrCode = 0 ;
	STARTUPINFOW StartupInfo = { 0 };
	SHELLEXECUTEINFOW sei = { 0 };
	PROCESS_INFORMATION ProcInfo = { 0 };
	ULONG i = 0, length = 0, TotalLength = 0, TotalLengthDummy = 0, LastLength = 0 ;
	LPWSTR ptr = NULL , lpCommand = NULL, lpCommandSub = NULL, lpFile = NULL, lpParameters = NULL, pData = NULL ;

	// 1. ȥ�������в����еĿո�.����Ϊ "   d:\1.exe",��ת��Ϊ"d:\1.exe"
	RtlZeroMemory( &ProcInfo, sizeof(PROCESS_INFORMATION) );
	RtlZeroMemory( &StartupInfo, sizeof(STARTUPINFOW) );
	StartupInfo.cb = sizeof(STARTUPINFOW) ;

	for ( ptr = g_CommandLine_strings; ptr; g_CommandLine_strings = ptr )
	{
		if ( *ptr != ' ' )
			break;

		++ptr;
	}

	// 2. �����ڴ��С,����֮,�������в�����������
	length = 2 * wcslen(ptr) + 0x40 ;
	lpCommand = (LPWSTR) HeapAlloc( GetProcessHeap(), 4, length );
	memcpy( lpCommand, g_CommandLine_strings, length );

	// 3. ȥ�������в�������Ŀո�. ����Ϊ"d:\1.exe   ",��ת��Ϊ"d:\1.exe"
	TotalLengthDummy = TotalLength = wcslen( lpCommand );
	if ( ' ' == lpCommand[TotalLength - 1] )
	{
		do
		{
			--TotalLength;
			lpCommand[ TotalLength ] = 0;
		}
		while ( ' ' == lpCommand[TotalLength - 1] );

		TotalLengthDummy = TotalLength ;
	}

	// 4. ����"��ɳ��"�Ľ���
	if ( g_bFlag_system )
	{
		// ��ϵͳȨ������"��SB"�Ľ���
		LPWSTR lpCommandLine = (LPWSTR) HeapAlloc( GetProcessHeap(), 0, 0x4000 );
		ExpandEnvironmentStringsW( lpCommand, lpCommandLine, 0x2000 );

	//	GetSetDeviceMap();
		PB_StartCOM();

		SetThreadToken( NULL, g_new_ImpersonationToken );

		BOOL bRet = CreateProcessAsUserW(g_new_hToken, 0, lpCommandLine, 0, 0, 0, 0, 0, 0, &StartupInfo, &ProcInfo);
		if ( FALSE == bRet ) { dwErrCode = GetLastError(); }

		SetThreadToken( NULL, NULL );
		CloseHandle( g_new_ImpersonationToken );
		CloseHandle( g_new_hToken );
	}
	else
	{
		// ����ͨȨ������"��SB"�Ľ���
		sei.cbSize = sizeof(SHELLEXECUTEINFOW) ;
		sei.fMask = 0x700 ;
		sei.hwnd = NULL ;
		sei.lpVerb = NULL ;
		sei.lpFile = lpCommand ;
		sei.lpParameters = NULL ;
		sei.lpDirectory = g_lpCurrentDirectory ;
		sei.nShow = 1 ;
		sei.hInstApp = NULL ;

		if ( g_bFlag_Start_RunAsAdmin ) { sei.lpVerb = L"runas"; }

		bRet = ShellExecuteExW( &sei );
		dwErrCode = GetLastError();
		if ( bRet ) { return 0; }

		if ( dwErrCode == ERROR_DDE_FAIL || dwErrCode == ERROR_NO_ASSOCIATION )
		{
			sei.fMask &= 0xFE00 ;
			bRet = ShellExecuteExW( &sei );
			sei.fMask |= 0x100 ;
			
			if ( bRet ) { return 0; }
		}

		lpCommandSub = CParseCMD::GetInstance().GetCommandLineWEx( lpCommand );
		if ( lpCommandSub && *lpCommandSub )
		{
			lpFile = (LPWSTR) HeapAlloc( GetProcessHeap(), 0, 2 * TotalLengthDummy );
			lpParameters = (LPWSTR) HeapAlloc( GetProcessHeap(), 0, 2 * TotalLengthDummy );

			LastLength = (ULONG)((PCHAR)lpCommandSub - (PCHAR)lpCommand);

			wcsncpy( lpFile, lpCommand, LastLength / sizeof(WCHAR) );
			lpFile[ LastLength / sizeof(WCHAR) ] = 0 ;

			for ( i = wcslen(lpFile); i; i = wcslen(lpFile) )
			{
				pData = (LPWSTR)(lpFile + 2 * i - 2);
				if ( *pData != ' ' ) { break; }
				*pData = 0;
			}

			wcscpy( lpParameters, (LPWSTR)(lpCommand + LastLength) );
			sei.lpFile = (LPCWSTR) lpFile ;
			sei.lpParameters = (LPCWSTR) lpParameters ;

			bRet = ShellExecuteExW( &sei );
			dwErrCode = GetLastError();
			if ( bRet ) { return 0; }

			if ( dwErrCode == ERROR_DDE_FAIL || dwErrCode == ERROR_NO_ASSOCIATION )
			{
				sei.fMask &= 0xFE00 ;
				bRet = ShellExecuteExW( &sei );

				if ( bRet ) { return 0; }
			}
		}

		if ( dwErrCode == ERROR_BAD_EXE_FORMAT && GetModuleHandleW(L"oawatch.dll") && PB_RunFromHome(L"PBStart.exe", lpCommand, &StartupInfo, &ProcInfo) )
		{
			return 0;
		}

		LPWSTR lpBuffer = (LPWSTR) HeapAlloc( GetProcessHeap(), 0, 0x4000 );
		ExpandEnvironmentStringsW( lpCommand, lpBuffer, 0x2000 );
		bRet = CreateProcessW( NULL, lpBuffer, 0, 0, 0, 0, 0, g_lpCurrentDirectory, &StartupInfo, &ProcInfo );
		if ( bRet )
			dwErrCode = NO_ERROR ;
		else
			dwErrCode = GetLastError();
	}

	if ( bRet ) { return 0; }
	return 1;
}


///////////////////////////////   END OF FILE   ///////////////////////////////