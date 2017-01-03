/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/06/12 [12:6:2010 - 16:04]
* MODULE : d:\Work\Program\Coding\ɳ��\SandBox\Code\Project\ProteinBoxDrv\ObjectProc.c
* 
* Description:
*      
*   ObjectHook���������д���ģ��                       
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "ObjectData.h"
#include "ObjectHook.h"
#include "ProcessData.h"
#include "Security.h"
#include "Version.h"
#include "Config.h"
#include "GrayList.h"
#include "SdtData.h"
#include "ObjectProc.h"


//////////////////////////////////////////////////////////////////////////

extern OBHOOKDATA g_OBHookData[ MaxObjectCounts ] ;


#define _get_ob_orignal_func( Index, Tag )		g_OBHookData[ Index - 1 ].Tag.OrignalAddr
#define get_ob_orignal_func( Index, Tag )		(IS_OBJECT_INDEX(Index) ? _get_ob_orignal_func( Index, Tag ) : (ULONG)0)


_ZwQueryInformationThread_ g_ZwQueryInformationThread_addr = NULL ;

/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


VOID 
fake_TokenOpen (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN PVOID Object,
	IN ACCESS_MASK GrantedAccess,
	IN ULONG HandleCount
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE pNode = NULL ;
	OB_OPEN_METHOD Open = (OB_OPEN_METHOD) get_ob_orignal_func( TOKEN_INDEX, Open ) ; 

	PreOpenHelper();

 	status = Reduce_TokenPrivilegeGroups( (PVOID)pNode, (HANDLE)Object, GrantedAccess );
 	if( ! NT_SUCCESS( status ) ) { return ; }

_Call_OrignalFunc_ :
	if ( Open ) { Open( OpenReason, Process, Object, GrantedAccess, HandleCount ); }
	return  ;
}



VOID 
fake_TokenOpen_Vista (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN ULONG Reserved1,
	IN PVOID Object,
	IN ACCESS_MASK* pGrantedAccess,
	IN ULONG HandleCount
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE pNode = NULL ;
	OB_OPEN_METHOD_VISTA Open = (OB_OPEN_METHOD_VISTA) get_ob_orignal_func( TOKEN_INDEX, Open ) ; 

	PreOpenHelper();

	status = Reduce_TokenPrivilegeGroups( (PVOID)pNode, (HANDLE)Object, *pGrantedAccess );
	if( ! NT_SUCCESS( status ) ) { return ; }

_Call_OrignalFunc_ :
	if ( Open ) { Open( OpenReason, Process, Reserved1, Object, pGrantedAccess, HandleCount ); }
	return  ;
}



VOID 
fake_ProcessOpen (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN PVOID Object,
	IN ACCESS_MASK GrantedAccess,
	IN ULONG HandleCount
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE pNode = NULL ;
	OB_OPEN_METHOD Open = (OB_OPEN_METHOD) get_ob_orignal_func( PROCESS_INDEX, Open ) ;

	PreOpenHelper();

	status = ObjectOPProcessFilter( (PVOID)pNode, Object, OpenReason, GrantedAccess );
	if( ! NT_SUCCESS( status ) ) { return ; }

_Call_OrignalFunc_ :
	if ( Open ) { Open( OpenReason, Process, Object, GrantedAccess, HandleCount ); }
	return  ;
}



VOID 
fake_ProcessOpen_Vista (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN ULONG Reserved1,
	IN PVOID Object,
	IN ACCESS_MASK* pGrantedAccess,
	IN ULONG HandleCount
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE pNode = NULL ;
	OB_OPEN_METHOD_VISTA Open = (OB_OPEN_METHOD_VISTA) get_ob_orignal_func( PROCESS_INDEX, Open ) ;

	PreOpenHelper();

	status = ObjectOPProcessFilter( (PVOID)pNode, Object, OpenReason, *pGrantedAccess );
	if( ! NT_SUCCESS( status ) ) { return ; }

_Call_OrignalFunc_ :
	if ( Open ) { Open( OpenReason, Process, Reserved1, Object, pGrantedAccess, HandleCount ); }
	return  ;
}



VOID 
fake_ThreadOpen (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN PVOID Object,
	IN ACCESS_MASK GrantedAccess,
	IN ULONG HandleCount
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE pNode = NULL ;
	OB_OPEN_METHOD Open = (OB_OPEN_METHOD) get_ob_orignal_func( THREAD_INDEX, Open ) ;

	PreOpenHelper();

	status = ObjectOPThreadFilter( (PVOID)pNode, Object, OpenReason, GrantedAccess );
	if( ! NT_SUCCESS( status ) ) { return ; }

_Call_OrignalFunc_ :
	if ( Open ) { Open( OpenReason, Process, Object, GrantedAccess, HandleCount ); }
	return  ;
}



VOID 
fake_ThreadOpen_Vista (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN ULONG Reserved1,
	IN PVOID Object,
	IN ACCESS_MASK* pGrantedAccess,
	IN ULONG HandleCount
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE pNode = NULL ;
	OB_OPEN_METHOD_VISTA Open = (OB_OPEN_METHOD_VISTA) get_ob_orignal_func( THREAD_INDEX, Open ) ;

	PreOpenHelper();

	status = ObjectOPThreadFilter( (PVOID)pNode, Object, OpenReason, *pGrantedAccess );
	if( ! NT_SUCCESS( status ) ) { return ; }

_Call_OrignalFunc_ :
	if ( Open ) { Open( OpenReason, Process, Reserved1, Object, pGrantedAccess, HandleCount ); }
	return  ;
}



VOID 
fake_EventOpen (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN PVOID Object,
	IN ACCESS_MASK GrantedAccess,
	IN ULONG HandleCount
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE pNode = NULL ;
	OB_OPEN_METHOD Open = (OB_OPEN_METHOD) get_ob_orignal_func( EVENT_INDEX, Open ) ;

	PreOpenHelper();

	status = ObjectOPFilter( (PVOID)pNode, Object, OpenReason, GrantedAccess );
	if( ! NT_SUCCESS( status ) ) { return ; }

_Call_OrignalFunc_ :
	if ( Open ) { Open( OpenReason, Process, Object, GrantedAccess, HandleCount ); }
	return  ;
}



VOID 
fake_EventOpen_Vista (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN ULONG Reserved1,
	IN PVOID Object,
	IN ACCESS_MASK* pGrantedAccess,
	IN ULONG HandleCount
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE pNode = NULL ;
	OB_OPEN_METHOD_VISTA Open = (OB_OPEN_METHOD_VISTA) get_ob_orignal_func( EVENT_INDEX, Open ) ;

	PreOpenHelper();

	status = ObjectOPFilter( (PVOID)pNode, Object, OpenReason, *pGrantedAccess );
	if( ! NT_SUCCESS( status ) ) { return ; }

_Call_OrignalFunc_ :
	if ( Open ) { Open( OpenReason, Process, Reserved1, Object, pGrantedAccess, HandleCount ); }
	return  ;
}



VOID 
fake_MutantOpen (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN PVOID Object,
	IN ACCESS_MASK GrantedAccess,
	IN ULONG HandleCount
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE pNode = NULL ;
	OB_OPEN_METHOD Open = (OB_OPEN_METHOD) get_ob_orignal_func( MUTANT_INDEX, Open ) ;

	PreOpenHelper();

	status = ObjectOPFilter( (PVOID)pNode, Object, OpenReason, GrantedAccess );
	if( ! NT_SUCCESS( status ) ) { return ; }

_Call_OrignalFunc_ :
	if ( Open ) { Open( OpenReason, Process, Object, GrantedAccess, HandleCount ); }
	return  ;
}



VOID 
fake_MutantOpen_Vista (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN ULONG Reserved1,
	IN PVOID Object,
	IN ACCESS_MASK* pGrantedAccess,
	IN ULONG HandleCount
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE pNode = NULL ;
	OB_OPEN_METHOD_VISTA Open = (OB_OPEN_METHOD_VISTA) get_ob_orignal_func( MUTANT_INDEX, Open ) ;

	PreOpenHelper();

	status = ObjectOPFilter( (PVOID)pNode, Object, OpenReason, *pGrantedAccess );
	if( ! NT_SUCCESS( status ) ) { return ; }

_Call_OrignalFunc_ :
	if ( Open ) { Open( OpenReason, Process, Reserved1, Object, pGrantedAccess, HandleCount ); }
	return  ;
}



VOID 
fake_SemaphoreOpen (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN PVOID Object,
	IN ACCESS_MASK GrantedAccess,
	IN ULONG HandleCount
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE pNode = NULL ;
	OB_OPEN_METHOD Open = (OB_OPEN_METHOD) get_ob_orignal_func( SEMAPHORE_INDEX, Open ) ;

	PreOpenHelper();

	status = ObjectOPFilter( (PVOID)pNode, Object, OpenReason, GrantedAccess );
	if( ! NT_SUCCESS( status ) ) { return ; }

_Call_OrignalFunc_ :
	if ( Open ) { Open( OpenReason, Process, Object, GrantedAccess, HandleCount ); }
	return  ;
}



VOID 
fake_SemaphoreOpen_Vista (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN ULONG Reserved1,
	IN PVOID Object,
	IN ACCESS_MASK* pGrantedAccess,
	IN ULONG HandleCount
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE pNode = NULL ;
	OB_OPEN_METHOD_VISTA Open = (OB_OPEN_METHOD_VISTA) get_ob_orignal_func( SEMAPHORE_INDEX, Open ) ;

	PreOpenHelper();

	status = ObjectOPFilter( (PVOID)pNode, Object, OpenReason, *pGrantedAccess );
	if( ! NT_SUCCESS( status ) ) { return ; }

_Call_OrignalFunc_ :
	if ( Open ) { Open( OpenReason, Process, Reserved1, Object, pGrantedAccess, HandleCount ); }
	return  ;
}



VOID 
fake_SectionOpen (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN PVOID Object,
	IN ACCESS_MASK GrantedAccess,
	IN ULONG HandleCount
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE pNode = NULL ;
	OB_OPEN_METHOD Open = (OB_OPEN_METHOD) get_ob_orignal_func( SECTION_INDEX, Open ) ;

	PreOpenHelper();

	status = ObjectOPFilter( (PVOID)pNode, Object, OpenReason, GrantedAccess );
	if( ! NT_SUCCESS( status ) ) { return ; }

_Call_OrignalFunc_ :
	if ( Open ) { Open( OpenReason, Process, Object, GrantedAccess, HandleCount ); }
	return  ;
}



VOID 
fake_SectionOpen_Vista (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN ULONG Reserved1,
	IN PVOID Object,
	IN ACCESS_MASK* pGrantedAccess,
	IN ULONG HandleCount
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE pNode = NULL ;
	OB_OPEN_METHOD_VISTA Open = (OB_OPEN_METHOD_VISTA) get_ob_orignal_func( SECTION_INDEX, Open ) ;

	PreOpenHelper();

	status = ObjectOPFilter( (PVOID)pNode, Object, OpenReason, *pGrantedAccess );
	if( ! NT_SUCCESS( status ) ) { return ; }

_Call_OrignalFunc_ :
	if ( Open ) { Open( OpenReason, Process, Reserved1, Object, pGrantedAccess, HandleCount ); }
	return  ;
}



VOID 
fake_LpcPortOpen (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN PVOID Object,
	IN ACCESS_MASK GrantedAccess,
	IN ULONG HandleCount
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE pNode = NULL ;
	PVOID NewObject = NULL ;
	OB_OPEN_METHOD Open = (OB_OPEN_METHOD) get_ob_orignal_func( LPCPORT_INDEX, Open ) ;

	PreOpenHelper();

	NewObject = GetFileObject( Object );
	if ( NULL == NewObject ){ goto _Call_OrignalFunc_ ; }

	status = ObjectOPFilter( (PVOID)pNode, NewObject, OpenReason, GrantedAccess );
	if( ! NT_SUCCESS( status ) ) { return ; }

_Call_OrignalFunc_ :
	if ( Open ) { Open( OpenReason, Process, Object, GrantedAccess, HandleCount ); }
	return  ;
}



VOID 
fake_LpcPortOpen_Vista (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN ULONG Reserved1,
	IN PVOID Object,
	IN ACCESS_MASK* pGrantedAccess,
	IN ULONG HandleCount
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	LPPDNODE pNode = NULL ;
	PVOID NewObject = NULL ;
	OB_OPEN_METHOD_VISTA Open = (OB_OPEN_METHOD_VISTA) get_ob_orignal_func( LPCPORT_INDEX, Open ) ;

	PreOpenHelper();

	NewObject = GetFileObject( Object );
	if ( NULL == NewObject ){ goto _Call_OrignalFunc_ ; }

	status = ObjectOPFilter( (PVOID)pNode, NewObject, OpenReason, *pGrantedAccess );
	if( ! NT_SUCCESS( status ) ) { return ; }

_Call_OrignalFunc_ :
	if ( Open ) { Open( OpenReason, Process, Reserved1, Object, pGrantedAccess, HandleCount ); }
	return  ;
}



NTSTATUS 
fake_FileParse ( 
	IN PVOID DirectoryObject,
	IN PVOID ObjectType,
	IN OUT PACCESS_STATE AccessState,
	IN KPROCESSOR_MODE AccessMode,
	IN ULONG Attributes,
	IN OUT PUNICODE_STRING CompleteName,
	IN OUT PUNICODE_STRING RemainingName,
	IN OUT PVOID Context OPTIONAL,
	IN PSECURITY_QUALITY_OF_SERVICE SecurityQos OPTIONAL,
	OUT PVOID *Object
	)
{
	NTSTATUS status = STATUS_SUCCESS ; 
	IopParseFile_Context pBuffer ;
	DEVICE_TYPE DeviceType ;
	LPPDNODE pNode = NULL ;
	OB_PARSE_METHOD Parse = (OB_PARSE_METHOD) get_ob_orignal_func( FILE_INDEX, Parse ) ;

	if ( FALSE == CheckOpenPactInfo( (PVOID)&pBuffer, AccessMode, (ULONG)Context, AccessState ) )
	{
		goto _Call_OrignalFunc_ ; // �������ں�̬�ĵ���,���ù���,��ԭʼ��������
	}

	PreParseHelper();

	DeviceType = ((PFILE_OBJECT)DirectoryObject)->DeviceObject->DeviceType ;
	status = ObjectPAFileDeviceFilter (
		RemainingName,
		DeviceType,
		(PVOID)pNode,
		DirectoryObject,
		(PVOID)&pBuffer
		);

	if( ! NT_SUCCESS( status ) ) { return status ; }

_Call_OrignalFunc_ :
	if ( Parse ) { status =  Parse( DirectoryObject, ObjectType, AccessState, AccessMode, Attributes, CompleteName, RemainingName, Context, SecurityQos, Object ); }
	return status ;
}



NTSTATUS 
fake_DeviceParse ( 
	IN PVOID DirectoryObject,
	IN PVOID ObjectType,
	IN OUT PACCESS_STATE AccessState,
	IN KPROCESSOR_MODE AccessMode,
	IN ULONG Attributes,
	IN OUT PUNICODE_STRING CompleteName,
	IN OUT PUNICODE_STRING RemainingName,
	IN OUT PVOID Context OPTIONAL,
	IN PSECURITY_QUALITY_OF_SERVICE SecurityQos OPTIONAL,
	OUT PVOID *Object
	)
{
	NTSTATUS status = STATUS_SUCCESS ; 
	IopParseFile_Context pBuffer ;
	DEVICE_TYPE DeviceType ;
	LPPDNODE pNode = NULL ;
	OB_PARSE_METHOD Parse = (OB_PARSE_METHOD) get_ob_orignal_func( DEVICE_INDEX, Parse ) ; 

	if ( FALSE == CheckOpenPactInfo( (PVOID)&pBuffer, AccessMode, (ULONG)Context, AccessState ) )
	{
		goto _Call_OrignalFunc_ ; // �������ں�̬�ĵ���,���ù���,��ԭʼ��������
	}

	PreParseHelper();

	DeviceType = ((PDEVICE_OBJECT)DirectoryObject)->DeviceType ;
	status = ObjectPAFileDeviceFilter (
		RemainingName,
		DeviceType,
		(PVOID)pNode,
		DirectoryObject,
		(PVOID)&pBuffer
		);

	if( ! NT_SUCCESS( status ) ) { return status ; }

_Call_OrignalFunc_ :
	if ( Parse ) { status =  Parse( DirectoryObject, ObjectType, AccessState, AccessMode, Attributes, CompleteName, RemainingName, Context, SecurityQos, Object ); }
	return status ;
}



NTSTATUS
fake_CmpParseKey (
	IN PVOID ParseObject,
	IN PVOID ObjectType,
	IN OUT PACCESS_STATE AccessState,
	IN KPROCESSOR_MODE AccessMode,
	IN ULONG Attributes,
	IN OUT PUNICODE_STRING CompleteName,
	IN OUT PUNICODE_STRING RemainingName,
	IN OUT PVOID Context OPTIONAL,
	IN PSECURITY_QUALITY_OF_SERVICE SecurityQos OPTIONAL,
	OUT PVOID *Object
	)
{
	NTSTATUS status = STATUS_SUCCESS ; 
	LPPDNODE pNode = NULL ;
	OB_PARSE_METHOD Parse = (OB_PARSE_METHOD) get_ob_orignal_func( KEY_INDEX, Parse ) ; 

	if ( 0 == AccessMode ) { goto _Call_OrignalFunc_ ; }

	PreParseHelper();

	status = ObjectKeyFilter( (PVOID)pNode, Context, ParseObject, AccessState, RemainingName );
	if( ! NT_SUCCESS( status ) ) { return status;  }

_Call_OrignalFunc_ :
	if ( Parse ) { status =  Parse( ParseObject, ObjectType, AccessState, AccessMode, Attributes, CompleteName, RemainingName, Context, SecurityQos, Object ); }
	return status ;
}



/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                    Object�ռ�Filter                       +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


NTSTATUS 
ObjectOPProcessFilter (
	IN PVOID _pNode,
	IN PVOID Object,
	IN OB_OPEN_REASON OpenReason,
	IN ACCESS_MASK GrantedAccess
	)
{

	BOOL bRet = FALSE ;

	if ( ! OpenReason ) { return STATUS_SUCCESS ; }

	//
	// 1. ����������ɳ���еĽ���,����֮
	//

	bRet = IsApprovedPID( _pNode, (ULONG)Object, GrantedAccess & 0xFFEDFBEF );
	if ( TRUE == bRet ) { return STATUS_SUCCESS ; }

	//
	// 2. �ų�����: ���ڲ���ɳ���ⲿ���̵�һ������
	//

	bRet = __CaptureStackBackTrace( OpenReason, 2 );
	if ( TRUE == bRet ) { return STATUS_SUCCESS ; }

	return STATUS_ACCESS_DENIED ;
}



NTSTATUS
ObjectOPThreadFilter (
	IN PVOID _pNode,
	IN PVOID Object,
	IN OB_OPEN_REASON OpenReason,
	IN ACCESS_MASK GrantedAccess
	)
{
	BOOL bRet = FALSE ;
	PVOID ProcessObj = NULL;

	if ( ! OpenReason ) { return STATUS_SUCCESS ; }

	//
	// 1. ����������ɳ���еĽ���,����֮
	//

	ProcessObj = (PVOID)IoThreadToProcess( Object );
	bRet = IsApprovedPID( _pNode, (ULONG)Object, GrantedAccess & 0xFFEDFFB7 );
	if ( TRUE == bRet ) { return STATUS_SUCCESS ; }

	//
	// 2. �ų�����: ���ڲ���ɳ���ⲿ���̵�һ������
	//

	bRet = __CaptureStackBackTrace( OpenReason, 2 );
	if ( TRUE == bRet ) { return STATUS_SUCCESS ; }

	return STATUS_ACCESS_DENIED ;
}



NTSTATUS
ObjectOPFilter (
	IN PVOID _pNode,
	IN PVOID Object,
	IN OB_OPEN_REASON OpenReason,
	IN ACCESS_MASK GrantedAccess
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/03 [3:8:2010 - 11:25]

Routine Description:
  ͨ���ļ�����@FileObject�õ��ļ���,�鿴���Ƿ�����Ӧ�ĺڰ�������. ���ں�������,ֱ�ӽ�ֹ��;
  ���ڰ�������,ҲҪ��ֹ����"\\KnownDlls\\*"�µĶ����2������Ȩ��:
  #define DELETE                0x00010000
  #define SECTION_EXTEND_SIZE   0x0010

Return Value:
  ���� - STATUS_SUCCESS
  ��ֹ - STATUS_ACCESS_DENIED,STATUS_OBJECT_NAME_NOT_FOUND
    
--*/
{
	BOOL bRet = TRUE, bHasNoName = FALSE ;
	BOOL bIsSelfSession = FALSE ;
	BOOL bIsWhite = FALSE, bIsBlack =FALSE ;
	NTSTATUS status = STATUS_SUCCESS ; 
	UNICODE_STRING NameInfo = { 0 };
	ULONG NameInfoLength = 0 ;
	POBJECT_NAME_INFORMATION ObjectNameInfo = NULL ;
	LPPDNODE pNode = (LPPDNODE) _pNode ;

	// 1. У������Ϸ���
	if ( NULL == pNode )
	{
		OBTrace( "error! | ObjectOPFilter(); | Invalid Paramaters; failed! \n" );
		return STATUS_SUCCESS ;
	}

	// 2. ������Ӧ�ĺڰ�����
	if ( FALSE == pNode->XIpcPath.bFlagInited )
	{
		bRet = BuildGrayList_for_OpenProcedure( (PVOID)pNode );
		if ( FALSE == bRet )
		{
			OBTrace( "error! | ObjectOPFilter() - BuildGrayList_for_IopParse(); | ����������ʧ��. \n" );
			pNode->bDiscard = 1 ;
			return STATUS_PROCESS_IS_TERMINATING;
		}
	}

	// 3. �õ�������
// 	NameInfo.Buffer = NULL;
// 	status = QueryNameString( Object, &NameInfo, &NameInfoLength );
// 	if ( ! NT_SUCCESS(status) || _key_unknown_executable_image_for_QueryNameString_ == NameInfoLength ) { return status; }

 	ObjectNameInfo = SepQueryNameString( Object, &bHasNoName ); // �ɵ������ͷ��ڴ�
 	if ( NULL == ObjectNameInfo )
 	{
 		if ( bHasNoName ) {
 			return STATUS_SUCCESS ;
 		} else {
 			return STATUS_UNSUCCESSFUL ;
 		}
 	}

	if ( MyRtlCompareUnicodeString( (LPName_Info)ObjectNameInfo, pNode->pNode_C->LpcRootPath1, pNode->pNode_C->LpcRootPath1Length) )
	{
		bIsSelfSession = TRUE ;
	}

	status = STATUS_SUCCESS;
	if ( FALSE == bIsSelfSession )
	{
		WCHAR		OutName[ MAX_PATH ]	= L"" ;
		PERESOURCE	QueueLockList = pNode->XIpcPath.pResource ;
		LPWSTR		lpCompareName = ObjectNameInfo->Name.Buffer ;

		// ��������
		bIsBlack = GLFindNodeEx( &pNode->XIpcPath.ClosedIpcPathListHead, QueueLockList, lpCompareName, OutName );
		if ( bIsBlack ) // �ں�����ClosedIpcPath��
		{
			status = STATUS_ACCESS_DENIED ; 

			if ( pNode->XIpcPath.bFlag_NotifyStartRunAccessDenied ) // ����ֹ������������Ժ���Ҫ֪ͨ�û�,�������������֪ͨ.
			{
				if ( L'*' == OutName[0] && ! OutName[1] )
				{
					pNode->XIpcPath.bFlag_NotifyStartRunAccessDenied = 0 ;
				}
			}
		}

		// ��������
		bIsWhite = GLFindNodeEx( &pNode->XIpcPath.OpenIpcPathListHead, QueueLockList, lpCompareName, OutName );
		if ( bIsWhite ) // �ڰ�����.OpenIpcPath��
		{
			if ( (L'\\' == OutName[0]) && (L'K' == OutName[1]) && (L'n' == OutName[2]) )
			{
				if ( 0 == _wcsicmp( OutName, L"\\KnownDlls\\*" ) )
				{
					ULONG DenneyAccess = SECTION_EXTEND_SIZE | DELETE ;

					if ( GrantedAccess & DenneyAccess )
					{
						status = STATUS_ACCESS_DENIED ;
					}

					if ( pNode->XIpcPath.bFlag_Denny_Access_KnownDllsSession )
					{
						status = STATUS_OBJECT_NAME_NOT_FOUND ;
					}
				}
			}
		}

		if ( bIsBlack || FALSE == bIsWhite )
			status = STATUS_ACCESS_DENIED;
	}

	if ( STATUS_ACCESS_DENIED == status )
	{
		if ( __CaptureStackBackTrace( OpenReason, 2 ) ) 
		{ 
			status = STATUS_SUCCESS; 
		}
		else
		{
			dprintf( "ko! | ObjectOPFilter() - __CaptureStackBackTrace() | �쳣���ã��ܾ��� (ObjName=%ws) \n", ObjectNameInfo->Name.Buffer );
		}


	
		
		
		//
		// just fotr test,��R3��hook�㶼��������,�����ȥ��.Ҫ��Ȼ�öණ�����ᱻ�ܾ�,�������Ѿ���.
		//

	//	status = STATUS_SUCCESS; 
	}

//	kfree( NameInfo.Buffer );
	kfree( (PVOID)ObjectNameInfo );
	return status ;
}



NTSTATUS
ObjectPAFileDeviceFilter (
	IN OUT PUNICODE_STRING RemainingName,
	IN DEVICE_TYPE DeviceType,
	IN PVOID _pNode,
	IN PVOID DirectoryObject,
	IN PVOID pBuffer
	)
{
	BOOL bRet = FALSE ;
	BOOL bDeviceType_Is_Pipe_Mailslot = FALSE, bIs_DefaultBox_Self = FALSE ;
	BOOL bSuspiciousOption = FALSE ;
	BOOL bNeed2Free = FALSE ;
	LPWSTR pw = NULL, pTmp = NULL ;
	LPWSTR StartPtr = NULL ;
	LPWSTR lpCompareName = NULL ;
	WCHAR OutName[ MAX_PATH ]	= L"" ;
	ULONG StartIndex = 0 ;
	ULONG NameIndex = 0 ;
	ULONG Disposition, CreateOptions ; 
	UNICODE_STRING Name = { 0 };
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	PERESOURCE QueueLockList = NULL ;
	LPPDNODE pNode = (LPPDNODE) _pNode ;
	LPIopParseFile_Context Buffer = (LPIopParseFile_Context) pBuffer ;

	//
	// 1. У������Ϸ���
	//
	
	if ( NULL == pNode || NULL == pBuffer )
	{
		OBTrace( "error! | ObjectPAFileDeviceFilter(); | Invalid Paramaters; failed! \n" );
		return STATUS_SUCCESS ;
	}

	//
	// 2. ������Ӧ�ĺڰ�����
	//

	if ( FALSE == pNode->XFilePath.bFlagInited )
	{
		bRet = BuildGrayList_for_IopParse( (PVOID)pNode );
		if ( FALSE == bRet )
		{
			OBTrace( "error! | ObjectPAFileDeviceFilter() - BuildGrayList_for_IopParse(); | ����������ʧ��. \n" );
			pNode->bDiscard = 1 ;
			return STATUS_PROCESS_IS_TERMINATING;
		}
	}

	//  
	// 3. ֻ�����ض����豸����,�����Ĳ���
	//

	if (   DeviceType != FILE_DEVICE_DISK
		&& DeviceType != FILE_DEVICE_NAMED_PIPE
		&& DeviceType != FILE_DEVICE_MAILSLOT
		&& DeviceType != FILE_DEVICE_NETWORK
		&& DeviceType != FILE_DEVICE_MULTI_UNC_PROVIDER
		&& DeviceType != FILE_DEVICE_NETWORK_FILE_SYSTEM
		&& DeviceType != FILE_DEVICE_DFS 
		)
	{
		return STATUS_SUCCESS;
	}

	if ( NULL == RemainingName || 0 == RemainingName->Length ) { return STATUS_SUCCESS; }

	status = GetRemainingName( DirectoryObject, RemainingName, &Name ); // �ɹ�����Ҫ�������ͷ��ڴ�
	if( ! NT_SUCCESS( status ) ) { return STATUS_SUCCESS ; }

	//  
	// 4. �����������Ϊ"�ܵ�(PIPE)"����"�Ͳ�(MAILSLOT)"�����
	//  

	if ( FILE_DEVICE_NAMED_PIPE == DeviceType || FILE_DEVICE_MAILSLOT == DeviceType )
	{
		bDeviceType_Is_Pipe_Mailslot = TRUE ;

		pw = Name.Buffer ;
		if ( _wcsnicmp( pw, L"\\Device\\", 8 ) )
			goto _Check_DesiredAccess_ ;

		pw += 8 ;
		if ( 0 == _wcsnicmp( pw, L"NamedPipe", 9) )
		{
			pTmp = pw + 9 ;
		}
		else if ( 0 == _wcsnicmp( pw, L"MailSlot", 8 ) )
		{
			pTmp = pw + 8 ;
		}
		else
		{
			goto _Check_DesiredAccess_ ;
		}

		if ( pTmp && *pTmp )
		{			
			if ( '\\' == *pTmp )
			{
				++ pTmp ;
				if ( 0 != _wcsnicmp( pTmp, pNode->pNode_C->LpcRootPath2, pNode->pNode_C->LpcRootPath2Length / sizeof(WCHAR) - 1 ) )
					goto _Check_DesiredAccess_;

				bIs_DefaultBox_Self = TRUE ;
		//		StartIndex = (ULONG) ((PCHAR)pTmp - (PCHAR)Name.Buffer) / sizeof(WCHAR)  ;
		//		StartPtr = &pTmp[ pNode->pNode_C->LpcRootPath2Length / sizeof(WCHAR) ] ;
			}
	
			pTmp =  NULL ;
		}
	}
	else
	{
		if ( 0 == _wcsnicmp( Name.Buffer, pNode->pNode_C->FileRootPath, pNode->pNode_C->FileRootPathLength / sizeof(WCHAR) - 1 ) )
		{
			bIs_DefaultBox_Self = TRUE ;
		}
	} 

	//  
	// 5. Ȩ�޹���
	//  

_Check_DesiredAccess_ :

	if ( bDeviceType_Is_Pipe_Mailslot )
	{
		bSuspiciousOption = TRUE  ;     // ������ļ�������Pipe | Mailslot,����Ϊ"������ò����Ŀ�����,���ڰ������в鿴�Ƿ�����ǰ��Խ�����"
		
		if ( pTmp && !*pTmp && !(Buffer->OriginalDesiredAccess & 0x7FEDFF56) )
		{
			bSuspiciousOption = FALSE ;
		}
	}
	else
	{
		if ( Buffer->OriginalDesiredAccess & 0x7FEDFF56 )
			bSuspiciousOption = TRUE ; // ������ļ��ķ�ʽDesiredAccess������,����Ϊ"������ò����Ŀ�����,���ڰ������в鿴�Ƿ�����ǰ��Խ�����"
	}

	if ( Buffer->u.AccessMode )
	{
		// �����û�̬�����
		if ( FILE_OPEN != Buffer->Disposition ) // ������ļ��ķ�ʽ����"FILE_OPEN",����Ϊ"������ò����Ŀ�����,���ڰ������в鿴�Ƿ�����ǰ��Խ�����" 
			bSuspiciousOption = TRUE ;                        

		CreateOptions = Buffer->CreateOptions;
		if ( Buffer->CreateOptions & FILE_DELETE_ON_CLOSE )  // �����ļ��ķ�ʽ��"�򿪺�ر��ļ�"������,����Ϊ"������ò����Ŀ�����,���ڰ������в鿴�Ƿ�����ǰ��Խ�����"          
			bSuspiciousOption = TRUE ; 
	}
	else
	{        
		// ��KernelMode
		Disposition = 0xFFFFFFFF ;
		CreateOptions = 0xFFFFFFFF ;
	}

	 // 5.0 ����������ɳ�䱾��"\Device\HarddiskVolume1\Sandbox\AV\DefaultBox"
	status = STATUS_SUCCESS ;
	if ( bIs_DefaultBox_Self ) 
	{ 
		if ( !(Buffer->Options & 4) && (Buffer->OriginalDesiredAccess & 0x52000106) )
		{
			if ( !(CreateOptions & FILE_NON_DIRECTORY_FILE) || (CreateOptions & FILE_DIRECTORY_FILE) )
			{
				status = STATUS_ACCESS_DENIED ;
			}
		}

		goto _END_ ;
	}        

	QueueLockList = pNode->XFilePath.pResource ;

	// 5.1 ��������·��
	lpCompareName = Name.Buffer ;

	if ( 0 == _wcsnicmp( lpCompareName, L"\\Device\\LanmanRedirector", 0x18 ) ) {
		NameIndex = 0x18 ;
	} else if ( 0 == _wcsnicmp( lpCompareName, L"\\Device\\Mup\\;LanmanRedirector", 0x1D ) ) {
		NameIndex = 0x1D ;
	}
	else {
		NameIndex = 0 ;
	}

	if ( NameIndex )
	{
		pw = &lpCompareName[ NameIndex ] ;

		if ( '\\' == lpCompareName[ NameIndex ] && pw[ 1 ] )
		{
			if ( ';' == pw[ 1 ] ) { pw = wcschr( pw + 2, '\\ '); }

			if ( pw && *pw && pw[ 1 ] )
			{
				ULONG l = wcslen( pw + 1 ) * sizeof( WCHAR ) + 0x26 ;
				LPWSTR NewName = (LPWSTR) kmalloc( l );

				if ( NULL == NewName )
				{
					status = STATUS_ACCESS_DENIED ;
				}
				else
				{
					l = (wcslen( L"\\Device\\Mup" ) + 1) * sizeof(WCHAR) ;
					
					wcscpy( NewName, L"\\Device\\Mup" );
					*(NewName + l) = L'\\';
					wcscpy( NewName + l + 1, pw + 1 );

					lpCompareName = NewName ;
					bNeed2Free =  TRUE ;
				}
			}
		}
	}

	// 5.2 ��һ�����ǹ������� ClosedFilePath; ����ǰ��Ϊ��Ҫ����"��ֹ���ʵ�Ŀ¼",�ܾ���
	bRet = GLFindNodeEx( &pNode->XFilePath.ClosedFilePathListHead, QueueLockList, lpCompareName, OutName );
	if ( bRet ) // �ں�����ClosedFilePath��
	{
		if ( pNode->XFilePath.bFlag_NotifyInternetAccessDenied ) // ����ֹ������������Ժ���Ҫ֪ͨ�û�,�������������֪ͨ.
		{
			if ( 0 == wcscmp( OutName, L"\\Device\\Afd*") )
			{
				pNode->XFilePath.bFlag_NotifyInternetAccessDenied = 0 ;
			}
		}

		status = STATUS_ACCESS_DENIED ;
		goto _END_ ;
	}

	// 5.3 �һ��,����ǰ��Ϊ������,ֱ�ӷ���
	if ( FALSE == bSuspiciousOption ) { goto _END_ ; }  
	
	// 5.4 ��Ϊ����,�������� OpenFilePath;
	bRet = GLFindNodeEx( &pNode->XFilePath.OpenFilePathListHead, QueueLockList, Name.Buffer, OutName );
	if ( bRet ) // �ڰ�����OpenFilePath��
	{
		// ��ͬʱ���ں�����ReadFilePath��,���ý�ֹ��;
		bRet = GLFindNodeEx( &pNode->XFilePath.ReadFilePathListHead, QueueLockList, Name.Buffer, OutName );
		if ( bRet ) { status = STATUS_ACCESS_DENIED ; }
	}
	else
	{
		// 5.5 ������,����Ϊ����,ֱ�ӽ�ֹ��
		status = STATUS_ACCESS_DENIED ;
	}

_END_ :
	if ( bNeed2Free ) { kfree( (PVOID)lpCompareName ); }
	kfree( (PVOID)Name.Buffer );

	return status ;
}



BOOL g_Monitor_Key = FALSE ; // ����ȵ�R3��ProteinBox.dll���ע�����ض����,�����е�fake_CmParseKey���ܿ���

NTSTATUS
ObjectKeyFilter (
	IN PVOID _pNode,
	IN PVOID Context,
	IN PVOID ParseObject,
	IN OUT PACCESS_STATE AccessState,
	IN OUT PUNICODE_STRING RemainingName
	)
{
	BOOL bRet = TRUE, bDangerousAccess = FALSE ;
	PERESOURCE QueueLockList = NULL ;
	NTSTATUS status = STATUS_SUCCESS ; 
	UNICODE_STRING Name = { 0 };
	LPPDNODE pNode = (LPPDNODE) _pNode ;

	// 1. У������Ϸ���
	if( FALSE == g_Monitor_Key ) { return STATUS_SUCCESS; }

	if ( NULL == _pNode )
	{
		dprintf( "error! | ObjectKeyFilter(); | Invalid Paramaters; failed! \n" );
		return STATUS_UNSUCCESSFUL ;
	}

	// 2. ������Ӧ�ĺڰ�����,����ʧ���򽫸ýڵ���Ϊ"������",���ؾܾ���־λ
	if ( FALSE == pNode->XWnd.bFlagInited ) 
	{
		bRet = BuildGrayList_for_RegKey( (PVOID)pNode );
		if ( FALSE == bRet )
		{
			dprintf( "error! | ObjectKeyFilter() - BuildGrayList_for_RegKey(); | ����������ʧ��. \n" );
			pNode->bDiscard = 1 ;
			return STATUS_PROCESS_IS_TERMINATING ;
		}
	}

	// 3.��ȡ������������·��.
	status = GetRemainingName( ParseObject, RemainingName, &Name ); // �ɹ�����Ҫ�������ͷ��ڴ�
	if( ! NT_SUCCESS( status ) ) { return STATUS_ACCESS_DENIED ; }

	// 4. �Ź�����Ŀ¼�µ�RegKey����
	if ( 0 == _wcsnicmp( Name.Buffer, pNode->pNode_C->KeyRootPath, pNode->pNode_C->KeyRootPathLength / sizeof(WCHAR) - 1 ) ) { status = STATUS_SUCCESS ; goto _CLEARUP_ ; }

	// 5. ����������ʲôȨ�޷���,��������ֹ
	QueueLockList = pNode->XRegKey.pResource ;

	bRet = GLFindNodeEx( &pNode->XRegKey.DennyListHead, QueueLockList, Name.Buffer, NULL );
	if ( bRet ) { status = STATUS_ACCESS_DENIED ; goto _CLEARUP_ ; } // �ں�����ClosedKeyPath��

	// 6.1 �Ǻ�ֻ�����ض���Ȩ��,��û��Σ�յ�Ȩ��,����е�
	bDangerousAccess = FALSE ;
	if ( AccessState->OriginalDesiredAccess & 0x7FEDFFE6 ) { bDangerousAccess = TRUE ; }

	if ( Context )
	{
		if ( g_Version_Info.IS___win2k || g_Version_Info.IS___xp || *(BYTE *)Context & 1 )
		{
			bDangerousAccess = TRUE ;
		}
	}

	if ( FALSE == bDangerousAccess ) { status = STATUS_SUCCESS ; goto _CLEARUP_ ; } // û��Σ�յ�Ȩ��,ֱ�ӷ���

	// 6.2 ��Σ�յ�Ȩ��,���ڷǰ������Ĳ���,һ�ɽ�ֹ
	bRet = GLFindNodeEx( &pNode->XRegKey.DirectListHead, QueueLockList, Name.Buffer, NULL );
	if ( FALSE == bRet ) { status = STATUS_ACCESS_DENIED ; goto _CLEARUP_ ; } 

	// 6.3 ��Σ�յ�Ȩ��,��ʹ�ڰ�������,ҲҪ��ֹ�Զ� @ReadKeyPathָ����ֵ(Read Only RegKey)�ķ���
	bRet = GLFindNodeEx( &pNode->XRegKey.ReadOnlyListHead, QueueLockList, Name.Buffer, NULL );
	if ( bRet ) { 
		status = STATUS_ACCESS_DENIED ;  
	} else {
		status = STATUS_SUCCESS ;
	}

_CLEARUP_ :
	kfree( (PVOID)Name.Buffer );
	return status ;
}


BOOL
IsApprovedTID (
	IN PVOID _pNode,
	IN HANDLE TID,
	OUT HANDLE *PID
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/25 [25:8:2010 - 0:11]

Routine Description:
  ����ZwOpenThread,ZwQueryInformationThread�õ�@TID��Ӧ��PID,���浽@PID��,
  ����IsApprovedPIDEx()����PID�Ƿ���ɳ���е�ĳ�����,�����̲����ڴ˽��������. 

Return Value:
  TRUE - �����Ϸ�,����֮;
  FALSE - ��ǰPID��������������,˵���ǲ�������ɳ����Ľ���,��Ҫ��ֹ
    
--*/
{
	BOOL bRet = TRUE ;
	HANDLE hThread = NULL ;
	CLIENT_ID ClientId ;
	NTSTATUS status = STATUS_SUCCESS ; 
	OBJECT_ATTRIBUTES ObjectAttributes ;
	THREAD_BASIC_INFORMATION ThreadInfo ; 

	// 1. У������Ϸ���
	if ( NULL == _pNode || NULL == TID )
	{
		dprintf( "error! | IsApprovedTID(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	// 2.����߳̾��
	InitializeObjectAttributes( &ObjectAttributes, NULL, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, 0, 0 );

	ClientId.UniqueProcess =  0 ;
	ClientId.UniqueThread  = TID ;

	status = ZwOpenThread (
		&hThread,
		THREAD_QUERY_INFORMATION,
		&ObjectAttributes,
		&ClientId
		);

	if ( ! NT_SUCCESS(status) )
	{
		dprintf( "error! | IsApprovedTID() - ZwOpenThread(); | status=0x8lx \n", status );
		return FALSE ;
	}

	// 3. ��ȡ ZwQueryInformationThread ��ԭʼ��ַ,����֮
	bRet = kgetaddrSDT( ZwQueryInformationThread );
	if ( FALSE == bRet )
	{ 
		dprintf( "error! | IsApprovedTID() - kgetaddrSDT(); | �޷���ȡZwQueryInformationThread��ַ \n" );
		return FALSE ; 
	}

	status = g_ZwQueryInformationThread_addr (
		hThread ,
		ThreadBasicInformation ,
		&ThreadInfo ,
		sizeof (ThreadInfo) ,
		NULL
		);

	if ( ! NT_SUCCESS(status) )
	{
		dprintf( "error! | IsApprovedTID() - ZwQueryInformationThread(); | status=0x8lx \n", status );
		ZwClose( hThread );
		return FALSE ;
	}

	// 4. ���TID��Ӧ��PID�Ƿ�����ɳ��
	if ( PID )
	{
		*PID = ThreadInfo.ClientId.UniqueProcess ;
	}

	bRet = IsApprovedPIDEx( _pNode, (ULONG)ThreadInfo.ClientId.UniqueProcess );
	
	ZwClose( hThread );
	return bRet ;
}



BOOL 
IsApprovedPID (
	IN PVOID _pNode,
	IN ULONG Process,
	IN ULONG GrantedAccess
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/17 [17:6:2010 - 17:13]

Routine Description:
  ����PsGetProcessId�õ�EPROCESS��Ӧ��PID,������Is_CurrentPID_match_Node()���кϷ��Լ��
  ����ɳ����Ľ���,����FALSE,������δ����ɵĲ���,�ܾ�������ɳ������̵���Ϊ.  

Arguments:
  Process - ���̶���
  GrantedAccess - ��Ӧ�Ĳ���Ȩ��

--*/
{
	ULONG PID = 0, bRet = 0 ;

	if ( NULL == _pNode || 0 == Process || 0 == GrantedAccess )
	{
	//	dprintf( "error! | IsApprovedPID(); | Invalid Paramaters; failed! (Process=0x%08lx, GrantedAccess=%d) \n", Process, GrantedAccess );
		return TRUE ; // Ĭ�ϷŹ�,������TRUE,��������ɵ���Ϊ; ���������������ϵͳ������
	}

	if ( g_PsGetProcessId_addr ) 
	{
		PID = (ULONG) g_PsGetProcessId_addr( (PEPROCESS)Process );
	}
	else
	{
		dprintf( "error! | IsApprovedPID(); | g_PsGetProcessId_addr ��ַΪ�գ����»�ȡPIDʧ�ܣ������ˡ� \n" );
	}

	if ( 0 == PID )
	{
		dprintf( "error! | IsApprovedPID(); | 0 == PID \n" );
		return TRUE ;
	}
	
	if ( !GrantedAccess || IsApprovedPIDEx( _pNode, PID ) )
	{
		return TRUE ;// ��������ɳ���еĳ���,����֮
	}

//	dprintf( "��ǰ�������ڲ���ɳ���ⲿ�Ľ���(PID=%d),�ѱ��ܾ���. \n", PID );
	return FALSE;
}



BOOL
IsApprovedPIDEx (
	IN PVOID _pNode,
	IN ULONG PID
	)
{
	LPPDNODE pNodeVictim = NULL ;				// �ܺ���			
	LPPDNODE pNodeInvader = (LPPDNODE) _pNode ; // ������

	// 1. У������Ϸ���
	if ( NULL == pNodeInvader )
	{
		dprintf( "error! | IsApprovedPIDEx(); | Invalid Paramaters; failed! \n" );
		return TRUE ; // Ĭ�ϷŹ�,������TRUE,��������ɵ���Ϊ; ���������������ϵͳ������
	}

	// 2. ����PID��Ӧ�Ľ����ܽڵ�
	pNodeVictim = (LPPDNODE) kgetnodePD( PID );
	if ( NULL == pNodeVictim ) { return FALSE ; } // ���ܺ��߲���ɳ����,������

	if ( pNodeVictim->bDiscard || pNodeVictim->bReserved1 ) { return FALSE ; } // ���ܺ�����ɳ����,���Ǳ�������,������

	if ( pNodeInvader->pNode_C->SessionId != pNodeVictim->pNode_C->SessionId ) { return FALSE ; } // ������ɳ���ڵĽ���,���û�ID��ͬ,������

	if ( pNodeInvader->pNode_C->BoxNameLength != pNodeVictim->pNode_C->BoxNameLength ) { return FALSE ; } // ������ͬһ��ɳ��Ľ���,������
	if ( _wcsicmp( pNodeInvader->pNode_C->BoxName, pNodeVictim->pNode_C->BoxName ) ) { return FALSE ; } // ������ͬһ��ɳ��Ľ���,������

	return TRUE ; // Ĭ�ϱ���,�����ǲ�����ɳ���еĳ���,����е�ǰ����
}



BOOL
__CaptureStackBackTrace (
	IN OB_OPEN_REASON OpenReason,
	IN ULONG __FramesToSkip
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/17 [17:6:2010 - 17:33]

Routine Description:
  �����������Ҫ��ͨ��ջ����(ָ��N��),�ж������������Ƿ�Ϸ���
  ����������3�����ô˺��������Ǵ�fake_xx��������2��ջ�����ģ�����@__FramesToSkip Ϊ2��
  ��������ط����øú�����Ҫ�Լ���������˼���ջ�Ž����ģ�Ȼ��ָ��@__FramesToSkip
    
Arguments:
  OpenReason - �����˵Ĳ���
  __FramesToSkip - ��Ҫ�����Ķ�ջ����

--*/
{
	ULONG Frames[ 32 ] ;
	ULONG Addr = 0 ;
	ULONG FramesToSkip = __FramesToSkip + 1 ;

	if ( TRUE == g_Version_Info.IS___win7 )
	{
		if ( 1 == RtlCaptureStackBackTrace( FramesToSkip + 2, 1, (PVOID*)Frames, NULL) )
		{
			Addr = Frames[ 0 ];
			if ( (Addr >= g_CaptureStackBackTrace_needed_data->ObOpenObjectByPointerWithTag_addr_vista) 
				&& (Addr <= g_CaptureStackBackTrace_needed_data->ObOpenObjectByPointerWithTag_addr_vista + 0xF0) 
				)
			{
				if ( 1 == RtlCaptureStackBackTrace( FramesToSkip + 3, 1, (PVOID*)Frames, NULL) )
				{
					Addr = Frames[ 0 ];
					if ( (Addr >= g_CaptureStackBackTrace_needed_data->ObOpenObjectByPointer_addr_vista) 
						&& (Addr <= g_CaptureStackBackTrace_needed_data->ObOpenObjectByPointer_addr_vista + 0x50) 
						)
					{
						++ FramesToSkip ;
					}
				}
			}
		}
	}

	if ( OpenReason && OpenReason != 1 )
	{
		if ( 2 != OpenReason ) { return FALSE ; }

		if ( 1 != RtlCaptureStackBackTrace( FramesToSkip + 2, 1, (PVOID*)Frames, NULL ) ) { return FALSE ; }
		
		Addr = Frames[ 0 ];
		if ( Addr > g_CaptureStackBackTrace_needed_data->KeFindConfigurationEntry_addr )
		{ 
			dprintf( "__CaptureStackBackTrace(); ƥ�䵽�Ϸ���ַ,����TRUE. (Addr=0x%08lx) \n", Addr );
			return TRUE ;
		}
	}
	else
	{
		if ( 1 == RtlCaptureStackBackTrace( FramesToSkip + 3, 1, (PVOID*)Frames, NULL ) ) 
		{ 
			Addr = Frames[ 0 ];
			if ( Addr > g_CaptureStackBackTrace_needed_data->KeFindConfigurationEntry_addr )
			{ 
				dprintf( "__CaptureStackBackTrace(); ƥ�䵽�Ϸ���ַ,����TRUE. (Addr=0x%08lx) \n", Addr );
				return TRUE ;
			}
		}

		if ( (TRUE == g_Version_Info.IS_before_vista) || (g_Version_Info.BuildNumber <= 0x1770) ) { return FALSE ; }

		if ( 1 != RtlCaptureStackBackTrace( FramesToSkip + 6, 1, (PVOID*)Frames, NULL ) ) { return FALSE ; }

		Addr = Frames[ 0 ];
		if ( Addr >= g_CaptureStackBackTrace_needed_data->PsCreateSystemThread_addr
			&& Addr <= g_CaptureStackBackTrace_needed_data->PsCreateSystemThread_addr + 0xC0
			)
		{ 
			dprintf( "__CaptureStackBackTrace(); ƥ�䵽�Ϸ���ַ,����TRUE. (Addr=0x%08lx) \n", Addr );
			return TRUE ;
		}
	}

	return FALSE ;
}



NTSTATUS
GetRemainingName (
	IN PVOID Object,
	IN PUNICODE_STRING RemainingName,
	OUT PUNICODE_STRING Name
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/07/22 [22:7:2010 - 15:46]

Routine Description:
  ͨ���������ObQueryNameString��ȡ������,�����@RemainingNameƴ�ӵõ�����·��.
  �ɹ�����Ҫ�������ͷ��ڴ�, eg: kfree( (PVOID)Name.Buffer );
    
Arguments:
  Object - ����ѯ�Ķ�����
  RemainingName - ��ƴ�ӵ�UNICODE�ַ���
  Name - ����ƴ�Ӻ�õ���ȫ·��

Return Value:
  NTSTATUS
    
--*/
{
	BOOL bHasNoName = FALSE ;
	ULONG StartIndex = 0 ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	POBJECT_NAME_INFORMATION ObjectNameInfo = NULL ;

	// 1. У������Ϸ���
	if ( NULL == Object || NULL == RemainingName || NULL == Name )
	{
		dprintf( "error! | GetRemainingName(); | Invalid Paramaters; failed! \n" );
		return STATUS_INVALID_PARAMETER ;
	}

	// 2. �õ�������
	ObjectNameInfo = SepQueryNameString( Object, &bHasNoName );
	if ( NULL == ObjectNameInfo )
	{
	//	dprintf( "error! | GetRemainingName() - SepQueryNameString(); | NULL == ObjectNameInfo \n" );
		return STATUS_UNSUCCESSFUL ;
	}

	// 3. ƴ�Ӷ����� & RemainingName	
	Name->Length		= 0 ;
	Name->MaximumLength = ObjectNameInfo->Name.Length + 2 * sizeof(WCHAR) + RemainingName->Length + UNICODE_NULL ;
	Name->Buffer		= kmalloc( Name->MaximumLength );

	if ( NULL == Name->Buffer )
	{
		dprintf( "error! | GetRemainingName() - kmalloc(); | �����ڴ�ʧ��. \n" );
		kfree( (PVOID)ObjectNameInfo );
		return STATUS_UNSUCCESSFUL ;
	}

	RtlZeroMemory( Name->Buffer, Name->MaximumLength );
	RtlAppendUnicodeStringToString( Name, &ObjectNameInfo->Name );

	StartIndex = Name->Length / sizeof(WCHAR);
	if ( '\\' != Name->Buffer[ StartIndex ] && '\\' != RemainingName->Buffer[ 0 ] )
	{
		RtlAppendUnicodeToString( Name, L"\\" );
	}

	if ( '\\' == Name->Buffer[ StartIndex ] && '\\' == RemainingName->Buffer[ 0 ] )
	{
		Name->Buffer[ StartIndex ] = UNICODE_NULL ;
		Name->Length -- ;
		Name->MaximumLength -- ;
	}

	RtlAppendUnicodeStringToString( Name, RemainingName );
	Name->Buffer[ Name->Length / sizeof(WCHAR) ] = L'\0' ;

	kfree( (PVOID)ObjectNameInfo );
	return STATUS_SUCCESS ;
}



BOOL
CheckOpenPactInfo (
	IN PVOID Buffer,
	IN KPROCESSOR_MODE AccessMode,
	IN ULONG Context,
	IN PACCESS_STATE AccessState
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/12 [12:6:2010 - 17:42]

Routine Description:
  ����IopParseFile�����Ĳ���Context & AccessState,����ȡ��������Ϣ,�������ڲ���1 ��:
  *(DWORD *)(pBuffer + 4)    = *(DWORD *)(Context + 0x34);   // Disposition
  *(DWORD *)(pBuffer + 8)    = *(DWORD *)(Context + 0x20);   // CreateOptions
  *(DWORD *)(pBuffer + 0xC)  = *(DWORD *)(Context + 0x30);   // Options
  *(DWORD *)(pBuffer + 0x10) = *(DWORD *)(_AccessState_ + 0x18);     // OriginalDesiredAccess

  kd> dd f73c190c l14
  f73c190c 00000100 00000001 00000000 00000101
  f73c191c 40000000

Arguments:
  pBuffer - ��������е�������Ϣ
  AccessMode - һ��ΪKernelMode
  Context - IopParseFile�����Ĳ���8,�ṹ��ָ��nt!_OPEN_PACKET
  AccessState - IopParseFile�����Ĳ���3,�ṹ��ָ��nt!_ACCESS_STATE

Return Value:
  #define IO_NO_PARAMETER_CHECKING 0x100
  ����Context�Ϸ� & ���ں�̬�ĵ���(��AccessModeΪKernelMode) & Context.Options �ĵ�3λ����.
  ��3������ͬʱ�����򷵻سɹ�;���򷵻�ʧ��. Ҳ����˵�ں�̬�ĵ������Ƿ���,ֻ�����û�̬��ɳ����̵ĵ���.

--*/
{
	BOOL bIsUserMode = FALSE ;
	LPIopParseFile_Context pBuffer = (LPIopParseFile_Context) Buffer ;

	//
	// 1. У������Ϸ���
	//
	
	if ( NULL == pBuffer )
	{
		OBTrace( "error! | CheckOpenPactInfo(); | Invalid Paramaters; ��ԭʼ��������  \n" );
		return FALSE ;
	}

	if ( Context && 8 == *(WORD *)Context )
	{
		pBuffer->u.AccessMode = 1;
		if ( FALSE == g_Version_Info.IS_before_vista )
			Context += 8 ;

		pBuffer->Disposition	= *(DWORD *)(Context + 0x34) ;
		pBuffer->CreateOptions	= *(DWORD *)(Context + 0x20) ;
		pBuffer->Options		= *(DWORD *)(Context + 0x30) ;
	}
	else
	{
		pBuffer->u.AccessMode	= 0 ;
		pBuffer->Disposition	= 0 ;
		pBuffer->CreateOptions	= 0 ;
		pBuffer->Options		= 0 ;
	}

	pBuffer->u.LimitLow = AccessMode ;
	pBuffer->OriginalDesiredAccess = AccessState->OriginalDesiredAccess ;

	bIsUserMode = AccessMode || pBuffer->u.AccessMode && pBuffer->Options & 4 ;
	return bIsUserMode ;
}


///////////////////////////////   END OF FILE   ///////////////////////////////