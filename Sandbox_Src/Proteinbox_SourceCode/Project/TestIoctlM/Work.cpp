#include "stdafx.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "Work.h"


//////////////////////////////////////////////////////////////////////////

CDriver* g_drv_Test = NULL ;
CDriver* g_drv_ProteinBoxDrv = NULL ;

CConfigEx* g_Conf = NULL ;

//////////////////////////////////////////////////////////////////////////


static CHAR g_szProcName[ MAX_PATH ] = 
//"C:\\Program Files\\Tencent\\Foxmail\\Foxmail.exe" ;
//	"C:\\Program Files\\Mozilla Thunderbird\\thunderbird.exe" ;
"D:\\1.exe" ;

//////////////////////////////////////////////////////////////////////////

BOOL
RunPE (
	IN PCHAR szPath,
	IN PCHAR szCmdLine
	)
{
	PROCESS_INFORMATION  pi;
    STARTUPINFO          si;
	
	if ( NULL == szPath )
	{
		return FALSE ;
	}
	
	memset( &pi, 0, sizeof(PROCESS_INFORMATION) );
    memset( &si, 0, sizeof(STARTUPINFO) );
    si.cb = sizeof( STARTUPINFO );
	
	if ( !CreateProcess( szPath, szCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi) )
	{
		// printf( "[ʧ��] ��������\"%s\" \n", g_szProcName );
		return FALSE ;
	}
	
	// printf( "[�ɹ�] ��������\"%s\" \n", g_szProcName );
	WaitForSingleObject( pi.hProcess, 0xffffffff);
	CloseHandle( pi.hProcess);
	return TRUE ;
}



CDriver* 
InitDriver(
	IN LPSTR szDriverPath,
	IN LPSTR szDriverLinkName
	) 
{
	//
	// 1. У������Ϸ���
	//
	
	if ( NULL == szDriverPath || NULL == szDriverLinkName )
	{
		// printf( "error! | InitDriver() | Invalid Paramaters; failed! \n" );
		return NULL ;
	}

	// ��������
	CDriver* drv = new CDriver( szDriverPath, szDriverLinkName );
	if(!drv->StartDriver() || !drv->OpenDevice())
	{
		char sz[8] = "";
		wsprintf(sz, "%d", ::GetLastError());
		//	MessageBox("Load Driver Failed",sz);
	}
	
	return drv ;
}



BOOL
LoadDriver(
	IN LPSTR szDriverPath,
	IN LPSTR szDriverLinkName,
	CDriver** drv
	)
{
	//
	// 1. У������Ϸ���
	//
	
	if ( NULL == szDriverPath || NULL == szDriverLinkName )
	{
		// printf( "error! | LoadDriver() | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	//
	// 2. ���� & �������豸
	//
	
	*drv = InitDriver( szDriverPath, szDriverLinkName );
	
	if ( NULL == *drv ) 
	{
		// printf( "error! | LoadDriver() | Can't Load Driver:\"%ws\" \n", szDriverPath );
		return FALSE ;
	}

	return TRUE ;
}



VOID 
UnloadDriver(
	IN CDriver* drv
	)
{
	if ( drv )
	{
		delete drv;
	}
	
	return ;
}



/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


BOOL
Ioctl_StartProcess ( 
	IN CDriver* drv
	) 
{
	int nReturn = -1 ;
	IOCTL_PROTEINBOX_BUFFER buffer ;
	HANDLE new_hToken, new_ImpersonationToken ;
	WCHAR wszBoxName[ MAX_PATH ] = L"DefaultBox" ;

	//
	// 1. У������Ϸ���
	//
	
	if ( NULL == drv )
	{
		// printf( "error! | Ioctl_StartProcess(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	// ��ʼ��InputBuffer
	buffer.Head.Flag = FLAG_Ioctl_StartProcess ;
	buffer.StartProcessBuffer.new_hToken = &new_hToken ;
	buffer.StartProcessBuffer.new_ImpersonationToken = &new_ImpersonationToken ;
	buffer.StartProcessBuffer.wszRegisterUserID = NULL ;
	buffer.StartProcessBuffer.wszBoxName = wszBoxName ;

	// �ؼ�һ��
	nReturn = drv->IoControl(
		IOCTL_PROTEINBOX,
		(PVOID)&buffer,					// InBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER),	// InBuffer Length
		(PVOID)&buffer,							// OutBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER)
		);

	if ( -1 == nReturn ) {
		return FALSE ;
	}

	return TRUE ;
}


VOID
TestLoadDriver (
	)
{
	g_drv_Test = InitDriver( "d:\\Test.sys", "Test" );
	if ( NULL == g_drv_Test ) 
	{
		// printf( "error! Can't Load Driver:\"d:\\Test.sys\" \n" );
		return  ;
	}

	UnloadDriver( g_drv_Test );

	return ;
}

BOOL
HandlerConf (
	)
{
	BOOL bRet = FALSE ;

	g_Conf = new CConfigEx();
	if ( NULL == g_Conf )
	{
		// printf("error! | HandlerConf() - new CConfig(); | NULL == g_Conf \n");
		return FALSE ;
	}


// 	g_Conf->GetConfString( "GlobalSetting", "OpenIpcPath", &pNode );
// 
// 	printf( "Buffer: \n" );
// 	while ( pNode )
// 	{
// 		printf( "%s\n", pNode->ValueName );
// 		pNode = pNode->next ;
// 	}

//	g_Conf->WriteConfString( "GlobalSetting", "OpenIpcPath", "fuckingBCD" );

//	printf("�����߳�������,��������˳��߳�... \n");
	return TRUE ;
}



BOOL
Ioctl_HookShadow (
	IN CDriver* drv,
	IN BOOL bHook
	)
{
	int nReturn = -1 ;
	IOCTL_PROTEINBOX_BUFFER buffer ;
	
	// У������Ϸ���
	if ( NULL == drv )
	{
		// printf( "error! | Ioctl_HookShadow(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	// ��ʼ��InputBuffer
	buffer.Head.Flag = FLAG_Ioctl_HookShadowSSDT ;
	buffer.HookShadowBuffer.bHook = bHook ;
	
	// �ؼ�һ��
	nReturn = drv->IoControl(
		IOCTL_PROTEINBOX,
		(PVOID)&buffer,					// InBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER),	// InBuffer Length
		(PVOID)&buffer,							// OutBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER)
		);
	
	if ( -1 == nReturn ) {
		return FALSE ;
	}
	
	return TRUE ;
}



BOOL
Ioctl_HookObject (
	IN CDriver* drv,
	IN BOOL bHook
	)
{

	int nReturn = -1 ;
	IOCTL_PROTEINBOX_BUFFER buffer ;
	
	// У������Ϸ���
	if ( NULL == drv )
	{
		// printf( "error! | Ioctl_HookShadow(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	// ��ʼ��InputBuffer
	buffer.Head.Flag = FLAG_Ioctl_HookObject ;
	buffer.HookObjectBuffer.bHook = bHook ;
	
	// �ؼ�һ��
	nReturn = drv->IoControl(
		IOCTL_PROTEINBOX,
		(PVOID)&buffer,					// InBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER),	// InBuffer Length
		(PVOID)&buffer,							// OutBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER)
		);
	
	if ( -1 == nReturn ) {
		return FALSE ;
	}
	
	return TRUE ;
}
