/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/06/11 [11:6:2010 - 16:02]
* MODULE : d:\Work\Program\Coding\ɳ��\SandBox\Code\Project\ProteinBoxDrv\ObjectData.c
* 
* Description:
*      
*   ����OjectHook����                       
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "ObjectData.h"
#include "Common.h"

//////////////////////////////////////////////////////////////////////////

LPOBJECT_DATA_INFO_HEAND g_ListHead__ObjectData = NULL ;
BOOL g_ObjectData_Inited_ok = FALSE ;


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
InitObjectData (
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/01 [1:6:2010 - 18:36]

Routine Description:
  ��������ͷ    

--*/
{
	BOOL bRet = FALSE ;

	if ( FALSE == g_ObjectData_Inited_ok )
	{
		bRet = ODCreateTotalHead( (PVOID) &g_ListHead__ObjectData );
		g_ObjectData_Inited_ok = bRet ;

		if ( FALSE == bRet )
		{
			dprintf( "error! | InitObjectData(); \n" );
		}
	}

	return bRet ;
}



BOOL 
ODCreateTotalHead(
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
	LPODHEAD *pTotalHead = (LPODHEAD*) _TotalHead ;
	if ( NULL !=  *pTotalHead ) { return TRUE ; }

	// Ϊ�ܽṹ������ڴ�
	*pTotalHead = (LPODHEAD) kmalloc( sizeof( ODHEAD ) );
	if ( NULL == *pTotalHead )
	{
		dprintf( "error! | ODCreateTotalHead(); Allocate memory failed! \n" );
		return FALSE ;
	}

	RtlZeroMemory( *pTotalHead, sizeof( ODHEAD ) );

	// ��ʼ����Դ��
	bRet = InitResource( &((LPODHEAD)*pTotalHead)->QueueLockList );
	if ( FALSE == bRet )
	{
		dprintf( "error! | ODCreateTotalHead() - InitResource(); ������Դ���ڴ�ʧ��! \n" );
		kfree( (PVOID) *pTotalHead );
		return FALSE ;
	}

	// ��ʼ������ͷ
	InitializeListHead( (PLIST_ENTRY)&( (LPODHEAD)*pTotalHead )->ListHead );

	((LPODHEAD)*pTotalHead)->nTotalCounts = 0 ;

	return TRUE ;
}



VOID 
ODDeleteTotalHead(
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
ODAllocateNode(
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
	LPODNODE* pCurrenList = (LPODNODE*) _pCurrenList_ ;
	*pCurrenList = (LPODNODE) kmalloc( sizeof( ODNODE ) );

	if ( NULL == *pCurrenList )
	{
		dprintf( "error! | ODAllocateNode() | Allocate memory failed! \n" );
		return FALSE ;
	}

	RtlZeroMemory( *pCurrenList, sizeof( ODNODE ) );

	return TRUE ;
}



PVOID
ODBuildNode (
	IN PVOID _TotalHead,
	IN PVOID pInBuffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/19 [19:5:2010 - 20:11]

Routine Description:
  �½�&���OD�ṹ��
    
Arguments:
  _TotalHead - ����ͷ
  pInBuffer - ��������ʼ�������нڵ���Ϣ

Return Value:
  �½���OD�ṹ��ָ��
    
--*/
{
	BOOL bRet = FALSE ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ;
	PVOID pNode = NULL ;
	LPOBJECT_INIT Buffer = (LPOBJECT_INIT) pInBuffer ;

	//
	// 1. У������Ϸ���
	//

	if ( (NULL == _TotalHead) || (NULL == Buffer) || (FALSE == IS_OBJECT_INDEX( Buffer->ObjectIndex )) )
	{
		dprintf( "error! | ODBuildNode() | Invalid Parameters \n" );
		return NULL ;
	}

	//
	// 2. �����Ƿ����Ѵ��ڵĽڵ� 
	//

	pNode = kgetnodeOD( Buffer->ObjectIndex );
	if ( pNode )
	{ 
		dprintf( "ok! | ODBuildNode() - ODFindNode(); | ����OD�ڵ��Ѵ����������� ObjectIndex=%d \n", Buffer->ObjectIndex );
		return pNode ;
	}

	//
	// 3. ����½ڵ�
	//

	pNode = ODBuildNodeEx( _TotalHead, pInBuffer );

	return (PVOID) pNode ;
}



PVOID
ODBuildNodeEx (
	IN PVOID _TotalHead,
	IN PVOID pInBuffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/03 [3:6:2010 - 14:51]

Arguments:
  _TotalHead - ����ͷ
  pInBuffer - ��������ʼ�������нڵ���Ϣ 

--*/
{
	BOOL bRet = FALSE ;
	LPODNODE pNode = NULL ;
	LPODHEAD pTotalHead	 = (LPODHEAD) _TotalHead ;
	LPOBJECT_INIT Buffer = (LPOBJECT_INIT) pInBuffer ;

	//
	// 1. У������Ϸ���
	//
	
	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList || NULL == Buffer )
	{
		dprintf( "error! | ODBuildNodeEx(); | Invalid Paramaters; failed! \n" );
		return NULL ;
	}

	//
	// 2. �����ڴ�,���ڵ�
	//

	bRet = ODAllocateNode( &pNode );
	if ( FALSE == bRet )
	{
		dprintf( "error! | ODBuildNodeEx() - ODAllocateNode() | �����ڴ�ʧ�� \n" );
		return NULL ;
	}

	pNode->ObjectIndex		= Buffer->ObjectIndex ;
	pNode->TypeInfoOffset	= Buffer->TypeInfoOffset ;
	wcscpy( pNode->szObjecType, Buffer->szObjecType );

	if ( Buffer->FakeAddrOpen )
	{
		pNode->Open.bCare			= TRUE ;
		pNode->Open.bHooked			= FALSE ;
		pNode->Open.OrignalAddr		= 0 ;
		pNode->Open.UnhookPoint		= 0 ;
		pNode->Open.ProcedureOffset = Buffer->ProcedureOffsetOpen ;
		pNode->Open.FakeAddr		= Buffer->FakeAddrOpen ;
	}

	if ( Buffer->FakeAddrParse )
	{
		pNode->Parse.bCare				= TRUE ;
		pNode->Parse.bHooked			= FALSE ;
		pNode->Parse.OrignalAddr		= 0 ;
		pNode->Parse.UnhookPoint		= 0 ;
		pNode->Parse.ProcedureOffset	= Buffer->ProcedureOffsetParse ;
		pNode->Parse.FakeAddr			= Buffer->FakeAddrParse ;
	}

	//
	// 4. ����Ҫ,������½ڵ㵽����β
	//

	EnterCrit( pTotalHead->QueueLockList );	// ��������

	if ( 0 == pTotalHead->ListHead.ObjectIndex && 0 == pTotalHead->ListHead.TypeInfoOffset && NULL == pTotalHead->ListHead.szObjecType ) 
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
	return (PVOID)pNode ;
}



PVOID
ODFindNode (
	IN PVOID _TotalHead ,
	IN ULONG ObjectIndex
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/17 [17:5:2010 - 18:27]

Routine Description:
  �������в���ָ���ڵ�,����֮
    
Arguments:
  _TotalHead - �ܵ�����ͷ
  ObjectIndex - ��ƥ���ֵ

Return Value:
  �����ҵĽڵ㵥Ԫָ��
    
--*/
{
	PVOID  pResult = NULL ;
	LPODNODE pHead = NULL, pNode = NULL ;
	LPODHEAD pTotalHead	   = (LPODHEAD) _TotalHead ;

	//
	// 1. У������Ϸ���
	//

	if ( (NULL == pTotalHead) || (NULL == pTotalHead->QueueLockList) || (FALSE == IS_OBJECT_INDEX( ObjectIndex )) ) { return NULL ; }

	if ( 0 == pTotalHead->nTotalCounts ) { return NULL ; }

	//
	// 2. ��������,����ָ���ڵ�
	//

	EnterCrit( pTotalHead->QueueLockList );	// ��������
	pNode = pHead = &pTotalHead->ListHead ;

	do 
	{
		if ( ObjectIndex == pNode->ObjectIndex ) 
		{
			pResult = (PVOID) pNode ;
			break ;
		}

		pNode = pNode->pBlink ;
	} while ( pNode != pHead );

	LeaveCrit( pTotalHead->QueueLockList ); // �ͷ���
	return pResult ;
}



VOID
ODDistroyAll (
	IN PVOID _TotalHead
	)
{
	int i = 0 ;
	LPODNODE pHead = NULL, pNode = NULL ;
	LPODHEAD pTotalHead = (LPODHEAD) _TotalHead ;

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | ODDistroyAll() | Invalid ListHead \n" );
		return ;
	}

	//
	// 1. ɾ�����е��ӽڵ�
	//

	ODWalkNodes( pTotalHead );

	dprintf( "*** ��ʼж��OD�ڵ������ *** \n" );

	EnterCrit( pTotalHead->QueueLockList );	// ��������
	pNode = pHead = &pTotalHead->ListHead ;

	do
	{
		dprintf( "  [%d] Node:0x%08lx; szObjecType:\"%ws\" \n", i++, (PVOID)pNode, pNode->szObjecType );

		// ж��Hook
		if ( pNode->Open.bHooked )
		{
			*(PULONG) pNode->Open.UnhookPoint = pNode->Open.OrignalAddr ;
			pNode->Open.bHooked = FALSE ;
		}

		if ( pNode->Parse.bHooked )
		{
			*(PULONG) pNode->Parse.UnhookPoint = pNode->Parse.OrignalAddr ;
			pNode->Parse.bHooked = FALSE ;
		}

		if ( pHead != pNode )
		{
			RemoveEntryList( (PLIST_ENTRY) pNode );
			kfree( (PVOID)pNode );
		}

		pNode = pHead->pFlink ;
	} while ( FALSE == IsListEmpty( (PLIST_ENTRY) pHead ) );

	LeaveCrit( pTotalHead->QueueLockList ); // �ͷ���

	ExDeleteResource( pTotalHead->QueueLockList );
	kfree( pTotalHead->QueueLockList );

	//
	// 2. ɾ���ܽڵ�
	//

	ODDeleteTotalHead( &_TotalHead );

	dprintf( "*** ����ж��OD�ڵ������ *** \n" );
	return ;
}



VOID
ODWalkNodes (
	IN PVOID _TotalHead
	)
{
	INT i = 1 ;
	LPODNODE pHead = NULL, pNode = NULL ;
	LPODHEAD pTotalHead = (LPODHEAD) _TotalHead ;

#ifdef DBG

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | ODWalkNodes() | Invalid ListHead \n" );
		return ;
	}

	if ( 0 == pTotalHead->nTotalCounts )
	{
		dprintf( "error! | ODWalkNodes() | ����Ϊ��,���ɱ��� \n" );
		return ;
	}

	dprintf( "\n**** Starting walking ObjectData Lists **** \n" );

	EnterCrit( pTotalHead->QueueLockList );	// ��������
	pNode = pHead = &pTotalHead->ListHead ;

	do
	{
		dprintf( 
			"[ObjectIndex=%d] \n"
			"  szObjecType=\"%ws\" \n"
			"  Open.OrignalAddr: 0x%08lx\n"
			"  Parse.OrignalAddr: 0x%08lx\n",
			pNode->ObjectIndex,
			pNode->szObjecType,
			pNode->Open.OrignalAddr,
			pNode->Parse.OrignalAddr
			);

		pNode = pNode->pFlink ;
	} while ( pNode != pHead );

	LeaveCrit( pTotalHead->QueueLockList ); // �ͷ���
	dprintf( "**** End of walking ObjectData Lists **** \n\n" );

#endif
	return ;
}


///////////////////////////////   END OF FILE   ///////////////////////////////