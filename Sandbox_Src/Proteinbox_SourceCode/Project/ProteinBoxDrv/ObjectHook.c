/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/06/11 [11:6:2010 - 11:04]
* MODULE : d:\Work\Program\Coding\ɳ��\SandBox\Code\Project\ProteinBoxDrv\ObjectHook.c
* 
* Description:
*      
*   ObjectHook �ҹ���ز���                         
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "Common.h"
#include "ObjectData.h"
#include "DispatchIoctl.h"
#include "ObjectProc.h"
#include "Version.h"
#include "Memory.h"
#include "ObjectHook.h"


//////////////////////////////////////////////////////////////////////////

POBJECT_TYPE g_ObpTypeObjectType = NULL ;

_PsGetProcessId_ g_PsGetProcessId_addr		= NULL ;
_ObQueryNameInfo_ g_ObQueryNameInfo_addr	= NULL ;
_ObGetObjectType_ g_ObGetObjectType_addr	= NULL ;

OBHOOKDATA g_OBHookData[ MaxObjectCounts ] ;


LPADDR_FOR_CaptureStackBackTrace g_CaptureStackBackTrace_needed_data = NULL ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--



NTSTATUS
Ioctl_HookObject (
	IN PVOID pNode, 
	IN PVOID pInBuffer
	)
{
	BOOL bRet = TRUE ;
	LPIOCTL_HOOKOBJECT_BUFFER Buffer = NULL ;

	//
	// 1. У������Ϸ���
	//

	dprintf( "[ProteinBoxDrv] IOCTL_PROTEINBOX - Ioctl_HookObject(); \n" );

	if ( NULL == pInBuffer )
	{
		dprintf( "error! | Ioctl_HookObject(); | Invalid Paramaters; failed! \n" );
		return STATUS_INVALID_PARAMETER ;
	}

	//
	// 2. ȡ��R3��Buffer����,�����Ƿ�Hook
	//

	__try
	{
		Buffer = (LPIOCTL_HOOKOBJECT_BUFFER) getIoctlBufferBody( pInBuffer );
		if ( FALSE == MmIsAddressValid( (PVOID)Buffer ) )
		{
			dprintf( "error! | Ioctl_HookObject() - getIoctlBufferBody(); | Body��ַ���Ϸ� \n" );
			__leave ;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		dprintf( "error! | Ioctl_HookObject() - __try __except(); \n" );
		return STATUS_INVALID_ADDRESS ;
	}

	//
	// 3. ���ݱ�ǲ���Object����
	//

	if ( Buffer->bHook )
	{
		// ����Hook
		bRet = OBHook();
		if ( FALSE == bRet )
		{
			dprintf( "error! | Ioctl_HookShadow() - OBHook(); | Object Hookδ��ȫ�ɹ�! \n" );
			return STATUS_UNSUCCESSFUL ;
		}
	}
	else
	{
		// ֹͣHook
		OBUnhook();
	}

	return STATUS_SUCCESS ;
}



BOOL
HandlerObject (
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/11 [11:6:2010 - 11:06]

Routine Description:
  Ϊ����Object Hook��ؽ��гﱸ
    
--*/
{
	BOOL bRet = FALSE ;

	bRet = Prepare_for_ObjectHook() ;
	if ( FALSE == bRet )
	{
		dprintf( "error! | HandlerObject() - Prepare_for_ObjectHook(); | \n" );
		return FALSE ;
	}

	return TRUE ;
}



BOOL
Prepare_for_ObjectHook (
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/11 [11:6:2010 - 16:50]

Routine Description:
  ��ʼ�� ObjectHook Ҫ�õ�������  
    
--*/
{
	PVOID ret = NULL ;
	OBJECT_INIT Buffer ;
	LPWSTR szObjecType = NULL ;
	BOOL bIsXP = g_Version_Info.IS_before_vista ;

	//
	// A. ��ȡϵͳ������ַ
	//

	if ( FALSE == Init_objectXX_address() )
	{
		dprintf( "error! | Prepare_for_ObjectHook() - Init_objectXX_address(); | \n" );
		return FALSE ;
	}

	//
	// B. ��ʼ���ڵ�
	//

	Buffer.ProcedureOffsetOpen	= bIsXP ?  0x30 : 0x34 ;
	Buffer.ProcedureOffsetParse = bIsXP ?  0x3C : 0x40 ;
	Buffer.TypeInfoOffset		= bIsXP ?  0x60 : 0x28 ;

	// 1. ���� "Token" �ڵ�		(OpenProcedure)
	Buffer.ObjectIndex = TOKEN_INDEX ;
	szObjecType = L"Token" ;
	RtlCopyMemory( Buffer.szObjecType, szObjecType, ObjecTypeLength );
	Buffer.FakeAddrOpen = bIsXP ?  (ULONG)fake_TokenOpen : (ULONG)fake_TokenOpen_Vista ;
	Buffer.FakeAddrParse = 0 ;

	kbuildnodeOD( &Buffer );

	// 2. ���� "Process" �ڵ�	(OpenProcedure)
	Buffer.ObjectIndex = PROCESS_INDEX ;
	szObjecType = L"Process" ;
	RtlCopyMemory( Buffer.szObjecType, szObjecType, ObjecTypeLength );
	Buffer.FakeAddrOpen = bIsXP ?  (ULONG)fake_ProcessOpen : (ULONG)fake_ProcessOpen_Vista ;
	Buffer.FakeAddrParse = 0 ;

	kbuildnodeOD( &Buffer );

	// 3. ���� "Thread" �ڵ�	(OpenProcedure)
	Buffer.ObjectIndex = THREAD_INDEX ;
	szObjecType = L"Thread" ;
	RtlCopyMemory( Buffer.szObjecType, szObjecType, ObjecTypeLength );
	Buffer.FakeAddrOpen = bIsXP ?  (ULONG)fake_ThreadOpen : (ULONG)fake_ThreadOpen_Vista ;
	Buffer.FakeAddrParse = 0 ;

	kbuildnodeOD( &Buffer );

	// 4. ���� "Event" �ڵ�		(OpenProcedure)
	Buffer.ObjectIndex = EVENT_INDEX ;
	szObjecType = L"Event" ;
	RtlCopyMemory( Buffer.szObjecType, szObjecType, ObjecTypeLength );
	Buffer.FakeAddrOpen = bIsXP ?  (ULONG)fake_EventOpen : (ULONG)fake_EventOpen_Vista ;
	Buffer.FakeAddrParse = 0 ;

	kbuildnodeOD( &Buffer );

	// 5. ���� "Mutant" �ڵ�	(OpenProcedure)
	Buffer.ObjectIndex = MUTANT_INDEX ;
	szObjecType = L"Mutant" ;
	RtlCopyMemory( Buffer.szObjecType, szObjecType, ObjecTypeLength );
	Buffer.FakeAddrOpen = bIsXP ?  (ULONG)fake_MutantOpen : (ULONG)fake_MutantOpen_Vista ;
	Buffer.FakeAddrParse = 0 ;

	kbuildnodeOD( &Buffer );

	// 6. ���� "Semaphore" �ڵ� (OpenProcedure)
	Buffer.ObjectIndex = SEMAPHORE_INDEX ;
	szObjecType = L"Semaphore" ;
	RtlCopyMemory( Buffer.szObjecType, szObjecType, ObjecTypeLength );
	Buffer.FakeAddrOpen = bIsXP ?  (ULONG)fake_SemaphoreOpen : (ULONG)fake_SemaphoreOpen_Vista ;
	Buffer.FakeAddrParse = 0 ;

	kbuildnodeOD( &Buffer );

	// 7. ���� "Section" �ڵ�	(OpenProcedure)
	Buffer.ObjectIndex = SECTION_INDEX ;
	szObjecType = L"Section" ;
	RtlCopyMemory( Buffer.szObjecType, szObjecType, ObjecTypeLength );
	Buffer.FakeAddrOpen = bIsXP ?  (ULONG)fake_SectionOpen : (ULONG)fake_SectionOpen_Vista ;
	Buffer.FakeAddrParse = 0 ;

	kbuildnodeOD( &Buffer );

	// 8. ���� "Port" �ڵ�		(OpenProcedure)
	Buffer.ObjectIndex = LPCPORT_INDEX ;
	szObjecType = bIsXP ?  L"Port" : L"ALPC Port" ;
	RtlCopyMemory( Buffer.szObjecType, szObjecType, ObjecTypeLength );
	Buffer.FakeAddrOpen = bIsXP ?  (ULONG)fake_LpcPortOpen : (ULONG)fake_LpcPortOpen_Vista ;
	Buffer.FakeAddrParse = 0 ;

	kbuildnodeOD( &Buffer );

	// 9. ���� "File" �ڵ�		(ParseProcedure)
	Buffer.ObjectIndex = FILE_INDEX ;
	szObjecType = L"File" ;
	RtlCopyMemory( Buffer.szObjecType, szObjecType, ObjecTypeLength );
	Buffer.FakeAddrParse = (ULONG)fake_FileParse ;
	Buffer.FakeAddrOpen = 0 ;

	kbuildnodeOD( &Buffer );

	// 10. ���� "Device" �ڵ�	(ParseProcedure)
	Buffer.ObjectIndex = DEVICE_INDEX ;
	szObjecType = L"Device" ;
	RtlCopyMemory( Buffer.szObjecType, szObjecType, ObjecTypeLength );
	Buffer.FakeAddrParse = (ULONG)fake_DeviceParse ;
	Buffer.FakeAddrOpen = 0 ;

	kbuildnodeOD( &Buffer );

	// 11. ���� "Key" �ڵ�		(ParseProcedure)
	Buffer.ObjectIndex = KEY_INDEX ;
	szObjecType = L"Key" ;
	RtlCopyMemory( Buffer.szObjecType, szObjecType, ObjecTypeLength );
	Buffer.FakeAddrParse = (ULONG)fake_CmpParseKey ;
	Buffer.FakeAddrOpen = 0 ;

	kbuildnodeOD( &Buffer );

	return TRUE ;
}



BOOL
Init_objectXX_address(
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/11 [11:6:2010 - 18:02]

Routine Description:
  ��ȡϵͳ�����ĵ�ַ,eg: ObpTypeObjectType
    
--*/
{
	PCHAR ptr = NULL ;
	ULONG_PTR data1 = 0, data2 = 0 ;
	int ObTypeIndexTable_addr = 0, length = 0, addr = 0 ;
	UNICODE_STRING uniBuffer ;

	if ( NULL == g_CaptureStackBackTrace_needed_data )
	{
		g_CaptureStackBackTrace_needed_data = (LPADDR_FOR_CaptureStackBackTrace) kmallocMM( sizeof(ADDR_FOR_CaptureStackBackTrace), MTAG___ADDR_FOR_CaptureStackBackTrace );
		if ( NULL == g_CaptureStackBackTrace_needed_data )
		{
			dprintf( "error! | Init_objectXX_address() - kmallocMM | NULL == g_CaptureStackBackTrace_needed_data,�����ڴ�ʧ�� \n" );
			return FALSE ;
		}
	}

	if ( NULL == g_PsGetProcessId_addr )
	{
		RtlInitUnicodeString( &uniBuffer, L"PsGetProcessId" );
		g_PsGetProcessId_addr = (_PsGetProcessId_) MmGetSystemRoutineAddress( &uniBuffer );
	}

	if ( 0 == g_CaptureStackBackTrace_needed_data->KeFindConfigurationEntry_addr )
	{
		RtlInitUnicodeString( &uniBuffer, L"KeFindConfigurationEntry" );
		g_CaptureStackBackTrace_needed_data->KeFindConfigurationEntry_addr = (ULONG) MmGetSystemRoutineAddress( &uniBuffer );
	}

	if ( (0 == g_CaptureStackBackTrace_needed_data->ObOpenObjectByPointerWithTag_addr_vista) && (TRUE == g_Version_Info.IS___win7) )
	{
		RtlInitUnicodeString( &uniBuffer, L"ObOpenObjectByPointerWithTag" );
		g_CaptureStackBackTrace_needed_data->ObOpenObjectByPointerWithTag_addr_vista = (ULONG) MmGetSystemRoutineAddress( &uniBuffer );
	}

	if ( (0 == g_CaptureStackBackTrace_needed_data->ObOpenObjectByPointer_addr_vista) && (TRUE == g_Version_Info.IS___win7) )
	{
		RtlInitUnicodeString( &uniBuffer, L"ObOpenObjectByPointer" );
		g_CaptureStackBackTrace_needed_data->ObOpenObjectByPointer_addr_vista = (ULONG) MmGetSystemRoutineAddress( &uniBuffer );
	}

	if ( (0 == g_CaptureStackBackTrace_needed_data->PsCreateSystemThread_addr) && (FALSE == g_Version_Info.IS_before_vista) && (g_Version_Info.BuildNumber > 0x1770) )
	{
		RtlInitUnicodeString( &uniBuffer, L"PsCreateSystemThread" );
		g_CaptureStackBackTrace_needed_data->PsCreateSystemThread_addr = (ULONG) MmGetSystemRoutineAddress( &uniBuffer );
	}

	// win7 ����Ҫ��ȡ ObpTypeObjectType �ĵ�ַ
	if ( FALSE == g_Version_Info.IS___win7 || g_ObpTypeObjectType ) { return TRUE; }

	if ( NULL == g_ObQueryNameInfo_addr )
	{
		RtlInitUnicodeString( &uniBuffer, L"ObQueryNameInfo" );
		g_ObQueryNameInfo_addr = MmGetSystemRoutineAddress( &uniBuffer );
	}
	
	if ( NULL == g_ObGetObjectType_addr )
	{
		RtlInitUnicodeString( &uniBuffer, L"ObGetObjectType" );
		g_ObGetObjectType_addr = MmGetSystemRoutineAddress( &uniBuffer );
	}

	if ( NULL == g_ObQueryNameInfo_addr || NULL == g_ObGetObjectType_addr ) 
	{
		dprintf( "error! | Init_objectXX_address(); | ��ȡ ObQueryNameInfo / ObGetObjectType ������ַʧ�� \n" );
		return FALSE;
	}

	//  
	// ����Ĵ�������Win7�������õ�ObTypeIndexTable�ĵ�ַ,������ȡ����ĳ��ֵ
	//

	ptr = (PCHAR)g_ObGetObjectType_addr + 2 ;
	length = 0xFFFFFFFE - (int)g_ObGetObjectType_addr ;
	
	do
	{
		if ( 0x8B == *(ptr - 2) && 4 == *(ptr - 1) && 0x85 == *ptr )
		{
			ObTypeIndexTable_addr = *(int *)(ptr + 1);
		}

		if ( 0x8B == *(ptr - 1) && 4 == *ptr && 0x85 == ptr[1] )
		{
			ObTypeIndexTable_addr = *(int *)( (int)&ptr[length] + (int)g_ObGetObjectType_addr + 4 );
		}

		if ( 0x8B == *ptr && 4 == ptr[1] )
		{
			if ( 0x85 == *(BYTE *)( (int)&ptr[length] + (int)g_ObGetObjectType_addr + 4 ) )
				ObTypeIndexTable_addr = *(int *)( (int)&ptr[length] + (int)g_ObGetObjectType_addr + 5 );
		}

		if ( 0x8B == ptr[1] )
		{
			addr = (int) &ptr[ length ] ;
			if ( 4 == *(BYTE *)((int)&ptr[length] + (int)g_ObGetObjectType_addr + 4) )
			{
				if ( 0x85 == *((BYTE *)g_ObGetObjectType_addr + addr + 5) )
					ObTypeIndexTable_addr = *(int *)( (int)g_ObGetObjectType_addr + addr + 6 );
			}
		}

		ptr += 4 ;
	}
	while ( (ULONG)&ptr[length] < 0x10 );

	if ( 0 == ObTypeIndexTable_addr ) { return FALSE; }

	data1 = *(ULONG *)( ObTypeIndexTable_addr + 8 );

	if ( 0 == data1 || data1 >= 0xBAD00000 ) { return FALSE; }
	if ( 8 != *(WORD *)(data1 + 8) ) { return FALSE; }

	data2 = *(ULONG *)(data1 + 0xC);
	if ( *(WORD *)data2 == 0x54 )
	{
		if ( *(WORD *)(data2 + 2) == 0x79 && *(WORD *)(data2 + 4) == 0x70 && *(WORD *)(data2 + 6) == 0x65 )
		{
			g_ObpTypeObjectType = (POBJECT_TYPE) *(DWORD *)( ObTypeIndexTable_addr + 8 );
			return TRUE;
		}
	}

	return FALSE ;
}



BOOL
HookObjectAll (
	IN PVOID _TotalHead
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/11 [11:6:2010 - 17:59]

Routine Description:
  Object Hook ���нڵ�

--*/
{
	BOOL bRet = FALSE ;
	BOOL bOK = TRUE ;
	int i = 0 ;
	LPODNODE pNodeHead = NULL, pCurrentNode = NULL ;
	LPODHEAD pTotalHead	   = (LPODHEAD) _TotalHead ;

	//
	// 1. У������Ϸ���
	//

	if ( NULL == pTotalHead ) 
	{
		dprintf( "error! | HookObjectAll(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	if ( (0 == pTotalHead->nTotalCounts) || (TRUE == IsListEmpty( (PLIST_ENTRY) &pTotalHead->ListHead )) ) { return FALSE ; }

	//
	// 2. ��������,����ָ���ڵ�
	//

	EnterCrit( pTotalHead->QueueLockList );	// ��������

	pNodeHead    =  &pTotalHead->ListHead ;
	pCurrentNode = pNodeHead->pBlink ;

	while ( pCurrentNode != pNodeHead )
	{
		// hook OpenProcedure
		if ( TRUE == pCurrentNode->Open.bCare ) 
		{
			if ( FALSE == pCurrentNode->Open.bHooked && pCurrentNode->Open.FakeAddr )
			{
				bRet = HookObjectOne( (PVOID) &pCurrentNode->Open, (PVOID)pCurrentNode );
				if ( FALSE == bRet )
				{
					dprintf( "error! | HookObjectAll() - HookObjectOne(); | ��ǰObjectHook:\"%ws\":Open ����ʧ�� \n", pCurrentNode->szObjecType );
					bOK = FALSE ;
					break ;
				}
				else
				{
					dprintf( "ok! | [%d] ObjectHook:\"%ws\":Open �����ɹ�: OrignalAddr: 0x%08lx \n", i++, pCurrentNode->szObjecType, pCurrentNode->Open.OrignalAddr );
					g_OBHookData[ pCurrentNode->ObjectIndex - 1 ].Open.OrignalAddr = pCurrentNode->Open.OrignalAddr ; // ���浱ǰOjectԭʼ��ַ��ȫ��������
				}
			}
		}

		// hook ParseProcedure
		if ( TRUE == pCurrentNode->Parse.bCare ) 
		{
			if ( FALSE == pCurrentNode->Parse.bHooked && pCurrentNode->Parse.FakeAddr )
			{
				bRet = HookObjectOne( (PVOID) &pCurrentNode->Parse, (PVOID)pCurrentNode );
				if ( FALSE == bRet )
				{
					dprintf( "error! | HookObjectAll() - HookObjectOne(); | ��ǰObjectHook:\"%ws\":Parse ����ʧ�� \n", pCurrentNode->szObjecType );
					bOK = FALSE ;
					break ;
				}
				else
				{
					dprintf( "ok! | [%d] ObjectHook:\"%ws\":Parse �����ɹ�; OrignalAddr: 0x%08lx \n", i++, pCurrentNode->szObjecType, pCurrentNode->Parse.OrignalAddr );
					g_OBHookData[ pCurrentNode->ObjectIndex - 1 ].Parse.OrignalAddr = pCurrentNode->Parse.OrignalAddr ; // ���浱ǰOjectԭʼ��ַ��ȫ��������
				}
			}
		}

		pCurrentNode = pCurrentNode->pBlink ;
	}

	LeaveCrit( pTotalHead->QueueLockList ); // �ͷ���

	return bOK ;
}



BOOL
HookObjectOne (
	IN PVOID pInBufferS,
	IN PVOID pInBufferB
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/11 [11:6:2010 - 17:59]

Routine Description:
  Object Hook �����ڵ� 
    
Arguments:
  pInBuffer - ��Hook�Ľڵ���Ϣ,�ṹ�� OBJECT_DATA_LITTLE ָ��

--*/
{
	HANDLE hFile = NULL ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	OBJECT_ATTRIBUTES ObjectAttributes ;
	IO_STATUS_BLOCK IoStatusBlock ;
	UNICODE_STRING uniBuffer ;
	PVOID ObjectType = NULL ;
	ULONG Procedure_ptr = 0 ;
	WCHAR szBuffer[ MAX_PATH ] = L"" ;
	LPOBJECT_DATA_LITTLE DataLittle = (LPOBJECT_DATA_LITTLE) pInBufferS ;
	LPODNODE DataInfo = (LPODNODE) pInBufferB ;	

	//
	// 1. У������Ϸ���
	//
	
	if ( NULL == pInBufferS || NULL == pInBufferB || 0 == DataLittle->FakeAddr || NULL == DataInfo->szObjecType )
	{
		dprintf( "error! | HookObjectOne(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	if ( TRUE == DataLittle->bHooked || FALSE == DataLittle->bCare )
	{
		dprintf( 
			"? | HookObjectOne(); | �ýڵ�(\"%ws\")�ѱ�Hook��,�����ٴ�Hook���߲���Ҫ��ע��Hook��; bHooked=%d, bCare=%d \n",
			DataInfo->szObjecType, DataLittle->bHooked, DataLittle->bCare
			);

		return TRUE ;
	}

	//
	// 2. �õ�ָ��Ŀ¼��Type����,ͨ��Ӳ����ƫ�ƻ�ȡoject procedureָ��
	//

	wcscpy( szBuffer, L"\\ObjectTypes\\");
	wcscat( szBuffer, DataInfo->szObjecType );
	RtlInitUnicodeString( &uniBuffer, szBuffer );

	InitializeObjectAttributes( &ObjectAttributes, &uniBuffer, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, 0, NULL );

	status = ObOpenObjectByName(
		&ObjectAttributes,
		g_ObpTypeObjectType,
		KernelMode,
		NULL,
		0,
		NULL,
		&hFile
		);

	if( ! NT_SUCCESS( status ) )
	{
		dprintf( "error! | HookObjectOne() - ObOpenObjectByName(); | (status=0x%08lx) \n", status );
		return FALSE ;
	}

	status = ObReferenceObjectByHandle(
		hFile,
		0,
		NULL,
		KernelMode,
		(PVOID)&ObjectType,
		NULL
		);

	if( ! NT_SUCCESS( status ) )
	{
		dprintf( "error! | HookObjectOne() - ObReferenceObjectByHandle(); | (status=0x%08lx) \n", status );
		ZwClose( hFile );
		return FALSE ;
	}

	// 2.1 У���ַ�Ϸ���
	Procedure_ptr = (ULONG)ObjectType + DataInfo->TypeInfoOffset + DataLittle->ProcedureOffset ;
	
	if (	(FALSE == MmIsAddressValid( (PVOID)Procedure_ptr ))
		||	(FALSE == MmIsAddressValid( (PVOID)DataLittle->FakeAddr ))
		)
	{
		dprintf( 
			"error! | HookObjectOne() - MmIsAddressValid(Procedure_ptr); | oject procedureָ����ߴ�������ַ���Ϸ�;"
			"Procedure_ptr:0x%08lx, FakeAddr:0x%08lx \n",
			Procedure_ptr, DataLittle->FakeAddr
			);

		ZwClose( hFile );
		ObfDereferenceObject( ObjectType );
		return FALSE ;
	}

	// 2.2 Hook֮
	DataLittle->UnhookPoint	= Procedure_ptr ;
	DataLittle->OrignalAddr = *(PULONG) Procedure_ptr ; // ����ԭʼ��ַ,����Ϊ�� 
	*(PULONG) Procedure_ptr = DataLittle->FakeAddr ;	// �滻ָ��

	DataLittle->bHooked = TRUE ; // ������Ҫ,��Ҫж��������ʱ��Object�Ĺ��Ӳ��ᱻж��,ϵͳ�ͱ���.

	ZwClose( hFile );
	ObfDereferenceObject( ObjectType );
	return TRUE ;
}



VOID
UnhookObjectAll(
	IN PVOID _TotalHead
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/11 [11:6:2010 - 17:59]

Routine Description:
  Object UnHook ���нڵ�

--*/
{
	LPODNODE pNodeHead = NULL, pCurrentNode = NULL ;
	LPODHEAD pTotalHead	   = (LPODHEAD) _TotalHead ;

	// 1. У������Ϸ���
	if ( NULL == pTotalHead ) 
	{
		dprintf( "error! | UnhookObjectAll(); | Invalid Paramaters; failed! \n" );
		return ;
	}

	if ( (0 == pTotalHead->nTotalCounts) || (TRUE == IsListEmpty( (PLIST_ENTRY) &pTotalHead->ListHead )) ) { return ; }

	// 2. ��������,ж��Hook
	EnterCrit( pTotalHead->QueueLockList );	// ��������

	pNodeHead    =  &pTotalHead->ListHead ;
	pCurrentNode = pNodeHead->pBlink ;

	while ( pCurrentNode != pNodeHead )
	{
		if ( pCurrentNode->Open.bHooked )
		{
			*(PULONG) pCurrentNode->Open.UnhookPoint = pCurrentNode->Open.OrignalAddr ;
			pCurrentNode->Open.bHooked = FALSE ;
		}

		if ( pCurrentNode->Parse.bHooked )
		{
			*(PULONG) pCurrentNode->Parse.UnhookPoint = pCurrentNode->Parse.OrignalAddr ;
			pCurrentNode->Parse.bHooked = FALSE ;
		}

		pCurrentNode = pCurrentNode->pBlink ;
	}

	LeaveCrit( pTotalHead->QueueLockList ); // �ͷ���
	return ;
}


///////////////////////////////   END OF FILE   ///////////////////////////////