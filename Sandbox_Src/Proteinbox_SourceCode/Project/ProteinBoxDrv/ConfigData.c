/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/06/02 [2:6:2010 - 17:33]
* MODULE : d:\Work\Program\Coding\ɳ��\SandBox\Code\Project\ProteinBoxDrv\GrayList.c
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
#include "Config.h"
#include "ConfigThread.h"
#include "Common.h"
#include "Memory.h"
#include "ConfigData.h"

//////////////////////////////////////////////////////////////////////////

#define g_DeepCounts	3

LPPB_CONFIG_TOTAL g_ProteinBox_Conf_TranshipmentStation = NULL	;
LPPB_CONFIG_TOTAL g_ProteinBox_Conf = NULL	;
LPPB_CONFIG_TOTAL g_ProteinBox_Conf_Old = NULL ;
BOOL g_ProteinBox_Conf_Inited_ok	= FALSE	;


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
InitConfigData (
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/01 [1:6:2010 - 18:36]

Routine Description:
  ��������ͷ    

--*/
{
	BOOL bRet = FALSE ;

//	if ( FALSE == g_ProteinBox_Conf_Inited_ok )
	{
		if ( g_ProteinBox_Conf_TranshipmentStation )
		{
			g_ProteinBox_Conf_Old = g_ProteinBox_Conf_TranshipmentStation ;
			g_ProteinBox_Conf_TranshipmentStation = NULL;
		}

		bRet = CDCreateTotalHead( (PVOID) &g_ProteinBox_Conf_TranshipmentStation );
		g_ProteinBox_Conf_Inited_ok = bRet ;

		if ( FALSE == bRet )
		{
			dprintf( "error! | InitConfigData(); \n" );
		}
	}

	return bRet ;
}



BOOL 
CDCreateTotalHead(
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
	LPCDHEAD *pTotalHead = (LPCDHEAD*) _TotalHead ;
	if ( NULL !=  *pTotalHead ) { return TRUE ; }

	// Ϊ�ܽṹ������ڴ�
	*pTotalHead = (LPCDHEAD) kmalloc( sizeof( CDHEAD ) );
	if ( NULL == *pTotalHead )
	{
		dprintf( "error! | CDCreateTotalHead(); Allocate memory failed! \n" );
		return FALSE ;
	}

	RtlZeroMemory( *pTotalHead, sizeof( CDHEAD ) );
	
	// ��ʼ����Դ��
	bRet = InitResource( &((LPCDHEAD)*pTotalHead)->QueueLockList );
	if ( FALSE == bRet )
	{
		dprintf( "error! | CDCreateTotalHead() - InitResource(); ������Դ���ڴ�ʧ��! \n" );
		kfree( (PVOID) *pTotalHead );
		return FALSE ;
	}

	// ��ʼ������ͷ
	InitializeListHead( (PLIST_ENTRY)&( (LPCDHEAD)*pTotalHead )->SectionListHead );
	InitializeListHead( (PLIST_ENTRY)&(( (LPCDHEAD)*pTotalHead )->SectionListHead).KeyListHead );
	InitializeListHead( (PLIST_ENTRY)&((( (LPCDHEAD)*pTotalHead )->SectionListHead).KeyListHead).ValueListHead );

	((LPCDHEAD)*pTotalHead)->SectionCounts = 0 ;

	return TRUE ;
}



VOID 
CDDeleteTotalHead(
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
CDAllocateNode(
	OUT PVOID* pCurrenList,
	IN ULONG StructSize
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
	if ( NULL == pCurrenList || StructSize <= 0 )
	{
		dprintf( "error! | CDAllocateNode(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	*pCurrenList = (PVOID) kmalloc( StructSize );

	if ( NULL == *pCurrenList )
	{
		dprintf( "error! | CDAllocateNode() | Allocate memory failed! \n" );
		return FALSE ;
	}

	RtlZeroMemory( *pCurrenList, StructSize );
	return TRUE ;
}



BOOL
CDBuildNode (
	IN PVOID _TotalHead,
	IN PVOID pInBuffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/07/05 [5:7:2010 - 17:03]

Routine Description:
  ����ָ��Section -> Key -> Value 
    
Arguments:
  _TotalHead - �����ļ�������ͷ
  pInBuffer - Ҫ�����Ľڵ���Ϣ
    
--*/
{
	BOOL bRet = FALSE ;
	ULONG Type ;
	SEARCH_INFO OutBuffer = { 0 };
	LPCDNODES pSectionNode	= NULL ;
	LPCDNODEK pKeyNode		= NULL ;
	LPCDNODEV pValueNode	= NULL ;
	LPCDHEAD pTotalHead = (LPCDHEAD) _TotalHead ;
	LPIOCTL_HANDLERCONF_BUFFER_IniDataW lpData = (LPIOCTL_HANDLERCONF_BUFFER_IniDataW) pInBuffer ;
	
	// 1. У������Ϸ���
	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList || NULL == lpData )
	{
		dprintf( "error! | CDBuildNode(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	// 2. �����Ƿ����Ѵ��ڵĽڵ�
	bRet = CDFindNode( _TotalHead, pInBuffer, &OutBuffer );
	if ( FALSE == bRet )
	{
		dprintf( "error! | CDBuildNode() - CDFindNode(); | \n" );
		return FALSE ;
	}

	switch ( OutBuffer.NodeType )
	{
	case _NODE_IS_NOTHING_ :
		{
			// ����Section & Key & Value�ڵ㶼δ����
			pSectionNode = (LPCDNODES) CDBuildNodeEx( 
				pTotalHead->QueueLockList,
				(PVOID)&pTotalHead->SectionListHead, 
				sizeof(CDNODES), 
				lpData->SeactionName,
				lpData->SeactionNameLength
				);
			
			if ( NULL == pSectionNode )
			{
				dprintf( "error! | CDBuildNode() - GLBuildNodeS(); | (case _NODE_IS_NOTHING_) NULL == pSectionNode \n" );
				return FALSE ;
			}

			pKeyNode = (LPCDNODEK) CDBuildNodeEx( pTotalHead->QueueLockList, (PVOID)&pSectionNode->KeyListHead, sizeof(CDNODEK), lpData->KeyName, lpData->KeyNameLength );
			if ( NULL == pKeyNode )
			{
				dprintf( "error! | CDBuildNode() - CDBuildNodeEx(); | (case _NODE_IS_NOTHING_) NULL == pKeyNode \n" );
				return FALSE ;
			}

			pValueNode = (LPCDNODEV) CDBuildNodeEx( pTotalHead->QueueLockList, (PVOID)&pKeyNode->ValueListHead, sizeof(CDNODEV), lpData->ValueName, lpData->ValueNameLength );
			if ( NULL == pValueNode )
			{
				dprintf( "error! | CDBuildNode() - CDBuildNodeEx(); | (case _NODE_IS_NOTHING_) NULL == pValueNode \n" );
				return FALSE ;
			}
		}
		break ;

	case _NODE_IS_SECTION_ :
		{
			// ����ֻ�ж�Ӧ��Section�ڵ�,��Key & Value�ڵ㻹δ����
			pSectionNode = (LPCDNODES) OutBuffer.pNode ;
			pKeyNode = (LPCDNODEK) CDBuildNodeEx( pTotalHead->QueueLockList, (PVOID)&pSectionNode->KeyListHead, sizeof(CDNODEK), lpData->KeyName, lpData->KeyNameLength );
			if ( NULL == pKeyNode )
			{
				dprintf( "error! | CDBuildNode() - CDBuildNodeEx(); | (case _NODE_IS_SECTION_) NULL == pKeyNode \n" );
				return FALSE ;
			}

			pValueNode = (LPCDNODEV) CDBuildNodeEx( pTotalHead->QueueLockList, (PVOID)&pKeyNode->ValueListHead, sizeof(CDNODEV), lpData->ValueName, lpData->ValueNameLength );
			if ( NULL == pValueNode )
			{
				dprintf( "error! | CDBuildNode() - CDBuildNodeEx(); | (case _NODE_IS_SECTION_) NULL == pValueNode \n" );
				return FALSE ;
			}
		}
		break ;

	case _NODE_IS_KEY_ :
		{
			// ����ֻ�ж�Ӧ��Section & Key�ڵ�,��Value�ڵ㻹δ����
			pKeyNode = (LPCDNODEK) OutBuffer.pNode ;
			pValueNode = (LPCDNODEV) CDBuildNodeEx( pTotalHead->QueueLockList, (PVOID)&pKeyNode->ValueListHead, sizeof(CDNODEV), lpData->ValueName, lpData->ValueNameLength );
			if ( NULL == pValueNode )
			{
				dprintf( "error! | CDBuildNode() - CDBuildNodeEx(); | (case _NODE_IS_KEY_) NULL == pValueNode \n" );
				return FALSE ;
			}
		}
		break ;

	case _NODE_IS_VALUE_ :
		{
			dprintf( 
				"ok! | CDBuildNode() - CDFindNode(); | ��ǰ�ڵ�(Section:%ws; Key:%ws; Value:%ws)�Ѵ�����������,ֱ�ӷ��� \n",
				lpData->SeactionName, lpData->KeyName, lpData->ValueName
				);

			return TRUE ;
		}
		break ;
	
	default :
		dprintf( "error! | CDBuildNode();  | error Type! \n" );
		return FALSE ;
	}

	return TRUE ;
}



PVOID
CDBuildNodeEx (
	IN PERESOURCE QueueLockList,
	IN PVOID _ListHead,
	IN ULONG StructSize,
	IN LPWSTR wszName,
	IN int   NameLength
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/19 [19:5:2010 - 20:11]

Routine Description:
  �½�Value�ڵ�.eg: c:\Test.ext    
    
Arguments:
  _ListHead - ����ͷ
  StructSize - Ҫ�½��Ľṹ���С
  SectionName - �����Ľ���
  NameLength - �����Ľ�������

Return Value:
  �½���GL�ṹ��ָ��
    
--*/
{
	BOOL bRet = FALSE ;
	LPPB_CONFIG_COMMON pNode = NULL ;
	LPPB_CONFIG_COMMON ListHead = (LPPB_CONFIG_COMMON) _ListHead ;

	//
	// 1. У������Ϸ���
	//

	if ( NULL == QueueLockList || NULL == ListHead || NULL == wszName || NameLength <= 0 || StructSize <= 0 )
	{
		dprintf( "error! | CDBuildNodeEx() | Invalid Parameters \n" );
		return NULL ;
	}

	//
	// 2. �����ڴ�,���ڵ�
	//

	bRet = CDAllocateNode( &pNode, StructSize );
	if ( FALSE == bRet )
	{
		dprintf( "error! | CDBuildNodeEx() - CDAllocateNode() | �����ڴ�ʧ�� \n" );
		return NULL ;
	}

	pNode->wszName = (LPWSTR) kmalloc( NameLength );
	if ( NULL == pNode->wszName )
	{
		dprintf( "error! | CDBuildNodeEx() - kmalloc(); | �����ڴ�ʧ��! \n" );
		kfree( (PVOID)pNode ); // �ͷ��ڴ�
		return NULL ;
	}

	pNode->NameLength = NameLength ;
	RtlZeroMemory( pNode->wszName, NameLength );
	RtlCopyMemory( pNode->wszName, wszName, NameLength );

	//
	// 3. ����Ҫ,������½ڵ㵽����β
	//

	EnterCrit( QueueLockList );	// ��������

	if ( 0 == ListHead->NameLength && NULL == ListHead->wszName ) 
	{
		// ����ͷδ��ʹ��.use it
		*ListHead = *pNode ;
		InitializeListHead( (PLIST_ENTRY)ListHead );

		kfree( (PVOID)pNode ); // �ͷ��ڴ�
		pNode = ListHead ;
	}
	else
	{
		InsertTailList( (PLIST_ENTRY)ListHead, (PLIST_ENTRY)pNode );
	}

	LeaveCrit( QueueLockList );	// �ͷ���
	return (PVOID)pNode ;
}



BOOL
CDFindNode (
	IN PVOID _TotalHead ,
	IN PVOID pInBuffer,
	OUT PVOID pOutBuffer
	)
{
	LPCDNODES pSectionHead = NULL, pSectionNode = NULL ;
	LPCDNODEK pKeyHead = NULL, pKeyNode = NULL ;
	LPCDNODEV pValueHead = NULL, pValueNode = NULL ;
	LPCDHEAD pTotalHead = (LPCDHEAD) _TotalHead ;
	LPIOCTL_HANDLERCONF_BUFFER_IniDataW lpData = (LPIOCTL_HANDLERCONF_BUFFER_IniDataW) pInBuffer ;
	LPSEARCH_INFO pInfo = (LPSEARCH_INFO) pOutBuffer ;

	// 1. У������Ϸ���
	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList || NULL == lpData || NULL == pInfo )
	{
		dprintf( "error! | CDFindNode(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	pInfo->NodeType = _NODE_IS_NOTHING_ ;
	pInfo->pNode	= NULL ;

	// 2. ��������,����ָ���ڵ�

	EnterCrit( pTotalHead->QueueLockList );	// ��������
	pSectionNode = pSectionHead = &pTotalHead->SectionListHead ;

	// 2.1 ����Section����
	do 
	{
		if ( (lpData->SeactionNameLength == pSectionNode->NameLength) && (0 == _wcsicmp( lpData->SeactionName, pSectionNode->SectionName )) ) 
		{
			pInfo->NodeType = _NODE_IS_SECTION_ ;
			pInfo->pNode	= (PVOID) pSectionNode ;

			pKeyNode = pKeyHead = &pSectionNode->KeyListHead ;
			if ( NULL == pKeyHead || FALSE == MmIsAddressValid( (PVOID)pKeyHead ) ) { break ; }

			// 2.2 ��ƥ�䵽Section,����Key����
			do
			{
				if ( (lpData->KeyNameLength == pKeyNode->NameLength) && (0 == _wcsicmp( lpData->KeyName, pKeyNode->KeyName )) ) 
				{
					pInfo->NodeType = _NODE_IS_KEY_ ;
					pInfo->pNode	= (PVOID) pKeyNode ;
					
					pValueNode = pValueHead = &pKeyNode->ValueListHead ;
					if ( NULL == pValueHead || FALSE == MmIsAddressValid( (PVOID)pValueHead ) ) { break ; }

					// 2.3 ��ƥ�䵽Key,����Value����
					do
					{
						if ( (lpData->ValueNameLength == pValueNode->NameLength) && (0 == _wcsicmp( lpData->ValueName, pValueNode->ValueName )) ) 
						{
							pInfo->NodeType = _NODE_IS_VALUE_ ;
							pInfo->pNode	= (PVOID) pValueNode ;

							break ;
						}

						pValueNode = pValueNode->pBlink ;
					} while( pValueNode != pValueHead );
					break ;
				}

				pKeyNode = pKeyNode->pBlink ;
			} while( pKeyNode != pKeyHead );
			break ;
		}

		pSectionNode = pSectionNode->pBlink ;
	} while ( pSectionNode != pSectionHead );

	LeaveCrit( pTotalHead->QueueLockList );	// �ͷ���

	return TRUE ;
}



BOOL
CDFindNodeEx (
	IN PVOID _TotalHead ,
	IN LPWSTR SeactionName,
	IN LPWSTR KeyName,
	IN LPWSTR ValueName,
	OUT PVOID pOutBuffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/07/09 [9:7:2010 - 14:33]

Routine Description:
  �ú�����Ҫ�ǲ���Name��Ӧ�Ľڵ�.��Ҫ��SeactionName�Ľڵ�ʱ,�ὫKeyName & ValueName�ÿ�;
  ��Ҫ��KeyName�Ľڵ�ʱ,�ὫValueName�ÿ�; ��Ҫ��ValueName�Ľڵ�ʱ,��û�пյ�
    
--*/
{
	BOOL bRet = FALSE ;
	IOCTL_HANDLERCONF_BUFFER_IniDataW InBuffer = { 0 } ;

	// 1. У������Ϸ���
	if ( NULL == _TotalHead || NULL == SeactionName || NULL == pOutBuffer )
	{
		dprintf( "error! | IsValueNameExist(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	// 2. ƥ��֮
	InBuffer.SeactionName = SeactionName ;
	InBuffer.SeactionNameLength = ( wcslen(SeactionName) + 1 ) * sizeof(WCHAR) ; 

	if ( KeyName )
	{
		InBuffer.KeyName = KeyName ;
		InBuffer.KeyNameLength =  ( wcslen(KeyName) + 1 ) * sizeof(WCHAR) ; 
	}
	
	if ( ValueName )
	{
		InBuffer.ValueName = ValueName ;
		InBuffer.ValueNameLength = ( wcslen(ValueName) + 1 ) * sizeof(WCHAR) ; 
	}

	bRet = CDFindNode( _TotalHead, &InBuffer, pOutBuffer );
	if ( FALSE == bRet )
	{
		dprintf( "error! | CDFindNodeEx() - CDFindNode(); | \n" );
		return FALSE ;
	}

	return TRUE ;
}



VOID
CDDistroyList (
	IN PVOID ListHead,
	IN ULONG DeepCounts
	)
{
	LPPB_CONFIG_COMMONEX pNode = NULL, pHead = NULL ;

	// 1. У������Ϸ���
	if ( NULL == ListHead || DeepCounts > g_DeepCounts )
	{
		dprintf( "error! | CDDistroyList(); | Invalid Paramaters; failed! \n" );
		return ;
	}

	if ( 0 == DeepCounts ) { return ; }

	// 2.�ͷŵ�ǰ������������
	pNode = pHead = (LPPB_CONFIG_COMMONEX) ListHead ;

	do
	{
		kfree( (PVOID)pNode->wszName ); // 2.1 �����ǰ�ڵ��е��ַ����ڴ�

		CDDistroyList( &pNode->SunList, DeepCounts - 1 ); // 2.2 �����ǰ�ڵ��������

		if ( pHead != pNode )
		{
			RemoveEntryList( (PLIST_ENTRY) pNode );
			kfree( (PVOID)pNode );
		}

		pNode = (LPPB_CONFIG_COMMONEX) pHead->List.Flink ;

	} while ( FALSE == IsListEmpty( (PLIST_ENTRY) pHead ) );
}



VOID
CDDistroyAll (
	IN PVOID _TotalHead
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2011/08/05 [5:8:2011 - 18:08]

Routine Description:
  ����ж��ʱ���ô˺���
    
--*/
{
	LPCDHEAD pTotalHead = (LPCDHEAD) _TotalHead ;

	// 1. У������Ϸ���
	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | CDDistroyAll() | Invalid ListHead \n" );
		return ;
	}

	// 2. ɾ�����е��ӽڵ�
	AbortWait_for_DriverUnload();
	CreateConfigThread( TRUE );

	CDWalkNodes( _TotalHead );

	dprintf( "*** ��ʼж��ConfigData�ڵ������ *** \n" );

	EnterCrit( pTotalHead->QueueLockList );	// ��������
	CDDistroyList( &pTotalHead->SectionListHead, 3 );
	LeaveCrit( pTotalHead->QueueLockList ); // �ͷ���

	ExDeleteResource( pTotalHead->QueueLockList );
	kfree( pTotalHead->QueueLockList );

	// 4. ɾ���ܽڵ�
	CDDeleteTotalHead( &_TotalHead );

	dprintf( "*** ����ж��ConfigData�ڵ������ *** \n" );
	return ;
}




VOID
CDDistroyAllEx (
	IN PVOID _TotalHead
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2011/08/05 [5:8:2011 - 18:08]

Routine Description:
  R3ͨ��PBStart.exe /Reload����ʱ,���ջ��߽�����,����ɵ�����
    
--*/
{
	LPCDHEAD pTotalHead = (LPCDHEAD) _TotalHead ;

	// 1. У������Ϸ���
	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | CDDistroyAll() | Invalid ListHead \n" );
		return ;
	}

	// 2. ɾ�����е��ӽڵ�
	EnterCrit( pTotalHead->QueueLockList );	// ��������
	CDDistroyList( &pTotalHead->SectionListHead, 3 );
	LeaveCrit( pTotalHead->QueueLockList ); // �ͷ���

	ExDeleteResource( pTotalHead->QueueLockList );
	kfree( pTotalHead->QueueLockList );

	// 4. ɾ���ܽڵ�
	CDDeleteTotalHead( &_TotalHead );
	return ;
}




VOID
CDWalkNodes (
	IN PVOID _TotalHead
	)
{
	LPCDNODES pSectionHead = NULL, pSectionNode = NULL ;
	LPCDNODEK pKeyHead = NULL, pKeyNode = NULL ;
	LPCDNODEV pValueHead = NULL, pValueNode = NULL ;
	LPCDHEAD pTotalHead = (LPCDHEAD) _TotalHead ;

#ifdef DBG

	// 1. У������Ϸ���
	if ( NULL == pTotalHead || NULL == pTotalHead->QueueLockList )
	{
		dprintf( "error! | CDWalkNodes() | Invalid ListHead \n" );
		return ;
	}

	dprintf( "\n**** Starting walking ConfigDatas **** \n" );
	EnterCrit( pTotalHead->QueueLockList );	// ��������

	pSectionNode = pSectionHead =  &pTotalHead->SectionListHead ;

	// 2.1 ����Section����
	do 
	{
		dprintf( "\n[%ws]", pSectionNode->SectionName );

		pKeyNode = pKeyHead = &pSectionNode->KeyListHead ;
		if ( NULL == pKeyHead || FALSE == MmIsAddressValid( (PVOID)pKeyHead ) ) { break ; }

		// 2.2 ����Key����
		do
		{
			dprintf( "\n%ws=", pKeyNode->KeyName );

			pValueNode = pValueHead = &pKeyNode->ValueListHead ;
			if ( NULL == pValueHead || FALSE == MmIsAddressValid( (PVOID)pValueHead ) ) { break ; }

			// 2.3 ����Value����
			do
			{
				dprintf( "%ws,", pValueNode->ValueName );
				pValueNode = pValueNode->pBlink ;

				if ( NULL == pValueNode ) { break ; }
			} while( pValueNode != pValueHead );

			pKeyNode = pKeyNode->pBlink ;
			if ( NULL == pKeyNode ) { break ; }

		} while( pKeyNode != pKeyHead );

		dprintf( "\n" );
		pSectionNode = pSectionNode->pBlink ;
		if ( NULL == pSectionNode ) { break ; }

	} while ( pSectionNode != pSectionHead );

	LeaveCrit( pTotalHead->QueueLockList ); // �ͷ���
	dprintf( "\n**** End of walking ConfigDatas **** \n\n" );

#endif
	return ;
}



BOOL
Is_ValueName_Exist (
	IN PVOID _TotalHead ,
	IN LPWSTR SeactionName,
	IN LPWSTR KeyName,
	IN LPWSTR ValueName
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/07/06 [6:7:2010 - 14:37]

Routine Description:
  �鿴�����ļ����Ƿ����ָ����ValueName,���򷵻�TRUE  
    
Arguments:
  ValueName - �����ҵ��ַ���
    
--*/
{
	BOOL bRet = FALSE ;
	ULONG Type = -1 ;
	PVOID pNode = NULL ;
	SEARCH_INFO InBuffer = { 0 } ;

	// 1. У������Ϸ���
	if ( NULL == _TotalHead || NULL == SeactionName || NULL == KeyName || NULL == ValueName )
	{
		dprintf( "error! | Is_ValueName_Exist(); | Invalid Paramaters; failed! (_TotalHead=%x) (KeyName=%ws) (ValueName=%ws) \n", _TotalHead, KeyName, ValueName );
		return FALSE ;
	}

	// 2. ƥ��֮
	bRet = CDFindNodeEx( _TotalHead, SeactionName, KeyName, ValueName, &InBuffer );
	if ( FALSE == bRet )
	{
		dprintf( "error! | Is_ValueName_Exist() - CDFindNodeEx(); | \n" );
		return FALSE ;
	}

	if ( _NODE_IS_VALUE_ != InBuffer.NodeType ) { return FALSE ; }
	return TRUE ;
}


static char* g_InjectDllPath = NULL;

BOOL 
GetPBDllPath(
	IN ULONG uMethodType,
	IN ULONG MaxLength,
	OUT LPSTR* szPath
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2012/01/02 [2:1:2012 - 10:46]

Routine Description:
  ��ȡ��Ҫ����ģ��Proteinboxdll.dll��ȫ·��.
  ����ͨ�� @uMethodType ָ����ȡ��ʽ
    
Arguments:
  uMethodType - 1: ��ע����ȡ; 2: �������ļ���ȡ
  MaxLength - ������󳤶�
  szPath - �����ȡ��������
    
--*/
{
	BOOL bRet = FALSE, bAppend = FALSE;
	CHAR szFinalPath[ MAX_PATH + 0x20 ] = "";

	// 1. У������Ϸ���
	if ( FALSE == IsPBDLLPathTypeValid(uMethodType) || NULL == szPath )
	{
		dprintf( "error! | GetPBDllPath(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	if ( g_InjectDllPath ) { goto _END_; }
	
	// 2.
	switch (uMethodType)
	{
	case PBDLLPathType_Reg :
		bRet = GetPBDllPathFromReg( MaxLength, szFinalPath );
		break;

// 	case PBDLLPathType_Ini :
// 		bRet = GetPBDllPathFromIni( MaxLength, szFinalPath );
// 		break;

	default:
		break;
	}

	if ( FALSE == bRet || NULL == szFinalPath )
	{
		dprintf( "error! | GetPBDllPath() - GetPBDllPathFromReg; | \n" );
		return FALSE ;
	}

	if ( strncmp(szFinalPath, "\\??\\", 4) )
	{
		// ��ʽ����, Ҫ��"c:\\" ����"\\??\\c:\\"
		if ( ':' != szFinalPath[1] ) 
			return FALSE;

		bAppend = TRUE;
	}
	
	// �ǵ�һ�β�ѯ
	g_InjectDllPath = (char*) kmallocMM( MaxLength + 0x30, MTAG___InjectDllPath );
	if ( NULL == g_InjectDllPath ) 
	{ 
		dprintf( "error! | GetPBDllPath() - kmallocMM(); | \n" );
		return FALSE; 
	}

	RtlZeroMemory( g_InjectDllPath, MaxLength + 0x30 );

	if (bAppend)
	{
		memcpy( g_InjectDllPath, "\\??\\", 4 );
		strcat( g_InjectDllPath, szFinalPath );
	} 
	else
	{
		strcpy( g_InjectDllPath, szFinalPath );
	}

_END_:
	*szPath = g_InjectDllPath;
	return TRUE;
}


BOOL 
GetPBDllPathFromReg(
	IN ULONG MaxLength,
	OUT LPSTR szPath
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2012/01/02 [2:1:2012 - 10:46]

Routine Description:
  ��ȡע���,��ȡ��Ҫ����ģ��Proteinboxdll.dll��ȫ·��.
    
Arguments:
  MaxLength - ������󳤶�
  szPath - �����ȡ��������
    
--*/
{
	BOOL bRet = FALSE;
	HANDLE KeyHandle = NULL;
	LPWSTR pData = NULL;
	char szBuffer[ MAX_PATH * 2 ] = "";
	ULONG Length = 0, ResLength = 0;
	PKEY_VALUE_PARTIAL_INFORMATION KeyValuePartialInfo = NULL ;
	UNICODE_STRING KeyName, uniBuffer ;
	OBJECT_ATTRIBUTES objattri ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 

	// 1. У������Ϸ���
	if ( NULL == szPath )
	{
		dprintf( "error! | GetPBDllPathFromReg(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	RtlInitUnicodeString( &KeyName, L"RootFolder" );
	RtlInitUnicodeString( &uniBuffer, L"\\REGISTRY\\MACHINE\\SOFTWARE\\Proteinbox\\Config" );

	InitializeObjectAttributes (
		&objattri,
		&uniBuffer,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL,
		NULL
		);

	status = ZwOpenKey( &KeyHandle, KEY_READ, &objattri );
	if ( ! NT_SUCCESS(status) ) 
	{
		dprintf( "error! | GetPBDllPathFromReg() - ZwOpenKey(); | (status=0x%08lx) \n", status );
		return FALSE ;
	}

	status = ZwQueryValueKey( KeyHandle, &KeyName, KeyValuePartialInformation, 0, 0, &ResLength );
	if( STATUS_BUFFER_TOO_SMALL != status )
	{
		dprintf( "error! | GetPBDllPathFromReg() - ZwQueryValueKey(); | (status=0x%08lx) \n", status );
		goto _END_ ;
	}

	ResLength += sizeof( KEY_VALUE_PARTIAL_INFORMATION );
	KeyValuePartialInfo = kmalloc( ResLength );
	Length = ResLength ;

	if( NULL == KeyValuePartialInfo ) 
	{
		dprintf( "error! | GetPBDllPathFromReg() - kmalloc(); | �����ڴ�ʧ��,��С:%d \n", ResLength );
		goto _END_ ;
	}

	status = ZwQueryValueKey( KeyHandle, &KeyName, KeyValuePartialInformation, (PVOID)KeyValuePartialInfo, Length, &ResLength );
	if( ! NT_SUCCESS(status) || (REG_SZ != KeyValuePartialInfo->Type) )
	{
		dprintf( "error! | GetPBDllPathFromReg() -  ZwQueryValueKey(); | (status=0x%08lx) \n", status );
		goto _FREEMEMORY_ ;
	}

	pData = (LPWSTR)KeyValuePartialInfo->Data;
	if ( NULL == pData || !pData[0] || KeyValuePartialInfo->DataLength + 0x20 > MaxLength )
	{
		dprintf( "error! | GetPBDllPathFromReg();| ��ѯ�������ݲ��Ϸ� \n" );
		goto _FREEMEMORY_ ;
	}

	bRet = w2a( pData, szBuffer, MAX_PATH * 2 );
	if ( FALSE == bRet ) 
	{ 
		dprintf( "error! | GetPBDllPathFromReg() - w2a(); | wszPath:%ws \n", pData );
		goto _FREEMEMORY_ ;
	}

	strcpy( szPath, szBuffer );
	strcat( szPath, "\\ProteinBoxDLL.dll" );

_FREEMEMORY_ :
	kfree( KeyValuePartialInfo );
_END_ :
	ZwClose( KeyHandle );
	return TRUE ;
}


BOOL GetPBDllPathFromIni( OUT LPSTR* lpszPath, IN BOOL bReload )
/*++

Author: sudami [sudami@163.com]
Time  : 2011/12/21 [21:12:2011 - 14:11]

Routine Description:
  ��ѯ�����ļ�����ȡProteinboxDll.dll������·����������ȫ�ֱ����С�
    
Arguments:
  lpwszPath - �����ȡ���Ĵ�ע���DLLȫ·��; ����ΪNULL
  bReload - TRUE ����Ҫ���²�ѯ / FALSE ����������ǵ�һ�β�ѯ���Ͳ����ٲ��ˣ�ֱ������ǰ�����ֵ����

Return Value:
  TRUE / FALSE
    
--*/
{
	BOOL bRet = FALSE, bNeedQuery = FALSE ;
	WCHAR wszPath[ MAX_PATH ] = L"";

	do 
	{
		if ( NULL == g_InjectDllPath ) 
		{
			// �ǵ�һ�β�ѯ
			g_InjectDllPath = (char*) kmallocMM( MAX_PATH + 2, MTAG___InjectDllPath );
			if ( NULL == g_InjectDllPath ) 
			{ 
				dprintf( "error! | GetPBDllPathFromIni() - kmallocMM(); | \n" );
				return FALSE; 
			}

			bNeedQuery = TRUE;
		}

		if ( FALSE == bReload && FALSE == bNeedQuery )
			break;

		RtlZeroMemory( g_InjectDllPath, MAX_PATH );

		bRet = GetPBDllPathFromIniW( wszPath );
		if ( FALSE == bRet ) 
		{ 
			dprintf( "error! | GetPBDllPathFromIni() - GetPBDllPathFromIniW(); | \n" );
			return FALSE; 
		}

		bRet = w2a( wszPath, g_InjectDllPath, MAX_PATH );
		if ( FALSE == bRet ) 
		{ 
			dprintf( "error! | GetPBDllPathFromIni() - w2a(); | wszPath:%ws \n", wszPath );
			return FALSE; 
		}

	} while (FALSE);

 	if ( lpszPath ) { *lpszPath = g_InjectDllPath; }
	dprintf( "ok! | GetPBDllPathFromIni();| Inject dll path:\"%s\"  \n", g_InjectDllPath );
	return TRUE;
}



BOOL GetPBDllPathFromIniW( OUT LPWSTR lpwszPath )
{
	BOOL bRet = FALSE ;

	// �����ļ��ѱ����¼��أ���ѯ֮
	bRet = kGetConfSingle( L"GlobalSetting", L"InjectDllPath", lpwszPath );
	if ( FALSE == bRet ) 
	{ 
		dprintf( "error! | GetPBDllPathFromIniW() - kGetConfSingle(); | get InjectDllPath failed \n" );
		return FALSE; 
	}

	return TRUE;
}



///////////////////////////////   END OF FILE   ///////////////////////////////