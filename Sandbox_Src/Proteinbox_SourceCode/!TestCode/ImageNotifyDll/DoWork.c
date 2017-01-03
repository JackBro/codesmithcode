/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/05/10 [10:5:2010 - 14:01]
* MODULE : D:\Work\Program\Coding\ɳ��\SandBox\Code\!TestCode\ImageNotifyDll\DoWork.c
* 
* Description:
*   
*   ������ImageNotify��ע��DLL��ָ���Ľ���.��ʽΪ�޸��ڴ��е�PE�ṹ                     
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include <stdio.h>
#include <ntddk.h>

#include "struct.h"
#include "DoWork.h"
#include "InjectDll.h"
#include "Common.h"
#include "DllData.h"

//////////////////////////////////////////////////////////////////////////

IMAGENOTIY_INFO g_ImageNotify_Info = { FALSE, NULL } ;


//////////////////////////////////////////////////////////////////////////

void
DoWork (
	BOOL bWork
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/12 [12:5:2010 - 11:50]

Routine Description:
  �ɻ�ĺ���

Arguments:
  bWork - [IN] �Ƿ�ɻ�?

--*/
{
	NTSTATUS status ;

	//
	// ע��ģ����ػص�
	//

	if ( TRUE == bWork )
	{	
		if ( TRUE == CheckNotifyState() ) { return ; }

		// ��ʼ����ȡSSDT������ַ
		if ( FALSE == InitSSDT() ) { return ; }

		// �ͷŲ���������C�̸�Ŀ¼
		status = PutFile( g_szDllPathW, g_dll_data, sizeof(g_dll_data) ) ;
		if( ! NT_SUCCESS( status ) )
		{
			dprintf( "PutFile(); Failed: 0x%08lx \n", status );
			return ;
		}

		// ע��ģ��ص�
		status = PsSetLoadImageNotifyRoutine( g_ImageNotify_Info.NotifyRoutine );
		if( ! NT_SUCCESS( status ) )
		{
			dprintf( "PsSetLoadImageNotifyRoutine(); Failed: 0x%08lx \n", status );
			g_ImageNotify_Info.bNotifyState = FALSE ;
			return ;
		}

		g_ImageNotify_Info.bNotifyState = TRUE ;
		dprintf( "*** ImageNotify Set OK! \n" );
	}

	//
	// ж��ģ����ػص�
	//

	else
	{
		if ( FALSE == CheckNotifyState() ) { return ; }

		PsRemoveLoadImageNotifyRoutine( g_ImageNotify_Info.NotifyRoutine );

		g_ImageNotify_Info.bNotifyState  = FALSE	;
		g_ImageNotify_Info.NotifyRoutine = NULL		;

		dprintf( "*** ImageNotify Remove OK! \n" );
	}

	return ;
}



BOOL 
CheckNotifyState (
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/12 [12:5:2010 - 11:50]

Routine Description:
  ���ģ��ص���״̬

Return Value:
  TRUE - ��ע��; FALSE - δע��
    
--*/
{
	BOOL bNotifyState = g_ImageNotify_Info.bNotifyState ;

	if ( NULL == g_ImageNotify_Info.NotifyRoutine )
	{
		g_ImageNotify_Info.NotifyRoutine = (PVOID) ImageNotifyRoutine ;
	}

	return bNotifyState ;
}



//
// ��ע�Ľ����б� [���б��еĽ��̻ᱻע��sudami.dll]
//

static const LPCSTR g_special_processName_Array [] = 
{
	"C:\\Program Files\\Internet Explorer\\IEXPLORE.EXE" ,
	"C:\\WINDOWS\\NOTEPAD.EXE"
};


BOOL
Is_special_process (
	IN PUNICODE_STRING  FullImageName
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/12 [12:5:2010 - 11:50]

Routine Description:
  ƥ������,�Ƿ�Ϊ���ǹ�ע�Ľ���
    
Arguments:
  FullImageName - [IN] ��ƥ����ַ���; ��ʽ����:
  "\Device\HGFS\vmware-host\Shared Folders\Transhipment station\1.exe"
  "\Device\HarddiskDmVolumes\PhysicalDmVolumes\BlockVolume2\1.exe"
  "\Device\HarddiskDmVolumes\PhysicalDmVolumes\BlockVolume1\1.exe"
  "\Device\HarddiskVolume1\Documents and Settings\KHacker\1.exe"
  ...

Return Value:
  �Ƿ�ƥ��ɹ�
    
--*/
{
	int		i			= 0		;
	BOOL	bRet		= FALSE ;
	PCHAR	pImagePath	= NULL	;
	CHAR szDosPath[ MAX_PATH ] = "" ;
	ANSI_STRING  ansiNameString = { 0, 0, NULL } ;

	//
	// 1. ��֤�����Ϸ���
	//

	if ( (NULL == FullImageName) || (0 == FullImageName->Length) && (FALSE == MmIsAddressValid( (PVOID)FullImageName->Buffer )) )
	{
		dprintf( "error! | Is_special_process(); Invalid Paramaters. \n" );
		return FALSE ;
	}
	
	//
	// 2. ת���ַ���UNICODE Ϊ ANSI ��ʽ
	//

	RtlZeroMemory( &ansiNameString, sizeof(ansiNameString) );
	RtlUnicodeStringToAnsiString( &ansiNameString, FullImageName, TRUE );

	pImagePath = ansiNameString.Buffer ;
	if ( FALSE == MmIsAddressValid( (PVOID)pImagePath ) )
	{
		dprintf( "error! | Is_special_process(); NULL == pImagePath; \n" );
		return FALSE ;
	}

	//
	// 3. ��nt·��ת��ΪDos·��;�� "\Device\HarddiskVolume1\xx" --> "c:\xx"
	//

	NtPath2DosPathA( pImagePath, szDosPath, MAX_PATH );

	//
	// 4. ƥ��֮
	//

	for( i = 0; i < ARRAYSIZEOF( g_special_processName_Array ); ++ i )
	{
		if ( 0 == _stricmp( szDosPath, g_special_processName_Array[ i ] ) )
		{
			bRet = TRUE ;
			break ;
		}
	}

	if ( ansiNameString.Buffer ) { RtlFreeAnsiString( &ansiNameString ); } // �ͷ��ڴ�
	
	return bRet ;
}


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                       ģ��ص�����                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


void
ImageNotifyRoutine (
  IN PUNICODE_STRING  FullImageName,
  IN HANDLE  ProcessId,
  IN PIMAGE_INFO  ImageInfo
  )
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ;
	PROCESS_BASIC_INFORMATION ProcessBasicInfo;
	ULONG ReturnLength		= 0x18	;
	ULONG ImageBaseAddress	= 0		;
	
	//
	// 1. У������Ϸ���
	//

	if ( (NULL == ProcessId) || (TRUE == ImageInfo->SystemModeImage ) ) { return ; }

	//
	// 2. ����������
	//

	if ( FALSE == Is_special_process( FullImageName ) ) { return ; }

	//
	// 3. ��һ����֤�Ƿ��ʺ�ע��
	//

	status = ZwQueryInformationProcess(
		(HANDLE) -1 ,
		ProcessBasicInformation ,
		&ProcessBasicInfo ,
		sizeof( ProcessBasicInfo ) ,
		&ReturnLength
		);

	if ( !NT_SUCCESS(status) )
	{
		dprintf( "ImageNotifyRoutine() - ZwQueryInformationProcess(); Failed: 0x%08lx \n", status );
		return ;
	}

	if ( NULL == ProcessBasicInfo.PebBaseAddress )
	{
		dprintf( "ImageNotifyRoutine(); NULL == ProcessBasicInfo.PebBaseAddress; \n" );
		return ;
	}

	ProbeForRead( (LPVOID)((ULONG)ProcessBasicInfo.PebBaseAddress + 8), 4, 4 );

	ImageBaseAddress = *(PULONG) ((ULONG)ProcessBasicInfo.PebBaseAddress + 8) ;

	if ( ImageBaseAddress != (ULONG)ImageInfo->ImageBase )
	{
		dprintf ( 
			"ImageNotifyRoutine(); ImageBaseAddress != ImageInfo->ImageBase; \n"
			"ImageBaseAddress: 0x%08lx \n"
			"ImageInfo->ImageBase: 0x%08lx \n",
			ImageBaseAddress,
			ImageInfo->ImageBase
			);

		return ;
	}

	if ( 0 == ProcessBasicInfo.InheritedFromUniqueProcessId )
	{
		dprintf( "ImageNotifyRoutine(); 0 == ProcessBasicInfo.InheritedFromUniqueProcessId; \n" );
		return ;
	}

	//
	// 4. �ؼ�һ��,ע��DLL
	//

	dprintf( "Starting Inject DLL with modifying PE ...\n" );

	status = InjectDll_by_reconstruct_pe( ImageBaseAddress ); 
	if( NT_SUCCESS( status ) )
	{
		dprintf( "[ImageNotify Reconstruct PE]   Inject \"%s\",Done! (�t_�s)#  \n", g_szDllPathA );
	}

	return ;
}


///////////////////////////////   END OF FILE   ///////////////////////////////
