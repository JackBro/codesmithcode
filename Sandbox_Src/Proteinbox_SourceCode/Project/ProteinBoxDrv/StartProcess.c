/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/05/25 [25:5:2010 - 17:42]
* MODULE : d:\Work\Program\Coding\ɳ��\SandBox\Code\Project\ProteinBoxDrv\StartProcess.c
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
#include "Common.h"
#include "StartProcess.h"
#include "DispatchIoctl.h"
#include "ProcessData.h"
#include "SdtData.h"
#include "SandboxsList.h"

//////////////////////////////////////////////////////////////////////////

extern BOOL g_bMonitor_Notify ;
extern HANDLE g_NewTokenHandle_system ;

_ZwSetInformationToken_ g_ZwSetInformationToken_addr = NULL ;


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
Ioctl_StartProcess (
	IN PVOID pNode,
	IN PVOID pInBuffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/25 [25:5:2010 - 17:49]

Routine Description:
  Shit,����������ԴȪ��. 
  �������������֪��Ҫ"��ɳ��"�ĳ��������ĸ�ɳ��������,���ڴ˼�¼ɳ��ĸ���������
    
--*/
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	BOOL bRet = FALSE, bOverload = FALSE ;
	ULONG PID = 0 ;
	ULONG SessionId = 0 ;
	LPPDNODE pNodeNew = NULL ;
	PNODE_INFO_C pNode_c = NULL ;
	LPIOCTL_STARTPROCESS_BUFFER Buffer = NULL ;
	OBJECT_ATTRIBUTES ObjectAttributes ; 
	SECURITY_QUALITY_OF_SERVICE SecurityQos ;
	HANDLE new_hToken, new_ImpersonationToken ;
	KPROCESSOR_MODE PreviousMode = ExGetPreviousMode();
	
	//
	// 1. У������Ϸ���
	//

	dprintf( "[ProteinBoxDrv] IOCTL_PROTEINBOX - Ioctl_StartProcess(); \n" );
	if ( FALSE == g_bMonitor_Notify ) { return STATUS_SERVER_DISABLED ; }
	
	if ( NULL == pInBuffer )
	{
		dprintf( "error! | Ioctl_StartProcess(); | Invalid Paramaters; failed! \n" );
		return status ;
	}

	__try
	{
		Buffer = (LPIOCTL_STARTPROCESS_BUFFER) getIoctlBufferBody( pInBuffer );
		if ( FALSE == MmIsAddressValid( (PVOID)Buffer ) )
		{
			dprintf( "error! | Ioctl_StartProcess() - getIoctlBufferBody(); | Body��ַ���Ϸ� \n" );
			__leave ;
		}

		ProbeForWrite( Buffer->new_hToken, 4, 4 );
		ProbeForWrite( Buffer->new_ImpersonationToken, 4, 4 );		
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		dprintf( "error! | Ioctl_StartProcess() - __try __except(); | ProbeForWrite��Ӧ�ĵ�ַ���Ϸ� \n" );
		return status ;
	}

	//
	// 2. ����Ƿ����Ѵ����Ľ��̽ڵ�
	//

	PID = (ULONG) PsGetCurrentProcessId();

	if ( kgetnodePD( PID ) )
	{
		dprintf( "error! | Ioctl_StartProcess() - kbuildnodePD(); | ���ڽ��̽ڵ��Ѵ����������� PID=%d \n", PID );
		return STATUS_ALREADY_COMMITTED ;
	}

	//
	// 3.1 �������̽ڵ�
	//

	pNodeNew = (LPPDNODE) kbuildnodePD( PID, FALSE );
	if ( NULL == pNodeNew )
	{
		dprintf( "error! | Ioctl_StartProcess() - kbuildnodePD(); | �������̽ڵ�ʧ�� PID=%d \n", PID );
		return status ;
	}

	//
	// 3.2 ���C�ڵ�
	//

	__try
	{
		if ( UserMode == PreviousMode ) 
		{ 
			if ( Buffer->wszBoxName ) { ProbeForRead( Buffer->wszBoxName, 0x40, 1 ); }
			if ( Buffer->wszRegisterUserID ) { ProbeForRead( Buffer->wszRegisterUserID, 0xC0, 1 ); }
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		PDClearNode( (PVOID)pNodeNew ); // ��δ��������,��ʼ��ʧ�����ͷ��ڴ�
		dprintf( "error! | Ioctl_StartProcess() - ProbeForRead(); | R3��������Buffer���Ϸ� \n" );
		return STATUS_UNSUCCESSFUL ;
	}

	pNode_c = PDBuildNodeC( Buffer );
	if ( NULL == pNode_c )
	{
		PDClearNode( (PVOID)pNodeNew ); // ��δ��������,��ʼ��ʧ�����ͷ��ڴ�
		dprintf( "error! | Ioctl_StartProcess() - PDBuildNodeC(); | �������̽ڵ�Cʧ�� PID=%d \n", PID );
		return status ;
	}

	//
	// 4. ������ǰ���̽ڵ�
	//

	pNodeNew->bSelfExe = 1 ;
	pNodeNew->pNode_C = pNode_c ;

	bRet = PDFixupNode( pNodeNew, L"PBStart.exe" );
	if ( FALSE == bRet )
	{
		dprintf( "error! | Ioctl_StartProcess() - PDFixupNode(); | �������̽ڵ�ʧ�� PID=%d \n", PID );
		PDClearNode( (PVOID)pNodeNew ); // ��δ��������,��ʼ��ʧ�����ͷ��ڴ�
		return status ;
	}

	kInsertTailPD( pNodeNew );

	//
	// 5. ������ǰ���̵�Ȩ��; R3��PBStart.exe����Start_sun_process(),���ڲ�����������в��������Ƿ���ϵͳȨ������"��SB"�Ľ���
	//    CreateProcessAsUserW����ָ����Ȩ�޴��� ���� ShellExecuteExW ����ͨȨ�޴���
	//

	// 5.0 ��Ҫ��Ȩ,�򲻻Ὣ����ϵͳȨ�޵ľ����䵽Buffer��
	if ( pNodeNew->bDropAdminRights )
	{
		*(Buffer->new_hToken) = 0 ;
		*(Buffer->new_ImpersonationToken) = 0 ;
	}

	// 5.1 ����g_NewTokenHandle_system�ľ��
	SecurityQos.Length					= sizeof( SecurityQos );
	SecurityQos.ImpersonationLevel		= SecurityImpersonation ;
	SecurityQos.ContextTrackingMode		= FALSE ;
	SecurityQos.EffectiveOnly			= FALSE ;

	ObjectAttributes.SecurityQualityOfService = &SecurityQos ;

	InitializeObjectAttributes (
		&ObjectAttributes,
		NULL,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL
		);

	status = ZwDuplicateToken( g_NewTokenHandle_system, 0, &ObjectAttributes, 0, TokenPrimary, &new_hToken );
	if ( !NT_SUCCESS (status) )
	{
		dprintf( "error! | Ioctl_StartProcess() - ZwDuplicateToken(�׶�1) | status == 0x%08lx \n", status );
		return status ;
	}

	// 5.2 ��ȡ ZwCreateToken ��ԭʼ��ַ,����֮
	bRet = kgetaddrSDT( ZwSetInformationToken );
	if ( FALSE == bRet )
	{ 
		dprintf( "error! | DropAdminRights_phrase1() - DropAdminRights_phrase2(); | �޷���ȡZwSetInformationToken��ַ \n" );
		return STATUS_UNSUCCESSFUL ; 
	}

	SessionId = pNodeNew->pNode_C->SessionId ;
	status = g_ZwSetInformationToken_addr( new_hToken, TokenSessionId, &SessionId, sizeof(DWORD) );
	if ( !NT_SUCCESS (status) )
	{
		dprintf( "error! | Ioctl_StartProcess() - ZwSetInformationToken() | status == 0x%08lx \n", status );
		ZwClose( new_hToken );
		return status ;
	}

	// 5.3 ����new_hToken�ľ��
	status = ZwDuplicateToken (
		new_hToken,
		0,
		&ObjectAttributes,
		0,
		TokenImpersonation,
		&new_ImpersonationToken
		);

	if ( !NT_SUCCESS (status) )
	{
		dprintf( "error! | Ioctl_StartProcess() - ZwDuplicateToken(�׶�2) | status == 0x%08lx \n", status );
		ZwClose( new_hToken );
		return status ;
	}

	// 5.4 ���Ƴɹ�,���õ����¾���׸�R3
	*(Buffer->new_hToken) = new_hToken ;
	*(Buffer->new_ImpersonationToken) = new_ImpersonationToken ;

	pNodeNew->bProcessNodeInitOK = TRUE ;

	// 6. ��¼��ǰɳ����������
	kbuildnodeSBL( pNode_c->BoxName, pNode_c->BoxNameLength, &bOverload );
	if ( bOverload ) { return STATUS_UNSUCCESSFUL ; }

	return STATUS_SUCCESS ;
}



NTSTATUS
Ioctl_QueryProcess (
	IN PVOID pNode,
	IN PVOID pInBuffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/25 [25:5:2010 - 17:49]

Routine Description:
  ��ѯָ��PID��Ӧ�� ɳ����/���̶���/ SID / �û��� ����Ϣ 
    
--*/
{
	ULONG PID = 0, length = 0 ;
	LPIOCTL_QUERYPROCESS_BUFFER Buffer = NULL ;
	LPPDNODE ProcessNode = (LPPDNODE) pNode ;

//	dprintf( "[ProteinBoxDrv] IOCTL_PROTEINBOX - Ioctl_QueryProcess(); \n" );

	if ( NULL == pInBuffer ) { return STATUS_NOT_IMPLEMENTED ; }

	__try
	{
		Buffer = (LPIOCTL_QUERYPROCESS_BUFFER) getIoctlBufferBody( pInBuffer );
		if ( FALSE == MmIsAddressValid( (PVOID)Buffer ) )
		{
			dprintf( "error! | Ioctl_QueryProcess() - getIoctlBufferBody(); | Body��ַ���Ϸ� \n" );
			__leave ;
		}

		PID = Buffer->PID ;
		if ( ProcessNode )
		{
			if ( PID == ProcessNode->PID || 0xFFFFFFFF == PID )
			{
				PID = 0 ; // ��������ѯ�Ľ��̶�Ӧ�ļ��ǵ�ǰ��ProcessNode�ڵ�
			}
		}
		else
		{
			if ( 0 == PID || 0xFFFFFFFF == PID )
				return STATUS_INVALID_CID;
		}

		if ( PID )
		{
			ProcessNode = (LPPDNODE) kgetnodePD( PID );
			if ( NULL == ProcessNode || ProcessNode->bDiscard ) { return STATUS_INVALID_CID ; }
		}

		// ����ɳ����
		length = ProcessNode->pNode_C->BoxNameLength ;
		if ( Buffer->BoxNameMaxLength < length ) { return STATUS_BUFFER_TOO_SMALL ; }

		ProbeForWrite( Buffer->lpBoxName, length, sizeof(DWORD) );
		memcpy( Buffer->lpBoxName, ProcessNode->pNode_C->BoxName, length );

		// �������̶���
		length = ProcessNode->ImageNameLength ;
		if ( Buffer->ProcessShortNameMaxLength < length ) { return STATUS_BUFFER_TOO_SMALL ; }

		ProbeForWrite( Buffer->lpCurrentProcessShortName, length, sizeof(DWORD) );
		memcpy( Buffer->lpCurrentProcessShortName, ProcessNode->lpProcessShortName, length );

		// ����SID
		length = ProcessNode->pNode_C->RegisterUserIDLength ;
		if ( Buffer->RegisterUserIDMaxLength < length ) { return STATUS_BUFFER_TOO_SMALL ; }

		ProbeForWrite( Buffer->lpRegisterUserID, length, sizeof(DWORD) );
		memcpy( Buffer->lpRegisterUserID, ProcessNode->pNode_C->RegisterUserID, length );

		// ����SessionId
		ProbeForWrite( Buffer->SessionId, 4, sizeof(DWORD) );
		*Buffer->SessionId = ProcessNode->pNode_C->SessionId ; 

		// ����FilePath
		length = ProcessNode->pNode_C->FileRootPathLength ;
		if ( Buffer->FileRootPathLength < length ) { return STATUS_BUFFER_TOO_SMALL ; }

		ProbeForWrite( Buffer->FileRootPath, length, sizeof(DWORD) );
		memcpy( Buffer->FileRootPath, ProcessNode->pNode_C->FileRootPath, length );

		// ����RegPath
		length = ProcessNode->pNode_C->KeyRootPathLength ;
		if ( Buffer->KeyRootPathLength < length ) { return STATUS_BUFFER_TOO_SMALL ; }

		ProbeForWrite( Buffer->KeyRootPath, length, sizeof(DWORD) );
		memcpy( Buffer->KeyRootPath, ProcessNode->pNode_C->KeyRootPath, length );

		// ����LpcPath
		length = ProcessNode->pNode_C->LpcRootPath1Length ;
		if ( Buffer->LpcRootPathLength < length ) { return STATUS_BUFFER_TOO_SMALL ; }

		ProbeForWrite( Buffer->LpcRootPath, length, sizeof(DWORD) );
		memcpy( Buffer->LpcRootPath, ProcessNode->pNode_C->LpcRootPath1, length );
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		dprintf( "error! | Ioctl_QueryProcess() - __try __except(); | ProbeForWrite��Ӧ�ĵ�ַ���Ϸ� \n" );
		return STATUS_UNSUCCESSFUL ;
	}

	return STATUS_SUCCESS ;
}




NTSTATUS
Ioctl_QueryProcessPath (
	IN PVOID pNode,
	IN PVOID pInBuffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/25 [25:5:2010 - 17:49]

Routine Description:
  ��ѯָ��PID��Ӧ�� ɳ���ļ��и�Ŀ¼

--*/
{
	ULONG PID = 0, length = 0 ;
	LPIOCTL_QUERYPROCESSPATH_BUFFER Buffer = NULL ;
	LPPDNODE ProcessNode = (LPPDNODE) pNode ;

	dprintf( "[ProteinBoxDrv] IOCTL_PROTEINBOX - Ioctl_QueryProcessPath(); \n" );

	if ( NULL == pInBuffer ) { return STATUS_NOT_IMPLEMENTED ; }

	__try
	{
		Buffer = (LPIOCTL_QUERYPROCESSPATH_BUFFER) getIoctlBufferBody( pInBuffer );
		if ( FALSE == MmIsAddressValid( (PVOID)Buffer ) )
		{
			dprintf( "error! | Ioctl_QueryProcess() - getIoctlBufferBody(); | Body��ַ���Ϸ� \n" );
			__leave ;
		}

		PID = Buffer->PID ;
		if ( ProcessNode )
		{
			if ( PID == ProcessNode->PID || 0xFFFFFFFF == PID )
			{
				PID = 0 ; // ��������ѯ�Ľ��̶�Ӧ�ļ��ǵ�ǰ��ProcessNode�ڵ�
			}
		}
		else
		{
			if ( 0 == PID || 0xFFFFFFFF == PID )
				return STATUS_INVALID_CID;
		}

		if ( PID )
		{
			ProcessNode = (LPPDNODE) kgetnodePD( PID );
			if ( NULL == ProcessNode || ProcessNode->bDiscard ) { return STATUS_INVALID_CID ; }
		}

		// ����FilePath
		length = ProcessNode->pNode_C->FileRootPathLength ;
		if ( Buffer->FileRootPathLength < length ) { return STATUS_BUFFER_TOO_SMALL ; }

		ProbeForWrite( Buffer->FileRootPath, length, sizeof(DWORD) );
		memcpy( Buffer->FileRootPath, ProcessNode->pNode_C->FileRootPath, length );
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		dprintf( "error! | Ioctl_QueryProcessPath() - __try __except(); | ProbeForWrite��Ӧ�ĵ�ַ���Ϸ� \n" );
		return STATUS_UNSUCCESSFUL ;
	}

	return STATUS_SUCCESS ;
}



NTSTATUS
Ioctl_QueryBoxPath (
	IN PVOID pNode,
	IN PVOID pInBuffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/25 [25:5:2010 - 17:49]

Routine Description:
  ��ѯָ����ɳ����ȫ·��
    
--*/
{
	BOOL bRet = FALSE ;
	WCHAR FileRootPath[ MAX_PATH ] = L"" ;
	LPIOCTL_QUERYBOXPATH_BUFFER Buffer = NULL ;
	LPPDNODE ProcessNode = (LPPDNODE) pNode ;

	dprintf( "[ProteinBoxDrv] IOCTL_PROTEINBOX - Ioctl_QueryBoxPath(); \n" );

	if ( NULL == pInBuffer ) { return STATUS_NOT_IMPLEMENTED ; }

	__try
	{
		Buffer = (LPIOCTL_QUERYBOXPATH_BUFFER) getIoctlBufferBody( pInBuffer );
		if ( FALSE == MmIsAddressValid( (PVOID)Buffer ) )
		{
			dprintf( "error! | Ioctl_QueryBoxPath() - getIoctlBufferBody(); | Body��ַ���Ϸ� \n" );
			__leave ;
		}

		// ����FilePath
		bRet = kGetConfSingle( L"GlobalSetting", L"BoxRootFolder", FileRootPath );
		if ( FALSE == bRet || wcslen(FileRootPath) > MAX_PATH - 0x10 - Buffer->BoxNamLength )
		{
			dprintf( "error! | Ioctl_QueryBoxPath() - kGetConfSingle(); |  \n" );
			__leave ;
		}

		wcscat( FileRootPath, L"\\SUDAMI\\" );
		wcscat( FileRootPath, Buffer->lpBoxName );

		if ( Buffer->BoxPathMaxLength < (wcslen(FileRootPath)+1) * sizeof(WCHAR) )  { return STATUS_BUFFER_TOO_SMALL ; }

		if ( IsWrittenKernelAddr(Buffer->lpBoxPath, (ULONG_PTR)Buffer->BoxPathMaxLength) ) 
		{
			dprintf( "error! | Ioctl_QueryBoxPath() - IsWrittenKernelAddr(); | �������Ϸ�,��д��ĵ�ַ:0x%08lx, ����:0x%08lx \n", Buffer->lpBoxPath, Buffer->BoxPathMaxLength );
			return STATUS_INVALID_PARAMETER_4 ;
		}

		ProbeForWrite( Buffer->lpBoxPath, Buffer->BoxPathMaxLength, 2 );
		wcscpy( Buffer->lpBoxPath, FileRootPath );
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		dprintf( "error! | Ioctl_QueryBoxPath() - __try __except(); | ProbeForWrite��Ӧ�ĵ�ַ���Ϸ� \n" );
		return STATUS_UNSUCCESSFUL ;
	}

	return STATUS_SUCCESS ;
}



NTSTATUS
Ioctl_EnumProcessEx (
	IN PVOID pNode,
	IN PVOID pInBuffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/25 [25:5:2010 - 17:49]

Routine Description:
  ö��ָ��ɳ���а��������н���
    
--*/
{
	BOOL bRet = FALSE ;
	WCHAR szBoxName[ MAX_PATH ] = L"" ;
	LPIOCTL_ENUMPROCESSEX_BUFFER Buffer = NULL ;
	LPPDNODE ProcessNode = (LPPDNODE) pNode ;

	dprintf( "[ProteinBoxDrv] IOCTL_PROTEINBOX - Ioctl_EnumProcessEx(); \n" );

	if ( NULL == pInBuffer ) { return STATUS_NOT_IMPLEMENTED ; }

	__try
	{
		Buffer = (LPIOCTL_ENUMPROCESSEX_BUFFER) getIoctlBufferBody( pInBuffer );
		if ( FALSE == MmIsAddressValid( (PVOID)Buffer ) )
		{
			dprintf( "error! | Ioctl_EnumProcessEx() - getIoctlBufferBody(); | Body��ַ���Ϸ� \n" );
			__leave ;
		}

		if ( NULL == Buffer->pArray ) { return STATUS_INVALID_PARAMETER; }

		if ( ProcessNode )
		{
			wcscpy( szBoxName, ProcessNode->pNode_C->BoxName );
		}

		if ( !*szBoxName && Buffer->lpBoxName )
		{
			ProbeForRead( Buffer->lpBoxName, 0x40, 1 );
			if ( *(Buffer->lpBoxName) )
			{
				wcsncpy( szBoxName, Buffer->lpBoxName, 0x20 );
			}
		}

		ProbeForWrite( Buffer->pArray, 0x800, sizeof(ULONG) );
		return PDEnumProcessEx( szBoxName, TRUE, Buffer->pArray );
		
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		dprintf( "error! | Ioctl_EnumProcessEx() - __try __except(); | ProbeForWrite��Ӧ�ĵ�ַ���Ϸ� \n" );
		return STATUS_UNSUCCESSFUL ;
	}

	return STATUS_SUCCESS ;
}


///////////////////////////////   END OF FILE   ///////////////////////////////