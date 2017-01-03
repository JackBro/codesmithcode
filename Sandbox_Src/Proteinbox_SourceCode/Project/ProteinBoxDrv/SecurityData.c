/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/06/02 [2:6:2010 - 17:33]
* MODULE : d:\Work\Program\Coding\ɳ��\SandBox\Code\Project\ProteinBoxDrv\SecurityData.c
* 
* Description:
*      
*                        
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "SecurityData.h"
#include "Common.h"

//////////////////////////////////////////////////////////////////////////

LPREGISTRY_USERSID_INFO_HEAND g_ListHead__RegistryUserSID = NULL ;
BOOL g_SecurityDataManager_Inited_ok = FALSE ;


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
InitSecurityData (
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/01 [1:6:2010 - 18:36]

Routine Description:
  ��������ͷ    

--*/
{
	BOOL bRet = FALSE ;

	if ( FALSE == g_SecurityDataManager_Inited_ok )
	{
		bRet = SDCreateTotalHead( (PVOID) &g_ListHead__RegistryUserSID );
		g_SecurityDataManager_Inited_ok = bRet ;

		if ( FALSE == bRet )
		{
			dprintf( "error! | InitSecurityData(); \n" );
		}
	}

	return bRet ;
}



BOOL 
SDCreateTotalHead(
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
	LPSDHEAD *pTotalHead = (LPSDHEAD*) _TotalHead ;
	if ( NULL !=  *pTotalHead ) { return TRUE ; }

	// Ϊ�ܽṹ������ڴ�
	*pTotalHead = (LPSDHEAD) kmalloc( sizeof( SDHEAD ) );
	if ( NULL == *pTotalHead )
	{
		dprintf( "error! | SDCreateTotalHead(); Allocate memory failed! \n" );
		return FALSE ;
	}

	RtlZeroMemory( *pTotalHead, sizeof( SDHEAD ) );

	// ��ʼ����Դ��
	bRet = InitResource( &((LPSDHEAD)*pTotalHead)->QueueLockList );
	if ( FALSE == bRet )
	{
		dprintf( "error! | SDCreateTotalHead() - InitResource(); ������Դ���ڴ�ʧ��! \n" );
		kfree( (PVOID) *pTotalHead );
		return FALSE ;
	}

	// ��ʼ������ͷ
	InitializeListHead( (PLIST_ENTRY)&( (LPSDHEAD)*pTotalHead )->ListHead );

	((LPSDHEAD)*pTotalHead)->nTotalCounts = 0 ;

	return TRUE ;
}



VOID 
SDDeleteTotalHead(
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
SDAllocateNode(
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
	LPSDNODE* pCurrenList = (LPSDNODE*) _pCurrenList_ ;
	*pCurrenList = (LPSDNODE) kmalloc( sizeof( SDNODE ) );

	if ( NULL == *pCurrenList )
	{
		dprintf( "error! | SDAllocateNode() | Allocate memory failed! \n" );
		return FALSE ;
	}

	RtlZeroMemory( *pCurrenList, sizeof( SDNODE ) );

	return TRUE ;
}



PVOID
SDBuildNode (
	IN PVOID _TotalHead,
	IN LPWSTR RegisterUserID
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/19 [19:5:2010 - 20:11]

Routine Description:
  �½�&���SD�ṹ��
    
Arguments:
  _TotalHead - ����ͷ
  RegisterUserID - SID

Return Value:
  �½���SD�ṹ��ָ��
    
--*/
{
	BOOL bRet = FALSE ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ;
	LPSDNODE pNode = NULL ;

	//
	// 1. У������Ϸ���
	//

	if ( NULL == _TotalHead || NULL == RegisterUserID )
	{
		dprintf( "error! | SDBuildNode() | Invalid Parameters \n" );
		return NULL ;
	}

	//
	// 2. �����Ƿ����Ѵ��ڵĽڵ� 
	//

	pNode = (LPSDNODE) kgetnodeSD( RegisterUserID );
	if ( pNode )
	{ 
	//	dprintf( "ok! | SDBuildNode() - SDFindNode(); | ����SD�ڵ��Ѵ����������� SID=%ws \n", RegisterUserID );
		return pNode ;
	}

	//
	// 3. ����½ڵ�
	//

	pNode = SDBuildNodeEx( _TotalHead, RegisterUserID );
	if ( NULL == pNode )
	{
		dprintf( "error! | SDBuildNode() - SDBuildNodeEx() | \n" );
		return NULL ;
	}

	//
	// 4. ��R3������,��ʱ��������
	//

	if ( pNode->Length_RegitryUserSID == 0xFFFFFFFF )
	{
		OBJECT_ATTRIBUTES ObjectAttributes ; 
		UNICODE_STRING uniBuffer ;
		HANDLE Handle = NULL ;

		RtlInitUnicodeString( &uniBuffer, L"\\BaseNamedObjects\\Global\\Proteinbox_WorkEvent" );
		InitializeObjectAttributes( &ObjectAttributes, &uniBuffer, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, 0, 0 );

		status = ZwOpenEvent( &Handle, EVENT_MODIFY_STATE, &ObjectAttributes );
		if( NT_SUCCESS( status ) )
		{
			ZwSetEvent( Handle, 0 );                      // ����Ϊ����״̬
			ZwClose( Handle );
		}
	}

	return (PVOID) pNode ;
}



PVOID
SDBuildNodeEx (
	IN PVOID _TotalHead,
	IN LPWSTR RegisterUserID
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/03 [3:6:2010 - 14:51]

Routine Description:
  ���SD�ṹ��   

--*/
{
	BOOL bRet = FALSE ;
	LPSDNODE pNode = NULL ;
	LPSDHEAD pTotalHead	 = (LPSDHEAD) _TotalHead ;

	//
	// 1. У������Ϸ���
	//
	
	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList || NULL == RegisterUserID )
	{
		dprintf( "error! | SDBuildNodeEx(); | Invalid Paramaters; failed! \n" );
		return NULL ;
	}

	//
	// 2. �����ڴ�,���ڵ�
	//

	bRet = SDAllocateNode( &pNode );
	if ( FALSE == bRet )
	{
		dprintf( "error! | SDBuildNodeEx() - SDAllocateNode() | �����ڴ�ʧ�� \n" );
		return NULL ;
	}

	pNode->Length_RegitryUserSID = 0xFFFFFFFF ;
	pNode->Length_CurrentUserName = 0x12340015 ;
	pNode->Length_CurrentUserName = 0 ;
	pNode->Length_RegitryUserSID = wcslen( RegisterUserID );

	wcscpy( pNode->RegitryUserSID, RegisterUserID );

	//
	// 3. ����Ҫ,������½ڵ㵽����β
	//

	EnterCrit( pTotalHead->QueueLockList );	// ��������

	if ( NULL == pTotalHead->ListHead.RegitryUserSID && 0 == pTotalHead->ListHead.Length_RegitryUserSID ) 
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
SDFindNode (
	IN PVOID _TotalHead ,
	IN LPWSTR RegisterUserID
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/17 [17:5:2010 - 18:27]

Routine Description:
  �������в���ָ���ڵ�,����֮
    
Arguments:
  _TotalHead - �ܵ�����ͷ
  RegisterUserID - ��ƥ���ֵ

Return Value:
  �����ҵĽڵ㵥Ԫָ��
    
--*/
{
	PVOID  pResult = NULL ;
	LPSDNODE pHead = NULL, pNode = NULL ;
	LPSDHEAD pTotalHead	= (LPSDHEAD) _TotalHead ;

	//
	// 1. У������Ϸ���
	//

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList || NULL == RegisterUserID ) { return NULL ; }

	if ( 0 == pTotalHead->nTotalCounts ) { return NULL ; }

	//
	// 2. ��������,����ָ���ڵ�
	//

	EnterCrit( pTotalHead->QueueLockList );	// ��������
	pNode = pHead = &pTotalHead->ListHead ;

	do 
	{
		if ( 0 == _wcsicmp( RegisterUserID, pNode->RegitryUserSID ) ) 
		{
			pResult = (PVOID) pNode ;
			break ;
		}

		pNode = pNode->pBlink ;
	} while ( pNode != pHead );

	LeaveCrit( pTotalHead->QueueLockList );	// ��������
	return pResult ;
}



VOID
SDDistroyAll (
	IN PVOID _TotalHead
	)
{
	int i = 0 ;
	LPSDNODE pHead = NULL, pNode = NULL ;
	LPSDHEAD pTotalHead = (LPSDHEAD) _TotalHead ;

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | SDDistroyAll() | Invalid ListHead \n" );
		return ;
	}

	//
	// 1. ɾ�����е��ӽڵ�
	//

	SDWalkNodes( pTotalHead );

	dprintf( "*** ��ʼж��SID�ڵ������ *** \n" );

	EnterCrit( pTotalHead->QueueLockList );	// ��������
	pNode = pHead = &pTotalHead->ListHead ;

	do
	{
		dprintf( "  [%d] Node:0x%08lx; RegitryUserSID:\"%ws\" \n", i++, (PVOID)pNode, pNode->RegitryUserSID );

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

	SDDeleteTotalHead( &_TotalHead );

	dprintf( "*** ����ж��SID�ڵ������ *** \n" );
	return ;
}



VOID
SDWalkNodes (
	IN PVOID _TotalHead
	)
{
	int i = 1 ;
	LPSDNODE pHead = NULL, pNode = NULL ;
	LPSDHEAD pTotalHead	= (LPSDHEAD) _TotalHead ;

#ifdef DBG

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | SDWalkNodes() | Invalid ListHead \n" );
		return ;
	}

	if ( 0 == pTotalHead->nTotalCounts )
	{
		dprintf( "error! | SDWalkNodes() | ����Ϊ��,���ɱ��� \n" );
		return ;
	}

	dprintf( "\n**** Starting walking SecurityData Lists **** \n" );

	EnterCrit( pTotalHead->QueueLockList );	// ��������
	pNode = pHead = &pTotalHead->ListHead ;

	do
	{
		dprintf( 
			"[%d] RegitryUserSID=\"%ws\", CurrentUserName=\"%ws\" \n",
			i++,
			pNode->RegitryUserSID,
			pNode->CurrentUserName
			);

		pNode = pNode->pFlink ;
		if ( NULL == pNode ) { break ; }
	} while ( pNode != pHead );

	LeaveCrit( pTotalHead->QueueLockList );	// ��������
	dprintf( "**** End of walking SecurityData Lists **** \n\n" );

#endif
	return ;
}


///////////////////////////////   END OF FILE   ///////////////////////////////