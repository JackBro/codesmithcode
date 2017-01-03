/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/05/20 [20:5:2010 - 15:29]
* MODULE : d:\Work\Program\Coding\ɳ��\SandBox\Code\Project\ProteinBoxDrv\SdtData.c
* 
* Description:
*      
*   ������ SSDT & Shadow SSDT����ģ��                       
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "Version.h"
#include "Common.h"
#include "LDasm.h"
#include "asm_xde.h"
#include "HookEngine.h"
#include "ShadowSSDTProc.h"
#include "SdtData.h"


//////////////////////////////////////////////////////////////////////////

LPMPHEAD	g_ListHead__MappedPE	= NULL  ; 
BOOL		g_MappedPE_Inited_ok	= FALSE ;
SDT_DATA	g_SdtData				= { FALSE, 0, NULL, 0, 0 }	;


//
// ssdt & shadow ssdt ����
//

static SSDT_SSSDT_FUNC g_ssdt_Array_special [] = 
{
	{ L"NTDLL", "NtRequestPort",			 0x2, -1, 0, (ULONG)fake_NtRequestPort, 0, CallHookE8 },
	{ L"NTDLL", "NtRequestWaitReplyPort",	 0x3, -1, 0, (ULONG)fake_NtRequestWaitReplyPort, 0, CallHookE8 },
	{ L"NTDLL", "NtAlpcSendWaitReceivePort", 0x8, -1, __vista, (ULONG)fake_NtAlpcSendWaitReceivePort, 0, CallHookE8 },
};


static SSDT_SSSDT_FUNC g_ssdt_sssdt_Array [] = 
{
	{ L"NTDLL", "ZwCreateToken",			 0xD, -1, 0, 0, Tag_ZwCreateToken		},
	{ L"NTDLL", "ZwSetInformationToken",	 0x4, -1, 0, 0, Tag_ZwSetInformationToken	},
	{ L"NTDLL", "ZwProtectVirtualMemory",	 0x5, -1, 0, 0, Tag_ZwProtectVirtualMemory	},
	{ L"NTDLL", "ZwQueryInformationThread",  0x5, -1, 0, 0, Tag_ZwQueryInformationThread },
	
	// ******* ssdt & shadow ssdt �ķֽ��� *******

	{ L"USER32", "GetForegroundWindow",	0x0, -1, 0, 0, Tag_GetForegroundWindow }, // ��Ӧg_NtUserGetForegroundWindow_addr
	{ L"USER32", "IsHungAppWindow",		0x2, -1, 0, 0, Tag_IsHungAppWindow },		// ��Ӧg_NtUserQueryWindow_addr
	{ L"USER32", "GetClassNameW",		0x3, -1, 0, 0, Tag_GetClassNameW },		// ��Ӧg_NtUserGetClassName_addr
	{ L"USER32", "SetWindowsHookExW",	0x6, -1, 0, (ULONG)fake_NtUserSetWindowsHookEx, Tag_SetWindowsHookExW, InlineHookPre1 },
	{ L"USER32", "SetWinEventHook",		0x8, -1, 0, (ULONG)fake_NtUserSetWinEventHook, Tag_SetWinEventHook, InlineHookPre1 },
	{ L"USER32", "PostMessageW",		0x4, -1, 0, (ULONG)fake_NtUserPostMessage, Tag_PostMessageW, InlineHookPre1 },
	{ L"USER32", "PostThreadMessageW",	0x4, -1, 0, (ULONG)fake_NtUserPostThreadMessage, Tag_PostThreadMessageW, InlineHookPre1 },
	{ L"USER32", "EnableWindow",		0x3, -1, 0, (ULONG)fake_NtUserCallHwndParamLock, Tag_EnableWindow, InlineHookPre1 },
	{ L"USER32", "DestroyWindow",		0x1, -1, 0, (ULONG)fake_NtUserDestroyWindow, Tag_DestroyWindow, InlineHookPre1 },
	{ L"USER32", "SendInput",			0x3, -1, 0, (ULONG)fake_NtUserSendInput, Tag_SendInput, CallHookE8 },
	{ L"USER32", "BlockInput",			0x1, -1, 0, (ULONG)fake_NtUserBlockInput, Tag_BlockInput, InlineHookPre1 },
	{ L"USER32", "SetSysColors",		0x4, -1, 0, (ULONG)fake_NtUserSetSysColors, Tag_SetSysColors, InlineHookPre1 },
	{ L"USER32", "SystemParametersInfoW",0x4,-1, 0, (ULONG)fake_NtUserSystemParametersInfo, Tag_SystemParametersInfoW, CallHookE8 },

	/*
	�˺����ĵ�����:

	SendMessageTimeoutW---\
						   |---> SendMessageTimeoutWorker->NtUserMessageCall
	SendMessageW-----------/
	*/
	{ L"USER32", "SendMessageTimeoutW",	0x7, -1, 0, (ULONG)fake_NtUserMessageCall, Tag_SendMessageTimeoutW, InlineHookPre1 }, 
	
	{ L"USER32", "SendNotifyMessageW",	0x4, -1, __win2k, (ULONG)fake_NtUserSendNotifyMessage, Tag_SendNotifyMessageW_win2k, InlineHookPre1 }, // win2k����NtUserSendNotifyMessage
	{ L"USER32", "SendNotifyMessageW",	0x7, -1, 0,		(ULONG)fake_NtUserMessageCall, Tag_SendNotifyMessageW, InlineHookPre1 },		  // ����ƽ̨ΪNtUserMessageCall

	//
	// ��ǰ��ַ   ���2����        ���ָ��       
	// 77314e3c   ff148dd8ee3177   call dword ptr USER32!gapfnScSendMessage (7731eed8)[ecx*4]
	// ������õ������е�ĳ������,ecx����Index����
	//
	{ L"USER32", "SendMessageCallbackW", 0x6, -1, __win2k, (ULONG)fake_NtUserSendMessageCallback, Tag_SendMessageCallbackW_win2k, InlineHookPre1 }, // win2k����NtUserSendMessageCallback
	{ L"USER32", "SendMessageCallbackW", 0x7, -1, 0,		 (ULONG)fake_NtUserMessageCall, Tag_SendMessageCallbackW, InlineHookPre1 },         // ����ƽ̨ΪNtUserMessageCall

};


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
InitSdtData (
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/19 [19:5:2010 - 18:29]

Routine Description:
  ��ʼ��MappedPE�ṹ������� & sdt����ָ��

--*/
{
	int i = 0 ;
	BOOL bRet = FALSE ;

	// 1. ��ʼ��sdt����ָ��
	if ( g_SdtData.bInited ) { return TRUE; }
	 
	g_SdtData.SpecArray		= g_ssdt_Array_special ; 
	g_SdtData.SpecCounts	= ARRAYSIZEOF( g_ssdt_Array_special );

	g_SdtData.pSdtArray		= g_ssdt_sssdt_Array ; 
	g_SdtData.TotalCounts	= ARRAYSIZEOF( g_ssdt_sssdt_Array );

	g_SdtData.ShadowArrayIndex = 0 ;

	// 2. ��ʼ��MappedPE�ṹ�������
	if ( TRUE == g_MappedPE_Inited_ok && g_ListHead__MappedPE ) { return TRUE ; }
	
	bRet = MPCreateTotalHead( (PVOID) &g_ListHead__MappedPE );
	if ( FALSE == bRet || NULL == g_ListHead__MappedPE ) 
	{ 
		dprintf( "error! | InitSdtData() - MPCreateTotalHead(); \n" );
		g_MappedPE_Inited_ok = FALSE ;
		return FALSE ;
	}

	// 3. �������е�FAKE_FUNC_INFO(���а������ü���) �ṹ��ʼ��Ϊ0
	for( i = 0; i < g_SdtData.SpecCounts;  i++ ) { g_ssdt_Array_special[i].FakeFuncInfo.RefCounts	= 0 ; }
	for( i = 0; i < g_SdtData.TotalCounts; i++ ) { g_ssdt_sssdt_Array[i].FakeFuncInfo.RefCounts		= 0 ; }

	g_MappedPE_Inited_ok = TRUE ;
	g_SdtData.bInited = TRUE ;
	return TRUE ;
}



ULONG
GetProcAddress (
	IN LPWSTR wszModuleName,
	IN LPSTR szFunctionName,
	IN BOOL bReloc
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/19 [19:5:2010 - 15:20]

Routine Description:
  �õ�ָ��ģ��ָ���������ĵ�ַ(ӳ�䵽�Լ��ڴ��һ����Ե�ַ)    
    
Arguments:
  wszModuleName - ��Map��ģ�����. eg: L"ntdll"
  szFunctionName - ��ƥ��ĺ�����
  bReloc - ʼ��ΪFALSE

Return Value:
  ������ַ
    
--*/
{
	PVOID pMappedInfo	= NULL	;
	ULONG FunctionAddr	= 0		;

	//
	// 1. У������Ϸ���
	//
	
	if ( NULL == wszModuleName || NULL == szFunctionName )
	{
		dprintf( "error! | GetProcAddress() | Invalid Paramaters; failed! \n" );
		return 0 ;
	}

	//
	// 2. ӳ��ģ�鵽�ڴ�
	//

	pMappedInfo = LoadPE( wszModuleName );
	if ( NULL == pMappedInfo )
	{
		dprintf( "error! | GetProcAddress() - LoadPE() | NULL == pMappedInfo \n" );
		return 0 ;
	}

	//
	// 3. �õ�ӳ�������е�ָ��������ַ
	//

	FunctionAddr = GetMappedFuncAddr( pMappedInfo, szFunctionName, bReloc );
	if ( 0 == FunctionAddr )
	{
		dprintf( "error! | GetProcAddress() - GetMappedFuncAddr() | 0 == FunctionAddr \n" );
		return 0 ;
	}

	SDTTrace( "ok! | GetProcAddress(); | FunctionName:%s; FunctionAddr: 0x%08lx \n", szFunctionName, FunctionAddr );
	return FunctionAddr ;
}



PVOID
LoadPE (
	IN LPWSTR wszModuleName
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/19 [19:5:2010 - 15:16]

Routine Description:
  ӳ��ָ��ģ�鵽�ڴ�,Ϊ����������EAT������ַ��׼��
    
Arguments:
  wszModuleName - ��Map��ģ�����. eg: L"ntdll"

Return Value:
  ����ӳ��ṹ��ָ��
    
--*/
{
	LPMPHEAD pTotalHead = (LPMPHEAD) g_ListHead__MappedPE ;
	LPMPNODE  pResult = NULL ;

	//
	// 1. У������Ϸ���
	//
	
	if ( NULL == wszModuleName )
	{
		dprintf( "error! | LoadPE() | Invalid Paramaters; failed! \n" );
		return NULL ;
	}

	//
	// 2. �Ƿ����Ѵ��ڵĽڵ�
	//

	pResult = (LPMPNODE) MPFindNode( (PVOID)pTotalHead, wszModuleName );
	if ( pResult )
	{
	//	dprintf( "ok! | LoadPE() - MPFindNode(); | Ҫӳ���ģ��\"%ws.DLL\"����������,ֱ�ӷ���֮ \n", wszModuleName );
		return pResult ;
	}

	//
	// 3. �½��ڵ�,���֮
	//

	pResult = (LPMPNODE) MPBuildNode( (PVOID)pTotalHead, wszModuleName );
	if ( NULL == pResult )
	{
		dprintf( "error! | LoadPE() - MPBuildNode() \n" );
		return NULL ;
	}

	SDTTrace( "ok! | LoadPE(); | �����½ڵ�\"%ws.DLL\"������β. \n", wszModuleName );
	return (PVOID) pResult ;
}



ULONG 
GetMappedFuncAddr (
	IN PVOID _pMappedInfo,
	IN LPSTR szFunctionName, 
	IN BOOL bReloc
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/19 [19:5:2010 - 15:16]

Routine Description:
  ������Mapped���ڴ��PE�����в�����EAT,����@szFunctionName��Ӧ�ĵ�ַ
    
Arguments:
  _pMappedInfo - ӳ��ṹ��ָ��
  szFunctionName - ��ƥ��ĺ�����
  bReloc - ʼ��ΪTRUE

Return Value:
  ����ӳ���ڴ��е�ָ��������ַ
    
--*/
{
	ULONG NumberOfNames, nIndex, ret ;
	PIMAGE_EXPORT_DIRECTORY pEAT	= NULL	; 
	PCHAR	szCurrentName			= NULL	;
	PULONG	AddressOfNames			= NULL	;
	PULONG	AddressOfFunctions		= NULL	;
	PUSHORT AddressOfNameOrdinals	= NULL	; 
	LPMPNODE pMappedInfo			= (LPMPNODE) _pMappedInfo ;
	
	//
	// 1. У������Ϸ���
	//

	if ( NULL == _pMappedInfo || NULL == szFunctionName )
	{
		dprintf( "error! | GetMappedFuncAddr(); | Invalid Paramaters; failed! \n" );
		return 0 ;
	}

	//
	// 2. �õ�EAT��ĸ�����Ҫ��Ա
	//

	pEAT = pMappedInfo->pEATAddr;
	AddressOfNames			= (PULONG)  GetRealAddr( pEAT->AddressOfNames,		pMappedInfo	);
	AddressOfFunctions		= (PULONG)  GetRealAddr( pEAT->AddressOfFunctions,	pMappedInfo	);
	AddressOfNameOrdinals	= (PUSHORT) GetRealAddr( pEAT->AddressOfNameOrdinals, pMappedInfo	);
	NumberOfNames			= pEAT->NumberOfNames ;

	if ( NULL == AddressOfNames || NULL == AddressOfFunctions || NULL == AddressOfNameOrdinals || 0 == NumberOfNames )
	{
		dprintf( "error! | GetMappedFuncAddr(); | 0 == AddressOfNames \n" );
		return 0 ;
	}

	//
	// 3. ����EAT��,�ҵ�ָ���ĺ�����ַ
	//

	nIndex = 0 ;
	while ( 1 )
	{
		szCurrentName = (PCHAR) GetRealAddr( AddressOfNames[ nIndex ], pMappedInfo );
		if ( NULL != szCurrentName )
		{
			if ( 0 == _stricmp( szCurrentName, szFunctionName ) )
			{
				break ;
			}	
		}

		++ nIndex ;
		if ( nIndex > NumberOfNames )
		{
			dprintf( "error! | GetMappedFuncAddr(); | nIndex > NumberOfNames \n" );
			return 0 ;
		}
	}

	ret = AddressOfFunctions[ AddressOfNameOrdinals[ nIndex ] ];
	if ( TRUE == bReloc )
	{
		ret = GetRealAddr( ret, pMappedInfo );
	}

	return ret;
}



BOOL 
MPCreateTotalHead(
	OUT PVOID* _TotalHead
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/04/29 [29:4:2009 - 14:00]

Routine Description:
  �����ܽṹ��,��ʼ��֮    
    
Arguments:
  _TotalHead - Ҫ��ʼ�����ܽṹ��ָ��ĵ�ַ

Return Value:
  BOOL
    
--*/
{
	BOOL bRet = FALSE ;
	LPMPHEAD *pTotalHead = (LPMPHEAD*) _TotalHead ;
	if ( NULL !=  *pTotalHead ) { return TRUE ; }

	// Ϊ�ܽṹ������ڴ�
	*pTotalHead = (LPMPHEAD) kmalloc( sizeof( MPHEAD ) );
	if ( NULL == *pTotalHead )
	{
		dprintf( "error! | MPCreateTotalHead(); Allocate memory failed! \n" );
		return FALSE ;
	}

	RtlZeroMemory( *pTotalHead, sizeof( MPHEAD ) );

	// ��ʼ����Դ��
	bRet = InitResource( &((LPMPHEAD)*pTotalHead)->QueueLockList );
	if ( FALSE == bRet )
	{
		dprintf( "error! | MPCreateTotalHead() - InitResource(); ������Դ���ڴ�ʧ��! \n" );
		kfree( (PVOID) *pTotalHead );
		return FALSE ;
	}

	// ��ʼ������ͷ
	InitializeListHead( (PLIST_ENTRY)&( (LPMPHEAD)*pTotalHead )->ListHead );
	((LPMPHEAD)*pTotalHead)->nTotalCounts = 0	;

	return TRUE ;
}



VOID 
MPDeleteTotalHead(
	IN PVOID* _TotalHead
	)
{
	if ( NULL ==  *_TotalHead ) { return  ; }

	kfree( *_TotalHead );
	*_TotalHead = NULL ;

	return  ;
}



BOOL 
MPAllocateNode(
	OUT PVOID* _pCurrenList_
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/04/29 [29:4:2009 - 14:01]

Routine Description:
  ���� & ��ʼ��һ������ڵ�  
    
Arguments:
  _pCurrenList_ - �������õ�����Node��ָ��

Return Value:
  BOOL
    
--*/
{
	LPMPNODE* pCurrenList = (LPMPNODE*) _pCurrenList_ ;
	*pCurrenList = (LPMPNODE) kmalloc( sizeof( MPNODE ) );

	if ( NULL == *pCurrenList )
	{
		dprintf( "error! | MPAllocateNode() | Allocate memory failed! \n" );
		return FALSE ;
	}

	RtlZeroMemory( *pCurrenList, sizeof( MPNODE ) );

	return TRUE ;
}



PVOID
MPBuildNode (
	IN PVOID _TotalHead ,
	IN LPWSTR wszModuleName
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/19 [19:5:2010 - 20:11]

Routine Description:
  ӳ��ģ�鵽�ڴ�,��������ָ���ĺ�����ַ,�½�&���MP�ṹ��
    
Arguments:
  _TotalHead - ����ͷ
  wszModuleName - ��Map��ģ�����. eg: L"ntdll.dll"

Return Value:
  �½���MP�ṹ��ָ��
    
--*/
{
	BOOL bRet = FALSE ;
	LPMPNODE pMappedInfo = NULL ;
	LPMPHEAD pTotalHead	 = (LPMPHEAD) _TotalHead ;

	// 1. У������Ϸ���
	if ( NULL == _TotalHead || NULL == wszModuleName )
	{
		dprintf( "error! | MPBuildNode() | Invalid Parameters \n" );
		return NULL ;
	}

	// 2. ���ڵ�
	pMappedInfo = MPBuildNodeEx( _TotalHead, wszModuleName );
	if ( NULL == pMappedInfo )
	{
		dprintf( "error! | MPBuildNode() - MPBuildNodeEx() | \n" );
		return NULL ;
	}

	// 3. Dump
	SDTTrace( 
		"ok! | MPBuildNode(); | �½���MP�ڵ���Ϣ����:\n"
		"struct _MAPPED_PE_HEAND_ * 0x%08lx \n"
		" +0x000 pFlink : 0x%08lx			\n"
		" +0x004 pBlink : 0x%08lx			\n"
		" +0x008 wszModuleName : \"%ws\"	\n"
		" +0x048 hFile : 0x%08lx			\n"
		" +0x04C SectionHandle : 0x%08lx	\n"
		" +0x050 ImageBase : 0x%08lx		\n"
		" +0x054 SizeOfImage : %d			\n"
		" +0x058 MappedAddr : 0x%08lx		\n"
		" +0x05C inh : 0x%08lx				\n"
		" +0x060 pEATAddr : 0x%08lx			\n\n",
		(PVOID)pMappedInfo,
		pMappedInfo->pFlink, pMappedInfo->pBlink, pMappedInfo->wszModuleName, pMappedInfo->hFile, pMappedInfo->SectionHandle,
		pMappedInfo->ImageBase, pMappedInfo->SizeOfImage, pMappedInfo->MappedAddr, pMappedInfo->pinh, pMappedInfo->pEATAddr
		);

	return (PVOID)pMappedInfo ;
}



PVOID
MPBuildNodeEx (
	IN PVOID _TotalHead ,
	IN LPWSTR wszModuleName
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/19 [19:5:2010 - 20:48]

Routine Description:
  ���MP�ڵ�@pNode  
    
Arguments:
  wszModuleName - ��Map��ģ�����. eg: L"ntdll"

Return Value:
  �ڵ�ָ��
    
--*/
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ;
	UNICODE_STRING uniBuffer ;
	OBJECT_ATTRIBUTES objectAttributes ;
	IO_STATUS_BLOCK IoStatusBlock ;
	FILE_STANDARD_INFORMATION FileInformation ;
	PIMAGE_DOS_HEADER		pidh		 = NULL ;
	PIMAGE_NT_HEADERS		pinh		 = NULL ;
	PIMAGE_DATA_DIRECTORY	pidd		 = NULL ; 
	BOOL					bRet	 = FALSE ;
	ULONG					ViewSize = 0 ;
	ULONG					EATAddr	 = 0 ;
	USHORT					Magic	 = 0 ;
	WCHAR	wszName[ MAX_PATH ] = L"" ;
	LPMPNODE pNode = NULL ;
	LPMPHEAD pTotalHead	 = (LPMPHEAD) _TotalHead ;

	//
	// 1. У������Ϸ���
	//
	
	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList || NULL == wszModuleName )
	{
		dprintf( "error! | MPBuildNodeEx() | Invalid Paramaters; failed! \n" );
		return NULL ;
	}

	// 1.2 Ϊ�½ڵ�����ڴ�,���֮
	bRet = MPAllocateNode( &pNode );
	if ( FALSE == bRet )
	{
		dprintf( "error! | MPBuildNodeEx() - MPAllocateNode() | �����ڴ�ʧ�� \n" );
		return NULL ;
	}

	//
	// 2. ����ģ�������ṹ��, ���ļ�,��ȡ�ļ���С
	//

	wcscpy( pNode->wszModuleName, wszModuleName );

	swprintf( wszName, L"\\SystemRoot\\System32\\%s.dll", wszModuleName );
	RtlInitUnicodeString( &uniBuffer, wszName );

	InitializeObjectAttributes(
		&objectAttributes ,
		&uniBuffer ,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL 
		);

	// 2.1 ���ļ�
	status = ZwCreateFile ( 
		(PHANDLE) &pNode->hFile,
		0x120089,
		&objectAttributes,
		&IoStatusBlock,
		NULL,
		0,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE ,
		FILE_OPEN ,
		FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE ,
		0,
		0
		);

	if ( !NT_SUCCESS(status) )
	{
		dprintf( "error! | MPBuildNode() - ZwCreateFile() | status = 0x%08lx \n", status );
		goto _error_over_ ;
	}

	// 2.2 ��ѯ�ļ���С
	status = ZwQueryInformationFile ( 
		pNode->hFile ,
		&IoStatusBlock,
		&FileInformation,
		sizeof( FILE_STANDARD_INFORMATION ),
		FileStandardInformation
		);

	if ( !NT_SUCCESS(status) )
	{
		dprintf( "error! | MPBuildNode() - ZwQueryInformationFile() | status = 0x%08lx \n", status );
		ZwClose( pNode->hFile ); // �ر��ļ����
		goto _error_over_ ;
	}

	//
	// 3. �����ļ���Сӳ���ļ����ڴ�
	//

	status = ZwCreateSection (
		&pNode->SectionHandle ,
		SECTION_MAP_READ | SECTION_QUERY ,
		0,
		&FileInformation.EndOfFile,
		PAGE_READONLY,
		SEC_RESERVE,
		pNode->hFile
		);

	if ( !NT_SUCCESS(status) )
	{
		dprintf( "error! | MPBuildNode() - ZwCreateSection() | status = 0x%08lx \n", status );
		ZwClose( pNode->hFile ); // �ر��ļ����
		goto _error_over_ ;
	}

	ViewSize = FileInformation.EndOfFile.LowPart ;
	status = ZwMapViewOfSection (
		pNode->SectionHandle ,
		(HANDLE) 0xFFFFFFFF,
		(PVOID *)&pNode->MappedAddr,
		0,
		0,
		0,
		&ViewSize,
		ViewUnmap,
		0,
		PAGE_READONLY
		);

	if ( !NT_SUCCESS(status) )
	{
		dprintf( "error! | MPBuildNode() - ZwMapViewOfSection() | status = 0x%08lx \n", status );
		goto _error_clear_up_ ;
	}

	//
	// 4. ��ȡPE������Ϣ
	//

	pidh = (PIMAGE_DOS_HEADER) pNode->MappedAddr ;
	bRet = IsValidPE( (ULONG)pidh );
	if ( FALSE == bRet )
	{
		dprintf( "error! | MPBuildNode() - IsValidPE() | Invalid PE \n" );
		goto _error_clear_up_ ;
	}

	// 4.1 ��ȡPE��ַImageBase
	pNode->pinh = pinh = (PIMAGE_NT_HEADERS)( (PCHAR)pidh + pidh->e_lfanew );
	pNode->SizeOfImage = pinh->OptionalHeader.SizeOfImage ;

	Magic = pinh->OptionalHeader.Magic ;

	switch ( Magic )
	{
	case 0x10B : // Ӧ������0x10B
		{
			pNode->ImageBase = pinh->OptionalHeader.ImageBase ;
			pidd = pinh->OptionalHeader.DataDirectory ;
		}
		break ;

	case 0x20B :
		{
			pNode->ImageBase = pinh->OptionalHeader.BaseOfData ;
			pidd = &pinh->OptionalHeader.DataDirectory[2] ;
		}
		break ;

	default :
		pidd = NULL ;
		break ;
	}

	if ( NULL == pidd )
	{
		dprintf( "error! | MPBuildNode();| Invalid Magic: 0x%08lx \n", Magic );
		goto _error_clear_up_ ;
	}

	// 4.2 ��ȡEAT��ַ
	EATAddr = GetRealAddr( pidd[ IMAGE_DIRECTORY_ENTRY_EXPORT ].VirtualAddress, (PVOID)pNode );
	if ( 0 == EATAddr )
	{
		dprintf( "error! | MPBuildNode() - GetRealAddr();| 0 == EATAddr \n" );
		goto _error_clear_up_ ;
	}

	pNode->pEATAddr = (PIMAGE_EXPORT_DIRECTORY) EATAddr ;

	//
	// 5. ����Ҫ,������½ڵ㵽����β
	//

	EnterCrit( pTotalHead->QueueLockList );	// ��������

	if ( 0 == pTotalHead->ListHead.MappedAddr && NULL == pTotalHead->ListHead.hFile ) 
	{
		// ����ͷδ��ʹ��.use it
		pTotalHead->nTotalCounts = 1 ;
		pTotalHead->ListHead = *pNode ;
		InitializeListHead( (PLIST_ENTRY)&pTotalHead->ListHead );

		kfree( (PVOID)pNode ); // �ͷ��ڴ�
		pNode = &pTotalHead->ListHead ;
	}
	else
	{
		InsertTailList( (PLIST_ENTRY) &pTotalHead->ListHead, (PLIST_ENTRY) pNode );
		pTotalHead->nTotalCounts ++ ;
	}

	LeaveCrit( pTotalHead->QueueLockList );	// �ͷ���
	return (PVOID)pNode ; // �ɹ�,���ļ�������ر�,��Ϊ�����´λ�Ҫ�õ�; �ȵ�����ж��ʱ��ͳһ����ر�

_error_clear_up_ : // ʧ����ر����д򿪵ľ��
	ZwClose( pNode->hFile );			// �ر��ļ����
	ZwClose( pNode->SectionHandle );	// �ر�Section���

_error_over_ :
	kfree( (PVOID)pNode ); // �ͷ��ڴ�
	return NULL ;
}



PVOID
MPFindNode (
	IN PVOID  _TotalHead,
	IN LPWSTR wszModuleName
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/19 [19:5:2010 - 17:11]

Routine Description:
  ����@ListHead����,����@wszModuleName��Ӧ�Ľڵ�
    
Arguments:
  _TotalHead - ����ͷ
  wszModuleName - ��ƥ���ģ�����.eg: L"ntdll.dll"

Return Value:
  �ɹ� - ��Ӧ�Ľ��ָ��; ʧ�� - NULL
    
--*/
{
	PVOID  pResult = NULL ;
	LPMPNODE pHead = NULL, pNode = NULL ;
	LPMPHEAD pTotalHead	   = (LPMPHEAD) _TotalHead ;
	
	//
	// 1. У������Ϸ���
	//

	if ( NULL == _TotalHead || NULL == pTotalHead->QueueLockList || NULL == wszModuleName )
	{
		dprintf( "error! | MPFindNode() | Invalid Paramaters; failed! \n" );
		return NULL ;
	}

	if ( 0 == pTotalHead->nTotalCounts ) { return NULL ; }

	//
	// 2. ��������,����ָ���ڵ�
	//

	EnterCrit( pTotalHead->QueueLockList );	// ��������
	pNode = pHead = &pTotalHead->ListHead ;

	do 
	{
		if ( 0 == _wcsicmp( pNode->wszModuleName, wszModuleName ) ) 
		{
			pResult = (PVOID)pNode ;
			break ;
		}

		pNode = pNode->pBlink ;
	} while ( pNode != pHead );

	LeaveCrit( pTotalHead->QueueLockList ); // �ͷ���
	return pResult ;
}



VOID
MPDistroyAll (
	IN PVOID _TotalHead
	)
{
	int i = 0 ;
	LPMPNODE pHead = NULL, pNode = NULL ;
	LPMPHEAD pTotalHead = (LPMPHEAD) _TotalHead ;

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | MPDistroyAll() | Invalid ListHead \n" );
		return ;
	}

	//
	// 1. ɾ�����е��ӽڵ�
	//

	MPWalkNodes( pTotalHead );

	dprintf( "*** ��ʼж��MappedPE���� *** \n" );

	EnterCrit( pTotalHead->QueueLockList );	// ��������
	pNode = pHead = &pTotalHead->ListHead ;

	do
	{
		dprintf( "  [%d] Node:0x%08lx \n", i++, (PVOID)pNode );

		if ( pNode->MappedAddr )
			ZwUnmapViewOfSection( (HANDLE)0xFFFFFFFF, (PVOID)pNode->MappedAddr );

		if ( pNode->SectionHandle )
			ZwCloseSafe( pNode->SectionHandle, TRUE );

		if ( pNode->hFile )
			ZwCloseSafe( pNode->hFile, TRUE );

		if ( pHead != pNode )
		{
			RemoveEntryList( (PLIST_ENTRY) pNode );
			kfree( (PVOID)pNode );
		}

		pNode = pHead->pFlink ;
	} while ( FALSE == IsListEmpty( (PLIST_ENTRY) pHead ) );

	LeaveCrit( pTotalHead->QueueLockList );	// ��������

	ExDeleteResource( pTotalHead->QueueLockList );
	kfree( pTotalHead->QueueLockList );

	//
	// 2. ɾ���ܽڵ�
	//

	MPDeleteTotalHead( &_TotalHead );

	dprintf( "*** ����ж��MappedPE���� *** \n" );
	return ;
}



VOID
MPWalkNodes (
	IN PVOID _TotalHead
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/17 [17:5:2010 - 17:13]

Routine Description:
  ��ӡ���нڵ���Ϣ
    
--*/
{
	int i = 0 ;
	LPMPNODE pHead = NULL, pNode = NULL ;
	LPMPHEAD pTotalHead	   = (LPMPHEAD) _TotalHead ;

#ifdef DBG

	//
	// 1. У������Ϸ���
	//

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | MPWalkNodes() | Invalid ListHead \n" );
		return ;
	}

	if ( 0 == pTotalHead->nTotalCounts )
	{
		dprintf( "error! | MPWalkNodes() | ����Ϊ��,���ɱ��� \n" );
		return ;
	}

	//
	// 2. ��������,����ָ���ڵ�
	//

	dprintf( "\n**** Starting walking MappedPE Lists **** \n\n" );

	EnterCrit( pTotalHead->QueueLockList );	// ��������
	pNode = pHead = &pTotalHead->ListHead ;

	do
	{
		dprintf ( 
			"[%d] struct _MAPPED_PE_HEAND_ * 0x%08lx \n"
			" +0x000 pFlink : 0x%08lx			\n"
			" +0x004 pBlink : 0x%08lx			\n"
			" +0x008 wszModuleName : \"%ws\"	\n"
			" +0x048 hFile : 0x%08lx			\n"
			" +0x04C SectionHandle : 0x%08lx	\n"
			" +0x050 ImageBase : 0x%08lx		\n"
			" +0x054 SizeOfImage : 0x%08lx		\n"
			" +0x058 MappedAddr : 0x%08lx		\n"
			" +0x05C inh : 0x%08lx				\n"
			" +0x060 pEATAddr : 0x%08lx			\n\n",
			i++, (PVOID)pNode,
			pNode->pFlink, pNode->pBlink, pNode->wszModuleName, pNode->hFile, pNode->SectionHandle,
			pNode->ImageBase, pNode->SizeOfImage, pNode->MappedAddr, pNode->pinh, pNode->pEATAddr
			);

		pNode = pNode->pFlink ;
		if ( NULL == pNode ) { break ; }
	} while ( pNode != pHead );

	LeaveCrit( pTotalHead->QueueLockList );	// ��������
	dprintf( "**** End of walking MappedPE Lists **** \n\n" );

#endif
	return ;
}



ULONG 
GetRealAddr (
	IN ULONG addr,
	IN PVOID _pMappedInfo
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/19 [19:5:2010 - 14:24]

Routine Description:
  ����DLL���Լ�MAP���ڴ��,����Ҫ�ض�λ���е���ʵ��ַ,�����ҵõ���EAT�������ַaddr,
  ��ôҪ�������·�ʽ����,���ܶ�λ��������ôMAP���ڴ�ռ��еĵ�ַ:

  1. ��λ��EAT���ĸ��ڱ���(eg: ish)
  2. ����õ�����������MAP�ڴ����ݵĵ�ַ

  offset = addr - pish->VirtualAddress ; // �õ���Ϊ���ƫ��,���Ӵ���ӳ�䵽�ڴ�����Ŀ�϶,Ҳ��ƫ��
  addr_in_local = MappedAddr + pish->PointerToRawData // ��λ��ǰ���ڴ����ϵ�λ��
  RealAddr = addr_in_local + offset ;   // �õ���ǰ��ַ(addr)�����map�����е�λ��

Arguments:
  addr - ������������ַ

Return Value:
  ���ؼ���õ�����ʵ��ַ
    
--*/
{
	ULONG ret = 0, nIndex = 0 ;
	ULONG NumberOfSections = 0, VirtualAddress = 0 ;
	PIMAGE_NT_HEADERS		pinh = NULL ;
	PIMAGE_SECTION_HEADER	pish = NULL ;
	LPMPNODE pMappedInfo = (LPMPNODE) _pMappedInfo ;

	//
	// 1. У������Ϸ���
	//
	
	if ( 0 == addr || NULL == _pMappedInfo || NULL == pMappedInfo->pinh )
	{
		dprintf( "error! | GetRealAddr(); | Invalid Paramaters; failed! \n" );
		return 0 ;
	}

	//
	// 2. ���¼��㵱ǰ��ַ
	//

	pinh = pMappedInfo->pinh ;
	pish = (PIMAGE_SECTION_HEADER)( (PCHAR)&pinh->OptionalHeader + pinh->FileHeader.SizeOfOptionalHeader );
	
	NumberOfSections = pinh->FileHeader.NumberOfSections;
	if ( 0 == NumberOfSections )
	{
		dprintf( "error! | GetRealAddr(); | 0 == NumberOfSections \n" );
		return 0 ;
	}
	
	while ( TRUE )
	{
		VirtualAddress = pish->VirtualAddress;
		if ( (addr >= VirtualAddress) && (addr < VirtualAddress + pish->SizeOfRawData) )
		{
			ret = (pMappedInfo->MappedAddr + pish->PointerToRawData) + (addr - pish->VirtualAddress) ;
			break ;
		}

		++ pish ;
		++ nIndex ;
		
		if ( nIndex >= NumberOfSections ) 
		{ 
			ret = 0 ;
			break ; 
		}
	}

	return ret;
}



BOOL
IsValidPE (
	ULONG PEAddr
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/20 [20:5:2010 - 10:31]

Routine Description:
  У��PE�ļ��ĺϷ���
    
Arguments:
  PEAddr - PE����ʼ�ڴ��ַ

Return Value:
  BOOL
    
--*/
{
	PIMAGE_NT_HEADERS32	pinh			 = NULL	; 
	PIMAGE_DOS_HEADER	ImageBaseAddress = (PIMAGE_DOS_HEADER) PEAddr ;

	//
	// 1. У������Ϸ���
	//
	
	if ( 0 == PEAddr )
	{
		dprintf( "error! | IsValidPE(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	ProbeForRead( (LPVOID)ImageBaseAddress, 0x40, 1 );

	//  
	// 2. У��PE�Ϸ���
	//  

	if ( ('MZ' != ImageBaseAddress->e_magic) && ('ZM' != ImageBaseAddress->e_magic) )
	{
		dprintf( "error! | IsValidPE(); | Invalid PE; ImageBaseAddress->e_magic; \n" );
		return FALSE ;
	}

	pinh = (PIMAGE_NT_HEADERS)( (PCHAR)ImageBaseAddress + ImageBaseAddress->e_lfanew );
	ProbeForRead( pinh, 0x108, 1 );

	if ( 'EP' != pinh->Signature )
	{
		dprintf( "error! | IsValidPE(); | Invalid PE; pinh->Signature; \n" );
		return FALSE ;
	}

	return TRUE ;
}


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+               ��SSDT & Shadow SSDT��غ���                +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


VOID
SDTWalkNodes (
	IN int Index
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/17 [17:5:2010 - 17:13]

Routine Description:
  ��ӡ���нڵ���Ϣ

Arguments:
  Index - 0Ϊ��ӡȫ��; 1Ϊ��ӡSSDT����; 2Ϊ��ӡShadow SSDT����
    
--*/
{
	int		i	= 0	 ;
	int		TotalCounts = 0 ;
	LPSSDT_SSSDT_FUNC pNode = NULL ;
	CHAR    LogInfo[ MAX_PATH ] = "" ;

#ifdef DBG

	if ( Index < 0 || Index > 2 )
	{
		dprintf( "error! | SDTWalkNodes() | Invalid Paramaters; (Index=%d) \n", Index );
		return ;
	}

	TotalCounts = g_SdtData.TotalCounts ;

	if ( _SDTWalkNodes_All_ == Index ) 
	{
		strcpy( LogInfo, "Total SDT Array" );
	} 
	else if ( _SDTWalkNodes_SSDT_ == Index ) 
	{
		strcpy( LogInfo, "SSDT Array" );
		TotalCounts = g_SdtData.ShadowArrayIndex ;
	} 
	else if ( _SDTWalkNodes_Shadow_ == Index )
	{
		strcpy( LogInfo, "Shadow SSDT Array" );
		i = g_SdtData.ShadowArrayIndex ; 
	}

	dprintf( "\n**** Starting walking %s **** \n", LogInfo );

	for( i; i<TotalCounts; i++ )
	{
		pNode = (LPSSDT_SSSDT_FUNC) &g_ssdt_sssdt_Array[ i ] ;

		if ( NULL == pNode ) { continue ; }

		dprintf (
			"g_ssdt_sssdt_Array[%d]:			\n"
			"struct _SSDT_SSSDT_FUNC_ * 0x%08lx \n"
			" +0x000 wszModuleName : \"%ws\"	\n"
			" +0x004 szFunctionName : \"%s\"	\n"
			" +0x008 ArgumentNumbers : 0x%0x	\n"
			" +0x00C xxIndex : 0x%x				\n"
			" +0x010 MappedFuncAddr : 0x%08lx	\n"
			" +0x014 RealFuncAddr : 0x%08lx		\n"
			" +0x018 Tag : 0x%x					\n\n",
			i, (PVOID)pNode, 
			pNode->wszModuleName, pNode->szFunctionName, pNode->ArgumentNumbers,
			pNode->_IndexU_.xxIndex, pNode->MappedFuncAddr, pNode->RealFuncAddr, pNode->Tag
			);
	}

	dprintf( "**** End of walking %s  **** \n\n", LogInfo );

#endif
	return ;
}



BOOL
Get_sdt_function_addr (
	OUT PVOID _pNode,
	IN int AddressFindMethod,
	IN int IndexCheckMethod
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/20 [20:5:2010 - 15:13]

Routine Description:
  �õ�ָ����(Shadow)SSDT������ַ;    
    
Arguments:
  AddressFindMethod - �õ�������ַ�ķ���, һ��SSDT�ĵ�ַ�Ǳ�������,������2; Shadow SSDT�ĵ�ַ������ͨ��ʽ����,������1
  IndexCheckMethod - �õ�������Ӧ��Index��,���м��; ��Ϊ����NtRequestPort�Ⱥ�����ͨ��@AddressFindMethod == _IndexCheckMethod_Shadow_
					 ��ʽ�õ�,�������Ӧ��Index��ssdt��Χ��,��У�鷽ʽ@IndexCheckMethod == _IndexCheckMethod_SSDT_

Return Value:
  BOOL
    
--*/
{
	ULONG RealFuncAddr = 0, xxIndex = -1 ;
	LPSSDT_SSSDT_FUNC pNode = (LPSSDT_SSSDT_FUNC) _pNode ;
	CHAR Log[ 100 ] = "CrossFire" ;

	//
	// 1. У������Ϸ���
	//

	if ( NULL == pNode || FALSE == IS_INVALID_METHOD(AddressFindMethod) || FALSE == IS_INVALID_METHOD(IndexCheckMethod) )
	{
		dprintf( "error! | Get_sdt_function_addr() | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	pNode->RealFuncAddr = 0 ;

	//
	// 2.��ȡ������
	//

	pNode->_IndexU_.xxIndex = xxIndex = GetSDTIndexEx( pNode->MappedFuncAddr ); // ���õ���SDT����������ýṹ��
	if ( -1 == xxIndex || xxIndex >= 0x2000 || (xxIndex & 0xFFF) >= 0x600 )
	{
		dprintf( "error! | Get_sdt_function_addr() | ����ȡ�������Ų��Ϸ�. Index: %d; FuncName: \"%s\" \n", xxIndex, pNode->szFunctionName );
		return FALSE ;
	}

	switch ( IndexCheckMethod )
	{
	case _IndexCheckMethod_SSDT_ :	// ��֤ssdt Index�Ϸ���
		break ;

	case _IndexCheckMethod_Shadow_ : // ��֤shadow ssdt Index�Ϸ���
		{
			if ( 0x1000 != (xxIndex & 0xF000) )
			{
				dprintf( "error! | Get_sdt_function_addr() | shadow ssdt �����Ų��Ϸ�. Index: %d; FuncName: \"%s\" \n", xxIndex, pNode->szFunctionName );
				return FALSE ;
			}
		}
		break ;
	
	default :
		break ;
	}
	
	//
	// 3. ����������,��ȡ������ַ
	//
	
	if ( _AddressFindMethod_Shadow_ == AddressFindMethod )
	{
		// Shadow SSDT�ĵ�ַ������ͨ��ʽ����
		RealFuncAddr = Get_sdt_function_addr_normal( xxIndex, pNode->ArgumentNumbers );
#if DBG
		strcpy( Log, "Get_sdt_function_addr_normal();" );
#endif
	}
	else if ( _AddressFindMethod_SSDT_ == AddressFindMethod )
	{
		// SSDT�ĵ�ַ�Ǳ�������
		RealFuncAddr = Get_sdt_function_addr_force( xxIndex );
#if DBG
		strcpy( Log, "Get_sdt_function_addr_force();" );
#endif	
	}
	
	if ( 0 == RealFuncAddr )
	{
		dprintf( 
			"error! | Get_sdt_function_addr() - %s | "
			"FuncName: \"%s\"; ������: 0x%x; ArgumentNumbers: 0x%x \n",
			Log,
			pNode->szFunctionName,
			pNode->_IndexU_.xxIndex,
			pNode->ArgumentNumbers
			);

		return FALSE ;
	}

	pNode->RealFuncAddr = RealFuncAddr ;

	SDTTrace (
		"ok! | Get_sdt_function_addr(); | ��ǰ�ڵ���Ϣ����: \n"
		"struct _SSDT_SSSDT_FUNC_ * 0x%08lx \n"
		" +0x000 wszModuleName : \"%ws\"	\n"
		" +0x004 szFunctionName : \"%s\"	\n"
		" +0x008 ArgumentNumbers : 0x%0x	\n"
		" +0x00C xxIndex : 0x%x				\n"
		" +0x010 MappedFuncAddr : 0x%08lx	\n"
		" +0x014 RealFuncAddr : 0x%08lx		\n"
		" +0x018 Tag : 0x%x					\n\n",
		(PVOID)pNode, pNode->wszModuleName, pNode->szFunctionName, pNode->ArgumentNumbers,
		pNode->_IndexU_.xxIndex, pNode->MappedFuncAddr, pNode->RealFuncAddr, pNode->Tag
		);

	return TRUE ;
}



ULONG
Get_sdt_function_addr_normal (
	IN ULONG Index,
	IN ULONG ArgumentNumbers
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/20 [20:5:2010 - 15:13]

Routine Description:
  �ȵõ�nt!KeServiceDescriptorTableShadow�ĵ�ַ,�������Indexȡ��Nt*������ַ

  0: kd> dd nt!KeServiceDescriptorTable
  8055c700  80505428 00000000 0000011c 8050589c
  8055c710  00000000 00000000 00000000 00000000
  8055c720  00000000 00000000 00000000 00000000
  8055c730  00000000 00000000 00000000 00000000
  8055c740  00000002 00002710 bf80c575 00000000
  8055c750  867596ec b9f0c4a0 83d9cbb4 806f5040
  8055c760  008583b0 00000000 008583b0 00000000
  8055c770  914f4a70 01ca5057 00000000 00000000
  0: kd> db 8050589c  // ���ǲ���������; �����һ����0x18,�������Ϊ0�ĺ�����0x18 / 4 ������,�Դ�����
  8050589c  18 20 2c 2c 40 2c 40 44-0c 08 18 18 08 04 04 0c  . ,,@,@D........
  805058ac  10 18 08 08 0c 04 08 08-04 04 0c 08 0c 04 04 20  ............... 

  0: kd> dd 80505428 l2 // ����SSDT��ͷ2������,��֤һ��
  80505428  805a44bc 805f07f0
  0: kd> ln 805a44bc 
  (805a44bc)   nt!NtAcceptConnectPort   |  (805a4baa)   nt!NtCompleteConnectPort
  Exact matches:
  nt!NtAcceptConnectPort = <no type information>
  0: kd> u ntdll!NtAcceptConnectPort
  ntdll!ZwAcceptConnectPort:
  7c92ce5e b800000000      mov     eax,0
  7c92ce63 ba0003fe7f      mov     edx,offset SharedUserData!SystemCallStub (7ffe0300)
  7c92ce68 ff12            call    dword ptr [edx]
  7c92ce6a c21800          ret     18h  // ****************************************

  0: kd> ln 805f07f0
  (805f07f0)   nt!NtAccessCheck   |  (805f0822)   nt!NtAccessCheckByType
  Exact matches:
  nt!NtAccessCheck = <no type information>
  0: kd> u ntdll!NtAccessCheck
  ntdll!ZwAccessCheck:
  7c92ce6e b801000000      mov     eax,1
  7c92ce73 ba0003fe7f      mov     edx,offset SharedUserData!SystemCallStub (7ffe0300)
  7c92ce78 ff12            call    dword ptr [edx]
  7c92ce7a c22000          ret     20h  // ****************************************

Arguments:
  Index - ��ǰ������Ӧ�����
  ArgumentNumbers - ��ǰ�����Ĳ�������

Return Value:
  Nt*������ַ
    
--*/
{
	ULONG __Index = Index ;
	ULONG Table, KiArgumentTable ;
	ULONG SDTAddr = 0 ;
	
	//
	// 1. У������Ϸ���
	//
	
	if ( -1 == Index )
	{
		dprintf( "error! |  | Invalid Paramaters; failed! \n" );
		return 0 ;
	}

	//
	// 2. ��ȡϵͳΨһһ��SDT��ĵ�ַ (���а���4��SST��,ǰ2����ssdt & shadow ssdt,��2�ű���δʹ��)
	//

	SDTAddr = (ULONG) Get_KeServiceDescriptorTable_addr() ;
	if ( 0 == SDTAddr )
	{
		dprintf( "error! | Get_sdt_function_addr_normal() - Get_KeServiceDescriptorTable_addr(); | �޷���ȡSDTAddr�ĵ�ַ \n" );
		return 0 ;
	}

	//
	// 3. ��֤�����ŵĺϷ���
	//

	if ( IS_INVALID_INDEX( Index ) )
	{
		dprintf( "error! | Get_sdt_function_addr_normal() | �����Ų��Ϸ� \n" );
		return 0 ;
	}
	
	//
	// 4. ���ֳ� ssdt & shadow ssdt
	//

	if ( Index & 0x1000 ) 
	{
		// shadow ssdt
		Table			= *(DWORD *) ( SDTAddr + 0x10 );
		KiArgumentTable = *(DWORD *) ( SDTAddr + 0x1C );
		__Index = Index & 0xFFFFEFFF ;
	}
	else
	{
		// ssdt
		Table			= *(DWORD *) SDTAddr;
		KiArgumentTable = *(DWORD *) ( SDTAddr + 0xC );
	}

	if ( FALSE == MmIsAddressValid( (PVOID)Table ) || FALSE == MmIsAddressValid( (PVOID)KiArgumentTable ) )
	{
		dprintf( "error! | Get_sdt_function_addr_normal() | FALSE == MmIsAddressValid \n" );
		return 0 ;
	}

	//
	// ���������Ż�ȡ������ַ
	//

	if ( 4 * ArgumentNumbers == *(BYTE *)( KiArgumentTable + __Index ) )
		return *(ULONG *)(Table + 4 * __Index);

	return 0;
}



ULONG
Get_sdt_function_addr_force (
	IN ULONG Index
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/20 [20:5:2010 - 17:31]

Routine Description:
  ���ñ�����������ʽ,�õ�ָ����SSDT������ַ;

Arguments:
  Index - [IN] Ҫ���ҵ�Zw*���������к� 

Return Value:
  Zw*������ַ
    
--*/
{
	ULONG Addr = 0 ;

	if ( (char *)ZwAccessCheckAndAuditAlarm == (char *) ZwYieldExecution )
	{
		Addr = Get_sdt_function_addr_force_step2( Index );
	}
	else
	{
		Addr = Get_sdt_function_addr_force_step1( Index );
	}

	return Addr ;
}



ULONG
Get_sdt_function_addr_force_step1 (
	IN ULONG Index
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/20 [20:5:2010 - 17:31]

Routine Description:
  ���ñ��������ķ�ʽ,��SSDT�Ŀ�ʼ����ZwAccessCheckAndAuditAlarm �� ��������ZwYieldExecution
  ֮����б�������,ƥ�����Ӧ��Index,�����亯����ַ
    
Arguments:
  Index - Ҫ���ҵ�Zw*���������к� 

Return Value:
  Zw*������ַ
    
--*/
{
	PCHAR ptr, ptr_end ;
	int n = 0 ;

	ptr = (PCHAR) ZwAccessCheckAndAuditAlarm ;
	ptr_end = (PCHAR) ZwYieldExecution ;

	while ( 0xB8 == *(BYTE *) ptr )
	{
		if ( Index == *(ULONG *)(ptr + 1) ) { return (ULONG)ptr ; }

		ptr += 5 ;
		if ( 0x8D != *(BYTE *)ptr ) { return 0 ; }

		ptr += 4 ;

		if ( 0xCD == *(BYTE *)ptr ) {
			ptr += 2 ;
		} else if ( 0x9C == *(BYTE *)ptr ) {
			ptr += 8 ;
		} else {
			return 0 ;
		}

		n = 8;
		do
		{
			switch ( *(BYTE *)ptr )
			{
			case 0xC2:
				ptr += 3 ;
				break;

			case 0x90:
			case 0xC3:
				++ptr ;
				break;

			case 0x8B:
				ptr += 2 ;
				break;

			default:
				break;
			}

			-- n ;
		}
		while ( n );

		if ( ptr == ptr_end ) { return 0 ; }
	} 

	return 0 ;
}



ULONG
Get_sdt_function_addr_force_step2 (
	IN ULONG Index
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/20 [20:5:2010 - 17:31]

Routine Description:
  ���ü��ȱ�����������ʽ,��wcstombs+0x4000~0x6000֮�����Ӳ����ƥ��,ֱ���õ���һ��Zw*�����ĵ�ַ,
  �ٽ���Indexƥ��,���յõ�ָ����SSDT������ַ; �˷�����г,����������ʽ��ʧ�ܵ������ʹ�õ�!

Arguments:
  Index - Ҫ���ҵ�Zw*���������к� 

Return Value:
  Zw*������ַ
    
--*/
{
	int n = 0 ;
	PCHAR ptr, ptr_end ;
	CHAR pHardCode[ 8 ] = { 0x8D, 0x54, 0x24, 0x4, 0x9C, 0x6A, 0x8, 0xE8 };
	/*
	8D 54 24 04      lea     edx, [esp+4]
	9C               pushf
	6A 08            push    8
	E8 00 00 00 00   call    $+5
	*/

	ptr		= (PCHAR)( (PCHAR)wcstombs + 0x4000 );
	ptr_end = (PCHAR)( (PCHAR)wcstombs + 0x6000 );

	while ( TRUE )
	{
		if ( (0xB8 == *(BYTE *)ptr) && (8 == RtlCompareMemory( (PVOID)(ptr + 5), pHardCode, 8 )) )
		{
			break ;
		}

		++ ptr ;

		if ( ptr >= ptr_end )
		{
			dprintf( "error! | Get_sdt_function_addr_force_step2(); | RtlCompareMemoryƥ�䲻��ָ������� \n" );
			return 0 ;
		}
	}

	while ( 0xB8 == *(BYTE *)ptr )
	{
		if ( Index == *(ULONG *)(ptr + 1) ) { return (ULONG)ptr ; }
		
		ptr += 0x11 ;
		n = 0 ;
		while ( (n < 6) && (0xB8 != *(BYTE *)ptr) )
		{
			++ n ;
			++ ptr ;
		}
	}

	return 0 ;
}



BOOL
Get_sdtfunc_addr (
	IN PULONG addr,
	IN ULONG Tag
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/02 [2:6:2010 - 11:05]

Routine Description:
  ��ʵZw* & NtUser* ϵ�еĺ�����ַ�Ѿ���������ʼ��ʱ��ȡ������ȫ��������,�ú��������������в���ָ���ĺ�����ַ    

Arguments:
  Name - ������; eg:ZwCreateToken

--*/
{
	if ( *addr ) { return TRUE ; }
	*addr = (ULONG) Get_sdtfunc_addr_ex( Tag );

	if ( *addr ) { return TRUE ; }
	return FALSE ;
}



ULONG
Get_sdtfunc_addr_ex (
	IN int Tag
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/24 [24:5:2010 - 17:14]

Routine Description:
  ����@Tag����(Shadow) SSDT��ָ���ĺ�����ַ
    
Arguments:
  Tag - ������Ӧ��Tag,���ڲ��������е�ָ������

Return Value:
  ������ַ | 0
    
--*/
{
	ULONG addr = 0 ;
	LPSSDT_SSSDT_FUNC pNode = NULL ;

	pNode = (LPSSDT_SSSDT_FUNC) Get_sdt_Array( Tag );
	if ( NULL == pNode ) 
	{
		dprintf( "error! | Get_sdtfunc_addr_ex() - Get_sdt_Array(); |  \n" );
		return 0 ;
	}

	addr = pNode->RealFuncAddr ;
	if ( 0 == addr || 0x80000000 > addr )
	{
		dprintf( "error! | Get_sdtfunc_addr_ex(); | ���Ϸ�������ַaddr=0x%08lx \n", (PVOID)addr );
		return 0 ;
	}

	return addr ;
}



PVOID
Get_sdt_Array (
	IN int Tag
	)
{
	int Index = -1 ;

	//
	// 1. У������Ϸ���
	//

	if ( FALSE == g_SdtData.bInited ) 
	{
		dprintf( "error! | Get_sdt_Array(); | g_SdtDataȫ�ֱ�����δ��ʼ��,��������ʱ������,���޷�����Tag=%d�ĺ�����ַ \n", Tag );
		return NULL ;
	}

	//
	// 2. ȡ��������ָ����Ԫ�ĺ�����ַ
	//

	if ( IS_SSDT_TAG( Tag ) )
	{
		Index = Tag - 1 ; // �õ�SSDT �����������е����
	}
	else if ( IS_ShadowSSDT_TAG( Tag ) )
	{
		Index = g_SdtData.ShadowArrayIndex + ( Tag - (ShadowSSDT_TAG_LowerLimit + 1) ) ; // �õ�Shadow SSDT �����������е����
	}
	else
	{
		dprintf( "error! | Get_sdt_Array(); | ���Ϸ���Tag=%d \n", Tag );
		return NULL ;
	}

	if ( -1 == Index ) { return NULL ; }
	return (PVOID) &g_ssdt_sssdt_Array[ Index ] ;
}



PVOID
Get_sdt_Array_spec (
	IN int Tag
	)
{
	if ( Tag < 0 || Tag > 2 ) { return NULL ; }

	if ( FALSE == g_SdtData.bInited ) 
	{
		dprintf( "error! | Get_sdt_Array_spec(); | g_SdtDataȫ�ֱ�����δ��ʼ��,��������ʱ������,���޷�����Tag=%d�ĺ�����ַ \n", Tag );
		return NULL ;
	}

	return (PVOID) &g_ssdt_Array_special [ Tag ] ;
}


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                 ��λ��ȡ sdt��ĵ�ַ                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


ULONG g_SDT_addr = 0 ;

ULONG
Get_KeServiceDescriptorTable_addr (
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/20 [20:5:2010 - 17:31]

Routine Description:
  �õ�SDT��ĵ�ַ, ���а�����4��SST, ������ֻ��ǰ2��SST��Ч,��2������

  ϵͳ��SDT�� nt!KeServiceDescriptorTable

  +----------+
  |	 SST 1	 | // ntoskrnl,��SSDT��
  |__________| 
  |	 SST 2	 | // win32k, ��Shadow SSDT��
  |__________|
  |	 SST 3	 | 
  |__________| // ��2�ű�δʹ��
  |	 SST 4	 |
  |__________|
    
--*/
{
	ULONG ServiceTable = 0, tmp2 = 0 ;

	ServiceTable = (ULONG) KeServiceDescriptorTable.ServiceTable ;
	if ( 0 == ServiceTable ) { return 0 ; }

	if ( TRUE == g_Version_Info.IS___win2k )
	{
		g_SDT_addr = (ULONG)&KeServiceDescriptorTable + 0xE0 ;
		if ( ServiceTable == *(PULONG) g_SDT_addr ) { return g_SDT_addr ; }

		g_SDT_addr = (ULONG)&KeServiceDescriptorTable + 0x140 ;
		if ( ServiceTable == *(PULONG) g_SDT_addr ) { return g_SDT_addr ; }

		g_SDT_addr = (ULONG)&KeServiceDescriptorTable + 0x40 ;
		if ( ServiceTable == *(PULONG) g_SDT_addr ) { return g_SDT_addr ; }
	}

	else if ( TRUE == g_Version_Info.IS___xp || TRUE == g_Version_Info.IS___win2003 )
	{
		g_SDT_addr = (ULONG)&KeServiceDescriptorTable - 0x40 ;
		if ( ServiceTable == *(PULONG) g_SDT_addr ) { return g_SDT_addr ; }

		if ( TRUE == g_Version_Info.IS___win2003 )
		{
			g_SDT_addr = (ULONG)&KeServiceDescriptorTable - 0x20 ;
			if ( ServiceTable == *(PULONG) g_SDT_addr ) { return g_SDT_addr ; }
		}
	}

	else if ( TRUE == g_Version_Info.IS___vista || TRUE == g_Version_Info.IS___win7 )
	{
		g_SDT_addr = (ULONG)&KeServiceDescriptorTable + 0x40 ;
		if ( ServiceTable == *(PULONG) g_SDT_addr ) { return g_SDT_addr ; }
	}

	return 0 ;
}



ULONG
GetSDTIndex (
	IN ULONG pMappedAddr,
	IN BOOL bIsShadow
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/20 [20:5:2010 - 20:53]

Routine Description:
  �õ�Zw*������Ӧ��Index    
    
Arguments:
  pMappedAddr - ӳ���PEģ���ڴ��ַ
    Zw*�����ĵ�ַ,Ϊ���Լ�map���ڴ��,��ַһ���С.eg:
    kd> uf 0016cbfc 
    0016cbfc b837000000      mov     eax,37h
    0016cc01 ba0003fe7f      mov     edx,offset SharedUserData!SystemCallStub (7ffe0300)
    0016cc06 ff12            call    dword ptr [edx]
    0016cc08 c23400          ret     34h

	����shadow�ຯ��,����:
	0:006> uf USER32!IsHungAppWindow
	USER32!IsHungAppWindow:
	76c170d3 8bff            mov     edi,edi
	76c170d5 55              push    ebp
	76c170d6 8bec            mov     ebp,esp
	76c170d8 6a05            push    5
	76c170da ff7508          push    dword ptr [ebp+8]
	76c170dd e8df06feff      call    USER32!NtUserQueryWindow (76bf77c1)
	76c170e2 f7d8            neg     eax
	76c170e4 1bc0            sbb     eax,eax
	76c170e6 f7d8            neg     eax
	76c170e8 5d              pop     ebp
	76c170e9 c20400          ret     4
	0:006> u USER32!NtUserQueryWindow
	USER32!NtUserQueryWindow:
	76bf77c1 b803120000      mov     eax,1203h
	76bf77c6 ba0003fe7f      mov     edx,offset SharedUserData!SystemCallStub (7ffe0300)
	76bf77cb ff12            call    dword ptr [edx]
	76bf77cd c20800          ret     8

Return Value:
  Zw*������Ӧ��������
    
--*/
{
	ULONG Index = -1 ;
	PCHAR pCode = (PCHAR) pMappedAddr ;

	//
	// 1. У������Ϸ���
	//
	
	if ( 0 == pMappedAddr )
	{
		dprintf( "error! | GetSDTIndex(); | Invalid Paramaters; failed! \n" );
		return -1 ;
	}

	//
	// 2. ƥ��֮; shadow������������Щ�������õ���ntuser*,�����������ֱ��ȡ��Index 
	//

	if ( bIsShadow && ( 0xB8 != *(BYTE*) pCode ) )
	{
		ULONG pAddr = 0, Len = 0;
		PUCHAR opcode = NULL ;

		for( pAddr = (ULONG)pMappedAddr; pAddr<(ULONG)pMappedAddr + 0x60; pAddr += Len )
		{
			Len = SizeOfCode( (PUCHAR)pAddr, &opcode ) ;
			if( !Len ) { Len++; continue ; }

			if ( 0xE8 == *(PUCHAR)pAddr ) 
			{
				pCode = (PCHAR)( (ULONG)opcode + *(PULONG)(opcode+1) + 5 ) ;
				break ;
			}
		}
	}

	if ( 0xB8 != *(BYTE*) pCode )
	{
		dprintf( "error! | GetSDTIndex(); | 0xb8 != *(BYTE*) pCode;  (*(BYTE*) pCode = 0x%x) \n", *(BYTE*) pCode );
		return -1 ;
	}

	Index = (ULONG) *(PUSHORT) (pCode + 1);
	if ( IS_INVALID_INDEX( Index ) ) { return -1 ; }

	return Index ;
}




int
GetSDTIndexEx (
	IN ULONG pMappedAddr
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/09 [9:8:2010 - 17:14]

Routine Description:
  �õ�ָ��sdt������Ӧ��Index    
    
Arguments:
  pMappedAddr - ӳ��ĺ�����ַ

Return Value:
  �õ���Index ���� -1
    
--*/
{
	int		Index	= 0		 ;
	ULONG	size	= 0x400 ;
	PVOID	AddressArray = NULL	 ;

	// 1. У������Ϸ���
	if ( 0 == pMappedAddr )
	{
		dprintf( "error! | GetSDTIndexEx(); | Invalid Paramaters; failed! \n" );
		return -1 ;
	}

	// 2. ����һ����ڴ�,�����ѱ������ĺ���
	AddressArray = (PVOID) kmalloc( size );
	if ( NULL == AddressArray )
	{
		dprintf( "error! | GetSDTIndexEx() - kmalloc(); | �����ڴ�ʧ��. \n" );
		return -1 ;
	}

	RtlZeroMemory( AddressArray, size );

	// 3. ��ȡIndex
	Index = GetSDTIndexExp( pMappedAddr, (ULONG)AddressArray, 1, -1 );
	kfree( AddressArray );

	return Index ;
}



int
GetSDTIndexExp (
	IN ULONG pMappedAddr,
	IN ULONG AddressArray,
	IN ULONG DeepCounts,
	IN int CurrentResult
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/09 [9:8:2010 - 17:14]

Routine Description:
  �ݹ麯��,ͨ��������������ָ��,���յõ�sdt������Ӧ��Index    
    
Arguments:
  pMappedAddr - ӳ��ĺ�����ַ
  AddressArray - �������ѱ�ӳ��ĺ�����ַ,��ֹ�����ظ�����
  DeepCounts - ��ǰ�ĵݹ����,���ܳ�����ֵ
  CurrentResult - ��ǰ�ѵõ�������ָ������( -1 ���� Index)

Return Value:
  -1 ���� sdt������Ӧ��Index 
    
--*/
{
	int IndexKey = 0 ;
	DWORD len = 0, dwJumpTo = 0, flag = 0 ;
	PBYTE p = NULL ;
	BOOL bNeedFollowup = FALSE, bNormalJMP = FALSE ;
	struct	xde_instr code_instr = { 0 };

	//
	// 1. У������Ϸ���
	//

	if ( 0 == pMappedAddr || 0 == AddressArray )
	{
		dprintf( "error! | GetSDTIndexEx(); | Invalid Paramaters; failed! \n" );
		return -1 ;
	}

	if ( DeepCounts > 10 )
	{
	//	dprintf( "error! | GetSDTIndexEx(); | �ݹ����������ֵ\n" );
		return -1 ;
	}

	//
	// 2. ����Ƿ��Ǳ�������ӳ�亯����ַ,�����ظ�,���Ч��
	//

	if ( ((DWORD)pMappedAddr & 0xF0000000) < 0x70000000 )
	{
		int n = 0 ;
		int nTotalCounts = *(DWORD*) AddressArray ;

		if( nTotalCounts )
		{
			do 
			{
				if ( (DWORD)pMappedAddr == *(DWORD *)(AddressArray + 4 * n) )
				{
					// ���ѱ������ĵ�ַ,�����ظ�����
					return -1 ;
				}

				++ n ;
			} while ( n < nTotalCounts );
		}

		++ n ;
		if ( n > 0xFF8 ) { return -1 ; }

		*(DWORD *) AddressArray = n ;
		*(DWORD *)( AddressArray + 4 * n ) = (DWORD)pMappedAddr ;
	}

	//
	// 3. ����ָ��������
	//

	for( p = (PBYTE)pMappedAddr; ; p += len )
	{
		len = xde_disasm( p, &code_instr );
		if( 0 == len ) 
		{
			p ++ ;
			continue ;
		}

		// 3.1 �����������˳�
		if ( IsFunctionEnd( p ) ) { break ; }

		// 3.2 ���ݹ鵽KiFastSystemCall����,�������⴦��.��ʱ�ѵõ���ָ������@CurrentResult������Index.
		if ( IsKiFastSystemCall( p ) )
		{
			if ( -1 != CurrentResult ) { return CurrentResult ; }
		}

		// 3.3 ���ڸ�����תָ��Ĵ���
		bNeedFollowup = TRUE ;
		flag = code_instr.flag;

		if ( 0xE8 == code_instr.opcode ) // 0xE8 call����
		{
			dwJumpTo = (DWORD)p + *(DWORD*)(p+1) + 5 ;
		}
		else if ( 0xFF == *p && 0x15 == *(p+1) ) // 0xFF15 call����
		{ 
			dwJumpTo = *( DWORD* )(*( DWORD* )( p + 2 ));
		}
		else if ( 0xFF == *p && 0x14 == *(p+1) ) // 0xFF14 call ����[Index]
		{
			dwJumpTo = *(DWORD*)(p + 3) ;	
			if ( (dwJumpTo & 0xF0000000) < 0x70000000 ) { return -1 ; }

			ProbeForRead( (PVOID)dwJumpTo, 4, 4 );
			dwJumpTo = *( DWORD* ) dwJumpTo ;
		}
		else if ( 0xFF == *p && 0xD2 == *(p+1) ) // 0xFFD2 ����, �������ʲôָ��
		{
			dwJumpTo = (DWORD)(p + 2) ;	
		}
		else if ( 0xFF == *p && 0x12 == *(p+1) &&  0xba == *(p-5) ) // 0xFF12���� call [�Ĵ���]
		{
			dwJumpTo = *( DWORD* )( p - 4 );
			if ( (dwJumpTo & 0xF0000000) < 0x70000000 ) { return -1 ; }

			ProbeForRead( (PVOID)dwJumpTo, 4, 4 );
			dwJumpTo = *( DWORD* ) dwJumpTo ;
		}
		else if ( (flag & C_STOP) && ( (0xE9 == *p) || (0xEB == *p) ) ) // ��ͨ������ת; eg: 0xE9 / 0xEB
		{
			bNormalJMP = TRUE ;

			if( flag & C_DATA1 ) {
				dwJumpTo = (DWORD)p + len + code_instr.data_c[0] ;
			} else if( flag & C_DATA2 ) {
				dwJumpTo = (DWORD)p + len + code_instr.data_s[0] ;
			} else {
				dwJumpTo = (DWORD)p + len + code_instr.data_l[0] ;
			}
		}
		else
		{
			bNeedFollowup = FALSE ;
		}

		// 3.5
		if ( bNeedFollowup )
		{
			IndexKey = GetSDTIndexExp( dwJumpTo, AddressArray , DeepCounts + 1, CurrentResult );
			if ( -1 != IndexKey ) { return IndexKey ; }

			CurrentResult = -1 ;
			if ( bNormalJMP ) { return -1 ; }
		}

		if ( 0xB8 == *p )
		{
			CurrentResult =  (int) *(PUSHORT) (p + 1);
		}
	}

	return -1 ;
}



BOOL 
IsFunctionEnd (
	PBYTE CurrentEIP
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/12 [12:8:2010 - 18:16]

Routine Description:
  �жϺ����Ƿ����   
    
Arguments:
  CurrentEIP - ��ǰָ���ַ

--*/
{
	PBYTE p = CurrentEIP ;

	if( FALSE == MmIsAddressValid( (PVOID)CurrentEIP ) )
	{
		dprintf( "error! | IsFunctionEnd(); | Invalid Paramaters; failed! \n" );
		return TRUE ;
	}

	if( 0xc2 == *p || 0xc3 == *p ) 
	{		
		return TRUE ;
	}

	return FALSE ;
}



BOOL 
IsKiFastSystemCall (
	PBYTE CurrentEIP
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/11 [11:8:2010 - 11:28]

Routine Description:
  �жϵ�ǰָ���Ƿ���KiFastSystemCall 
  kd> u 7c92eb8b
  ntdll!KiFastSystemCall:
  7c92eb8b 8bd4     mov    edx,esp
  7c92eb8d 0f34     sysenter
    
Arguments:
  CurrentEIP - ��ǰָ���ַ 

--*/
{
	PBYTE p = CurrentEIP ;

	if( FALSE == MmIsAddressValid( (PVOID)CurrentEIP ) )
	{
		dprintf( "error! | IsKiFastSystemCall(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	if( 0xd48b == *(USHORT*)p && 0x340f == *(USHORT*)(p + 2) ) 
	{		
		return TRUE ;
	}

	return FALSE ;
}


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                     Inine Hook ����                       +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--



BOOL
PatchSDTFunc (
	IN PVOID _pNode,
	IN BOOL bHook
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/12 [12:8:2010 - 18:06]

Routine Description:
  ��ָ����sdt��������Inline(Hook/Unhook)   
    
Arguments:
  _pNode - sdt������Ӧ�Ľṹ���ַ
  bHook - TRUEΪ����InlineHook; FALSEΪ�ر�InlineHook

--*/
{
	BOOL bRet = TRUE ;
	PVOID OrignalAddr = NULL, fakeAddr = NULL ;
	LPSSDT_SSSDT_FUNC pNode = (LPSSDT_SSSDT_FUNC) _pNode ;

	// 1. У������Ϸ���
	if ( NULL == pNode || FALSE == MmIsAddressValid( (PVOID)pNode ) )
	{
		dprintf( "error! | PatchSDTFunc(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	OrignalAddr = (PVOID) pNode->RealFuncAddr ;
	fakeAddr	= (PVOID) pNode->_AddrU_.fakeFuncAddr ;

	// 2. Working
	if ( bHook )
	{
		// 2.1 ��ʼInline Hook, ��ͷ5�ֽ���ת
		if ( pNode->bHooked ) 
		{ 
			dprintf( "ko! | PatchSDTFunc() - Hook ; | ��ǰ����\"%s\"�ѱ�Hook�� \n", pNode->szFunctionName );
			return TRUE ;
		}

		if ( NULL == OrignalAddr || NULL == fakeAddr )
		{
			dprintf( "error! | PatchSDTFunc() - Hook ; | �ڵ���Ϣ������,ԭʼ����/���˺��� ��ַ���Ϸ� \n" );
			return FALSE ;
		}

		bRet = HookCode95( OrignalAddr, fakeAddr, pNode->HookType );
		if( FALSE == bRet ) 
		{
			dprintf( "error! | PatchSDTFunc() - HookCode95() - Hook ; | Hook \"%s\" Failed \n", pNode->szFunctionName );
			pNode->bHooked = FALSE ;	
			return FALSE ;
		}

		dprintf( "ok! | PatchSDTFunc() - HookCode95(); | Hook \"%s\" \n", pNode->szFunctionName );
		pNode->bHooked = TRUE ;	
	}
	else
	{
		// 2.2 UnHook
		KTIMER kTimer ;
		LARGE_INTEGER timeout ;
		timeout.QuadPart = 1000000 ; //.1 s
		
		if ( FALSE == pNode->bHooked ) { return TRUE ; }

		if ( NULL == fakeAddr )
		{
			dprintf( "error! | PatchSDTFunc() - UnHook ; | �ڵ���Ϣ������,���˺�����ַ���Ϸ�. 0x%08lx \n", pNode->_AddrU_.fakeFuncAddr );
			return FALSE ;
		}

		// ��ֹж��ʱfake �������ڱ��첽����,����ϵͳ�ҵ�
		KeInitializeTimer( &kTimer );

		while( pNode->FakeFuncInfo.RefCounts > 0 )
		{
			KeSetTimer( &kTimer, timeout, NULL );
			KeWaitForSingleObject( &kTimer, Executive, KernelMode, FALSE, NULL );
		}

		UnhookCode95( fakeAddr ) ;
		pNode->bHooked = FALSE ;
	}
	
	return TRUE ;
}


///////////////////////////////   END OF FILE   ///////////////////////////////