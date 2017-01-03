/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/05/24 [24:5:2010 - 15:50]
* MODULE : d:\Work\Program\Coding\ɳ��\SandBox\Code\Project\ProteinBoxDrv\ImageNotify.c
* 
* Description:
*      
*   ģ��ص����                       
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "Notify.h"
#include "Common.h"
#include "InjectDll.h"
#include "ProcessData.h"
#include "Security.h"
#include "ConfigData.h"
#include "ImageNotify.h"

//////////////////////////////////////////////////////////////////////////

// [����������] �Ƿ�ע��DLL��ɳ���еĽ���
#define _INJECT_DLL_		1

NOTIY_INFO g_ImageNotify_Info = { FALSE, NULL } ;


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
	LPPDNODE pNode = NULL ;
	LPWSTR szName = NULL ;
	WCHAR wszDosPath[ MAX_PATH ] = L""  ;

	//
	// 1. У������Ϸ���
	//

	if ( FALSE == g_Driver_Inited_phrase1 || FALSE == g_bMonitor_Notify ) { return ; }

	if ( (NULL == ProcessId) || (TRUE == ImageInfo->SystemModeImage ) ) { return ; }

	//
	// 2. ��һ����֤�Ƿ��ʺ�ע��
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
#if 0
		dprintf ( 
			"ImageNotifyRoutine(); ImageBaseAddress != ImageInfo->ImageBase; \n"
			"ImageBaseAddress: 0x%08lx \n"
			"ImageInfo->ImageBase: 0x%08lx \n",
			ImageBaseAddress,
			ImageInfo->ImageBase
			);
#endif
		return ;
	}

	if ( 0 == ProcessBasicInfo.InheritedFromUniqueProcessId )
	{
		dprintf( "ImageNotifyRoutine(); 0 == ProcessBasicInfo.InheritedFromUniqueProcessId; \n" );
		return ;
	}

	// 2.1 ��ȡDos��ʽ��ģ��ȫ·��
	if ( FullImageName && FullImageName->Length && FullImageName->Buffer )
	{
		szName = FullImageName->Buffer;
	}
	else
	{
		szName = L"unknown executable image" ;
	}

	NtPath2DosPathW( szName, wszDosPath, MAX_PATH );

	// 2.2 ����PID��Ӧ�Ľ����ܽڵ�	
	pNode = (LPPDNODE) kgetnodePD( ProcessId );
	if ( NULL == pNode ) 
	{ 
		// Ϊ��,������ʱ�����Ĳ���ɳ���н��̴������ӽ���,����ɳ���ⲿ����. ��ע"ǿ�б�������ɳ���е�"����
		pNode = kbuildnodePDF( ProcessId, wszDosPath );
	}
	
	if ( NULL == pNode ) { return ; }
	
	InterlockedIncrement( &pNode->XImageNotifyDLL.IncrementCounts );

	if ( (1 == pNode->bDiscard) || (1 != pNode->XImageNotifyDLL.IncrementCounts) )
	{
		dprintf( "error! | ImageNotifyRoutine(); | ��ǰ�ڵ��Ǳ�������,����IncrementCounts�Ѿ���Ϊ1(pNode->IncrementCounts=%d) \n", pNode->XImageNotifyDLL.IncrementCounts );
		return ;
	}

	// 2.3 ���ڵ�
	PDFixupNode( pNode, wszDosPath );

	// 3. �ؼ�һ��,ע��DLL
#if _INJECT_DLL_

	dprintf( "[ImageNotify Inject DLL] starting modifying PE:\"%ws\" \n", wszDosPath );
	do 
	{
		BOOL bRet = FALSE;
		LPSTR szDllPath = NULL;
		
		// 3.1 ��ѯ�����ļ����õ���ע���DLLȫ·��
		bRet = GetPBDllPath( PBDLLPathType_Reg, MAX_PATH, &szDllPath );
		if ( FALSE == bRet || NULL == szDllPath )
			break;

		// 3.2 У���ļ��Ƿ���ڣ�

		// 3.3 Inject ��
		status = InjectDll_by_reconstruct_pe( (PVOID)pNode, ImageBaseAddress, szDllPath ); 
		if( NT_SUCCESS( status ) )
		{
			dprintf( "[ImageNotify Inject DLL]  Done! (�t_�s)#  \n\n" );
		}

	} while (FALSE);
#endif

	pNode->bProcessNodeInitOK = TRUE ;
	return ;
}



BOOL 
CheckImageNotifyState (
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
	"C:\\WINDOWS\\NOTEPAD.EXE",
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


///////////////////////////////   END OF FILE   ///////////////////////////////