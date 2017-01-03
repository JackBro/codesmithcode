/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/06/07 [7:6:2010 - 15:55]
* MODULE : d:\Work\Program\Coding\ɳ��\SandBox\Code\Project\ProteinBoxDrv\RegHiveData.c
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
#include "RegHiveData.h"
#include "ProcessData.h"
#include "SdtData.h"
#include "Security.h"
#include "Memory.h"
#include "Common.h"

//////////////////////////////////////////////////////////////////////////


LPREGHIVE_NODE_INFO_HEAND g_ListHead__RegHive = NULL ;
BOOL g_RegHive_Inited_ok = FALSE ;

/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


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
InitRegHive (
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/01 [1:6:2010 - 18:36]

Routine Description:
  ��������ͷ    

--*/
{
	BOOL bRet = FALSE ;

	if ( FALSE == g_RegHive_Inited_ok )
	{
		bRet = RHCreateTotalHead( (PVOID) &g_ListHead__RegHive );
		g_RegHive_Inited_ok = bRet ;

		if ( FALSE == bRet )
		{
			dprintf( "error! | InitRegHive(); \n" );
		}
	}

	return bRet ;
}



BOOL 
RHCreateTotalHead(
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
	LPRHHEAD *pTotalHead = (LPRHHEAD*) _TotalHead ;
	if ( NULL !=  *pTotalHead ) { return TRUE ; }

	// Ϊ�ܽṹ������ڴ�
	*pTotalHead = (LPRHHEAD) kmalloc( sizeof( RHHEAD ) );
	if ( NULL == *pTotalHead )
	{
		dprintf( "error! | RHCreateTotalHead(); Allocate memory failed! \n" );
		return FALSE ;
	}

	RtlZeroMemory( *pTotalHead, sizeof( RHHEAD ) );

	// ��ʼ����Դ��
	bRet = InitResource( &((LPRHHEAD)*pTotalHead)->QueueLockList );
	if ( FALSE == bRet )
	{
		dprintf( "error! | RHCreateTotalHead() - InitResource(); ������Դ���ڴ�ʧ��! \n" );
		kfree( (PVOID) *pTotalHead );
		return FALSE ;
	}

	// ��ʼ������ͷ
	InitializeListHead( (PLIST_ENTRY)&( (LPRHHEAD)*pTotalHead )->ListHead );

	((LPRHHEAD)*pTotalHead)->nTotalCounts = 0 ;

	return TRUE ;
}



VOID 
RHDeleteTotalHead(
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
RHAllocateNode(
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
	LPRHNODE* pCurrenList = (LPRHNODE*) _pCurrenList_ ;
	*pCurrenList = (LPRHNODE) kmalloc( sizeof( RHNODE ) );

	if ( NULL == *pCurrenList )
	{
		dprintf( "error! | RHAllocateNode() | Allocate memory failed! \n" );
		return FALSE ;
	}

	RtlZeroMemory( *pCurrenList, sizeof( RHNODE ) );

	return TRUE ;
}



PVOID
RHBuildNode (
	IN PVOID _TotalHead,
	IN PVOID _ProcessNode
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/19 [19:5:2010 - 20:11]

Routine Description:
  �½�&���RH�ṹ��
    
Arguments:
  _TotalHead - ����ͷ
  _ProcessNode - ��ǰ���̵��ܽڵ�

Return Value:
  �½���RH�ṹ��ָ��
    
--*/
{
	BOOL bRet = FALSE ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ;
	LPRHNODE pNode = NULL ;
	LPRHHEAD pTotalHead	 = (LPRHHEAD) _TotalHead ;
	LPPDNODE ProcessNode = (LPPDNODE) _ProcessNode ;
	LPWSTR HiveRegPath = NULL ;
	LPWSTR HiveFilePath = NULL ;
	WCHAR __HiveFilePath [ MAX_PATH ] = L"" ;

	//
	// 1. У������Ϸ���
	//

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList || NULL == ProcessNode || NULL == ProcessNode->pNode_C )
	{
		dprintf( "error! | RHBuildNode() | Invalid Parameters \n" );
		return NULL ;
	}

	HiveRegPath	 = ProcessNode->pNode_C->KeyRootPath  ;
	HiveFilePath = ProcessNode->pNode_C->FileRootPath ;
	if (  NULL == HiveRegPath || NULL == HiveFilePath )
	{
		dprintf( "error! | RHBuildNode() | Invalid Parameters,(ProcessNode) \n" );
		return NULL ;
	}

	wcscpy( __HiveFilePath, HiveFilePath );
	wcscat( __HiveFilePath, L"\\RegHive" );

	//
	// ��ѭ������ǰɳ���н��̼��ͬ��
	//

	while ( TRUE )
	{
		//
		// 2. �����Ƿ����Ѵ��ڵĽڵ� 
		//

		pNode = (LPRHNODE) kgetnodeRH( HiveRegPath, __HiveFilePath, &bRet );
		if ( FALSE == bRet )
		{
			dprintf( "error! | RHBuildNode() - RHFindNode(); | ���ü���ԭ��,���Ϸ� \n" );
			return NULL ;
		}

		if ( pNode )
		{ 
			// 2.1 �Ѵ��ڽڵ�
		//	dprintf( "ok! | RHBuildNode() - RHFindNode(); | ��ǰRH�ڵ��Ѵ����������� \n" );
		}
		else
		{
			// 2.2 ����½ڵ�
			pNode = (LPRHNODE) RHBuildNodeEx( _TotalHead, HiveRegPath, __HiveFilePath );
			if ( NULL == pNode )
			{
				dprintf( "error! | RHBuildNode() - RHBuildNodeEx() | \n" );
				return NULL ;
			}
		}

		//
		// 3. ���Hiveע������,�������򴴽���Ӧ��Hive�ļ� [��δ�����ѿ� (�s3�t) ]
		//

		EnterCrit( pTotalHead->QueueLockList );	// ��������

		if (  pNode->ProcessesLock ) { goto _WHILE_NEXT_ ; }
		
		if ( pNode->PorcessRefCounts ) 
		{
_ok_ :
			pNode->PorcessRefCounts ++ ;
			pNode->bNeedToDistroy = FALSE ;
			ProcessNode->pNode_RegHive = (PVOID) pNode;

			LeaveCrit( pTotalHead->QueueLockList );	// ��������
			break ; 
		}

		pNode->ProcessesLock = 1 ;
		
		LeaveCrit( pTotalHead->QueueLockList );	// ��������
		bRet = CheckRegHive( (PVOID)pNode, (PVOID)ProcessNode );
		EnterCrit( pTotalHead->QueueLockList );	// ��������

		pNode->ProcessesLock = 0 ;

		if ( TRUE == bRet ) 
		{ 
			goto _ok_ ;
		}
		else
		{
			LeaveCrit( pTotalHead->QueueLockList );	// ��������
			return NULL ;
		}

_WHILE_NEXT_:
		LeaveCrit( pTotalHead->QueueLockList );	// ��������
		ZwYieldExecution();
	} // end of while
	
	return (PVOID) pNode ;
}



PVOID
RHBuildNodeEx (
	IN PVOID _TotalHead,
	IN LPWSTR HiveRegPath,
	IN LPWSTR HiveFilePath
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/03 [3:6:2010 - 14:51]

Routine Description:
  ���RH�ṹ��   

--*/
{
	BOOL bRet = FALSE ;
	LPRHNODE pNode = NULL ;
	LPRHHEAD pTotalHead	 = (LPRHHEAD) _TotalHead ;

	//
	// 1. У������Ϸ���
	//
	
	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList || NULL == HiveRegPath || NULL == HiveFilePath )
	{
		dprintf( "error! | RHBuildNodeEx(); | Invalid Paramaters; failed! \n" );
		return NULL ;
	}

	//
	// 2. �����ڴ�,���ڵ�
	//

	bRet = RHAllocateNode( &pNode );
	if ( FALSE == bRet )
	{
		dprintf( "error! | RHBuildNodeEx() - RHAllocateNode() | �����ڴ�ʧ�� \n" );
		return NULL ;
	}

	wcscpy( pNode->HiveRegPath, HiveRegPath );
	wcscpy( pNode->HiveFilePath, HiveFilePath );

	pNode->PorcessRefCounts = 0 ;
	pNode->ProcessesLock	= 0 ;	
	pNode->bNeedToDistroy	= FALSE ;

	//
	// 3. ����Ҫ,������½ڵ㵽����β
	//

	EnterCrit( pTotalHead->QueueLockList );	// ��������

	if ( NULL == pTotalHead->ListHead.HiveFilePath && NULL == pTotalHead->ListHead.HiveRegPath ) 
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
RHFindNode (
	IN PVOID _TotalHead ,
	IN LPWSTR HiveRegPath,
	IN LPWSTR HiveFilePath,
	OUT BOOL* bResult
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
	LPRHNODE pHead = NULL, pNode = NULL ;
	LPRHHEAD pTotalHead	= (LPRHHEAD) _TotalHead ;

	//
	// 1. У������Ϸ���
	//

	*bResult = TRUE ;
	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList || NULL == HiveRegPath || NULL == HiveFilePath ) { return NULL ; }

	if ( 0 == pTotalHead->nTotalCounts ) { return NULL ; }

	//
	// 2. ��������,����ָ���ڵ�
	//

	EnterCrit( pTotalHead->QueueLockList );	// ��������
	pNode = pHead = &pTotalHead->ListHead ;

	do 
	{
		if (	(0 == _wcsicmp( HiveRegPath, pNode->HiveRegPath ))
			&&	(0 == _wcsicmp( HiveFilePath, pNode->HiveFilePath ))
			) 
		{
			pResult = (PVOID) pNode ;
			break ;
		}

		if ( pNode->PorcessRefCounts )
		{
			dprintf( "error! | RHFindNode(); | ��ǰɳ���Hive���ü�����Ϊ�� \n" );
			*bResult = FALSE ;
			break ;
		}

		pNode = pNode->pBlink ;
	} while ( pNode != pHead );

	LeaveCrit( pTotalHead->QueueLockList );	// ��������
	return pResult ;
}



VOID
RHDistroyAll (
	IN PVOID _TotalHead
	)
{
	int i = 0 ;
	LPRHNODE pHead = NULL, pNode = NULL ;
	LPRHHEAD pTotalHead = (LPRHHEAD) _TotalHead ;

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | RHDistroyAll() | Invalid ListHead \n" );
		return ;
	}

	//
	// 1. ɾ�����е��ӽڵ�
	//

	RHWalkNodes( pTotalHead );

	dprintf( "*** ��ʼж��RegHive�ڵ������ *** \n" );

	EnterCrit( pTotalHead->QueueLockList );	// ��������
	pNode = pHead = &pTotalHead->ListHead ;

	do
	{
		dprintf( "  [%d] Node:0x%08lx; \nHiveRegPath:\"%ws\" \nHiveFilePath:\"%ws\" \n", i++, (PVOID)pNode, pNode->HiveRegPath, pNode->HiveFilePath );

		if ( pHead != pNode )
		{	
			RemoveEntryList( (PLIST_ENTRY)pNode );
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

	RHDeleteTotalHead( &_TotalHead );

	dprintf( "*** ����ж��RegHive�ڵ������ *** \n" );
	return ;
}



VOID
RHWalkNodes (
	IN PVOID _TotalHead
	)
{
	INT i = 1 ;
	LPRHNODE pHead = NULL, pNode = NULL ;
	LPRHHEAD pTotalHead	= (LPRHHEAD) _TotalHead ;

#ifdef DBG

	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | RHWalkNodes() | Invalid ListHead \n" );
		return ;
	}

	if ( 0 == pTotalHead->nTotalCounts )
	{
		dprintf( "error! | RHWalkNodes() | ����Ϊ��,���ɱ��� \n" );
		return ;
	}

	dprintf( "\n**** Starting walking RegHive Lists **** \n" );

	EnterCrit( pTotalHead->QueueLockList );	// ��������
	pNode = pHead = &pTotalHead->ListHead ;

	do
	{
		dprintf( 
			"[%d] PorcessRefCounts=%d,HiveFilePath=\"%ws\", HiveRegPath=\"%ws\" \n",
			i++,
			pNode->PorcessRefCounts,
			pNode->HiveFilePath,
			pNode->HiveRegPath
			);

		pNode = pNode->pFlink ;
		if ( NULL == pNode ) { break ; }
	} while ( pNode != pHead );

	LeaveCrit( pTotalHead->QueueLockList ); // �ͷ���
	dprintf( "**** End of walking RegHive Lists **** \n\n" );

#endif
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
CheckRegHive (
	IN PVOID _pNode,
	IN PVOID _ProcessNode
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/08 [8:6:2010 - 15:27]

Routine Description:
  ��鵱ǰɳ���Hive�����Ϣ    

--*/
{
	HANDLE KeyHandle;
	BOOL bRet = FALSE ;
	ULONG Length = 0, ResLength = 0;
	PKEY_VALUE_PARTIAL_INFORMATION KeyValuePartialInfo = NULL ;
	UNICODE_STRING FileName, KeyName, uniBuffer ;
	OBJECT_ATTRIBUTES FileObjectAttributes, KeyObjectAttributes, objattri ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPRHNODE pNode = (LPRHNODE) _pNode ;
	LPPDNODE ProcessNode = (LPPDNODE) _ProcessNode ;

	//
	// 1. У������Ϸ���
	//
	
	if ( NULL == _pNode || NULL == _ProcessNode )
	{
		dprintf( "error! | CheckRegHive(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	//
	// 2. ��HiveRegPath,�鿴��Ӧ��ע����ֵ�Ƿ����
	//

	RtlInitUnicodeString( &FileName, pNode->HiveFilePath );
	
	InitializeObjectAttributes (
		&FileObjectAttributes,
		&FileName,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL,
		NULL
		);

	RtlInitUnicodeString( &KeyName, pNode->HiveRegPath );

	InitializeObjectAttributes (
		&KeyObjectAttributes,
		&KeyName,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL,
		NULL
		);

	status = ZwOpenKey( &KeyHandle, KEY_READ | KEY_WRITE, &KeyObjectAttributes );
	if ( ! NT_SUCCESS(status) ) 
	{
		// ������,�򴴽�Hive�ļ�
		dprintf( "ok! | CheckRegHive() - ZwOpenKey(); | ��ǰ�ļ�ֵ\"%ws\"������,����CmLoadKey()����Hive�ļ� \n", pNode->HiveFilePath );

		bRet = CmLoadKey( _ProcessNode, &KeyObjectAttributes, &FileObjectAttributes );
		return bRet ;
	}

	//
	// 3. ����,У��Ϸ���
	//
	
	ZwClose( KeyHandle );
	
	RtlInitUnicodeString( &uniBuffer, L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Control\\hivelist" );

	InitializeObjectAttributes (
		&objattri,
		&uniBuffer,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL,
		NULL
		);

	status = ZwOpenKey( &KeyHandle, KEY_READ | KEY_WRITE, &objattri );
	if ( ! NT_SUCCESS(status) ) 
	{
		dprintf( "error! | CheckRegHive() - ZwOpenKey(); | (status=0x%08lx) \n", status );
		return FALSE ;
	}

	status = ZwQueryValueKey( KeyHandle, &KeyName, KeyValuePartialInformation, 0, 0, &ResLength );
	if( STATUS_BUFFER_TOO_SMALL != status )
	{
		dprintf( "error! | CheckRegHive() - ZwQueryValueKey(); | (status=0x%08lx) \n", status );
		goto _END_ ;
	}

	ResLength += sizeof( KEY_VALUE_PARTIAL_INFORMATION );
	KeyValuePartialInfo = kmalloc( ResLength );
	Length = ResLength ;

	if( NULL == KeyValuePartialInfo ) 
	{
		dprintf( "error! | CheckRegHive() - kmalloc(); | �����ڴ�ʧ��,��С:%d \n", ResLength );
		goto _END_ ;
	}

	status = ZwQueryValueKey( KeyHandle, &KeyName, KeyValuePartialInformation, (PVOID)KeyValuePartialInfo, Length, &ResLength );
	if( ! NT_SUCCESS(status) || (REG_SZ != KeyValuePartialInfo->Type) )
	{
		dprintf( "error! | CheckRegHive() - kmalloc(); | ZwQueryValueKey(); | (status=0x%08lx) \n", status );
		goto _FREEMEMORY_ ;
	}

	if ( 0 == _wcsicmp( (LPWSTR)KeyValuePartialInfo->Data, FileName.Buffer ) )
	{
		bRet = TRUE ;
	}

_FREEMEMORY_ :
	kfree( KeyValuePartialInfo );
_END_ :
	ZwClose( KeyHandle );
	return bRet ;
}



BOOL
CmLoadKey(
	IN PVOID _ProcessNode,
	IN POBJECT_ATTRIBUTES KeyObjectAttributes,
	IN POBJECT_ATTRIBUTES FileObjectAttributes
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/08 [8:6:2010 - 15:30]

Routine Description:
  ��ZwLoadKey�����İ�װ,����ǰ��Ҫ��Ȩ
    
--*/
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE ProcessNode = (LPPDNODE) _ProcessNode ;
	HANDLE TokenHandle = NULL ;
	BOOL bRet = FALSE ;
	ULONG DefaultDacl_Length = 0 ;
	PTOKEN_DEFAULT_DACL	LocalDefaultDacl = NULL	;


	//
	// 1. У������Ϸ���
	//
	
	if ( NULL == _ProcessNode || NULL == KeyObjectAttributes || NULL == FileObjectAttributes )
	{
		dprintf( "error! | CmLoadKey(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	//
	// 2.1 ����ɵ�Ȩ����Ϣ
	//

	status = ZwOpenProcessToken( (HANDLE)0xFFFFFFFF, TOKEN_ADJUST_DEFAULT | TOKEN_QUERY, &TokenHandle );
	if ( ! NT_SUCCESS(status) ) 
	{
		dprintf( "error! | CmLoadKey() - ZwOpenProcessToken(); | (status=0x%08lx) \n", status );
		return FALSE ;
	}

	bRet = QueryInformationToken( TokenHandle, TokenDefaultDacl, &LocalDefaultDacl, &DefaultDacl_Length );
	if ( FALSE == bRet )
	{
		dprintf( "error! | CmLoadKey() - QueryInformationToken(); |  \n" );
		ZwClose( TokenHandle );
		return FALSE ;
	}

	// 2.2 ��ȡ ZwSetInformationToken ��ԭʼ��ַ
	bRet = kgetaddrSDT( ZwSetInformationToken );
	if ( FALSE == bRet )
	{
		dprintf( "error! | CmLoadKey() - kgetaddrSDT(); | �޷���ȡZwSetInformationToken�ĵ�ַ \n" );
		goto _CLEAR_UP_ ;
	}

	// 2.3 ��Ȩ
	status = g_ZwSetInformationToken_addr( TokenHandle, TokenDefaultDacl, &g_DefaultDacl_new, sizeof(DWORD) );
	if ( !NT_SUCCESS (status) )
	{
		dprintf( "error! | CmLoadKey() - ZwSetInformationToken() | ������Tokenʧ��; status == 0x%08lx \n", status );
		bRet = FALSE ;
		goto _CLEAR_UP_ ;
	}

	// 2.4 ӳ��ע����Hive�ļ�
	bRet =  TRUE ;

	status = ZwLoadKey( KeyObjectAttributes, FileObjectAttributes );
	if ( ! NT_SUCCESS (status) )
	{
		dprintf( "error! | CmLoadKey() - ZwLoadKey() | status == 0x%08lx \n", status );
		bRet =  FALSE ;
	}

	// 2.5 �ָ����ɵ�Ȩ��
	status = g_ZwSetInformationToken_addr( TokenHandle, TokenDefaultDacl, LocalDefaultDacl, DefaultDacl_Length );
	if ( ! NT_SUCCESS (status) )
	{
		dprintf( "error! | CmLoadKey() - ZwSetInformationToken() | �ָ���Tokenʧ��; status == 0x%08lx \n", status );
		bRet =  FALSE ;
	}

_CLEAR_UP_ :
	kfree( (PVOID)LocalDefaultDacl );
	ZwClose( TokenHandle );
	return bRet ;
}

///////////////////////////////   END OF FILE   ///////////////////////////////