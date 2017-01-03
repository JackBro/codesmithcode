/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/05/26 [26:5:2010 - 17:16]
* MODULE : d:\Work\Program\Coding\ɳ��\SandBox\Code\Project\ProteinBoxDrv\ProcessData.c
* 
* Description:
*      
*   ÿ��"��ɳ��"�Ľ��̶���Ӧһ�����̽ڵ�,�ýڵ�����˸ý��̵�������Ϣ.
*   ��ģ�鸺�������Щ���ݽṹ
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "RegHiveData.h"
#include "ProcessData.h"
#include "Config.h"
#include "Memory.h"
#include "StartProcess.h"
#include "Security.h"
#include "SecurityData.h"
#include "GrayList.h"
#include "Common.h"
#include "ForceRunList.h"

//////////////////////////////////////////////////////////////////////////

LPPROCESS_NODE_INFO_HEAND g_ListHead__ProcessData = NULL ;
static BOOL g_ProcessDataManager_Inited_ok = FALSE ;


// ���׺е�·��,ǰ��Ӳ����,���ڶ�̬��ȡ
static WCHAR g_ProteinBox_path[ MAX_PATH ] = L"\\Device\\HarddiskVolume1\\Program Files\\Proteinboxie" ;
#define  g_ProteinBox_path_length sizeof( g_ProteinBox_path )

// ���׺��ض���Ŀ¼����ʾ��Ϣ��ر���
LPSTR g_Context_InfoTip = NULL ;
#define g_InfoTip_DESKTOP_icon "\\??\\C:\\Program Files\\Internet Explorer\\IEXPLORE.EXE"


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
InitProcessData (
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/01 [1:6:2010 - 18:36]

Routine Description:
  ��������ͷ    

--*/
{
	BOOL bRet = FALSE ;

	if ( FALSE == g_ProcessDataManager_Inited_ok )
	{
		bRet = PDCreateTotalHead( (PVOID) &g_ListHead__ProcessData );
		g_ProcessDataManager_Inited_ok = bRet ;

		if ( FALSE == bRet )
		{
			dprintf( "error! | InitProcessData(); \n" );
		}
	}

	return bRet ;
}



BOOL 
PDCreateTotalHead(
	OUT PVOID* _TotalHead
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/04/29 [29:4:2009 - 14:00]

Routine Description:
  �����ܽṹ��,��ʼ��֮    
    
Arguments:
  _TotalHead - Ҫ��ʼ�����ܽṹ��ָ��ĵ�ַ
    
--*/
{
	BOOL bRet = FALSE ;
	LPPDHEAD *pTotalHead = (LPPDHEAD*) _TotalHead ;
	if ( NULL !=  *pTotalHead ) { return TRUE ; }

	// Ϊ�ܽṹ������ڴ�
	*pTotalHead = (LPPDHEAD) kmalloc( sizeof( PDHEAD ) );
	if ( NULL == *pTotalHead )
	{
		dprintf( "error! | PDCreateTotalHead(); Allocate memory failed! \n" );
		return FALSE ;
	}

	RtlZeroMemory( *pTotalHead, sizeof( PDHEAD ) );

	// ��ʼ����Դ��
	bRet = InitResource( &((LPPDHEAD)*pTotalHead)->QueueLockList );
	if ( FALSE == bRet )
	{
		dprintf( "error! | PDCreateTotalHead() - InitResource(); ������Դ���ڴ�ʧ��! \n" );
		kfree( (PVOID) *pTotalHead );
		return FALSE ;
	}

	// ��ʼ������ͷ
	InitializeListHead( (PLIST_ENTRY)&( (LPPDHEAD)*pTotalHead )->ListHead );

	((LPPDHEAD)*pTotalHead)->nTotalCounts = 0 ;

	return TRUE ;
}



VOID 
PDDeleteTotalHead(
	IN PVOID* _TotalHead
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/04/29 [29:4:2009 - 14:00]

Routine Description:
  ɾ���ܽṹ��    

--*/
{
	if ( NULL ==  *_TotalHead ) { return  ; }

	kfree( *_TotalHead );
	*_TotalHead = NULL ;

	return  ;
}



BOOL 
PDAllocateNode(
	OUT PVOID* _pCurrenList_
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/04/29 [29:4:2009 - 14:01]

Routine Description:
  ���� & ��ʼ��һ������ڵ�  
    
Arguments:
  _pCurrenList_ - �������õ�����Node��ָ��

--*/
{
	LPPDNODE* pCurrenList = (LPPDNODE*) _pCurrenList_ ;
	*pCurrenList = (LPPDNODE) kmalloc( sizeof( PDNODE ) );

	if ( NULL == *pCurrenList )
	{
		dprintf( "error! | PDAllocateNode() | Allocate memory failed! \n" );
		return FALSE ;
	}

	RtlZeroMemory( *pCurrenList, sizeof( PDNODE ) );

	return TRUE ;
}



PVOID
PDBuildNode (
	IN PVOID _TotalHead,
	IN ULONG PID,
	IN BOOL bInsert
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/19 [19:5:2010 - 20:11]

Routine Description:
  �½�&���PD�ṹ��
    
Arguments:
  _TotalHead - ����ͷ
  PID - ����ID
  bInsert - �Ƿ���뵽������; һ�㴴�����ýڵ��,���к�����������е�Node_c����,�ʻ������һ�ж������Ϻ��ٲ�������β��Ϊ����!

Return Value:
  �½���PD�ṹ��ָ��
    
--*/
{
	BOOL bRet = FALSE ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ;
	LPPDNODE pNode = NULL ;
	LPPDHEAD pTotalHead	 = (LPPDHEAD) _TotalHead ;

	//
	// 1. У������Ϸ���
	//

	if ( NULL == _TotalHead || 0 == PID )
	{
		dprintf( "error! | PDBuildNode() | Invalid Parameters \n" );
		return NULL ;
	}

	//
	// 2. Ϊ�½ڵ�����ڴ�
	//

	bRet = PDAllocateNode( &pNode );
	if ( FALSE == bRet )
	{
		dprintf( "error! | PDBuildNode() - PDAllocateNode() | �����ڴ�ʧ�� \n" );
		return NULL ;
	}

	//
	// 3. ����½ڵ�
	//

	status = PDBuildNodeEx( pNode, PID );
	if ( !NT_SUCCESS(status) )
	{
		dprintf( "error! | PDBuildNode() - MPBuildNodeEx() | status = 0x%08lx \n", status );
		goto _error_ ;
	}

	//
	// 4. �����½ڵ㵽����β
	//

	if ( TRUE == bInsert )
	{
		kInsertTailPD( pNode );
	}

	return (PVOID) pNode ;

_error_ :
	kfree( pNode );
	return NULL ;
}



NTSTATUS
PDBuildNodeEx (
	OUT PVOID _pNode,
	IN ULONG PID
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/28 [28:5:2010 - 15:33]

Routine Description:
  ��������ܽ���������    
    
Arguments:
  pNode - �����Ľڵ�
  PID - ����ID
    
--*/
{
	BOOL bRet = FALSE ;
	LPPDNODE pNode = (LPPDNODE) _pNode ;

	//
	// 1. У������Ϸ���
	//
	
	if ( NULL == pNode )
	{
		dprintf( "error! | PDBuildNodeEx(); | Invalid Paramaters; failed! \n" );
		return STATUS_INVALID_PARAMETER ;
	}

	//
	// 2. ��ѯ������Ϣ
	//

	pNode->PID = PID ;
	pNode->FileTraceFlag	= GetConfigurationA( "FileTrace" );
	pNode->PipeTraceFlag	= GetConfigurationA( "PipeTrace" );
	pNode->KeyTraceFlag		= GetConfigurationA( "KeyTrace"	);
	pNode->GuiTraceFlag		= GetConfigurationA( "GuiTrace"	);

	pNode->XImageNotifyDLL.IncrementCounts = 0 ;

	//
	// 3. ��ʼ����������ͷ,��Դ�� 
	//

	// 3.1 ��ʼ��File
	InitializeListHead( &pNode->XFilePath.ClosedFilePathListHead );
	InitializeListHead( &pNode->XFilePath.OpenFilePathListHead );
	InitializeListHead( &pNode->XFilePath.ReadFilePathListHead );
	bRet = InitResource( &pNode->XFilePath.pResource );
	if ( FALSE == bRet )
	{
		dprintf( "error! | PDBuildNodeEx() - InitResource( &pNode->XFilePath.pResource ); ������Դ���ڴ�ʧ��! \n" );
		goto _error_ ;
	}

	// 3.2 ��ʼ��IPC
	InitializeListHead( &pNode->XIpcPath.OpenIpcPathListHead );
	InitializeListHead( &pNode->XIpcPath.ClosedIpcPathListHead );
	bRet = InitResource( &pNode->XIpcPath.pResource );
	if ( FALSE == bRet )
	{
		dprintf( "error! | PDBuildNodeEx() - InitResource( &pNode->XIpcPath.pResource ); ������Դ���ڴ�ʧ��! \n" );
		goto _error_ ;
	}

	// 3.3 ��ʼ��Wnd
	InitializeListHead( &pNode->XWnd.WndListHead );
	bRet = InitResource( &pNode->XWnd.pResource );
	if ( FALSE == bRet )
	{
		dprintf( "error! | PDBuildNodeEx() - InitResource( &pNode->XWnd.pResource ); ������Դ���ڴ�ʧ��! \n" );
		goto _error_ ;
	}

	// 3.4 ��ʼ��Reg
	InitializeListHead( &pNode->XRegKey.DennyListHead );
	InitializeListHead( &pNode->XRegKey.DirectListHead );
	InitializeListHead( &pNode->XRegKey.ReadOnlyListHead );
	bRet = InitResource( &pNode->XRegKey.pResource );
	if ( FALSE == bRet )
	{
		dprintf( "error! | PDBuildNodeEx() - InitResource( &pNode->XRegKey.pResource ); ������Դ���ڴ�ʧ��! \n" );
		goto _error_ ;
	}

	pNode->XWnd.TranshipmentStation = pNode->XWnd.VictimClassName = NULL ;
	return STATUS_SUCCESS ;

_error_ :
	pNode->bDiscard = 1 ;
	return STATUS_UNSUCCESSFUL ;
}



PVOID
PDBuildNodeForce (
	IN PVOID _TotalHead,
	IN ULONG PID,
	IN LPWSTR szFullImageName
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/10 [10:6:2010 - 15:36]

Routine Description:
  ��ѯ�����ļ�,�õ�"ForceFolder","ForceProcess"��Ӧ������. ��@szFullImageNameƥ��,��������,
  ����Ҫǿ����������ɳ����,��ע��*.dll����ǰ������
    
Arguments:
  _TotalHead - ����ͷ
  PID - ����ID
  szFullImageName - ģ��ص��Ĳ���1��ģ��ȫ·��

Return Value:
  �½���PD�ṹ��ָ�� | NULL

--*/
{
	BOOL bRet = FALSE, bRunInSandbox = TRUE ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	WCHAR ForcePRocessPath[ MAX_PATH ] = L"" ;
	WCHAR ForceFolderPath[ MAX_PATH ] = L"" ;
	WCHAR SeactionName[ MAX_PATH ] = L"GlobalSetting" ;
	PNODE_INFO_C pNode_c = NULL ;
	WCHAR UserSid[ MAX_PATH ] = L"" ;
	IOCTL_STARTPROCESS_BUFFER Buffer = { NULL, UserSid, NULL, NULL } ;
	LPPDNODE pNode = NULL ;
	LPPDHEAD pTotalHead	 = (LPPDHEAD) _TotalHead ;

	//
	// 1. У������Ϸ���
	//

	if ( NULL == _TotalHead || 0 == PID || NULL == szFullImageName )
	{
		dprintf( "error! | PDBuildNodeForce() | Invalid Parameters \n" );
		return NULL ;
	}

	//
	// 2. ȷ����ǰ�û��ĺϷ���
	//

	status = RtlGetUserSid( (HANDLE)0xFFFFFFFF, UserSid );
	if ( ! NT_SUCCESS(status) )
	{
		dprintf( "error! | PDBuildNodeForce() - RtlGetUserSid(); | (status=0x%08lx) \n", status );
		return NULL ;
	}

	if (	0 == _wcsicmp( UserSid, L"S-1-5-18" )
		||	0 == _wcsicmp( UserSid, L"S-1-5-19" )
		||	0 == _wcsicmp( UserSid, L"S-1-5-20" )
		)
	{
		dprintf( "error! | PDBuildNodeForce() - _wcsicmp(); | (UserSid=%ws) \n", UserSid );
		return NULL ;
	}

	//
	// 3. ��ѯ�����ļ�,�õ�"ForceFolder","ForceProcess"��Ӧ������
	//

	bRet = FPLGetState( szFullImageName, &bRunInSandbox );
	if ( FALSE == bRet || FALSE == bRunInSandbox )
	{
	//	dprintf( "error! | PDBuildNodeForce() | δƥ�䵽ǿ�����е��ļ�·��,��ǰģ�齫���ᱻSB \n" );
		return NULL ;
	}

	//
	// 4.1 OK! ��ǿ��������ɳ����,������Ӧ�Ľ��̽ڵ�
	//

	pNode = (LPPDNODE) kbuildnodePD( PID, FALSE );
	if ( NULL == pNode )
	{
		dprintf( "error! | PDBuildNodeForce() - kbuildnodePD(); | �������̽ڵ�ʧ�� PID=%d \n", PID );
		return NULL ;
	}

	//
	// 4.2 ���C�ڵ�
	//

	pNode->pNode_C = pNode_c = PDBuildNodeC( (PVOID)&Buffer );
	if ( NULL == pNode_c )
	{
		dprintf( "error! | PDBuildNodeForce() - PDBuildNodeC(); | �������̽ڵ�Cʧ�� PID=%d \n", PID );
		kfree( pNode ); // ��δ��������,��ʼ��ʧ�����ͷ��ڴ�
		return NULL ;
	}

	//
	// 5. �����½ڵ㵽����β
	//

	kInsertTailPD( pNode );
	return (PVOID)pNode ;
}



PVOID
PDCopyNodeC (
	IN PVOID Buffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/08 [8:6:2010 - 17:51]

Routine Description:
  ���Ƹ����̵Ľڵ�C����ǰ�ӽڵ�    
    
Arguments:
  Buffer - �����̵Ľڵ�Cָ��

Return Value:
  �¸��Ƶ�һ���ӽڵ�Cָ��
    
--*/
{
	PNODE_INFO_C pNode_c = NULL ;
	PNODE_INFO_C pNode_c_old = (PNODE_INFO_C) Buffer ;

	// 1. У������Ϸ���
	if ( NULL == Buffer )
	{
		dprintf( "error! | PDCopyNodeC() | Invalid Paramaters; failed! \n" );
		return NULL ;
	}

	// 2. ����C�ڵ��ڴ�
	pNode_c = (PNODE_INFO_C) kmalloc( sizeof( NODE_INFO_C ) );
	if ( NULL == pNode_c )
	{
		dprintf( "error! |  PDCopyNodeC() - kmalloc( sizeof( NODE_INFO_C ); | ����ڵ�C���ڴ�ʧ��! \n" );
		return NULL ;
	}

	// 3. ����֮
	memcpy( pNode_c, pNode_c_old, sizeof( NODE_INFO_C ) );

	// 4 �������� g_ListHead_RegistryUserSID
	if ( NULL == kbuildnodeSD( pNode_c->RegisterUserID ) )
	{
		dprintf( "error! | PDCopyNodeC() - kbuildnodeSD(); | ����SID�ڵ�ʧ��; RegisterUserID=\"%ws\" \n", pNode_c->RegisterUserID );
		return NULL ;
	}

	return (PVOID) pNode_c ;
}



PVOID
PDBuildNodeC (
	IN PVOID pInBuffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/02 [2:6:2010 - 14:28]

Routine Description:
  ���� & �������ܽڵ��е�С�ڵ�: C
  �ṹ��NodeCָ��
typedef struct _NODE_INFO_C_ {
	WCHAR wszName1[0x40] ; // eg:"DefaultBox"
	ULONG Name1Length ;
	PVOID wszRegisterUserID ; // ����ZwQueryInformationProcess()���ݲ���ProcessSessionInformation�õ���SID
                                      //eg:"S-1-5-21-583907252-261903793-1177238915-1003"
	ULONG nRegisterUserID_Length ; 
	ULONG SessionId ;
	PVOID pNode_D ;  // ָ��һ��0x10��С�Ľṹ��
	PVOID wszFilePath_BoxRootFolder ;  // eg:"\Device\HarddiskVolume1\Sandbox\AV\DefaultBox"
	ULONG FilePath_Length ;
	PVOID wszRegPath_KeyRootPath ; // eg:"\REGISTRY\USER\Sandbox_AV_DefaultBox"
	ULONG RegPath_Length ;
	PVOID wszLpcRootPath1 ;   // eg:"\Sandbox\AV\DefaultBox\Session_0"
	ULONG LpcRootPath1Length ;
	PVOID wszLpcRootPath2 ;   // eg:"_Sandbox_AV_DefaultBox_Session_0"
	ULONG LpcRootPath2Length ;
} NODE_INFO_C, *PNODE_INFO_C ;
    
Arguments:
  pInBuffer - LPIOCTL_STARTPROCESS_BUFFER�ṹ��ָ��
    
--*/
{
	BOOL bRet = FALSE ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	WCHAR BoxName[ MAX_PATH ] = L"DefaultBox" ;
	WCHAR RegisterUserID[ MAX_PATH ] = L"" ;
	PNODE_INFO_C pNode_c = NULL ;
	LPIOCTL_STARTPROCESS_BUFFER Buffer = (LPIOCTL_STARTPROCESS_BUFFER) pInBuffer ;

	//
	// 1. У������Ϸ���
	//
	
	if ( NULL == Buffer )
	{
		dprintf( "error! |  | Invalid Paramaters; failed! \n" );
		return NULL ;
	}

	//
	// 2. ����C�ڵ��ڴ�
	//

	pNode_c = (PNODE_INFO_C) kmalloc( sizeof( NODE_INFO_C ) );
	if ( NULL == pNode_c )
	{
		dprintf( "error! |  PDBuildNodeC() - kmalloc( sizeof( NODE_INFO_C ); | ����ڵ�C���ڴ�ʧ��! \n" );
		return NULL ;
	}

	//
	// 3.1 ��ȡ���ֱ�������
	//

	// ��ȡɳ����
	if ( Buffer->wszBoxName )
	{			
		RtlCopyMemory( BoxName, Buffer->wszBoxName, 0x40 );

		if ( FALSE == Check_IsValid_Characters( BoxName ) )
		{
			dprintf( "error! | PDBuildNodeC() - Check_IsValid_Characters(); | R3�ڴ���е�ǰɳ����Ϊ�����ַ���,����Ĭ�ϵ�����! \n" );
			RtlZeroMemory( BoxName, 0x40 );
			wcscpy( BoxName, L"DefaultBox" );
		}
	}

	// ��ȡ�û�SID
	if ( Buffer->wszRegisterUserID )
	{			
		RtlCopyMemory( RegisterUserID, Buffer->wszRegisterUserID, 0x5E );
	}

	if ( *RegisterUserID != 'S' && RegisterUserID[1] != '-' )
	{
		// R3�ڴ���е�SID�Ƿ�,ֱ���������л�ȡ֮
		status = RtlGetUserSid( NtCurrentProcess(), RegisterUserID );
		if ( ! NT_SUCCESS(status) ) 
		{
			dprintf( "error! | PDBuildNodeC() - RtlGetUserSid() | status = 0x%08lx \n", status );
			goto _error_ ;
		}
	}

	//
	// 3.2 ������еĳ�Ա
	//

	wcscpy( pNode_c->BoxName, BoxName );
	pNode_c->BoxNameLength = (wcslen(pNode_c->BoxName) + 1) * sizeof(WCHAR) ;

	wcscpy( pNode_c->RegisterUserID, RegisterUserID );
	pNode_c->RegisterUserIDLength = (wcslen(pNode_c->RegisterUserID) + 1) * sizeof(WCHAR) ;

	bRet = ProcessIdToSessionId( 0, &pNode_c->SessionId );
	if ( FALSE == bRet )
	{
		dprintf( "error! | PDBuildNodeC() - ProcessIdToSessionId(); |  \n" );
		goto _error_ ;
	}

	//
	// 4 �������� g_ListHead_RegistryUserSID
	//

	if ( NULL == kbuildnodeSD( RegisterUserID ) )
	{
		dprintf( "error! | PDBuildNodeC() - kbuildnodeSD(); | ����SID�ڵ�ʧ��; RegisterUserID=\"%ws\" \n", RegisterUserID );
		goto _error_ ;
	}

	//
	// 5. ���������Ա,��Ҫ���������ļ���ȡ�����Ϣ 
	//

	bRet = __PDBuildNodeC( (PVOID)pNode_c );
	if ( FALSE == bRet )
	{
		dprintf( "error! | PDBuildNodeC() - __PDBuildNodeC(); | \n" );
		goto _error_ ;
	}

	return (PVOID)pNode_c ;

_error_ :
	kfree( pNode_c );
	return NULL ;
}



BOOL
__PDBuildNodeC (
	OUT PVOID _pNode
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/03 [3:6:2010 - 14:51]

Routine Description:
  ���PD�ṹ���������Ա,��Ҫ���������ļ���ȡ�����Ϣ    

--*/
{
	BOOL bRet = FALSE ;
	WCHAR SeactionName[ MAX_PATH ] = L"GlobalSetting" ;
	WCHAR FileRootPath[ MAX_PATH ] = L"" ;
	WCHAR KeyRootPath[ MAX_PATH ] = L"" ;
	WCHAR IpcRootPath[ MAX_PATH ] = L"" ;
	LPWSTR ptr = NULL ;
	PNODE_INFO_C pNode = (PNODE_INFO_C) _pNode ;

	//
	// 1. У������Ϸ���
	//

	if ( NULL == pNode )
	{
		dprintf( "error! | __PDBuildNodeC(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	// 2.1 ��� FileRootPath ��Ӧ·��; "\Device\HarddiskVolume1\ProteinBox\SUDAMI\DefaultBox"
	bRet = kGetConfSingle( SeactionName, L"FileRootPath", FileRootPath );
	if ( FALSE == bRet )
	{
		bRet = kGetConfSingle( SeactionName, L"BoxRootFolder", FileRootPath );
		if ( TRUE == bRet )
		{
			wcscat( FileRootPath, L"\\SUDAMI\\" );
			wcscat( FileRootPath, pNode->BoxName );
		}
		else
		{
		//	wcscpy( FileRootPath, g_default_FileRootPath );
			return FALSE ;
		}
	}

	ptr = ParseCharacters( (PVOID)FileRootPath );
	if ( NULL == ptr )
	{ 
		dprintf( "error! | __PDBuildNodeC() - ParseCharacters(); | �����ַ���ʧ��:\"%ws\" \n", FileRootPath );
		return FALSE ;
	}

	wcscpy( pNode->FileRootPath, ptr );
	pNode->FileRootPathLength = ( wcslen(ptr) + 1 ) * sizeof(WCHAR) ;

	// 2.2 ��� KeyRootPath ��Ӧ·��; "\REGISTRY\USER\ProteinBox_SUDAMI_DefaultBox"
	bRet = kGetConfSingle( SeactionName, L"KeyRootPath", KeyRootPath );
	if ( FALSE == bRet ) { wcscpy( KeyRootPath, g_default_KeyRootPath ); }

	ptr = ParseCharacters( (PVOID)KeyRootPath );
	if ( NULL == ptr )
	{ 
		dprintf( "error! | __PDBuildNodeC() - ParseCharacters(); | �����ַ���ʧ��:\"%ws\" \n", KeyRootPath );
		return FALSE ;
	}

	wcscpy( pNode->KeyRootPath, ptr );
	pNode->KeyRootPathLength = ( wcslen(ptr) + 1 ) * sizeof(WCHAR) ;

	// 2.3 ��� IpcRootPath1 ��Ӧ·��; "\ProteinBox\SUDAMI\DefaultBox\Session_0"
	bRet = kGetConfSingle( SeactionName, L"IpcRootPath", IpcRootPath );
	if ( FALSE == bRet ) { wcscpy( IpcRootPath, g_default_IpcRootPath ); }

	ptr = ParseCharacters( (PVOID)IpcRootPath );
	if ( NULL == ptr )
	{ 
		dprintf( "error! | __PDBuildNodeC() - ParseCharacters(); | �����ַ���ʧ��:\"%ws\" \n", IpcRootPath );
		return FALSE ;
	}

	wcscpy( pNode->LpcRootPath1, ptr );
	pNode->LpcRootPath1Length = ( wcslen(ptr) + 1 ) * sizeof(WCHAR) ;

	// 2.4 ��� IpcRootPath2 ��Ӧ·��; "_Sandbox_AV_DefaultBox_Session_0"
	{
		LPWSTR pBuffer = (LPWSTR) pNode->LpcRootPath2 ;
		LPWSTR p = NULL ;

		wcscpy( pNode->LpcRootPath2, ptr );

		while ( *pBuffer )
		{
			p = wcschr( pBuffer, '\\' );
			if ( p )
			{
				pBuffer = p ;
				*p = '_';
			}
			else
			{
				pBuffer += wcslen( pBuffer );
			}
		}
	}

	return TRUE ;
}



BOOL 
PDFixupNode(
	OUT PVOID _pNode,
	IN LPCWSTR wszName
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/28 [28:5:2010 - 17:59]

Routine Description:
  �����ͽ��̽ڵ���ص�����    
    
Arguments:
  _pNode - �������Ľ��̽ڵ�

--*/
{
	BOOL bRet = FALSE ;
	ULONG length = 0 ;
	UNICODE_STRING uniBuffer ;
	WCHAR Temp[ MAX_PATH ] = L"unknown executable image" ;
	LPWSTR pBuffer, ptr = NULL ;
	LPWSTR lpImageShortName = (LPWSTR) wszName ;
	LPPDNODE pNode = (LPPDNODE) _pNode ;

	//
	// 1. У������Ϸ���
	//
	
	if ( NULL == pNode )
	{
		dprintf( "error! | PDFixupNode(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	//
	// 2. ���ݽ���ȫ·��,���ڵ��еı��λ,���Ƿ�Ϊ����Ŀ¼��ĳ��� or �Ƿ�Ϊ�ض���Ŀ¼�µĳ���
	//

	if ( NULL == wszName ) 
	{
		lpImageShortName = Temp ;
	}
	else 
	{
		ptr = wcsrchr(wszName, '\\');
		if ( ptr )
		{
			lpImageShortName = ptr + 1;

			if ( 0 == wcsncmp( wszName, g_ProteinBox_path, g_ProteinBox_path_length ) )
			{
				pNode->bSelfExe = 1 ; // �������Լ�Ŀ¼�µĳ���
			}

			if ( 0 == wcsncmp( wszName, pNode->pNode_C->FileRootPath, pNode->pNode_C->FileRootPathLength ) )
			{
				pNode->bIsAVDefaultBox = 1 ; // �������ض���Ŀ¼�µ��ļ�������
			}
		}
	}

	//  
	// 3. �����ڴ�,��������ȫ·��,��������̽ڵ���
	//

	length = (wcslen (lpImageShortName) + 1) * sizeof(WCHAR) ;
	pBuffer = (LPWSTR) kmalloc( length );
	if ( NULL == pBuffer )
	{
		dprintf( "error! | PDFixupNode() - kmallocMM(); NULL == pBuffer \n" );
		pNode->bDiscard = 1 ; // ����Ҫ���ٵ�ǰ���
		return FALSE ;
	}

	RtlCopyMemory( pBuffer, lpImageShortName, length );

	pNode->lpProcessShortName	= pBuffer ;
	pNode->ImageNameLength		= length  ;

	//  
	// 4. ��Ȩ�Ȳ���
	// 

	if (   FALSE == DropAdminRights( _pNode )
		|| FALSE == CreateRootBox( _pNode )
		|| FALSE == CreateSessionDirectoryObject( _pNode )
		|| FALSE == HandlerRegHive( _pNode )
		)
	{
		pNode->bDiscard = 1 ; // ����Ҫ���ٵ�ǰ���
		return FALSE ;
	}

	return TRUE ;
}



NTSTATUS
PDEnumProcessEx(
	IN LPWSTR szBoxName,
	IN BOOL bFlag,
	OUT int *pArrays
	)
{
	ULONG pAddress = 0, SessionId = 0, nIndex = 0 ;
	LPPDNODE pNodeHead = NULL, pCurrentNode = NULL ;
	LPPDHEAD pTotalHead = (LPPDHEAD) g_ListHead__ProcessData ;

	// 1. У������Ϸ���
	if ( (NULL == pTotalHead) || NULL == pTotalHead->QueueLockList )
	{
		return STATUS_INVALID_PARAMETER ;
	}

	if ( bFlag ) { GetSessionId( &SessionId ); }
	 
	// 2. ��������,����ָ���ڵ�
	EnterCrit( pTotalHead->QueueLockList );	// ��������

	pNodeHead    =  &pTotalHead->ListHead ;
	pCurrentNode = pNodeHead->pBlink ;

	while ( pCurrentNode != pNodeHead )
	{
		if ( pCurrentNode->pNode_C )
		{
			if (   (*szBoxName && 0 == _wcsicmp(szBoxName, pCurrentNode->pNode_C->BoxName))
				|| (SessionId && SessionId == pCurrentNode->pNode_C->SessionId)
				)
			{
				++ nIndex;
				if ( nIndex == 0x200 ) { break; }

				pArrays[ nIndex ] = pCurrentNode->PID ;
			}
		}

		pCurrentNode = pCurrentNode->pBlink ;
	}

	*pArrays = nIndex;

	LeaveCrit( pTotalHead->QueueLockList );	// ��������
	return STATUS_SUCCESS;
}



PVOID
PDFindNode (
	IN PVOID _TotalHead ,
	IN ULONG PID
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/17 [17:5:2010 - 18:27]

Routine Description:
  �������в���ָ���ڴ��ַ,����֮
    
Arguments:
  _TotalHead - �ܵ�����ͷ
  PID - ��ƥ��Ľ���ID, Ϊ0 ����Ҫ���ҵ�ǰPID,ͨ��PsGetCurrentProcessId��ȡ

Return Value:
  ���ҵ��Ľ��̽ڵ�
    
--*/
{
	KIRQL OldIrql ;
	ULONG pAddress	 = 0 ;
	PVOID pResult = NULL ;
	LPPDNODE pNodeHead = NULL, pCurrentNode = NULL ;
	LPPDHEAD pTotalHead = (LPPDHEAD) _TotalHead ;

	//
	// 1. У������Ϸ���
	//

	if ( (NULL == pTotalHead) || NULL == pTotalHead->QueueLockList || (-1 == PID) )
	{
	//	dprintf( "error! | PDFindNode() | Invalid Parameters \n" );
		return NULL ;
	}

	if ( 0 == PID )
	{
		// ����Ҫ���ҵ�ǰPID,ͨ��PsGetCurrentProcessId��ȡ
		PID = (ULONG) PsGetCurrentProcessId();
		if ( KernelMode == ExGetPreviousMode() || PID <= 8 ) { return NULL ; }
	}

	if ( (0 == pTotalHead->nTotalCounts) || (TRUE == IsListEmpty( (PLIST_ENTRY) &pTotalHead->ListHead )) )
	{
		return NULL ;
	}

	//
	// 2. ��������,����ָ���ڵ�
	//

	EnterCrit( pTotalHead->QueueLockList );	// ��������

	pNodeHead    =  &pTotalHead->ListHead ;
	pCurrentNode = pNodeHead->pBlink ;

	while ( pCurrentNode != pNodeHead )
	{
		if ( PID == pCurrentNode->PID ) 
		{
			pResult = (PVOID) pCurrentNode ;
			break ;
		}

		pCurrentNode = pCurrentNode->pBlink ;
	}

	LeaveCrit( pTotalHead->QueueLockList );	// ��������
	return pResult ;
}



VOID
PDConfResetAll (
	IN PVOID _TotalHead
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2011/12/27 [27:12:2011 - 10:39]

Routine Description:
  ��������ɳ����̽ڵ��е������ļ���ص����ݣ�����Reload Configurationʱ����
    
--*/
{
	int nTotalCounts = 0 ;
	LPPDNODE pCurrentNode = NULL, pNodeHead = NULL;
	LPPDHEAD pTotalHead = (LPPDHEAD) _TotalHead ;

	// 1. �������
	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | PDConfResetAll() | Invalid ListHead \n" );
		return ;
	}

	EnterCrit( pTotalHead->QueueLockList );	// ��������

	nTotalCounts = pTotalHead->nTotalCounts;
	pNodeHead    =  &pTotalHead->ListHead ;
	pCurrentNode = pNodeHead->pBlink ;

	while ( pCurrentNode != pNodeHead )
	{
		// 2.1 Reset Files
		if ( pCurrentNode->XFilePath.bFlagInited && pCurrentNode->XFilePath.pResource )
		{
			GLDistroyAll( pCurrentNode->XFilePath.pResource, &pCurrentNode->XFilePath.OpenFilePathListHead );
			GLDistroyAll( pCurrentNode->XFilePath.pResource, &pCurrentNode->XFilePath.ClosedFilePathListHead );
			GLDistroyAll( pCurrentNode->XFilePath.pResource, &pCurrentNode->XFilePath.ReadFilePathListHead );

			pCurrentNode->XFilePath.bFlagInited = FALSE;
		}

		// 2.2 Reset Ipcs
		if ( pCurrentNode->XIpcPath.bFlagInited && pCurrentNode->XIpcPath.pResource )
		{
			GLDistroyAll( pCurrentNode->XIpcPath.pResource, &pCurrentNode->XIpcPath.OpenIpcPathListHead );
			GLDistroyAll( pCurrentNode->XIpcPath.pResource, &pCurrentNode->XIpcPath.ClosedIpcPathListHead );

			pCurrentNode->XIpcPath.bFlagInited = FALSE;
		}

		// 2.3 Reset Wnds
		if ( pCurrentNode->XWnd.bFlagInited && pCurrentNode->XWnd.pResource )
		{
			GLDistroyAll( pCurrentNode->XWnd.pResource, &pCurrentNode->XWnd.WndListHead );

			pCurrentNode->XWnd.bFlagInited = FALSE;
		}

		// 2.4 Reset Regs
		if ( pCurrentNode->XRegKey.bFlagInited && pCurrentNode->XRegKey.pResource )
		{
			GLDistroyAll( pCurrentNode->XRegKey.pResource, &pCurrentNode->XRegKey.DennyListHead );
			GLDistroyAll( pCurrentNode->XRegKey.pResource, &pCurrentNode->XRegKey.DirectListHead );
			GLDistroyAll( pCurrentNode->XRegKey.pResource, &pCurrentNode->XRegKey.ReadOnlyListHead );

			pCurrentNode->XRegKey.bFlagInited = FALSE;
		}

		pCurrentNode = pCurrentNode->pBlink ;
	}

	LeaveCrit( pTotalHead->QueueLockList );	// ��������

	if ( nTotalCounts ) { dprintf( "*** ����ɳ����̽ڵ�(%d��)�е��������������� *** \n", nTotalCounts ); }
	return ;
}



VOID
PDClearNode (
	IN 	PVOID pNode
	)
{
	LPPDNODE pCurrentNode = (LPPDNODE)pNode ;

	// 1. У������Ϸ���
	if ( NULL == pCurrentNode ) { return ; }

	// 2. ����ǰ�ڵ�ĸ�����Դ
	kfree( pCurrentNode->pNode_C );
	kfree( pCurrentNode->lpProcessShortName );

	// 2.1 Release Files
	if ( pCurrentNode->XFilePath.pResource )
	{
		GLDistroyAll( pCurrentNode->XFilePath.pResource, &pCurrentNode->XFilePath.OpenFilePathListHead );
		GLDistroyAll( pCurrentNode->XFilePath.pResource, &pCurrentNode->XFilePath.ClosedFilePathListHead );
		GLDistroyAll( pCurrentNode->XFilePath.pResource, &pCurrentNode->XFilePath.ReadFilePathListHead );

		ExDeleteResource( pCurrentNode->XFilePath.pResource );
		kfree( pCurrentNode->XFilePath.pResource );
	}
	
	// 2.2 Release Ipcs
	if ( pCurrentNode->XIpcPath.pResource )
	{
		GLDistroyAll( pCurrentNode->XIpcPath.pResource, &pCurrentNode->XIpcPath.OpenIpcPathListHead );
		GLDistroyAll( pCurrentNode->XIpcPath.pResource, &pCurrentNode->XIpcPath.ClosedIpcPathListHead );

		ExDeleteResource( pCurrentNode->XIpcPath.pResource );
		kfree( pCurrentNode->XIpcPath.pResource );
	}

	// 2.3 Release Wnds
	if ( pCurrentNode->XWnd.pResource )
	{
		GLDistroyAll( pCurrentNode->XWnd.pResource, &pCurrentNode->XWnd.WndListHead );

		ExDeleteResource( pCurrentNode->XWnd.pResource );
		kfree( pCurrentNode->XWnd.pResource );
	}

	// 2.4 Release Regs
	if ( pCurrentNode->XRegKey.pResource )
	{
		GLDistroyAll( pCurrentNode->XRegKey.pResource, &pCurrentNode->XRegKey.DennyListHead );
		GLDistroyAll( pCurrentNode->XRegKey.pResource, &pCurrentNode->XRegKey.DirectListHead );
		GLDistroyAll( pCurrentNode->XRegKey.pResource, &pCurrentNode->XRegKey.ReadOnlyListHead );

		ExDeleteResource( pCurrentNode->XRegKey.pResource );
		kfree( pCurrentNode->XRegKey.pResource );
	}

	kfree( pCurrentNode->XImageNotifyDLL.BeCoveredOldData ); // ���ڴ���Ioctl_GetInjectSaveArea()���óɹ�ʱ�ᱻ�ͷ�; һ������²����ٴδ���,��Ҫ��ֹ�ڴ�й¶
	kfree( pCurrentNode->XWnd.VictimClassName );
	kfree( pCurrentNode );
	return ;
}



BOOL
PDDeleteNode (
	IN PVOID _TotalHead ,
	IN ULONG PID
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/09 [9:6:2010 - 10:51]

Routine Description:
  ��ɳ���еĽ�������ʱ,ͬ��Ҫɾ�����������ж�Ӧ�Ľڵ�
    
Arguments:
  _TotalHead - �ܵ�����ͷ
  PID - ��ƥ��Ľ���ID
 
--*/
{
	PVOID pResult = NULL ;
	LPPDNODE pNodeHead = NULL, pCurrentNode = NULL ;
	LPPDHEAD pTotalHead = (LPPDHEAD) _TotalHead ;

	// 1. У������Ϸ���
	if ( (NULL == pTotalHead) || NULL == pTotalHead->QueueLockList || ( 0 == PID) ) { return FALSE ; }

	if ( (0 == pTotalHead->nTotalCounts) || (TRUE == IsListEmpty( (PLIST_ENTRY) &pTotalHead->ListHead )) )
	{
//		dprintf( "error! | PDDeleteNode(); | ����Ϊ��,�޷�����ɾ��ָ���ڵ� PID=%d \n", PID );
		return FALSE ;
	}

	// 2. ��������,����ָ���ڵ�
	EnterCrit( pTotalHead->QueueLockList );	// ��������

	pNodeHead    =  &pTotalHead->ListHead ;
	pCurrentNode = pNodeHead->pBlink ;

	while ( pCurrentNode != pNodeHead )
	{
		if ( PID == pCurrentNode->PID ) 
		{
			RemoveEntryList( (PLIST_ENTRY) pCurrentNode ); // ���������Ƴ�
			pResult = (PVOID) pCurrentNode ;
			break ;
		}

		pCurrentNode = pCurrentNode->pBlink ;
	}

	LeaveCrit( pTotalHead->QueueLockList );	// ��������

	// 3. �����ǰ�ڵ���ڴ�
	if ( NULL == pResult )
	{ 
	//	dprintf( "error! | PDDeleteNode(); | δ�ҵ�ָ���ڵ� PID=%d \n", PID );
		return FALSE ;
	}

	PDClearNode( (PVOID)pCurrentNode );
	--pTotalHead->nTotalCounts;
	
//	dprintf( "ok! | PDDeleteNode(); | ���Ƴ���ǰ���̽ڵ�(PID=%d) \n", PID );
	return TRUE ;
}



VOID
PDDistroyAll (
	IN PVOID _TotalHead
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/09 [9:6:2010 - 11:08]

Routine Description:
  ��������������
    
--*/
{
	int i = 0 ;
	PLIST_ENTRY ListHead = NULL, CurrentListEntry = NULL ;
	LPPDNODE pCurrentNode = NULL ;
	LPPDHEAD pTotalHead = (LPPDHEAD) _TotalHead ;

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | PDDistroyAll() | Invalid ListHead \n" );
		return ;
	}

	//
	// 1. ɾ�����е��ӽڵ�
	//

	PDWalkNodes( pTotalHead );

	dprintf( "*** ��ʼж�ؽ��̽ڵ������ *** \n" );

	EnterCrit( pTotalHead->QueueLockList );	// ��������

	ListHead = (PLIST_ENTRY) &pTotalHead->ListHead ;

	while( FALSE == IsListEmpty( ListHead ) )
	{
		CurrentListEntry = (PLIST_ENTRY) RemoveHeadList( ListHead );
		pCurrentNode = (LPPDNODE) CurrentListEntry ;

		dprintf( "  [%d] Node:0x%08lx \n", i++, (PVOID)pCurrentNode );
		PDClearNode( (PVOID)pCurrentNode );
	}

	LeaveCrit( pTotalHead->QueueLockList );	// ��������

	ExDeleteResource( pTotalHead->QueueLockList );
	kfree( pTotalHead->QueueLockList );

	//
	// 2. ɾ���ܽڵ�
	//

	PDDeleteTotalHead( &_TotalHead );

	dprintf( "*** ����ж�ؽ��̽ڵ������ *** \n" );
	return ;
}



VOID
PDCheckOut(
	IN PVOID _TotalHead,
	IN ULONG InvalidPID
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/09 [9:6:2010 - 11:11]

Routine Description:
  �ú����ڽ��̽���notify���汻����,������У���������PID�ĺϷ���    
    
Arguments:
  InvalidPID - ���ڽ�����,���˸�PID(��Ϊ���Ӧ�Ľ����Ѿ�����),�����Ķ���ҪУ��

--*/
{
	PVOID pBuffer = NULL, ptr = NULL ;
	PEPROCESS Eprocess = NULL ;
	ULONG i = 0, CurrentPID = 0, Length = 0x1000 ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE pNodeHead = NULL, pCurrentNode = NULL ;
	LPPDHEAD pTotalHead = (LPPDHEAD) _TotalHead ;

	//
	// 1. У������Ϸ���
	//
	
	if ( NULL == _TotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | PDCheckOut(); | Invalid Paramaters; failed! \n" );
		return ;
	}

	//
	// 2. ��������,�洢����PID��������֤��
	//

	pBuffer = kmalloc( Length );
	if ( NULL == pBuffer )
	{
		dprintf( "error! | PDCheckOut() - kmalloc(); | �����ڴ�ʧ��, Length=%d \n", Length );
		return ;
	}

	RtlZeroMemory( pBuffer, Length );

	EnterCrit( pTotalHead->QueueLockList );	// ��������

	pNodeHead    =  &pTotalHead->ListHead ;
	pCurrentNode = pNodeHead->pBlink ;

	while ( pCurrentNode != pNodeHead )
	{
		*((DWORD *)pBuffer + i) = pCurrentNode->PID ; // �����洢ÿ��PID
		pCurrentNode = pCurrentNode->pBlink ;

		i ++ ;
		if ( i >= 0x3FE ) { break ; }
	}

	*((DWORD *)pBuffer + i) = 0 ;

	LeaveCrit( pTotalHead->QueueLockList );	// ��������

	//  
	// 3. ��֤����������PID�ĺϷ���
	//

	if ( 0 == *(DWORD*) pBuffer )
	{
	//	dprintf( "error! | PDCheckOut(); | ������������,���κ�PID,˵���������ǿյ� \n" );
		goto _CLEAR_UP_ ;
	}

	ptr = pBuffer ;
	while ( TRUE )
	{
		CurrentPID = *(DWORD*) ptr ;
		ptr = (PCHAR)ptr + 4 ;

		if ( 0 == CurrentPID ) { break ; }
		if ( CurrentPID == InvalidPID ) { continue ; }

		status = PsLookupProcessByProcessId( (HANDLE)CurrentPID, &Eprocess );
		if ( ! NT_SUCCESS(status) )
		{
			kRemovePD( CurrentPID );    // ���Ϸ���ɾ����ǰPID��Ӧ�Ľڵ�
		}
		else
		{
			ObfDereferenceObject( (PVOID)Eprocess );  // �Ϸ������У����һ��
		}
	}

_CLEAR_UP_ :
	kfree( pBuffer );
	return ;
}



VOID
PDWalkNodes (
	IN PVOID _TotalHead
	)
{
	return ;
}




/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+          "��ɳ��"�Ľ��̴�����ʼ�������е�һЩ����         +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


BOOL
CreateRootBox (
	IN PVOID _pNode
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/04 [4:6:2010 - 11:52]

Routine Description:
  ����"\Device\HarddiskVolume1\Sandbox\AV"Ŀ¼ & "\Device\HarddiskVolume1\Sandbox\AV\DefaultBox"Ŀ¼.
							   ------  --											 ----------
						   ɳ����Ŀ¼  ��ǰ�û���                                    ��ǰɳ���� 

  ���ڵ�ǰĿ¼�´���2���ļ�:"desktop.ini","DONT-USE.TXT"
    
--*/
{
	ULONG n = 0 ;
	HANDLE hFile = NULL ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPWSTR ptrOld = NULL, ptrCurrent = NULL ;
	OBJECT_ATTRIBUTES ObjectAttributes ;
	IO_STATUS_BLOCK IoStatusBlock ;
	UNICODE_STRING uniBuffer ;
	LPWSTR FileRootPath = NULL ;
	LPPDNODE pNode = (LPPDNODE) _pNode ;

	//
	// 1. У������Ϸ���
	//

	if ( NULL == _pNode )
	{
		dprintf( "error! | DropAdminRights(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	//
	// 2. ѭ��������ǰɳ���Ŀ¼
	//

	FileRootPath = pNode->pNode_C->FileRootPath ;
	InitializeObjectAttributes( &ObjectAttributes, &uniBuffer, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, 0, g_SecurityDescriptor_new );

	do
	{
		++ n ;

		RtlInitUnicodeString( &uniBuffer, FileRootPath );
		status = ZwCreateFile (
			(PHANDLE) &hFile,
			FILE_GENERIC_READ | FILE_GENERIC_WRITE, // 0x120189, // 
			&ObjectAttributes,
			&IoStatusBlock,
			0,
			0,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			FILE_OPEN_IF,
			FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE,
			0,
			0
			);

		if ( STATUS_OBJECT_PATH_NOT_FOUND == status/* || STATUS_OBJECT_PATH_SYNTAX_BAD == status*/ )
		{
			//
			// ����(���ҵ���) ���νض�,���Դ�Ŀ¼,���Ƿ����
			// eg: "c:\Test\1\2\3" ��Ҫ�������ļ���,����ϵͳ��ǰֻ��"c:\Test"
			// �����γ��Դ�(���������½�) "c:\Test\1\2\3" --> "c:\Test\1\2" --> "c:\Test\1"
			// ������ "c:\Test\1" ʱ,�ᴴ���ɹ�
			//

			ptrCurrent = wcsrchr( FileRootPath, '\\' );
			if ( ptrOld ) { *ptrOld = '\\' ; } // �ָ��ɵı��ضϵ�·��

			ptrOld = ptrCurrent ; // ����ɵ�·����ǰֵ
			if ( ptrCurrent ) { *ptrCurrent = 0 ; } // �ضϵ�ǰ·��

			continue ;
		}

		if( NT_SUCCESS( status ) )
		{
			if ( FILE_CREATED == IoStatusBlock.Information  )
			{
				// ������Ŀ¼���´�����,�����Ѿ��򿪹���,����Ҫ������д���ļ�
				CreateRootBoxEx( hFile );
			}

			ZwClose( hFile );
		}

		if ( NULL == ptrOld ) { break ; } // ��ʱ��ptrOld ��Ϊ��ǰ��ptrCurrent,��Ϊ��������ѭ��

		*ptrOld = '\\' ;
		ptrOld = NULL ;     

	} while ( n < 0x40 );

	if ( n >= 0x40 ) { return FALSE ; }
	if( ! NT_SUCCESS( status ) )
	{
		dprintf( " \n" );
		return status ;
	}

	return TRUE ;
}



NTSTATUS
CreateRootBoxEx (
	IN HANDLE hDirectory
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/04 [4:6:2010 - 16:15]

Routine Description:
  �ڵ�ǰ��Ӧ��Ŀ¼�´���&���2���ļ�"desktop.ini" �� "README.TXT"

  // desktop.ini����������
  [.ShellClassInfo]
  IconFile=C:\Program Files\Internet Explorer\IEXPLORE.EXE
  IconIndex=0
  IconResource=C:\Program Files\Internet Explorer\IEXPLORE.EXE,0
  InfoTip=This folder is a work area created by the program ProteinBox.  This folder might be 
  deleted at any time.  Use at your own risk.

  // README.TXT ��Ϊ����InfoTip������
    
Arguments:
  hDirectory - ��Ӧ���������ļ�Ŀ¼

--*/
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	HANDLE hFile = NULL ;
	FILE_BASIC_INFORMATION FileInformation ; 
	OBJECT_ATTRIBUTES ObjectAttributes ; 
	UNICODE_STRING uniBuffer; 
	IO_STATUS_BLOCK IoStatusBlock ;
	CHAR README[ MAX_PATH ] = "folder is a work area created by the program ProteinBox.  This folder might be deleted at any time.  Use at your own risk." ;

	//
	// 1. �����ڴ�
	//

	if ( NULL == g_Context_InfoTip )
	{
		g_Context_InfoTip = (LPSTR) kmallocMM( 0x300, MTAG___Context_InfoTip );
		if ( NULL == g_Context_InfoTip )
		{
			dprintf( "error! | CreateRootBoxEx() - kmallocMM( 0x300, MTAG___Context_InfoTip ) | �����ڴ�ʧ�� \n" );
			return status ;
		}
		
		sprintf(
			g_Context_InfoTip ,
			"[.ShellClassInfo]\r\nIconFile=%s\r\nIconIndex=0\r\nIconResource=%s,0\r\nInfoTip=%s",
			g_InfoTip_DESKTOP_icon + 4 ,
			g_InfoTip_DESKTOP_icon + 4 ,
			README
			);
	}

	//
	// 2. ���õ�ǰ�ļ��е�����
	//

	InitializeObjectAttributes( &ObjectAttributes, &uniBuffer, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, hDirectory, g_SecurityDescriptor_new );

	status = ZwQueryInformationFile( 
		hDirectory,
		&IoStatusBlock, 
		&FileInformation, 
		sizeof(FILE_BASIC_INFORMATION), 
		FileBasicInformation
		);

	if( NT_SUCCESS( status ) )
	{
		FileInformation.FileAttributes = FileInformation.FileAttributes & 0xFFFFFF79 | 1 ;
		ZwSetInformationFile( hDirectory, &IoStatusBlock, &FileInformation, sizeof(FILE_BASIC_INFORMATION), FileBasicInformation );
	}

	//
	// 3.1 д���ļ� desktop.ini
	//

	RtlInitUnicodeString( &uniBuffer, L"desktop.ini" );

	status = ZwCreateFile(
		&hFile,
		0x12019F, 
		&ObjectAttributes, 
		&IoStatusBlock, 
		NULL, 
		FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM ,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		FILE_OPEN_IF,
		FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE ,
		NULL, 0
		);

	if( NT_SUCCESS( status ) )
	{
		if ( FILE_CREATED == IoStatusBlock.Information  )
		{
			ZwWriteFile( hFile, 0, 0, 0, &IoStatusBlock, g_Context_InfoTip, strlen(g_Context_InfoTip), 0, 0 );
		}

		ZwClose( hFile );
	}
	
	//
	// 3.2 д���ļ� README.TXT
	//

	RtlInitUnicodeString( &uniBuffer, L"README.TXT" );

	status = ZwCreateFile(
		&hFile, 
		0x12019F,
		&ObjectAttributes,
		&IoStatusBlock,
		NULL,
		0,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		FILE_OPEN_IF,
		FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE ,
		NULL, 0
		);

	if( NT_SUCCESS( status ) )
	{
		if ( FILE_CREATED == IoStatusBlock.Information )
		{
			ZwWriteFile( hFile, 0, 0, 0, &IoStatusBlock, README, strlen(README), 0, 0 );
		}

		status = ZwClose( hFile );
	}

	return status ;
}



BOOL
CreateSessionDirectoryObject (
	IN PVOID _pNode
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/07 [7:6:2010 - 10:21]

Routine Description:
  ����"\\ProteinBox\\SUDAMI\\DefaultBox\\Session_0"����  
    
--*/
{
	ULONG n = 0 ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPWSTR ptrOld = NULL, ptrCurrent = NULL ;
	OBJECT_ATTRIBUTES ObjectAttributes ;
	UNICODE_STRING uniBuffer ;
	SECURITY_DESCRIPTOR SecurityDescriptor;
	LPWSTR wszLpcRootPath1 = NULL ;
	HANDLE hDirectory  = NULL ;
	LPPDNODE pNode = (LPPDNODE) _pNode ;

	//
	// 1. У������Ϸ���
	//

	if ( NULL == _pNode )
	{
		dprintf( "error! | CreateSessionDirectoryObject(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	//
	// 2. ѭ��������ǰɳ���SessionĿ¼
	//

	wszLpcRootPath1 = pNode->pNode_C->LpcRootPath1 ;
	
	RtlCreateSecurityDescriptor( &SecurityDescriptor, SECURITY_DESCRIPTOR_REVISION1 );
	RtlSetDaclSecurityDescriptor( &SecurityDescriptor, TRUE, 0, FALSE );

	InitializeObjectAttributes( &ObjectAttributes, &uniBuffer, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE | OBJ_PERMANENT, 0, &SecurityDescriptor );

	do
	{
		++ n ;

		RtlInitUnicodeString( &uniBuffer, wszLpcRootPath1 );
		
		status = ZwCreateDirectoryObject( &hDirectory, 0xF000F, &ObjectAttributes );
		if ( STATUS_OBJECT_PATH_NOT_FOUND == status )
		{
			//
			// ����(���ҵ���) ���νض�,���Դ�Ŀ¼,���Ƿ����
			// eg: "c:\Test\1\2\3" ��Ҫ�������ļ���,����ϵͳ��ǰֻ��"c:\Test"
			// �����γ��Դ�(���������½�) "c:\Test\1\2\3" --> "c:\Test\1\2" --> "c:\Test\1"
			// ������ "c:\Test\1" ʱ,�ᴴ���ɹ�
			//

			ptrCurrent = wcsrchr( wszLpcRootPath1, '\\' );
			if ( ptrOld ) { *ptrOld = '\\' ; } // �ָ��ɵı��ضϵ�·��

			ptrOld = ptrCurrent ; // ����ɵ�·����ǰֵ
			if ( ptrCurrent ) { *ptrCurrent = 0 ; } // �ضϵ�ǰ·��

			continue ;
		}

		if( NT_SUCCESS( status ) ) { ZwClose( hDirectory ); }
		if ( STATUS_OBJECT_NAME_COLLISION == status ) { status = STATUS_SUCCESS ; }

		if ( NULL == ptrOld ) { break ; } // ��ʱ��ptrOld ��Ϊ��ǰ��ptrCurrent,��Ϊ��������ѭ��

		*ptrOld = '\\' ;
		ptrOld = NULL ;     

	} while ( n < 0x40 );

	if ( n >= 0x40 ) { return FALSE ; }
	if( ! NT_SUCCESS( status ) )
	{
		dprintf( " \n" );
		return status ;
	}

	return TRUE ;
}



BOOL
HandlerRegHive (
	IN PVOID _pNode
	)
{
	LPRHNODE pCurrentNode = NULL ;

	//
	// 1. У������Ϸ���
	//

	if ( NULL == _pNode )
	{
		dprintf( "error! | DropAdminRights(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	//
	// 2. ����RegHive�ڵ�
	//

	pCurrentNode = kbuildnodeRH( _pNode );
	if ( NULL == pCurrentNode )
	{
		dprintf( "error! | HandlerRegHive() - kbuildnodeRH(); | ����RegHive�ڵ�ʧ�� \n" );
		return FALSE ;
	}

// 	dprintf( 
// 		"ok! | HandlerRegHive() | �½�����RegHive�ڵ���Ϣ����:"
// 		"\nHiveRegPath:\"%ws\" \nHiveFilePath:\"%ws\" \n",
// 		pCurrentNode->HiveRegPath, pCurrentNode->HiveFilePath
// 		);

	return TRUE ;
}



BOOL 
Check_IsValid_Characters(
	IN LPWSTR pBuffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/02 [2:6:2010 - 14:49]

Routine Description:
  �ж��ַ����ĺϷ���,����R3������Ĭ�ϵ�ɳ����,���ǻ����ַ���,���޷�������Ӧ���ļ���  
  �������0~9 | a~z | A~Z ֮��

Arguments:
  pBuffer - �������ַ���

Return Value:
  TRUE - �Ϸ�; FALSE - �Ƿ�

--*/
{
	ULONG n ;
	unsigned __int16 tmp ; 

	n = 0;
	do
	{
		tmp = pBuffer[n];
		if ( !tmp )
			break;

		if ( (tmp < '0' || tmp > '9') && (tmp < 'A' || tmp > 'Z') && (tmp < 'a' || tmp > 'z') && tmp != '_' )
			return FALSE ;

		++ n ;
	}
	while ( n < 0x20 );

	if ( n && !pBuffer[n] )
		return TRUE ;

	return FALSE ;
}



PVOID
ParseCharacters (
	IN PVOID pInBuffer
	)
{

	return pInBuffer ;
}

///////////////////////////////   END OF FILE   ///////////////////////////////