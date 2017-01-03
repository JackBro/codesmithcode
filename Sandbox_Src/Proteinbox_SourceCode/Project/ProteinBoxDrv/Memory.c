/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/05/17 [17:5:2010 - 15:16]
* MODULE : d:\Work\Program\Coding\ɳ��\SandBox\Code\Project\ProteinBoxDrv\Memory.c
* 
* Description:
*      
*   �ڴ����ģ��                       
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "Common.h"
#include "Memory.h"

//////////////////////////////////////////////////////////////////////////

LPMMHEAD g_ListHead__MemoryManager = NULL  ;
BOOL	 g_MemoryManager_Inited_ok = FALSE ;


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
InitMemoryManager (
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/01 [1:6:2010 - 18:36]

Routine Description:
  ��������ͷ    

--*/
{
	BOOL bRet = FALSE ;

	if ( FALSE == g_MemoryManager_Inited_ok )
	{
		bRet = MMCreateTotalHead( (PVOID) &g_ListHead__MemoryManager );
		g_MemoryManager_Inited_ok = bRet ;

		if ( FALSE == bRet )
		{
			dprintf( "error! | InitMemoryManager(); \n" );
		}
	}

	return bRet ;
}



BOOL 
MMCreateTotalHead(
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
	LPMMHEAD *pTotalHead = (LPMMHEAD*) _TotalHead ;
	if ( NULL !=  *pTotalHead ) { return TRUE ; }

	// Ϊ�ܽṹ�����ڴ�
	*pTotalHead = (LPMMHEAD) kmalloc( sizeof( MMHEAD ) );
	if ( NULL == *pTotalHead )
	{
		dprintf( "error! | MMCreateTotalHead(); Allocate memory failed! \n" );
		return FALSE ;
	}

	RtlZeroMemory( *pTotalHead, sizeof( MMHEAD ) );

	// ��ʼ����Դ��
	bRet = InitResource( &((LPMMHEAD)*pTotalHead)->QueueLockList );
	if ( FALSE == bRet )
	{
		dprintf( "error! | MMCreateTotalHead() - InitResource(); ������Դ���ڴ�ʧ��! \n" );
		kfree( (PVOID) *pTotalHead );
		return FALSE ;
	}

	// ��ʼ������ͷ
	InitializeListHead( &( (LPMMHEAD)*pTotalHead )->ListHead.MemoryListEntry );

	((LPMMHEAD)*pTotalHead)->nTotalCounts = 0 ;

	return TRUE ;
}



VOID 
MMDeleteTotalHead(
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
MMAllocateNode(
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
	LPMMNODE* pCurrenList = (LPMMNODE*) _pCurrenList_ ;
	*pCurrenList = (LPMMNODE) kmalloc( sizeof( MMNODE ) );

	if ( NULL == *pCurrenList )
	{
		dprintf( "error! | MMAllocateNode() | Allocate memory failed! \n" );
		return FALSE ;
	}

	RtlZeroMemory( *pCurrenList, sizeof( MMNODE ) );

	return TRUE ;
}



ULONG
MMBuildNode (
	IN PVOID _TotalHead ,
	IN ULONG nSize,
	IN ULONG Tag
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/17 [17:5:2010 - 18:09]

Routine Description:
  ����@nSize�Ĵ�С�����ڴ�,���ڴ�ڵ����������,�����ڴ��ַ
    
Arguments:
  _TotalHead - �ܵ�����ͷ
  nSize - ��������ڴ��С

Return Value:
  ������ڴ��ַ
    
--*/
{
	ULONG pAddress	 = 0	;
	LPMMNODE pNewMember = NULL, pCurrent = NULL ;
	LPMMHEAD pTotalHead = (LPMMHEAD) _TotalHead ;

	//
	// 1. У������Ϸ���
	//

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList || 0 == nSize )
	{
		dprintf( "error! | MMBuildNode() | Invalid Parameters \n" );
		return 0 ;
	}

	//
	// 2. �����������Ƿ��Ѵ��ڸýڵ�; ������ֱ�ӷ���֮
	//

	pCurrent = (LPMMNODE) MMFindNode( _TotalHead, Tag, nSize );
	if ( NULL != pCurrent ) { return pCurrent->pAddress ; }

	//
	// 3. û�иýڵ�,�½�֮; �����ڴ�
	//

	pAddress = (ULONG) kmalloc( nSize );
	if ( 0 == pAddress )
	{
		dprintf( "error! | MMBuildNode() | Allocate memory failed! \n" );
		return 0 ;
	}

	RtlZeroMemory( (PVOID)pAddress, nSize );

	//
	// 3. ���뵽������
	//

	pNewMember = (LPMMNODE) MMBuildNodeEx( pTotalHead, nSize, pAddress, Tag );
	if ( NULL == pNewMember )
	{
		dprintf( "error! | MMBuildNode() - MMBuildNodeEx() | NULL == pNode; Memory Tag = %d \n", Tag );
		kfree( (PVOID)pAddress );
		return 0 ;
	}

	dprintf( "[Memory Tag: %d] �����ڵ��Ѳ�������; \n", Tag );
	return pAddress ;
}



PVOID
MMBuildNodeEx (
	IN PVOID _TotalHead ,
	IN ULONG nSize,
	IN ULONG pAddress,
	IN ULONG Tag
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/04/29 [29:4:2009 - 14:01]

Routine Description:
  ������������һ���ڵ� 
    
Arguments:
  _TotalHead - �ܵ�����ͷ
  nSize - ��������ڴ��С
  pAddress - ��������ڴ��ַ

--*/
{
	LPMMNODE pCurrent = NULL, pNode = NULL ;
	LPMMHEAD pTotalHead = (LPMMHEAD) _TotalHead ;

	//
	// 1. У������Ϸ���
	//

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList || 0 == nSize || 0 == pAddress )
	{
		dprintf( "error! | MMBuildNodeEx() | Invalid Parameters \n" );
		return NULL ;
	}

	//
	// 2. �����ڴ�,���ڵ�
	//

	if ( FALSE == MMAllocateNode( &pNode ) || NULL == pNode )
	{
		dprintf( "error! | MMBuildNodeEx() | MMAllocateNode() failed \n" );
		return NULL ;
	}

	pNode->nSize	= nSize		;
	pNode->pAddress	= pAddress	;
	pNode->Tag		= Tag		;

	//
	// 3. ����Ҫ,������½ڵ㵽����β
	//

	EnterCrit( pTotalHead->QueueLockList );	// ��������

	if ( 0 == pTotalHead->ListHead.nSize && 0 == pTotalHead->ListHead.pAddress ) 
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
		InsertTailList( (PLIST_ENTRY)&pTotalHead->ListHead, (PLIST_ENTRY)pNode );
		pTotalHead->nTotalCounts ++ ;
	}

	LeaveCrit( pTotalHead->QueueLockList );	// �ͷ���
	return (PVOID)pNode ;
}



ULONG
MMFindNode (
	IN PVOID _TotalHead ,
	IN ULONG Tag,
	IN ULONG nSize OPTIONAL
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/17 [17:5:2010 - 18:27]

Routine Description:
  �������в���ָ���ڴ��ַ,����֮
    
Arguments:
  _TotalHead - �ܵ�����ͷ
  Tag - ��ƥ����ڴ��ַ���
  nSize - �������ڴ�������ͬ��TAGʱ��,��Ҫ�����ڴ���С�����ҵ�ָ�����ڴ��

Return Value:
  �����ҵ��ڴ��ַ
    
--*/
{
	ULONG  pAddress	 = 0 ;
	LPMMNODE pResult	 = NULL ;
	LPMMHEAD pTotalHead = (LPMMHEAD) _TotalHead ;

	// 1. У������Ϸ���
	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList || -1 == Tag ) { return 0 ; }

	// 2. ����Tag��Ӧ���ڴ�ڵ�
	pResult = MMFindNodeEx( pTotalHead, Tag, nSize );
	if ( NULL == pResult ) { return 0 ; }

	// 3. ���ض�Ӧ���ڴ��ַ
	pAddress = pResult->pAddress ;
	return pAddress ;
}



PVOID
MMFindNodeEx (
	IN PVOID _TotalHead ,
	IN ULONG Tag,
	IN ULONG nSize OPTIONAL
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/04/29 [29:4:2009 - 14:01]

Routine Description:
  �������в���ָ���ڵ�,����֮
    
Arguments:
  _TotalHead - �ܵ�����ͷ
  Tag - ��ƥ����ڴ��ַ���
  nSize - �������ڴ�������ͬ��TAGʱ��,��Ҫ�����ڴ���С�����ҵ�ָ�����ڴ��

--*/
{
	PVOID pResult = NULL ;
	LPMMNODE pHead = NULL, pNode = NULL ;
	LPMMHEAD pTotalHead = (LPMMHEAD) _TotalHead ;

	//
	// 1. У������Ϸ���
	//
	
	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList || -1 == Tag ) { return NULL ; }

	if ( 0 == pTotalHead->nTotalCounts /* || TRUE == IsListEmpty( &pTotalHead->ListHead.MemoryListEntry )*/ ) { return NULL ; }

	//
	// 2. ��������,����ָ���ڵ�
	//

	EnterCrit( pTotalHead->QueueLockList );	// ��������
	pNode = pHead = &pTotalHead->ListHead ;

	do 
	{
		if ( Tag == pNode->Tag ) 
		{
			if ( nSize ) // ��TAG��ͬ��ǰ����,��nSize����,˵��Ҫ��һ���Ƚ��ڴ��С!�ҵ��ڴ��С��ͬ�Ľڵ�,����֮
			{
				if ( nSize == pNode->nSize )
				{
					pResult = pNode ;
					break ;
				}
			} 
			else // ���
			{
				pResult = pNode ;
				break ;
			}
		}

		pNode = (LPMMNODE) pNode->MemoryListEntry.Flink ;
	} while ( pNode != pHead );

	LeaveCrit( pTotalHead->QueueLockList );	// ��������
	return pResult ;
}



BOOL
MMDeleteNode (
	IN PVOID _TotalHead ,
	IN ULONG pAddress
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2009/04/29 [29:4:2009 - 14:01]

Routine Description:
  ɾ��������ָ���ڵ�,���ڴ��ַ����ƥ��
    
Arguments:
  _TotalHead - �ܵ�����ͷ
  pAddress - ��ƥ����ڴ��ַ

Return Value:
  BOOL

--*/
{
	BOOL bRet = FALSE ;
	PLIST_ENTRY ListHead = NULL, NextEntry = NULL ;
	LPMMNODE		pCurrent = NULL ;
	LPMMHEAD pTotalHead = (LPMMHEAD) _TotalHead ;

	//
	// 1. У������Ϸ���
	//

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList || 0 == pAddress )
	{
		dprintf( "error! | MMDeleteNode() | Invalid Parameters \n" );
		return FALSE ;
	}

	//
	// 2. ��������,ɾ��ָ���ڵ�
	//

	EnterCrit( pTotalHead->QueueLockList );	// ��������
	NextEntry = ListHead = &pTotalHead->ListHead.MemoryListEntry ;

	do 
	{
		pCurrent = CONTAINING_RECORD( NextEntry, MMNODE, MemoryListEntry );

		if ( pAddress == (ULONG)pCurrent->pAddress ) 
		{
			RemoveEntryList( NextEntry );

			kfree( (PVOID)pAddress );
			kfree( (PVOID)pCurrent );
			pTotalHead->nTotalCounts -- ;

			bRet = TRUE ;
			break ;
		}

		NextEntry = ListHead->Blink;
	} while ( NextEntry != ListHead );

	LeaveCrit( pTotalHead->QueueLockList );	// ��������
	return bRet ;
}



VOID
MMDistroyAll (
	IN PVOID _TotalHead
	)
{
	int i = 0 ;
	LPMMNODE pHead = NULL, pNode = NULL ;
	LPMMHEAD pTotalHead = (LPMMHEAD) _TotalHead ;

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | MMDistroyAll() | Invalid ListHead \n" );
		return ;
	}

	//
	// 1. ɾ�����е��ӽڵ�
	//

	MMWalkNodes( pTotalHead );

	dprintf( "*** ��ʼж���ڴ������ *** \n" );

	EnterCrit( pTotalHead->QueueLockList );	// ��������
	pNode = pHead = &pTotalHead->ListHead ;

	do
	{
		kfree( (PVOID) pNode->pAddress );

		if ( pHead != pNode )
		{
			dprintf( "  [%d] MemoryNode:0x%08lx \n", i++, (PVOID)pNode );

			RemoveEntryList( (PLIST_ENTRY) pNode );
			kfree( (PVOID)pNode );
		}

		pNode = (LPMMNODE) pHead->MemoryListEntry.Flink ;
	} while ( FALSE == IsListEmpty( (PLIST_ENTRY) pHead ) );

	LeaveCrit( pTotalHead->QueueLockList );	// ��������
	ExDeleteResource( pTotalHead->QueueLockList );
	kfree( pTotalHead->QueueLockList );

	//
	// 2. ɾ���ܽڵ�
	//

	MMDeleteTotalHead( &_TotalHead );

	dprintf( "*** ����ж���ڴ������ *** \n" );
	return ;
}



VOID
MMWalkNodes (
	IN PVOID _TotalHead
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/17 [17:5:2010 - 17:13]

Routine Description:
  ��ӡ���нڵ���Ϣ
    
--*/
{
	INT i = 1 ;
	LPMMNODE pHead = NULL, pNode = NULL ;
	LPMMHEAD pTotalHead = (LPMMHEAD) _TotalHead ;

#ifdef DBG

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | MMWalkNodes() | Invalid ListHead \n" );
		return ;
	}

	if ( 0 == pTotalHead->nTotalCounts )
	{
		dprintf( "error! | MMWalkNodes() | ����Ϊ��,���ɱ��� \n" );
		return ;
	}

	dprintf( "\n**** Starting walking Memory Lists **** \n" );

	EnterCrit( pTotalHead->QueueLockList );	// ��������
	pNode = pHead = &pTotalHead->ListHead ;

	do
	{
		dprintf( 
			"[%d] nSize = 0x%x, pAddress = 0x%08lx \n",
			i++,
			pNode->nSize,
			pNode->pAddress
			);

		pNode = (LPMMNODE) pNode->MemoryListEntry.Flink ;
		if ( NULL == pNode ) { break ; }
	} while ( pNode != pHead );

	LeaveCrit( pTotalHead->QueueLockList );	// ��������
	dprintf( "**** End of walking Memory Lists **** \n\n" );

#endif
	return ;
}

///////////////////////////////   END OF FILE   ///////////////////////////////