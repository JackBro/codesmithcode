// ProteinBoxDLL.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "Exportfunc.h"
#include "HookHelper.h"
#include "config.h"
#include "MemoryManager.h"
#include "PBUser32dll/PBUser32dllData.h"
#include "ProteinBoxDLL.h"

//////////////////////////////////////////////////////////////////////////

CProteinBoxDLL g_ProteinBoxDLL ;

PB_BOX_INFO g_BoxInfo = {} ;	

PIMAGE_IMPORT_DESCRIPTOR g_pImportTable_mscoree_dll = NULL ;
PIMAGE_IMPORT_DESCRIPTOR g_pImportTable_msvcm80_dll = NULL ;
PIMAGE_IMPORT_DESCRIPTOR g_pImportTable_msvcm90_dll = NULL ;

SID g_sid___SECURITY_NT_AUTHORITY = {} ;
PISECURITY_DESCRIPTOR g_SecurityDescriptor = NULL ;

ULONG g_CurrentPID_runInSandbox = 0 ;


//////////////////////////////////////////////////////////////////////////

BOOL APIENTRY 
DllMain( 
	HANDLE hModule, 
    DWORD  ul_reason_for_call, 
    LPVOID lpReserved
	)
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			{
				g_ProteinBoxDLL.DoWork( (HMODULE)hModule );
			}
			 break;

		case DLL_PROCESS_DETACH:
			{
				if ( g_bDoWork_OK && g_BoxInfo.BoxName )
					ClearHandlersCache(TRUE);	
			}
			break;
    }
    return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// ���ʵ��

CProteinBoxDLL::CProteinBoxDLL()
:m_hFile_ProteinboxDrv(INVALID_HANDLE_VALUE)
{ 

}

CProteinBoxDLL::~CProteinBoxDLL()
{
	
}


BOOL CProteinBoxDLL::DoWork( HMODULE hModule )
{
	BOOL bRet = FALSE ;
	PACL Dacl = NULL;
	PSID LocalSystemSid = NULL ;
    ULONG DaclSize = 0x100 ;
	NTSTATUS status = STATUS_SUCCESS ; 
	UNICODE_STRING SidName = {} ;

	if ( g_bDoWork_OK ) { return TRUE; }

	// 1.1 ���ر����DLL,��ȡ����ַ
	__ProcessNode = (LPPROCESSNODE) kmalloc( sizeof(__ProcessNode) );

	__ProcessNode->DllBaseAddr.hModule_Self = hModule ;
	__ProcessNode->DllBaseAddr.hModuleArrays[Ntdll_TAG]			= GetModuleHandle( L"ntdll.dll"		 );
    __ProcessNode->DllBaseAddr.hModuleArrays[Kernel32_TAG]		= GetModuleHandle( L"kernel32.dll"   );
    __ProcessNode->DllBaseAddr.hModuleArrays[KernelBase_TAG]	= GetModuleHandle( L"KernelBase.dll" );

	DisableThreadLibraryCalls( hModule );

	// 1.2 �ж��Ƿ���64λϵͳ
	bRet = PB_IsWow64(); 
	if ( bRet )
	{
		MYTRACE( L"error! | DoWork() - PB_IsWow64(); | �ݲ�֧��x64ϵͳ - -!  \n" );
		ExitProcess(0xFFFFFFFF);
	}

	bRet = CMemoryManager::GetInstance().InitTLS();
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | DoWork() - InitTLS(); |  \n" );
		ExitProcess(0xFFFFFFFF);
	}

	if ( IsInvalidProc() ) { return TRUE; }

	// 2.1 ��ѯ��ǰ�������ڵ�ɳ����Ϣ (ɳ����/������/SID/...)
	g_CurrentPID_runInSandbox = GetCurrentProcessId();
	bRet = PB_QueryProcess( g_CurrentPID_runInSandbox, &g_BoxInfo );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | DoWork() - PB_QueryProcess(); |  \n" );
		return FALSE ;
	}

	// 2.2 �ж��Լ��Ƿ��� explorer / IE �����ڲ�
	if ( 0 == _wcsicmp( g_BoxInfo.ProcessName, L"explorer.exe" ) )
	{
		g_bIs_Inside_Explorer_exe = TRUE ;
	}
	else if ( 0 == _wcsicmp( g_BoxInfo.ProcessName, L"iexplore.exe" ) )
	{
		g_bIs_Inside_iexplore_exe = TRUE ;
	}

	// 3. ��ԭ���޸ĵ�PE�ڴ��
	FixupModifiedPE();

	// 4. ��ѯ��ǰ��SID��Ϣ
	bRet = GetProcessToken( &SidName );
	if ( bRet )
	{
		if ( SidName.Length && 0 == _wcsicmp( SidName.Buffer, L"S-1-5-18") )
			g_bSID_is_S_1_5_18 = TRUE ;
	
		RtlFreeUnicodeString( &SidName );
	}

	bRet = InitializeSid();
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | DoWork() - InitializeSid(); |  \n" );
		return FALSE ;
	}

	Dacl = (PACL) kmalloc( DaclSize );
    if ( NULL == Dacl )
    {
        MYTRACE( L"error! | DoWork() - malloc(); | ΪDacl�����ڴ�ʧ�� \n" );
		return FALSE ;
    }
	
    status = RtlCreateAcl( Dacl, DaclSize, ACL_REVISION );
    if ( !NT_SUCCESS(status) )
    {
        MYTRACE( L"error! | DoWork() - RtlCreateAcl(); | status=0x%08lx \n", status );
		kfree( Dacl );
		return FALSE ;
    }
	
    status = RtlAddAccessAllowedAceEx (
		Dacl,
		ACL_REVISION,
		OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERITED_ACE,
		0x10000000,
		&g_sid___SECURITY_NT_AUTHORITY
		);

    if ( !NT_SUCCESS(status) )
    {
        MYTRACE( L"error! | DoWork() - RtlAddAccessAllowedAceEx(); | status=0x%08lx \n", status );
		kfree( Dacl );
		return FALSE ;
    }

	ConvertStringSidToSid ( g_BoxInfo.SID, &LocalSystemSid );
	if ( LocalSystemSid ) 
	{
		RtlAddAccessAllowedAceEx( Dacl, ACL_REVISION, 0x13, 0x10000000, LocalSystemSid );
	}

	g_SecurityDescriptor = (PISECURITY_DESCRIPTOR) kmalloc( sizeof(SECURITY_DESCRIPTOR) );
	if ( NULL == g_SecurityDescriptor )
	{
        MYTRACE( L"error! | DoWork() - malloc(); | Ϊg_SecurityDescriptor�����ڴ�ʧ�� \n" );
		kfree( Dacl );
		return FALSE ;
    }

    RtlCreateSecurityDescriptor( g_SecurityDescriptor, SECURITY_DESCRIPTOR_REVISION );
    RtlSetDaclSecurityDescriptor( g_SecurityDescriptor, SECURITY_DESCRIPTOR_REVISION, Dacl, 0 );

	//
	// 5. ��ʼ�����ĸɻ�
	//

	if ( FALSE == StartHook() ) { ExitProcess( 0xFFFFFFFF ); }

	return TRUE ;
}



VOID CProteinBoxDLL::StopWork( BOOL Flag )
{
//	StopHook ();
	return ;
}



BOOL CProteinBoxDLL::CheckGetHandle()
{
	if ( INVALID_HANDLE_VALUE != m_hFile_ProteinboxDrv ) { return TRUE ;  }
	
	m_hFile_ProteinboxDrv = CreateFileW (
		g_PBLinkName , 
		FILE_READ_ATTRIBUTES | FILE_READ_DATA | SYNCHRONIZE , 
		FILE_SHARE_READ | FILE_SHARE_WRITE , 
		NULL , 
		OPEN_EXISTING ,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
		0
		);
	
	if ( INVALID_HANDLE_VALUE == m_hFile_ProteinboxDrv ) { return FALSE ; }
	return TRUE ; 
}



BOOL CProteinBoxDLL::IoControl( DWORD nCode, PVOID pInBuffer, DWORD nInCount, PVOID pOutBuffer, DWORD nOutCount)
{
	if( FALSE == CheckGetHandle() )
	{
		MYTRACE( L"error! | IoControl() - CheckGetHandle(); | \n" );
		return FALSE ;
	}

	DWORD nBytesReturn ;
	BOOL bRet = ::DeviceIoControl(
		m_hFile_ProteinboxDrv, 
		nCode, 
		pInBuffer, nInCount, 
		pOutBuffer, nOutCount, 
		&nBytesReturn, NULL
		);

	return bRet ;
}



NTSTATUS CProteinBoxDLL::IoControlEx( DWORD dwIoControlCode, PVOID lpInBuffer, DWORD nInBufferSize, PVOID lpOutBuffer, DWORD nOutBufferSize)
{
	NTSTATUS status = STATUS_SUCCESS ; 
	IO_STATUS_BLOCK Iosb ;

	if( FALSE == CheckGetHandle() )
	{
		MYTRACE( L"error! | IoControlEx() - CheckGetHandle(); | \n" );
		return STATUS_UNSUCCESSFUL ;
	}

	status = ZwDeviceIoControlFile (
		m_hFile_ProteinboxDrv,
		NULL,
		NULL,
		NULL,
		&Iosb,
		dwIoControlCode,
		lpInBuffer,
		nInBufferSize,
		lpOutBuffer,
        nOutBufferSize
		);

	return status ;
}


static const LPWSTR g_SelfEXE_Arrays[ ] = 
{ 
//	L"PBStart.exe",
	L"PBSrv.exe",
};


BOOL CProteinBoxDLL::IsInvalidProc()
/*++

Author: sudami [sudami@163.com]
Time  : 2011/12/22 [22:12:2011 - 17:59]

Routine Description:
  ���Լ��Ľ�����dll����Ҫ��̫������; ��ȡע���·��,�͵�ǰ·������ƥ��
  PBStart.exe / PBSrv.exe

    
--*/
{
	WCHAR szCurPath[MAX_PATH] = {0};
	WCHAR szValue[MAX_PATH] = {0};
	WCHAR szPath[MAX_PATH + 0x20 ] = {0};
	DWORD dwSize = sizeof(szValue);
	DWORD dwType = REG_SZ;

	LONG ret = SHGetValue (
		HKEY_LOCAL_MACHINE,
		SUBKEY_PROTEINBOX,
		SUBVALUE_ROOTFOLDER,
		&dwType, 
		szValue, 
		&dwSize 
		); 

	if( ERROR_SUCCESS != ret || NULL == szValue[0] ) 
	{ 
		MYTRACE( L"error! | IsInvalidProc() - SHGetValue(); | ��ȡע���ɳ���Ŀ¼ʧ��!  \n" );

		MessageBox( 
			NULL, 
			L"��������ע�����ɳ��ĸ�Ŀ¼,������ɳ��... \n"
			L"[ע������] HKEY_LOCAL_MACHINE\\SOFTWARE\\Proteinbox\\Config\\ \n"
			L"[ע����ֵ] RootFolder \n",
			L"ɳ���ʼ��ʧ��!",
			MB_ICONERROR | MB_TOPMOST
			);

		ExitProcess(0xFFFFFFFF);
		return FALSE; 
	}

	GetModuleFileName( NULL, szCurPath, ARRSIZEOF(szCurPath) );

	int Size = MAX_PATH + 0x20;
	for (int Index = 0; Index < ARRAYSIZEOF(g_SelfEXE_Arrays); Index++ )
	{
		RtlZeroMemory( szPath, Size );
		StringCbPrintf( szPath, Size, _T("%s\\%s"), szValue, g_SelfEXE_Arrays[Index]);

		if ( 0 == wcsicmp( szCurPath, szPath ) ) 
		{
			return TRUE;
		}
	}
	
	return FALSE;
}



VOID CProteinBoxDLL::FixupModifiedPE()
{
	PVOID lpData = NULL ;
	PNEW_PE_IAT_INFO_TOTAL pCurrentData = NULL ;
	ULONG BeCoverdSize = 0x1000 ;
	NTSTATUS Status = STATUS_SUCCESS ;
	IOCTL_PROTEINBOX_BUFFER buffer ;
	
	// 1. ��ʼ��InputBuffer
	RtlZeroMemory( &buffer, sizeof(IOCTL_PROTEINBOX_BUFFER) );

	lpData = kmalloc( 0x1000 ) ;
	if ( NULL == lpData ) { return ; }
	
	buffer.Head.Flag = FLAG_Ioctl_GetInjectSaveArea ;
	buffer.GetInjectSaveAreaBuffer.BeCoveredAddr = (PULONG) &pCurrentData ;
	buffer.GetInjectSaveAreaBuffer.MaxLength	 = (PULONG) &BeCoverdSize ;
	buffer.GetInjectSaveAreaBuffer.lpData		 = lpData ;

	// 2. ��ȡ�������ݵ�PE��Ϣ
	Status = GetInjectSaveArea( &buffer ); 
	if ( !NT_SUCCESS(Status) )
	{
		if ( STATUS_ALREADY_COMMITTED == Status )
		{
			// ���޸���,�����ٴ��޸�,ֱ�ӷ���
			kfree( lpData );
			return ;
		}
		else
		{
			MYTRACE( L"error! | FixupModifiedPE() - GetInjectSaveArea(); | \n" );
			ExitProcess( 0xFFFFFFFF ); ;
		}
	}

	if ( 0 == BeCoverdSize || NULL == pCurrentData )
	{
		MYTRACE( L"error! | FixupModifiedPE(); | �������л�ȡ��������Ч \n" );
		ExitProcess( 0xFFFFFFFF ); ;
	}

#if _DEBUG
	MYTRACE (
		L"BeCoveredAddr=0x%08lx, BeCoverdSize=0x%08lx",
		(ULONG) pCurrentData,
		BeCoverdSize
		);
#endif

	// 3. �ָ�IDD (Image Data Directory)
	ULONG OldProtect, Size = 0x80 ;
	PIMAGE_DATA_DIRECTORY pidd = pCurrentData->Stub.pidd ;

	if ( NULL == pidd )
	{
		MYTRACE( L"error! | FixupModifiedPE(); | NULL == pidd \n" );
		ExitProcess( 0xFFFFFFFF ); ;
	}

	if ( FALSE == VirtualProtect( pidd, Size, PAGE_READWRITE, &OldProtect ) )
	{
		MYTRACE( L"error! | FixupModifiedPE() - VirtualProtect(); | ����IDD������Ϊ�ɶ�дʧ�� \n" );
		ExitProcess( 0xFFFFFFFF );
	}

// 	Status = ZwProtectVirtualMemory (
// 		(HANDLE)0xFFFFFFFF,
// 		(PVOID*)&pidd,
// 		&Size,
// 		PAGE_READWRITE,
// 		&OldProtect
// 		);
//                
// 	if ( ! NT_SUCCESS(Status))
// 	{
// 		MYTRACE( L"error! | FixupModifiedPE() - ZwProtectVirtualMemory(); | ����IDD������Ϊ�ɶ�дʧ��; status==0x%08lx \n", Status );
// 		ExitProcess( 0xFFFFFFFF ); ;
// 	}

	g_pImportTable_mscoree_dll = pCurrentData->Stub.pImportTable_mscoree_dll;
	g_pImportTable_msvcm80_dll = pCurrentData->Stub.pImportTable_msvcm80_dll;
	g_pImportTable_msvcm90_dll = pCurrentData->Stub.pImportTable_msvcm90_dll;
	
	pidd[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress		  = pCurrentData->Stub.IMAGE_DIRECTORY_ENTRY_IMPORT_VirtualAddress;
	pidd[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress	  = pCurrentData->Stub.IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT_VirtualAddress;
	pidd[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].Size			  = pCurrentData->Stub.IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT_Size;
	pidd[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress			  = pCurrentData->Stub.IMAGE_DIRECTORY_ENTRY_IAT_VirtualAddress;
	pidd[IMAGE_DIRECTORY_ENTRY_IAT].Size					  = pCurrentData->Stub.IMAGE_DIRECTORY_ENTRY_IAT_Size;
	pidd[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].VirtualAddress = pCurrentData->Stub.IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR_VirtualAddress;
	pidd[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].Size			  = pCurrentData->Stub.IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR_Size;

	VirtualProtect( pidd, Size, OldProtect, &OldProtect );
//	ZwProtectVirtualMemory( (HANDLE)0xFFFFFFFF, (PVOID*)&pidd, &Size, OldProtect, &OldProtect );

	// 4. �ָ������ǵ�PE��
	OldProtect = pCurrentData->Stub.OldProtection ;
	memcpy( pCurrentData, buffer.GetInjectSaveAreaBuffer.lpData, BeCoverdSize );

	if ( FALSE == VirtualProtect( pCurrentData, BeCoverdSize, OldProtect, &OldProtect ) )	
// 	Status = ZwProtectVirtualMemory (
// 		(HANDLE)0xFFFFFFFF,
// 		(PVOID*)&pCurrentData,
// 		&BeCoverdSize,
// 		OldProtect,
// 		&OldProtect
// 		);
// 	
// 	if ( ! NT_SUCCESS(Status))
	{
		MYTRACE(
			L"error! | FixupModifiedPE() - VirtualProtect(); | �ָ������ǵ�PE������ʧ�� \n"
			L"���޸ĵĵ�ַ: 0x%08lx, ���޸ĵĴ�С: 0x%x, �µ�����: 0x%08lx \n",
			pCurrentData, BeCoverdSize, OldProtect
			);
		ExitProcess( 0xFFFFFFFF ); ;
	}

	// 5. Clearup
	kfree( lpData );
	return ;
}



NTSTATUS CProteinBoxDLL::GetInjectSaveArea( PVOID buffer )
{
	if ( NULL == buffer ) { return STATUS_INVALID_PARAMETER ; }
	
	// �ؼ�һ��
	NTSTATUS status = IoControlEx (
		IOCTL_PROTEINBOX,
		(PVOID)buffer,						// InBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER),	// InBuffer Length
		(PVOID)buffer,						// OutBuffer
		sizeof(IOCTL_PROTEINBOX_BUFFER)
		);
	
	return status ;
}



BOOL CProteinBoxDLL::InitializeSid ()
{
	BOOL bRet = FALSE ;
	SID_IDENTIFIER_AUTHORITY SidAuthority1 = { SECURITY_NT_AUTHORITY } ; 

	bRet = __InitializeSid( &g_sid___SECURITY_NT_AUTHORITY, &SidAuthority1, 1, SECURITY_AUTHENTICATED_USER_RID );
	return bRet ;
}



BOOL CProteinBoxDLL::__InitializeSid (IN PSID Sid, IN PSID_IDENTIFIER_AUTHORITY pIdentifierAuthority, IN BYTE SubAuthorityCount, ULONG SubAuthority )
{
	PULONG SubAuthority__ = NULL ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ;

	status = RtlInitializeSid( Sid, pIdentifierAuthority, SubAuthorityCount );
	if ( !NT_SUCCESS (status) )
	{
		MYTRACE( L"error! | InitializeSid() - RtlInitializeSid(); status == 0x%08lx \n", status );
		return FALSE;
	}

	if ( SubAuthority )
	{
		SubAuthority__ = RtlSubAuthoritySid( Sid, 0 );
		*SubAuthority__ = SubAuthority ;
	}
	
	return TRUE;
}


///////////////////////////////   END OF FILE   ///////////////////////////////