/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/07/16 [16:7:2010 - 10:51]
* MODULE : d:\Work\Program\Coding\ɳ��\SandBox\Code\Project\ProteinBoxDrv\ForceRunList.c
* 
* Description:
*      
*   ����ǿ��������ɳ���еĽ���                       
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "ProcessData.h"
#include "Common.h"
#include "Version.h"
#include "Security.h"
#include "Config.h"
#include "ForceRunList.h"

//////////////////////////////////////////////////////////////////////////

LPFORCEPROC_NODE_INFO_HEAND g_ListHead__ForceProc ;
static BOOL g_ForceProcLists_Inited_ok = FALSE ;

BOOL g_bForceAll2RunInSandbox = TRUE ; // ��ʼ״̬Ϊ�������ļ���ָ�������н���ǿ��������ɳ����
BOOL g_bAllRunOutofSandbox = FALSE ;  // ��TRUE��ʾ�������ļ���ָ�������н���������ɳ����


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
InitForceProcData (
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/01 [1:6:2010 - 18:36]

Routine Description:
  ��������ͷ    

--*/
{
	BOOL bRet = FALSE ;

	if ( FALSE == g_ForceProcLists_Inited_ok )
	{
		bRet = FPLCreateTotalHead( (PVOID) &g_ListHead__ForceProc );
		g_ForceProcLists_Inited_ok = bRet ;

		if ( FALSE == bRet )
		{
			dprintf( "error! | InitForceProcData(); \n" );
		}
	}

	return bRet ;
}



BOOL 
FPLCreateTotalHead(
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
	LPFORCEPROC_NODE_INFO_HEAND *pTotalHead = (LPFORCEPROC_NODE_INFO_HEAND*) _TotalHead ;
	if ( NULL !=  *pTotalHead ) { return TRUE ; }

	// Ϊ�ܽṹ������ڴ�
	*pTotalHead = (LPFORCEPROC_NODE_INFO_HEAND) kmalloc( sizeof( FORCEPROC_NODE_INFO_HEAND ) );
	if ( NULL == *pTotalHead )
	{
		dprintf( "error! | FPLCreateTotalHead(); Allocate memory failed! \n" );
		return FALSE ;
	}

	RtlZeroMemory( *pTotalHead, sizeof( FORCEPROC_NODE_INFO_HEAND ) );

	// ��ʼ����Դ��
	bRet = InitResource( &((LPFORCEPROC_NODE_INFO_HEAND)*pTotalHead)->QueueLockList );
	if ( FALSE == bRet )
	{
		dprintf( "error! | FPLCreateTotalHead() - InitResource(); ������Դ���ڴ�ʧ��! \n" );
		kfree( (PVOID) *pTotalHead );
		return FALSE ;
	}

	// ��ʼ������ͷ
	InitializeListHead( (PLIST_ENTRY)&( (LPFORCEPROC_NODE_INFO_HEAND)*pTotalHead )->ListHead );

	((LPFORCEPROC_NODE_INFO_HEAND)*pTotalHead)->nTotalCounts = 0 ;
	return TRUE ;
}



VOID 
FPLDeleteTotalHead(
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
FPLAllocateNode(
	OUT PVOID* pNode
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
	ULONG Length = sizeof( FORCEPROC_NODE_INFO );

	if ( NULL == pNode )
	{
		dprintf( "error! | FPLAllocateNode(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	*pNode = kmalloc( Length );
	if ( NULL == *pNode )
	{
		dprintf( "error! | FPLAllocateNode() | Allocate memory failed! \n" );
		return FALSE ;
	}

	RtlZeroMemory( *pNode, Length );
	return TRUE ;
}



PVOID
FPLBuildNode (
	IN PVOID _TotalHead,
	IN LPWSTR wszName,
	IN int NameLength
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/07/05 [5:7:2010 - 17:03]

Routine Description:
  ע��:@ListHead ����ͷ����Զ����洢wszName,�������ò��������ڵ�. �ʱ����������������ʱ,�����ע��ͷ�д洢������
    
Arguments:
  _TotalHead - �����ļ�������ͷ
  wszName - ����ȫ·��
  NameLength - ����ȫ·���ĳ���
    
--*/
{
	BOOL bRet = FALSE ;
	LPFORCEPROC_NODE_INFO pNode = NULL ;
	LPFORCEPROC_NODE_INFO_HEAND pTotalHead = (LPFORCEPROC_NODE_INFO_HEAND) _TotalHead ;
	
	// 1. У������Ϸ���
	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList || NULL == wszName || NameLength <= 0 || NameLength > MAX_PATH )
	{
		dprintf( "error! | FPLBuildNode(); | Invalid Paramaters; failed! \n" );
		return NULL ;
	}

	pNode = kgetnodeFPL( wszName );
	if ( pNode ) 
	{ 
		dprintf( "ok! | FPLBuildNode(); | ��ɳ���Ѵ���,����������� \n" );
		return pNode; 
	}

	// 2. ����ڵ�
	bRet = FPLAllocateNode( &pNode );
	if ( FALSE == bRet )
	{
		dprintf( "error! | FPLBuildNode() - SBLAllocateNode() | �����ڴ�ʧ�� \n" );
		return NULL ;
	}

	// 3. ���֮
	pNode->wszProcFullPath = (LPWSTR) kmalloc( NameLength );
	if ( NULL == pNode->wszProcFullPath )
	{
		dprintf( "error! | FPLBuildNode() - kmalloc(); | �����ڴ�ʧ��! \n" );
		kfree( (PVOID)pNode ); // �ͷ��ڴ�
		return NULL ;
	}

	RtlZeroMemory( pNode->wszProcFullPath, NameLength );
	RtlCopyMemory( pNode->wszProcFullPath, wszName, NameLength );

	pNode->NameLength = NameLength ;
	pNode->bRunInSandbox = FALSE; // �½��ڵ�ʱ,��FALSE��ʾ�ýڵ㲻�ᱻǿ��������ɳ����

	// 4. ����������
	kInsertTailFP( pNode );
	return (PVOID)pNode ;
}



PVOID
FPLFindNode (
	IN PVOID _TotalHead ,
	IN LPWSTR wszName
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/17 [17:5:2010 - 18:27]

Routine Description:
  �������в���ָ��ɳ����,���ض�Ӧ�ڵ�
    
Arguments:
  _TotalHead - �ܵ�����ͷ
  wszName - ��ƥ���ɳ����

Return Value:
  ���ҵ��Ľڵ�
    
--*/
{
	PVOID pResult = NULL ;
	LPFORCEPROC_NODE_INFO pNodeHead = NULL, pCurrentNode = NULL ;
	LPFORCEPROC_NODE_INFO_HEAND pTotalHead = (LPFORCEPROC_NODE_INFO_HEAND) _TotalHead ;

	//
	// 1. У������Ϸ���
	//

	if ( (NULL == pTotalHead) || NULL == pTotalHead->QueueLockList || NULL == wszName )
	{
	//	dprintf( "error! | PDFindNode() | Invalid Parameters \n" );
		return NULL ;
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
	pCurrentNode = (LPFORCEPROC_NODE_INFO) pNodeHead->ListEntry.Blink ;

	while ( pCurrentNode != pNodeHead )
	{
		if ( 0 == _wcsicmp(wszName, pCurrentNode->wszProcFullPath) ) 
		{
			pResult = (PVOID) pCurrentNode ;
			break ;
		}

		pCurrentNode = (LPFORCEPROC_NODE_INFO) pCurrentNode->ListEntry.Blink ;
	}

	LeaveCrit( pTotalHead->QueueLockList );	// ��������
	return pResult ;
}



BOOL
FPLGetState (
	IN LPWSTR wszName,
	IN BOOL* bRunInSandbox
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2011/07/14 [14:7:2011 - 16:36]

Routine Description:
  ��ѯǿ�ƹ��ܵ�״̬
    
Arguments:
  wszName - ��ǰ�������Ľ���ȫ·��
  bRunInSandbox - ����״ֵ̬; ΪTRUE��������ǿ�ƹ���
 
--*/
{
	BOOL bRet = FALSE ;
	LPFORCEPROC_NODE_INFO pNode = NULL ;

	*bRunInSandbox = FALSE ;

	// �������ļ���ƥ��·��
	bRet = kIsValueNameExist( L"GlobalSetting", L"ForceProcess", wszName );
	if ( FALSE == bRet )
	{
		bRet = kIsValueNameExist( L"GlobalSetting", L"ForceFolder", wszName );
	}

	if ( FALSE == bRet ) { return FALSE ; }

	// �������ļ���,��һ����֤�Ƿ���Ҫ"��sb"
	*bRunInSandbox = TRUE;

	if ( g_bForceAll2RunInSandbox ) { return TRUE; }
	if ( g_bAllRunOutofSandbox ) { return FALSE; }
	
	// ������������,��������,�ҵ���Ӧ�ڵ�,���ص�ǰ״̬
	pNode = kgetnodeFPL( wszName );
	if ( pNode ) 
	{ 
		*bRunInSandbox = pNode->bRunInSandbox ; 
	}
	
	return TRUE;
}



VOID
FPLSetStateAll (
	IN PVOID _TotalHead,
	IN BOOL bRunInSandbox
	)
{
	LPFORCEPROC_NODE_INFO pNodeHead = NULL, pCurrentNode = NULL ;
	LPFORCEPROC_NODE_INFO_HEAND pTotalHead = (LPFORCEPROC_NODE_INFO_HEAND) _TotalHead ;

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | FPLSetStateAll(); | Invalid Paramaters; failed! \n" );
		return ;
	}

	EnterCrit( pTotalHead->QueueLockList );	// ��������

	pNodeHead    =  &pTotalHead->ListHead ;
	pCurrentNode = (LPFORCEPROC_NODE_INFO) pNodeHead->ListEntry.Blink ;

	while ( pCurrentNode != pNodeHead )
	{
		pCurrentNode->bRunInSandbox = bRunInSandbox;
		pCurrentNode = (LPFORCEPROC_NODE_INFO) pCurrentNode->ListEntry.Blink ;
	}

	LeaveCrit( pTotalHead->QueueLockList );	// ��������
	return ;
}




VOID
FPLDistroyAll (
	IN PVOID _TotalHead
	)
{
	int i = 0 ;
	LPFORCEPROC_NODE_INFO pCurrentNode = NULL ;
	PLIST_ENTRY ListHead = NULL, CurrentListEntry = NULL ;
	LPFORCEPROC_NODE_INFO_HEAND pTotalHead = (LPFORCEPROC_NODE_INFO_HEAND) _TotalHead ;

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | FPLDistroyAll(); | Invalid Paramaters; failed! \n" );
		return ;
	}

	// 1. ɾ�����е��ӽڵ�
	dprintf( "*** ��ʼж��SandboxsList�ڵ������ *** \n" );
	EnterCrit( pTotalHead->QueueLockList );	// ��������

	ListHead = (PLIST_ENTRY) &pTotalHead->ListHead ;

	while( FALSE == IsListEmpty( ListHead ) )
	{
		CurrentListEntry = (PLIST_ENTRY) RemoveHeadList( ListHead );
		pCurrentNode = (LPFORCEPROC_NODE_INFO) CurrentListEntry ;

		dprintf( "  [%d] Node:0x%08lx, ForceProcName:%ws, state:%n \n", i++, (PVOID)pCurrentNode, pCurrentNode->wszProcFullPath, pCurrentNode->bRunInSandbox );
		kfree( (PVOID)pCurrentNode->wszProcFullPath );
		kfree( (PVOID)pCurrentNode );
	}

	LeaveCrit( pTotalHead->QueueLockList );	// ��������

	ExDeleteResource( pTotalHead->QueueLockList );
	kfree( pTotalHead->QueueLockList );

	// 2. ɾ���ܽڵ�
	FPLDeleteTotalHead( &_TotalHead );

//	dprintf( "*** ����ж��SandboxsList�ڵ������ *** \n" );
	return ;
}



VOID
FPLWalkNodes (
	IN PVOID _TotalHead
	)
{
	return ;
}


///////////////////////////////   END OF FILE   ///////////////////////////////