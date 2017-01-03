/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/07/16 [16:7:2010 - 10:51]
* MODULE : d:\Work\Program\Coding\ɳ��\SandBox\Code\Project\ProteinBoxDrv\GrayList.c
* 
* Description:
*      
*   �������������,�ͽ��̽ڵ������                       
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
#include "SandboxsList.h"

//////////////////////////////////////////////////////////////////////////

LPSANDBOX_NODE_INFO_HEAND g_ListHead__Sandboxs = NULL ;
static BOOL g_SandboxLists_Inited_ok = FALSE ;

static const ULONG g_nSandboxMaxCouts = 5 ; // ���ƽ��ܴ���<=5��ɳ��


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
InitSandboxsData (
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/01 [1:6:2010 - 18:36]

Routine Description:
  ��������ͷ    

--*/
{
	BOOL bRet = FALSE ;

	if ( FALSE == g_SandboxLists_Inited_ok )
	{
		bRet = SBLCreateTotalHead( (PVOID) &g_ListHead__Sandboxs );
		g_SandboxLists_Inited_ok = bRet ;

		if ( FALSE == bRet )
		{
			dprintf( "error! | InitProcessData(); \n" );
		}
	}

	return bRet ;
}



int SBLGetSandboxsCounts()
{
	int nTotalCounts = 0 ;

	if ( g_ListHead__Sandboxs )
	{
		EnterCrit( g_ListHead__Sandboxs->QueueLockList );	// ��������

		nTotalCounts = g_ListHead__Sandboxs->nTotalCounts ;

		LeaveCrit( g_ListHead__Sandboxs->QueueLockList );	// �������� 
	}

	return nTotalCounts;
}



BOOL 
SBLCreateTotalHead(
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
	LPSANDBOX_NODE_INFO_HEAND *pTotalHead = (LPSANDBOX_NODE_INFO_HEAND*) _TotalHead ;
	if ( NULL !=  *pTotalHead ) { return TRUE ; }

	// Ϊ�ܽṹ������ڴ�
	*pTotalHead = (LPSANDBOX_NODE_INFO_HEAND) kmalloc( sizeof( SANDBOX_NODE_INFO_HEAND ) );
	if ( NULL == *pTotalHead )
	{
		dprintf( "error! | SBLCreateTotalHead(); Allocate memory failed! \n" );
		return FALSE ;
	}

	RtlZeroMemory( *pTotalHead, sizeof( SANDBOX_NODE_INFO_HEAND ) );

	// ��ʼ����Դ��
	bRet = InitResource( &((LPSANDBOX_NODE_INFO_HEAND)*pTotalHead)->QueueLockList );
	if ( FALSE == bRet )
	{
		dprintf( "error! | SBLCreateTotalHead() - InitResource(); ������Դ���ڴ�ʧ��! \n" );
		kfree( (PVOID) *pTotalHead );
		return FALSE ;
	}

	// ��ʼ������ͷ
	InitializeListHead( (PLIST_ENTRY)&( (LPSANDBOX_NODE_INFO_HEAND)*pTotalHead )->ListHead );

	((LPSANDBOX_NODE_INFO_HEAND)*pTotalHead)->nTotalCounts = 0 ;
	return TRUE ;
}



VOID 
SBLDeleteTotalHead(
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
SBLAllocateNode(
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
	ULONG Length = sizeof( SANDBOX_NODE_INFO );

	if ( NULL == pNode )
	{
		dprintf( "error! | SBLAllocateNode(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	*pNode = kmalloc( Length );
	if ( NULL == *pNode )
	{
		dprintf( "error! | SBLAllocateNode() | Allocate memory failed! \n" );
		return FALSE ;
	}

	RtlZeroMemory( *pNode, Length );
	return TRUE ;
}



PVOID
SBLBuildNode (
	IN PVOID _TotalHead,
	IN LPWSTR wszName,
	IN ULONG NameLength,
	OUT BOOL* bOverload
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/07/05 [5:7:2010 - 17:03]

Routine Description:
  ע��:@ListHead ����ͷ����Զ����洢wszName,�������ò��������ڵ�. �ʱ����������������ʱ,�����ע��ͷ�д洢������
    
Arguments:
  _TotalHead - �����ļ�������ͷ
  wszName - ɳ����
  NameLength - ɳ��������
  bOverload - ��ɳ��������������,��TRUE	
    
--*/
{
	BOOL bRet = FALSE ;
	LPSANDBOX_NODE_INFO pNode = NULL ;
	LPSANDBOX_NODE_INFO_HEAND pTotalHead = (LPSANDBOX_NODE_INFO_HEAND) _TotalHead ;
	
	// 1. У������Ϸ���
	*bOverload = FALSE;
	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList || NULL == wszName || NameLength <= 0 || NameLength > MAX_PATH )
	{
		dprintf( "error! | SBLBuildNode(); | Invalid Paramaters; failed! \n" );
		return NULL ;
	}

	if ( pTotalHead->nTotalCounts > g_nSandboxMaxCouts )
	{
		*bOverload = TRUE;
		dprintf( "error! | SBLBuildNode(); | ɳ��������������,�ֲ��贴����ɳ�� \n" );
		return NULL ;
	}

	pNode = kgetnodeSBL( wszName, NameLength );
	if ( pNode ) 
	{ 
		dprintf( "ok! | SBLBuildNode(); | ��ɳ���Ѵ���,����������� \n" );
		return pNode; 
	}

	// 2. ����ڵ�
	bRet = SBLAllocateNode( &pNode );
	if ( FALSE == bRet )
	{
		dprintf( "error! | SBLBuildNode() - SBLAllocateNode() | �����ڴ�ʧ�� \n" );
		return NULL ;
	}

	// 3. ���֮
	pNode->wszName = (LPWSTR) kmalloc( NameLength );
	if ( NULL == pNode->wszName )
	{
		dprintf( "error! | SBLBuildNode() - kmalloc(); | �����ڴ�ʧ��! \n" );
		kfree( (PVOID)pNode ); // �ͷ��ڴ�
		return NULL ;
	}

	RtlZeroMemory( pNode->wszName, NameLength );
	RtlCopyMemory( pNode->wszName, wszName, NameLength );

	pNode->NameLength = NameLength ;
	pNode->Index = pTotalHead->nTotalCounts + 1 ;

	// 4. ����������
	kInsertTailSB( pNode );
	return (PVOID)pNode ;
}



PVOID
SBLFindNode (
	IN PVOID _TotalHead ,
	IN LPWSTR wszName,
	IN ULONG NameLength
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
	LPSANDBOX_NODE_INFO pNodeHead = NULL, pCurrentNode = NULL ;
	LPSANDBOX_NODE_INFO_HEAND pTotalHead = (LPSANDBOX_NODE_INFO_HEAND) _TotalHead ;

	//
	// 1. У������Ϸ���
	//

	if ( (NULL == pTotalHead) || NULL == pTotalHead->QueueLockList || NULL == wszName || NameLength <= 0 )
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
	pCurrentNode = (LPSANDBOX_NODE_INFO) pNodeHead->ListEntry.Blink ;

	while ( pCurrentNode != pNodeHead )
	{
		if ( NameLength == pCurrentNode->NameLength && 0 == _wcsicmp(wszName, pCurrentNode->wszName) ) 
		{
			pResult = (PVOID) pCurrentNode ;
			break ;
		}

		pCurrentNode = (LPSANDBOX_NODE_INFO) pCurrentNode->ListEntry.Blink ;
	}

	LeaveCrit( pTotalHead->QueueLockList );	// ��������
	return pResult ;
}



PVOID
SBLFindNodeEx (
	IN PVOID _TotalHead ,
	IN ULONG Index
	)
{
	PVOID pResult = NULL ;
	LPSANDBOX_NODE_INFO pNodeHead = NULL, pCurrentNode = NULL ;
	LPSANDBOX_NODE_INFO_HEAND pTotalHead = (LPSANDBOX_NODE_INFO_HEAND) _TotalHead ;

	// 1. У������Ϸ���
	if ( (NULL == pTotalHead) || NULL == pTotalHead->QueueLockList || Index < 1 || Index > g_nSandboxMaxCouts ) { return NULL; }

	if ( (0 == pTotalHead->nTotalCounts) || (TRUE == IsListEmpty( (PLIST_ENTRY) &pTotalHead->ListHead )) )
	{
		return NULL ;
	}

	// 2. ��������,����ָ���ڵ�
	EnterCrit( pTotalHead->QueueLockList );	// ��������

	pNodeHead    =  &pTotalHead->ListHead ;
	pCurrentNode = (LPSANDBOX_NODE_INFO) pNodeHead->ListEntry.Blink ;

	while ( pCurrentNode != pNodeHead )
	{
		if ( Index == pCurrentNode->Index ) 
		{
			pResult = (PVOID) pCurrentNode ;
			break ;
		}

		pCurrentNode = (LPSANDBOX_NODE_INFO) pCurrentNode->ListEntry.Blink ;
	}

	LeaveCrit( pTotalHead->QueueLockList );	// ��������
	return pResult ;
}



VOID
SBLDistroyAll (
	IN PVOID _TotalHead
	)
{
	int i = 0 ;
	LPSANDBOX_NODE_INFO pCurrentNode = NULL ;
	PLIST_ENTRY ListHead = NULL, CurrentListEntry = NULL ;
	LPSANDBOX_NODE_INFO_HEAND pTotalHead = (LPSANDBOX_NODE_INFO_HEAND) _TotalHead ;

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | SBLDistroyAll(); | Invalid Paramaters; failed! \n" );
		return ;
	}

	// 1. ɾ�����е��ӽڵ�
	dprintf( "*** ��ʼж��SandboxsList�ڵ������ *** \n" );
	EnterCrit( pTotalHead->QueueLockList );	// ��������

	ListHead = (PLIST_ENTRY) &pTotalHead->ListHead ;

	while( FALSE == IsListEmpty( ListHead ) )
	{
		CurrentListEntry = (PLIST_ENTRY) RemoveHeadList( ListHead );
		pCurrentNode = (LPSANDBOX_NODE_INFO) CurrentListEntry ;

		dprintf( "  [%d] Node:0x%08lx, BoxName:%ws \n", i++, (PVOID)pCurrentNode, pCurrentNode->wszName );
		kfree( (PVOID)pCurrentNode->wszName );
		kfree( (PVOID)pCurrentNode );
	}

	LeaveCrit( pTotalHead->QueueLockList );	// ��������

	ExDeleteResource( pTotalHead->QueueLockList );
	kfree( pTotalHead->QueueLockList );

	// 2. ɾ���ܽڵ�
	SBLDeleteTotalHead( &_TotalHead );

//	dprintf( "*** ����ж��SandboxsList�ڵ������ *** \n" );
	return ;
}



VOID
SBLWalkNodes (
	IN PVOID _TotalHead
	)
{
	return ;
}


///////////////////////////////   END OF FILE   ///////////////////////////////