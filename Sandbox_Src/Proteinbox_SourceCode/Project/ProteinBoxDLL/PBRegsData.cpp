/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/09/20 [20:9:2010 - 16:50]
* MODULE : D:\Work\Program\Coding\ɳ��\SandBox\Code\Project\ProteinBoxDLL\PBRegsData.cpp
* 
* Description:
*      
*                        
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "StdAfx.h"
#include "HookHelper.h"
#include "ProteinBoxDLL.h"
#include "PBRegsData.h"

#pragma warning(disable : 4995 )

//////////////////////////////////////////////////////////////////////////


_NtQueryKey_	g_NtQueryKey_addr		= NULL ;
_NtOpenKey_		g_NtOpenKey_addr		= NULL ;
_NtCreateKey_	g_NtCreateKey_addr	= NULL ;
_NtDeleteKey_	g_NtDeleteKey_addr	= NULL ;
_NtEnumerateKey_ g_NtEnumerateKey_addr = NULL ;
_NtEnumerateValueKey_ g_NtEnumerateValueKey_addr = NULL ;


WCHAR szbfe[ MAX_PATH ] = L"\\registry\\machine\\system\\currentcontrolset\\services\\bfe" ;

LIST_ENTRY_EX g_XREG_INFORMATION_HEAD_Redirected = { NULL, NULL, 0 } ;
LIST_ENTRY_EX g_XREG_INFORMATION_HEAD_Orignal	 = { NULL, NULL, 0 } ;

/*static*/ CRITICAL_SECTION g_cs_Regedit ;

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
GetRegPath (
	IN HANDLE hRootKey,
	IN PUNICODE_STRING uniObjectName,
	OUT LPWSTR* OrignalPath,
	OUT LPWSTR* RedirectedPath,
	OUT BOOL* bIsHandlerSelfRegPath
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/09/20 [20:9:2010 - 18:16]

Routine Description:
  ��ȡ�����ԭʼ & �ض���·��. ���óɹ����������ͷ��ڴ�
    
Arguments:
  hRootKey - ������
  uniObjectName - ������
  OrignalPath - ����ԭʼ·��
  RedirectedPath - �����ض���·��

Return Value:
  NTSTATUS
    
--*/
{
	ULONG_PTR length = 0 ;
	LPWSTR DataOrig = NULL, DataRedirect = NULL ;
	NTSTATUS status = STATUS_SUCCESS ; 

	// 1. У������Ϸ���
	if ( NULL == uniObjectName || NULL == OrignalPath || NULL == RedirectedPath )
	{
		MYTRACE( L"error! | GetRegPath();| �������Ϸ�  \n" );
		return STATUS_INVALID_PARAMETER ;
	}

	length = uniObjectName->Length & 0xFFFE ;

	// 2.1 ���� hRootKey ���ڵ����
	if ( hRootKey )
	{
		/*
		��ѯhRootKey��Ӧ��ע���·��,�Ͳ�����@uniObjectName->Buffer(�������Ӽ�)����ƴ��
		eg: RootKeyΪ"\REGISTRY\USER\SANDBOX_AV_DEFAULTBOX\user" �Ӽ�Ϊ"current",��ƴ��
		���·��Ϊ: "\REGISTRY\USER\SANDBOX_AV_DEFAULTBOX\user\current"
		*/

		// �����ڴ�
		ULONG Size = 0x100 + length ;
		PKEY_NAME_INFORMATION pNameInfo = (PKEY_NAME_INFORMATION) kmalloc( Size );
		if ( NULL == pNameInfo )
		{
			MYTRACE( L"error! | GetRegPath() - RtlAllocateHeap();| �����ڴ�ʧ��  \n" );
			return STATUS_ALLOCATE_BUCKET ;
		}

		// ��ѯ
		status = g_NtQueryKey_addr( hRootKey, KeyNameInformation, pNameInfo, 0x100, &Size );
		if ( STATUS_BUFFER_OVERFLOW == status )
		{
			Size += length ;
			kfree( pNameInfo );

			pNameInfo = (PKEY_NAME_INFORMATION) kmalloc( Size );
			if ( NULL == pNameInfo )
			{
				MYTRACE( L"error! | GetRegPath() - RtlAllocateHeap();| �����ڴ�ʧ��(2)  \n" );
				return STATUS_ALLOCATE_BUCKET ;
			}

			status = g_NtQueryKey_addr( hRootKey, KeyNameInformation, pNameInfo, Size, &Size );
		}

		// ��ѯʧ�����ͷ��ڴ�
		if ( !NT_SUCCESS(status) )
		{
			kfree( pNameInfo );
			MYTRACE( L"error! | GetRegPath() - g_NtQueryKey_addr();| status=0x%08lx \n", status );
			return status ;
		}

		// ��ѯ�ɹ�,ƴ���ַ���
		DataOrig = (LPWSTR) kmalloc( pNameInfo->NameLength + length + sizeof(WCHAR)*5 );  // �����㹻���ڴ�

		memcpy( DataOrig, pNameInfo->Name, pNameInfo->NameLength );
		LPWSTR ptr = &DataOrig[ pNameInfo->NameLength / sizeof(WCHAR) ] ;

		if ( length )
		{
			LPWSTR ptr1 = NULL ;
			wcscat( ptr, L"\\" );
			ptr1 = ptr + 1 ;
			memcpy( ptr1, uniObjectName->Buffer, length );
			ptr = &ptr1[ length / sizeof(WCHAR) ] ;
		}
		
		*ptr = UNICODE_NULL ;
		kfree( pNameInfo );
	}
	else
	{
		// 2.2 ���� hRootKey �����ڵ����
		if ( 0 == length ) { return STATUS_OBJECT_PATH_SYNTAX_BAD ; }

		DataOrig = (LPWSTR) kmalloc( length + sizeof(WCHAR)*2 );  // �����㹻���ڴ�
		memcpy( DataOrig, uniObjectName->Buffer, length );
		DataOrig[ length / sizeof(WCHAR) ] = UNICODE_NULL ;
	}

	//
	// 3. ����õ���ע���ȫ·��
	//

	length = wcslen( DataOrig );
	if ( length < 9 ) { kfree( DataOrig ); return STATUS_OBJECT_PATH_SYNTAX_BAD ; }

	if ( wcsnicmp( DataOrig, L"\\registry", 0x9 ) ) { kfree( DataOrig ); return STATUS_OBJECT_PATH_SYNTAX_BAD ;  } 
	if ( DataOrig[ 0x9 ] && '\\' != DataOrig[ 0x9 ] ) { kfree( DataOrig ); return STATUS_OBJECT_PATH_SYNTAX_BAD ; }

	// 3.1 ��ȡԭʼע���·��
	LPWSTR ptr1 = NULL, ptr2 = NULL, tmp = NULL, FuckedBuffer = NULL ;
	ULONG_PTR KeyRootPathLength = (g_BoxInfo.KeyRootPathLength / sizeof(WCHAR)) - 1 ;
	ULONG_PTR SIDLength = g_RegInfo.RegSIDPathLength ;
	ULONG_PTR FuckedBufferLength = 0, TempLength = 0 ;

	while ( 1 )
	{
		while( TRUE )
		{
			if ( (length < KeyRootPathLength) || (wcsnicmp(DataOrig, g_BoxInfo.KeyRootPath, KeyRootPathLength)) )
			{
				break ; // �������Ĳ��� "\REGISTRY\USER\Sandbox_AV_DefaultBox" �µļ�ֵ,��������,����~
			}

			if ( bIsHandlerSelfRegPath ) { *bIsHandlerSelfRegPath = TRUE ; }

			// ��ʱ·������: "\REGISTRY\USER\SANDBOX_AV_DEFAULTBOX\user\current",����ת��Ϊ��ʵ·��
			ptr1 = &DataOrig[ KeyRootPathLength - 9 ];
			memcpy( ptr1, L"\\registry", 0x12 );

			FuckedBufferLength = (wcslen(ptr1) + 1) * sizeof(WCHAR) ;
			FuckedBuffer = (LPWSTR) kmalloc( FuckedBufferLength );
			memcpy( FuckedBuffer, ptr1, FuckedBufferLength );

			kfree( DataOrig );
			DataOrig = FuckedBuffer ;
			
			length += 9 - KeyRootPathLength ;
		}

		if ( length < 0x16 || wcsnicmp( (LPWSTR) (DataOrig + 0x9), L"\\user\\current", 0xD ) ) { break ; }

		TempLength = SIDLength + (length - 0x15) ;
		tmp = (LPWSTR) kmalloc( TempLength * sizeof(WCHAR) ); 

		memcpy( tmp, g_RegInfo.RegSIDPath, SIDLength * sizeof(WCHAR) );
		memcpy( (LPWSTR)(tmp + SIDLength), (LPWSTR)( DataOrig + 0x16), (length - 0x15) * sizeof(WCHAR) );

		kfree( DataOrig );
		DataOrig = tmp ;

		length = TempLength ;
		-- length ;
	}

	*OrignalPath = DataOrig ;

	// 3.2 ��ȡ�ض���ע���·��
	if ( g_bIs_Inside_PBRpcSs_exe && 0 == wcsnicmp(DataOrig, szbfe, wcslen(szbfe)) ) { kfree( DataOrig ); return STATUS_OBJECT_NAME_NOT_FOUND ; }

	DataRedirect = (LPWSTR) kmalloc( g_BoxInfo.KeyRootPathLength + length * sizeof(WCHAR) + 0x30 ); // �����㹻���ڴ�

	memcpy( DataRedirect, g_BoxInfo.KeyRootPath, g_BoxInfo.KeyRootPathLength );
	ptr1 = &DataRedirect[ KeyRootPathLength ] ;

	if ( length >= SIDLength && 0 == wcsnicmp( DataOrig, g_RegInfo.RegSIDPath, SIDLength ) )
	{
		memcpy( ptr1, L"\\user\\current", 0x1A );
		ptr1 = (LPWSTR) ( ptr1 + 0x1A / 2) ;
		length -= SIDLength ;
		
		ptr2 = &DataOrig[ SIDLength ];
	}
	else
	{
		length -= 9 ;
		ptr2 = (LPWSTR) (DataOrig + 0x9) ;
	}
	
	memcpy( ptr1, ptr2, length * sizeof(WCHAR) );
	ptr1[ length ] = UNICODE_NULL ;

	*RedirectedPath = DataRedirect ;
	return STATUS_SUCCESS ;
}


NTSTATUS
GetRegPathAAA (
	IN HANDLE hRootKey,
	IN PUNICODE_STRING uniObjectName,
	OUT LPWSTR* OrignalPath,
	OUT LPWSTR* RedirectedPath,
	OUT BOOL* bIsHandlerSelfRegPath
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/09/20 [20:9:2010 - 18:16]

Routine Description:
  ��ȡ�����ԭʼ & �ض���·��. ���óɹ����������ͷ��ڴ�
    
Arguments:
  hRootKey - ������
  uniObjectName - ������
  OrignalPath - ����ԭʼ·��
  RedirectedPath - �����ض���·��

Return Value:
  NTSTATUS
    
--*/
{
	NTSTATUS status = STATUS_SUCCESS ; 
	PKEY_NAME_INFORMATION pNameInfo = NULL ;
	LPWSTR ptr1 = NULL, ptr2 = NULL, tmp = NULL, FuckedBuffer = NULL, DataOrig = NULL, DataRedirect = NULL ;
	ULONG_PTR length = 0, KeyRootPathLength = 0, SIDLength = 0, TempLength = 0, Size = 0, FuckedBufferLength = 0 ;


	// 1. У������Ϸ���
	if ( NULL == uniObjectName || NULL == OrignalPath || NULL == RedirectedPath )
	{
		MYTRACE( L"error! | GetRegPath();| �������Ϸ�  \n" );
		return STATUS_INVALID_PARAMETER ;
	}

	length = uniObjectName->Length & 0xFFFE ;

	// 2.1 ���� hRootKey ���ڵ����
	if ( hRootKey )
	{
		/*
		��ѯhRootKey��Ӧ��ע���·��,�Ͳ�����@uniObjectName->Buffer(�������Ӽ�)����ƴ��
		eg: RootKeyΪ"\REGISTRY\USER\SANDBOX_AV_DEFAULTBOX\user" �Ӽ�Ϊ"current",��ƴ��
		���·��Ϊ: "\REGISTRY\USER\SANDBOX_AV_DEFAULTBOX\user\current"
		*/

		// �����ڴ�
		Size = 0x100 + length ;
		pNameInfo = (PKEY_NAME_INFORMATION) kmalloc( Size );
		if ( NULL == pNameInfo )
		{
			MYTRACE( L"error! | GetRegPath() - RtlAllocateHeap();| �����ڴ�ʧ��  \n" );
			return STATUS_ALLOCATE_BUCKET ;
		}

		// ��ѯ
		status = g_NtQueryKey_addr( hRootKey, KeyNameInformation, pNameInfo, 0x100, &Size );
		if ( STATUS_BUFFER_OVERFLOW == status )
		{
			Size += length ;
			kfree( pNameInfo );

			pNameInfo = (PKEY_NAME_INFORMATION) kmalloc( Size );
			if ( NULL == pNameInfo )
			{
				MYTRACE( L"error! | GetRegPath() - RtlAllocateHeap();| �����ڴ�ʧ��(2)  \n" );
				return STATUS_ALLOCATE_BUCKET ;
			}

			status = g_NtQueryKey_addr( hRootKey, KeyNameInformation, pNameInfo, Size, &Size );
		}

		// ��ѯʧ�����ͷ��ڴ�
		if ( !NT_SUCCESS(status) )
		{
			kfree( pNameInfo );
			MYTRACE( L"error! | GetRegPath() - g_NtQueryKey_addr();| status=0x%08lx \n", status );
			return status ;
		}

		// ��ѯ�ɹ�,ƴ���ַ���
		DataOrig = (LPWSTR) kmalloc( pNameInfo->NameLength + length + 0x20 );  // �����㹻���ڴ�

		memcpy( DataOrig, pNameInfo->Name, pNameInfo->NameLength );
		LPWSTR ptr = &DataOrig[ pNameInfo->NameLength / sizeof(WCHAR) ] ;

		if ( length )
		{
			LPWSTR ptr1 = NULL ;
			wcscat( ptr, L"\\" );
			ptr1 = ptr + 1 ;
			memcpy( ptr1, uniObjectName->Buffer, length );
			ptr = &ptr1[ length / sizeof(WCHAR) ] ;
		}
		
		*ptr = UNICODE_NULL ;
		kfree( pNameInfo );
	}
	else
	{
		// 2.2 ���� hRootKey �����ڵ����
		if ( 0 == length ) { return STATUS_OBJECT_PATH_SYNTAX_BAD ; }

		DataOrig = (LPWSTR) kmalloc( length + 0x20 );  // �����㹻���ڴ�
		memcpy( DataOrig, uniObjectName->Buffer, length );
		DataOrig[ length / sizeof(WCHAR) ] = UNICODE_NULL ;
	}

	//
	// 3. ����õ���ע���ȫ·��
	//

	TempLength = length = wcslen( DataOrig );
	if ( length < 9 ) { kfree( DataOrig ); return STATUS_OBJECT_PATH_SYNTAX_BAD ; }

	if ( wcsnicmp( DataOrig, L"\\registry", 0x9 ) ) { kfree( DataOrig ); return STATUS_OBJECT_PATH_SYNTAX_BAD ;  } 
	if ( DataOrig[ 0x9 ] && '\\' != DataOrig[ 0x9 ] ) { kfree( DataOrig ); return STATUS_OBJECT_PATH_SYNTAX_BAD ; }

	// 3.1 ��ȡԭʼע���·��
	KeyRootPathLength = (g_BoxInfo.KeyRootPathLength / sizeof(WCHAR)) - 1 ;
	SIDLength = g_RegInfo.RegSIDPathLength ;

	while ( TRUE )
	{
		while( TRUE )
		{
			if ( (length < KeyRootPathLength) || (RtlCompareUnicodeStringDummy(DataOrig, g_BoxInfo.KeyRootPath, (USHORT)KeyRootPathLength)) )
			{
				break ; // �������Ĳ��� "\REGISTRY\USER\Sandbox_AV_DefaultBox" �µļ�ֵ,��������,����~
			}

			if ( bIsHandlerSelfRegPath ) { *bIsHandlerSelfRegPath = TRUE ; }

			// ��ʱ·������: "\REGISTRY\USER\SANDBOX_AV_DEFAULTBOX\user\current",����ת��Ϊ��ʵ·��
			ptr1 = &DataOrig[ KeyRootPathLength - 9 ];
			memcpy( ptr1, L"\\registry", 0x12 );
			
			FuckedBufferLength = (wcslen(ptr1) + 1) * sizeof(WCHAR) ;
			FuckedBuffer = (LPWSTR) kmalloc( FuckedBufferLength );
			memcpy( FuckedBuffer, ptr1, FuckedBufferLength );
			
			kfree( DataOrig );
			DataOrig = FuckedBuffer ;

			length += 9 - KeyRootPathLength ;
		}

		if ( length < 0x16 || wcsnicmp( &DataOrig[ 0x9 ], L"\\user\\current", 0xD ) ) { break ; }

		TempLength = SIDLength + (length - 0x15) ;
		tmp = (LPWSTR) kmalloc( TempLength * sizeof(WCHAR) ); 
		
		memcpy( tmp, g_RegInfo.RegSIDPath, SIDLength * sizeof(WCHAR) );
		memcpy( &tmp[ SIDLength ], &DataOrig[ 0x22 ], (length - 0x15) * sizeof(WCHAR) );

		kfree( DataOrig );
		DataOrig = tmp ;

		length = TempLength ;
		-- length ;
	}

	*OrignalPath = DataOrig ;

	// 3.2 ��ȡ�ض���ע���·��
	if ( g_bIs_Inside_PBRpcSs_exe && 0 == wcsnicmp(DataOrig, szbfe, wcslen(szbfe)) ) { kfree( DataOrig ); return STATUS_OBJECT_NAME_NOT_FOUND ; }

	DataRedirect = (LPWSTR) kmalloc( g_BoxInfo.KeyRootPathLength + length + 0x30 ); // �����㹻���ڴ�

	memcpy( DataRedirect, g_BoxInfo.KeyRootPath, KeyRootPathLength * sizeof(WCHAR) );
	ptr1 = &DataRedirect[ KeyRootPathLength ] ;

	if ( length >= SIDLength && 0 == wcsnicmp( DataOrig, g_RegInfo.RegSIDPath, SIDLength ) )
	{
		memcpy( ptr1, L"\\user\\current", 0x1A );
		ptr1 += 0x1A / 2;
		
		length -= SIDLength ;
		ptr2 = &DataOrig[ SIDLength ];
	}
	else
	{
		length -= 9 ;
		ptr2 = &DataOrig[ 0x9 ];
	}
	
	memcpy( ptr1, ptr2, length * sizeof(WCHAR) );
	ptr1[ length ] = UNICODE_NULL ;

	*RedirectedPath = DataRedirect ;
	return STATUS_SUCCESS ;
}


BOOL
IsRedirectedKeyInvalid (
	IN LPWSTR RedirectedPath
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/09/27 [27:9:2010 - 11:06]

Routine Description:
  ɳ����� \HKEY_USERS\Sandbox_AV_DefaultBox\ �½����ض���ע����ֵ,��ֹɳ���еĳ���ֱ�ӷ��ʲ�����ǰĿ¼�µļ�ֵ!
    
Arguments:
  RedirectedPath - �������ض���ע���·��

Return Value:
  TRUE - �ܾ�����
  FALSE - �������
    
--*/
{
	NTSTATUS status = STATUS_SUCCESS ; 
	LPWSTR ptr, ptr1, szNameTmp = NULL ; 
	OBJECT_ATTRIBUTES	ObjectAttributes ;
	UNICODE_STRING		KeyName ;
	HANDLE				KeyHandle ;
	ULONG				length ;
	ULONG_PTR KeyRootPathLength = (g_BoxInfo.KeyRootPathLength / sizeof(WCHAR)) - 1 ;
	WCHAR tmp[ MAX_PATH ] = L"" ;

	if ( NULL == RedirectedPath ) { return FALSE ; }

	// "\REGISTRY\USER\Sandbox_AV_DefaultBox\Machine\software\0001"
	memcpy( tmp, RedirectedPath, MAX_PATH );

	ptr = &tmp[ KeyRootPathLength ] ;// "\Machine\software\0001"
	ptr1 = wcsrchr(tmp, '\\');  

	if ( NULL == ptr1 || ptr1 == tmp ) { return FALSE ; }

	*ptr1 = 0 ; // "\Machine\software"
	length = wcslen( ptr ); 

	switch ( length )
	{
	case 8:
		szNameTmp = L"\\machine";
		break;

	case 0x11:
		szNameTmp = L"\\machine\\software";
		break;

	case 0x19:
		szNameTmp = L"\\machine\\software\\classes";
		break;

	case 0x1F:
		szNameTmp = L"\\machine\\software\\classes\\clsid";
		break;

	case 0x21:
		szNameTmp = L"\\machine\\software\\classes\\typelib";
		break;

	case 0x23:
		szNameTmp = L"\\machine\\software\\classes\\interface";
		break;

	case 5:
		szNameTmp = L"\\user";
		break;

	case 0xD:
		szNameTmp = L"\\user\\current";
		break;

	case 0x16:
		szNameTmp = L"\\user\\current\\software";
		break;

	case 0x1E:
		szNameTmp = L"\\user\\current\\software\\classes";
		break;

	case 0x24:
		szNameTmp = L"\\user\\current\\software\\classes\\clsid" ;
		break;

	case 0x26:
		szNameTmp = L"\\user\\current\\software\\classes\\typelib" ;
		break;

	case 0x28:
		szNameTmp = L"\\user\\current\\software\\classes\\interface";
		break;

	default:
		break ;
	}

	// �����������ϰ������б�,�������
	if ( szNameTmp && 0 == wcsicmp(ptr, szNameTmp) ) { return FALSE ; }

	// ���ڷǰ׵�·��,���Ҫ������ֵ�ĸ����Ƿ���Sandbox������,����,���ֹ�ò���!�����������ɳ���Լ��������ض����ֵ!
	RtlInitUnicodeString( &KeyName, tmp );
	InitializeObjectAttributes( &ObjectAttributes, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	status = g_NtOpenKey_addr( &KeyHandle, KEY_READ, &ObjectAttributes );
	if ( NT_SUCCESS(status) )
	{
		BOOL bIsDirtyKey = IsDirtyKey( KeyHandle );
		ZwClose( KeyHandle );
		if ( bIsDirtyKey ) { return TRUE ;}  // ����TRUE��ʾ�ܾ� 
	}

	return FALSE ; // Ĭ�ϲ���ֹ
}


BOOL 
IsDirtyKey ( 
	IN HANDLE KeyHandle 
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/09/27 [27:9:2010 - 11:06]

Routine Description:
  ��⵱ǰ����Ƿ�Ϊ�����Ϊ"Dirty"�ļ�ֵ    
    
Arguments:
  KeyHandle - ����֤�ľ��
    
--*/
{
	NTSTATUS status ;
	ULONG ResultLength ;
	KEY_BASIC_INFORMATION KeyInformation = { 0 } ;

	if ( NULL == KeyHandle ) { return FALSE ; }

	status = g_NtQueryKey_addr( KeyHandle, KeyBasicInformation, &KeyInformation, 0x18, &ResultLength );
	
	if ( status && status != STATUS_BUFFER_OVERFLOW ) { return FALSE ; }

	return IsDirtyKeyEx( (PVOID)&KeyInformation ) ;
}


BOOL 
IsDirtyKeyEx ( 
	IN PVOID KeyBaseInfo 
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/09/27 [27:9:2010 - 11:06]

Routine Description:
  ����Ƿ�Ϊ�����Ϊ"Dirty"�ļ�ֵ    
    
Arguments:
  KeyBaseInfo - ����֤�ľ��ʱ������
    
--*/
{
	PLARGE_INTEGER LastWriteTime = (PLARGE_INTEGER) KeyBaseInfo ;

	if ( NULL == KeyBaseInfo ) { return FALSE ; }

	return (0x1B01234 == LastWriteTime->HighPart && 0xDEAD44A0 == LastWriteTime->LowPart) ;
}


NTSTATUS MarkDirtyKey( IN HANDLE KeyHandle )
{
	NTSTATUS status ;
	LARGE_INTEGER LastWriteTime = { 0xDEAD44A0, 0x1B01234 }; /*(LARGE_INTEGER)0x1B01234DEAD44A0i64 */
	//								LowPart		HighPart
	
	status = ZwSetInformationKey( KeyHandle, KeyWriteTimeInformation, &LastWriteTime, 8 );
	RemoveRegNodeEx( KeyHandle, TRUE );
	ZwClose( KeyHandle );

	return status ;
}


NTSTATUS MarkKeyTime( IN HANDLE KeyHandle )
{
	NTSTATUS status ;
	FILETIME SystemTimeAsFileTime ;

	if ( NULL == KeyHandle ) { return STATUS_INVALID_PARAMETER ; }

	GetSystemTimeAsFileTime( &SystemTimeAsFileTime );

	status = ZwSetInformationKey( KeyHandle, KeyWriteTimeInformation, &SystemTimeAsFileTime, 8 );
// 	if ( STATUS_ACCESS_DENIED == status )
// 	{
//		HANDLE hKey ;
//		OBJECT_ATTRIBUTES ObjectAttributes ;
//		UNICODE_STRING KeyName ; 

// 		RtlInitUnicodeString(&uniName, (PCWSTR)&g_TmpName_Total );
// 		InitializeObjectAttributes( &ObjectAttributes, &KeyName, OBJ_CASE_INSENSITIVE, KeyHandle, NULL );
// 
// 		status = ZwOpenKey( &hKey, KEY_SET_VALUE, &ObjectAttributes );
// 		if ( NT_SUCCESS(status) )
// 		{
// 			status = NtSetInformationKey( KeyHandle, KeyWriteTimeInformation, &SystemTimeAsFileTime, 8 );
// 			ZwClose( hKey );
// 		}
// 	}

	return status ;
}


NTSTATUS 
CreateRedirectedSubkeys (
	IN POBJECT_ATTRIBUTES objectAttributes
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/09/28 [28:9:2010 - 19:17]

Routine Description:
  �ݹ鴴��ָ���ļ�ֵ
    
Arguments:
  objectAttributes - ������������ֵ��������Ϣ
    
--*/
{
	LPWSTR ptr_start, ptr_end ;
	PUNICODE_STRING uniObjectName ;
	ULONG Disposition, NewLength ;
	USHORT OldMaxLength, OldLength ;
	NTSTATUS status ;
	WCHAR OldData ;
	HANDLE hKey ;

	if ( NULL == objectAttributes )
	{
		MYTRACE( L"error! | CreateRedirectedSubkeys();| �������Ϸ�  \n" );
		return STATUS_INVALID_PARAMETER ;
	}

	uniObjectName = objectAttributes->ObjectName;
	ptr_start = uniObjectName->Buffer ;
	ptr_end = &ptr_start[ uniObjectName->Length / sizeof(WCHAR) ];

	// 1. ������ʻ,ֱ���ɹ�����һ��ע����ֵ
	while ( TRUE )
	{
		-- ptr_end ;
		if ( ptr_end <= ptr_start ) { return STATUS_OBJECT_PATH_INVALID ; }

		while ( '\\' != *ptr_end ) { -- ptr_end ; if ( ptr_end <= ptr_start ) { return STATUS_OBJECT_PATH_INVALID ; } }

		if ( ptr_end <= ptr_start ) { return STATUS_OBJECT_PATH_INVALID ; }

		OldData = *ptr_end ;
		OldMaxLength = uniObjectName->MaximumLength ;
		OldLength	 = uniObjectName->Length ;

		*ptr_end = NULL ;
		NewLength = (ULONG)((PCHAR)ptr_end - (PCHAR)ptr_start) ;
		uniObjectName->Length		 = (USHORT) NewLength ;
		uniObjectName->MaximumLength = (USHORT) (NewLength + 2) ;

		status = g_NtCreateKey_addr( &hKey, 0x80000000, objectAttributes, 0, 0, 0, &Disposition );

		uniObjectName->Length		 = OldLength ;
		uniObjectName->MaximumLength = OldMaxLength ;
		*ptr_end = OldData ;

		if ( NT_SUCCESS(status) ) { break ; }

		if ( (STATUS_OBJECT_NAME_NOT_FOUND != status) && (STATUS_OBJECT_PATH_NOT_FOUND != status) ) { return status ; }
	}

	// 2. ��ʼ���ҵ����ǲ��ֵ,�������
	ZwClose( hKey );

	if ( REG_OPENED_EXISTING_KEY == Disposition && IsDirtyKey(hKey) ) { return STATUS_OBJECT_NAME_NOT_FOUND ; }
	
	while ( TRUE )
	{
		for ( ++ptr_end; *ptr_end; ++ptr_end )
		{
			if ( '\\' == *ptr_end ) { break ; }
		}

		OldData = *ptr_end ;
		OldMaxLength = uniObjectName->MaximumLength ;
		OldLength	 = uniObjectName->Length ;

		*ptr_end = NULL ;
		NewLength = (ULONG)((PCHAR)ptr_end - (PCHAR)ptr_start) ;
		uniObjectName->Length		 = (USHORT) NewLength ;
		uniObjectName->MaximumLength = (USHORT) (NewLength + 2) ;

		status = g_NtCreateKey_addr( &hKey, 0x80000000, objectAttributes, 0, 0, 0, &Disposition );

		uniObjectName->Length		 = OldLength ;
		uniObjectName->MaximumLength = OldMaxLength ;
		*ptr_end = OldData ;

		if ( ! NT_SUCCESS(status) ) { break ; }
		
		ZwClose( hKey );
		if ( ! OldData ) { return status ; }
	}

	return status ;
}


VOID
RemoveRegNodeEx (
	IN HANDLE KeyHandle,
	IN BOOL bRemoveFatherNode 
	)
{
	UNICODE_STRING KeyName ;
	NTSTATUS status = STATUS_SUCCESS ; 
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL ;

	if ( NULL == KeyHandle ) { return ; }

	RtlInitUnicodeString( &KeyName, NULL );

	status = GetRegPath (
		KeyHandle ,   
		&KeyName ,
		&OrignalPath ,
		&RedirectedPath ,
		NULL
		);

	if ( NT_SUCCESS(status) )
	{
		RemoveRegNodeExp( OrignalPath, bRemoveFatherNode );
	}
	else
	{
		kfree( OrignalPath );
		kfree( RedirectedPath );
	}

	return ;
}


VOID 
RemoveRegNodeExp ( 
	IN LPWSTR szPath,
	IN BOOL bRemoveFatherNode 
	)
{
	LPWSTR ptr = NULL ;
	LPXREG_INFORMATION_CLASS pCurrentNode, pNextNode ; 

	if ( NULL == szPath ) { return ; }

	ULONG length = wcslen( szPath );
	EnterCriticalSection( &g_cs_Regedit );

	pCurrentNode = (LPXREG_INFORMATION_CLASS) g_XREG_INFORMATION_HEAD_Redirected.Flink ;
	if ( NULL == pCurrentNode ) { LeaveCriticalSection( &g_cs_Regedit ); return ; }
	
	do
	{
		pNextNode = (LPXREG_INFORMATION_CLASS) pCurrentNode->ListEntry.Flink ;
		if ( (2 * length != pCurrentNode->NameLength) || (wcsnicmp(pCurrentNode->wszOrignalRegPath, szPath, length)) ) { goto _WHILE_NEXT_ ; }
		
		// �����������ҵ���ɾ���Ľڵ�
		if ( bRemoveFatherNode )
		{
			ptr = wcsrchr( pCurrentNode->wszOrignalRegPath, '\\' );
			if ( ptr )
			{
				*ptr = NULL ;
				RemoveRegNodeExp( pCurrentNode->wszOrignalRegPath, FALSE );
				*ptr = '\\';
				pNextNode = (LPXREG_INFORMATION_CLASS) pCurrentNode->ListEntry.Flink ;
			}
		}

		RemoveRegNodeCom( (PVOID)pCurrentNode, (PVOID)&g_XREG_INFORMATION_HEAD_Redirected );

_WHILE_NEXT_ :
		pCurrentNode = pNextNode ;
	}
	while ( pNextNode );

	LeaveCriticalSection( &g_cs_Regedit );
	return ;
}


VOID 
RemoveRegNodeCom ( 
	IN PVOID pNode,
	IN PVOID pHead
	)
{
	LPXREG_INFORMATION_SUBKEY   pNodeKey	 = NULL ;
	LPXREG_INFORMATION_VALUEKEY pNodeValue	 = NULL ;
	LPXREG_INFORMATION_CLASS	pCurrentNode = (LPXREG_INFORMATION_CLASS) pNode ;
	LPLIST_ENTRY_EX				pNodeHead	 = (LPLIST_ENTRY_EX) pHead ;

	if ( NULL == pNode || NULL == pHead ) { return ; }

	// ���������Ƴ���ǰ�ڵ�
	RemoveEntryListEx( pNodeHead, (PLIST_ENTRY)pCurrentNode );

	// �ͷŵ�ǰ�ڵ���������ڴ� (����Key)
	pNodeKey = pCurrentNode->pSubKey_ShowTime_Firstone ;
	if ( pNodeKey )
	{
		do
		{
			if ( FALSE == MmIsAddressValid( (PVOID)pNodeKey, sizeof(LIST_ENTRY) ) )
			{
				MYTRACE( L"error! | RemoveRegNodeCom(); | ���ͷŵ�ǰ�ڵ���������ڴ� (����Key)������,�������ָ��Ƿ�. JUMP OUT \n" );
				break;
			}

			RemoveEntryListEx( (LPLIST_ENTRY_EX) &pCurrentNode->pSubKey_ShowTime_Firstone, (PLIST_ENTRY)pNodeKey );
			kfree( pNodeKey );
			pNodeKey = pCurrentNode->pSubKey_ShowTime_Firstone ;
		}
		while ( pNodeKey );
	}

	// �ͷŵ�ǰ�ڵ���������ڴ� (����Value)
	pNodeValue = pCurrentNode->pValueKey_ShowTime_Lastone ;
	if ( pNodeValue )
	{
		do
		{
			if ( FALSE == MmIsAddressValid( (PVOID)pNodeValue, sizeof(LIST_ENTRY) ) )
			{
				MYTRACE( L"error! | RemoveRegNodeCom(); | ���ͷŵ�ǰ�ڵ���������ڴ� (����Value)������,�������ָ��Ƿ�. JUMP OUT \n" );
				break;
			}

			RemoveEntryListEx( (LPLIST_ENTRY_EX) &pCurrentNode->pValueKey_ShowTime_Lastone, (PLIST_ENTRY)pNodeValue );
			kfree( pNodeValue );
			pNodeValue = pCurrentNode->pValueKey_ShowTime_Lastone ;
		}
		while ( pNodeValue );
	}

	kfree( pCurrentNode );	
	return ;
}


VOID 
RemoveRegNodeComEx ( 
	IN PVOID pNode,
	IN BOOL bFlag_Free_Current_Node
	)
{
	LPXREG_INFORMATION_SUBKEY	pNodeKey	 = NULL ;
	LPXREG_INFORMATION_VALUEKEY pNodeValue	 = NULL ;
	LPXREG_INFORMATION_CLASS	pCurrentNode = (LPXREG_INFORMATION_CLASS) pNode ;

	if ( NULL == pNode ) { return ; }

	// �ͷŵ�ǰ�ڵ���������ڴ� (����Key)
	pNodeKey = pCurrentNode->pSubKey_ShowTime_Firstone ;
	if ( pNodeKey )
	{
		do
		{
			if ( FALSE == MmIsAddressValid( (PVOID)pNodeKey, sizeof(LIST_ENTRY) ) )
			{
				MYTRACE( L"error! | RemoveRegNodeComEx(); | ���ͷŵ�ǰ�ڵ���������ڴ� (����Key)������,�������ָ��Ƿ�. JUMP OUT \n" );
				break;
			}

			RemoveEntryListEx( (LPLIST_ENTRY_EX) &pCurrentNode->pSubKey_ShowTime_Firstone, (PLIST_ENTRY)pNodeKey );
			kfree( pNodeKey );
			pNodeKey = pCurrentNode->pSubKey_ShowTime_Firstone ;
		}
		while ( pNodeKey );
	}

	// �ͷŵ�ǰ�ڵ���������ڴ� (����Value)
	pNodeValue = pCurrentNode->pValueKey_ShowTime_Lastone ;
	if ( pNodeValue )
	{
		do
		{
			if ( FALSE == MmIsAddressValid( (PVOID)pNodeValue, sizeof(LIST_ENTRY) ) )
			{
				MYTRACE( L"error! | RemoveRegNodeComEx(); | ���ͷŵ�ǰ�ڵ���������ڴ� (����Value)������,�������ָ��Ƿ�. JUMP OUT \n" );
				break;
			}

			RemoveEntryListEx( (LPLIST_ENTRY_EX) &pCurrentNode->pValueKey_ShowTime_Lastone, (PLIST_ENTRY)pNodeValue );
			kfree( pNodeValue );
			pNodeValue = pCurrentNode->pValueKey_ShowTime_Lastone ;
		}
		while ( pNodeValue );
	}

	if ( bFlag_Free_Current_Node ) { kfree( pCurrentNode ); }	
	return ;
}


NTSTATUS
StubNtDeleteKey (
    IN HANDLE KeyHandle,
	IN BOOL bRecursive
	)
{
	NTSTATUS status = STATUS_SUCCESS ; 
	LPWSTR OrignalPath = NULL, RedirectedPath = NULL ;
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING KeyName ;
	HANDLE hRedirectedKey, hkey ;
	ULONG Length, ReturnLength ;
	BOOL bIsWhite = FALSE, bIsBlack = FALSE, bNeedRemoveNode = FALSE ;
	BOOL bIsHandlerSelfRegPath = FALSE ;
	LPKEYINFO KeyInfo = NULL ;

	if ( NULL == KeyHandle ) { return STATUS_INVALID_PARAMETER ; }

	// 1.1 �õ�������ԭʼ��ֵ & �ض����ֵ	
	RtlInitUnicodeString( &KeyName, NULL );
	InitializeObjectAttributes( &ObjAtr, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	status = GetRegPath (
		KeyHandle ,   
		&KeyName ,
		&OrignalPath ,
		&RedirectedPath ,
		NULL
		);

	if ( !NT_SUCCESS(status) )
	{
		MYTRACE( L"error! | StubNtDeleteKey() - GetRegPath(1); | status=0x%08lx \n", status );
		return status ;
	}

	// 1.2 �����ض����� & ��ֵ·��,�ٴβ�ѯ
	RtlInitUnicodeString( &KeyName, RedirectedPath );

	status = ZwOpenKey( &hRedirectedKey, KEY_READ | 0x40000000, &ObjAtr );
	if ( !NT_SUCCESS(status) )
	{
		MYTRACE( L"error! | StubNtDeleteKey() - ZwOpenKey(); | status=0x%08lx \n", status );
		goto _over_ ;
	}

	// ���ͷ��ڴ�,�ٽ��в�ѯ
	kfree( OrignalPath );
	RedirectedPath = NULL ;

	status = GetRegPath (
		hRedirectedKey ,   
		&KeyName ,
		&OrignalPath ,
		&RedirectedPath ,
		&bIsHandlerSelfRegPath
		);

	kfree( KeyName.Buffer );

	if ( !NT_SUCCESS(status) )
	{
		MYTRACE( L"error! | StubNtDeleteKey() - GetRegPath(2); | status=0x%08lx \n", status );
		return status ;
	}

	//  
	// 2. �������Ĳ���"\REGISTRY\USER\Sandbox_AV_DefaultBox" �µļ�ֵ,ֱ�ӵ���ԭʼ����ɾ����.
	//    ��ΪNtDeleteKey�Ĳ���@KeyHandle�϶��ǵ���NtOpenKey�õ�,��ʱ�Ѿ����ض�λ��ɳ��
	//    Ŀ¼����.��δ���ض���,�����������ǰ�������ֵ,����֮. �ʵ�����һ���ǰ�������ֵ...
	//

	if ( FALSE == bIsHandlerSelfRegPath )
	{
		ZwClose( hRedirectedKey );
		status = g_NtDeleteKey_addr( KeyHandle );
		goto _over_ ;
	}

	// 3. ɾ�����ض���ɳ��ע���Ŀ¼�µļ�ֵ
	Length = 0x100 ;
	KeyInfo = (LPKEYINFO) kmalloc( Length );
	if ( NULL == KeyInfo )
	{
		MYTRACE( L"error! | StubNtDeleteKey() - kmalloc(); | �����ڴ�ʧ�� \n" );
		status = STATUS_UNSUCCESSFUL  ;
		goto _over_ ;
	}

	if ( FALSE == bRecursive )
	{
		status = ZwQueryKey( hRedirectedKey, KeyFullInformation, KeyInfo, Length, &ReturnLength );
		if ( status && STATUS_BUFFER_OVERFLOW != status )
		{
			MYTRACE( L"error! | StubNtDeleteKey() - ZwQueryKey; | status=0x%08lx \n", status );
			goto _over_ ;
		}

		if ( KeyInfo->u.Full.SubKeys ) // �������Ӽ�,�ܾ�ɾ��
		{
			ZwClose( hRedirectedKey );
			status = STATUS_CANNOT_DELETE ;
			goto _over_ ;
		}
	}

	// 4. ɾ����ǰĿ¼�µ����м�
	while ( TRUE )
	{
		while ( TRUE )
		{
			status = g_NtEnumerateKey_addr( hRedirectedKey, 0, KeyBasicInformation, KeyInfo, Length, &ReturnLength );
			if ( STATUS_BUFFER_OVERFLOW != status ) { break ; }

			kfree( KeyInfo );
			Length += 0x100 ;
			KeyInfo = (LPKEYINFO) kmalloc( Length );
		}

		// 4.1 ��ǰ��û���Ӽ�,����ѭ��
		if ( status ) { break ; }

		// 4.2 ��ǰ��ӵ���Ӽ�,����Orignal_NtOpenKey��֮
		KeyName.Length = LOWORD( KeyInfo->u.Basic.NameLength ) ;
		KeyName.MaximumLength = KeyName.Length ;
		KeyName.Buffer = KeyInfo->u.Basic.Name ;
		ObjAtr.RootDirectory = hRedirectedKey ;
		
		status = g_NtOpenKey_addr( &hkey, 0x40010000, &ObjAtr );
		if ( !NT_SUCCESS(status) )
		{
			ZwClose( hRedirectedKey );
			goto _over_ ;
		}

		// 4.3 ��Ҫ��ݹ�,��ݹ�ֱ���ҵ��������Ӽ��ļ�
		if ( bRecursive ) { StubNtDeleteKey( hkey, TRUE ); }

		// 4.4 ����Ҫ��ݹ�,�����ԭʼ����ɾ����ǰ��ֵ
		g_NtDeleteKey_addr( hkey );
		ZwClose( hkey );
	}

	// 5. ��û���Ӽ���.���õ�ǰ��Ϊ����ʱ��.�����Ϊ"��ɾ��"
	if ( STATUS_NO_MORE_ENTRIES == status )
	{
		status = MarkDirtyKey( hRedirectedKey );
	}
	else
	{
		ZwClose( hRedirectedKey );
	}

_over_ :
	if ( KeyInfo ) { kfree( KeyInfo ); }
	kfree( OrignalPath );
	kfree( RedirectedPath );
	return status ;
}


NTSTATUS 
RegKeyFilter (
	IN HANDLE KeyHandle,
	IN LPWSTR OrignalPath,
	IN LPWSTR RedirectedPath,
	IN BOOL bIs_NtEnumerateKey_called,
	IN BOOL bIs_NtEnumerateValueKey_called,
	OUT PVOID * out_pCurrentRegNode_Redirected
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/10/13 [13:10:2010 - 11:38]

Routine Description:
  (1) ͨ������ƥ��@KeyHandle��������,�ҵ��򷵻ظý��,�����½�֮
  ����������STATUS_ACCESS_DENIED�ܾ���; ����������STATUS_BAD_INITIAL_PC��Ƿ���
  (2) ����Handler_RegNode_Orignal()����ˢ�� & �ع����
  (3) ����Handler_SubKeyNodes_NtEnumerateKey() �� Handler_SubValueNodes_NtEnumerateValueKey()������һ������

  ע��,���سɹ��Ļ�,��������Ҫ�ͷ�ȫ����,�� LeaveCriticalSection( &g_cs_Regedit );
    
Arguments:
  KeyHandle - 
  OrignalPath - ԭʼ��ֵ·��
  RedirectedPath - �ض����ֵ·��
  bIs_NtEnumerateKey_called - �Ƿ�ΪNtEnumerateKey�ĵ���
  bIs_NtEnumerateValueKey_called - �Ƿ�ΪNtEnumerateValueKey�ĵ��� 
  out_pCurrentRegNode_Redirected - ���������ĵ�ǰ�ض��򸸼����

Return Value:
  NTSTATUS
    
--*/
{
	NTSTATUS status = STATUS_SUCCESS ; 
	DWORD TickCount = GetTickCount();
	ULONG NameLength = 0 ;
	HANDLE hRedirectedKey = NULL, out_hKey_Redirected = NULL ;
	BOOL bNeedRemove = FALSE, bBuildNewNode = FALSE ;
	LPXREG_INFORMATION_CLASS pCurrentRedirectedNode = NULL, pNextRedirectedNode = NULL ; 
	LPXREG_INFORMATION_CLASS pCurrentOrignalNode = NULL, pNextOrignalNode = NULL ; 

	EnterCriticalSection( &g_cs_Regedit );

	NameLength = wcslen( OrignalPath ) * sizeof(WCHAR) ;
	pCurrentRedirectedNode = (LPXREG_INFORMATION_CLASS) g_XREG_INFORMATION_HEAD_Redirected.Flink ;
	
	if ( NULL == pCurrentRedirectedNode )
	{ 
		bBuildNewNode = TRUE ; 
	}
	else
	{
		// ͨ������ƥ��@hKey��������,�ҵ��򷵻ظý��,�����½�֮
		while ( TRUE )
		{
			bNeedRemove = FALSE ;
			pNextRedirectedNode = (LPXREG_INFORMATION_CLASS) pCurrentRedirectedNode->ListEntry.Flink ;
			
			if ( KeyHandle == pCurrentRedirectedNode->hKey ) 
			{ 
				if ( (pCurrentRedirectedNode->NameLength == NameLength) && (0 == wcsnicmp(pCurrentRedirectedNode->wszOrignalRegPath, OrignalPath, NameLength / sizeof(WCHAR))) ) { break ; }
				
				bNeedRemove = TRUE ;
			}
			else
			{
				if ( TickCount - pCurrentRedirectedNode->TickCount > 5000 ) { bNeedRemove = TRUE ; }
			}

			// �����ƥ�䵽,�����Ʋ�ƥ�����TickCounts���Ϸ�,������Ƴ���ǰ�ĳ¾ɽ��
			if ( bNeedRemove ) { RemoveRegNodeCom( (PVOID)pCurrentRedirectedNode, (PVOID)&g_XREG_INFORMATION_HEAD_Redirected ); }

			pCurrentRedirectedNode = pNextRedirectedNode ;
			if ( NULL == pCurrentRedirectedNode ) { bBuildNewNode = TRUE ; break ; }
		}
	}

	// �½����
	if ( bBuildNewNode )
	{
		pCurrentRedirectedNode = (LPXREG_INFORMATION_CLASS) kmalloc( NameLength + sizeof(WCHAR) + sizeof(XREG_INFORMATION_CLASS) );
		
		pCurrentRedirectedNode->TickCount		= TickCount  ;
		pCurrentRedirectedNode->NameLength	= NameLength ;
		pCurrentRedirectedNode->hKey			= KeyHandle  ;
		memcpy( pCurrentRedirectedNode->wszOrignalRegPath, OrignalPath, NameLength + sizeof(WCHAR) );

		InsertListA( &g_XREG_INFORMATION_HEAD_Redirected, NULL, (PLIST_ENTRY)pCurrentRedirectedNode );
	}

	// ����ƥ�䵽�Ľ��
	if ( pCurrentRedirectedNode->bFlag_IsIn_WhiteList )
	{
		LeaveCriticalSection( &g_cs_Regedit );
		return STATUS_BAD_INITIAL_PC ;
	}

	BOOL bNeedHandlerKeysVaules = FALSE ;
	BOOL bPleaseDonotFlush = ( FALSE == bIs_NtEnumerateKey_called || pCurrentRedirectedNode->bFlag_Handler_SubKeyNodes_NtEnumerateKey_OK ) 
				&& ( FALSE == bIs_NtEnumerateValueKey_called || pCurrentRedirectedNode->bFlag_Handler_SubValueNodes_NtEnumerateValueKey_OK );

	if ( bPleaseDonotFlush )
	{
		bNeedHandlerKeysVaules = TRUE ;
		hRedirectedKey = NULL ;
		pCurrentOrignalNode = NULL ;
		out_hKey_Redirected = NULL ;
	}
	else
	{
		// ��Ҫ��һ��,ˢ�� & �ع����  
		status = Handler_RegNode_Orignal (
			&out_hKey_Redirected,
			OrignalPath,
			RedirectedPath,
			(PVOID *) &pCurrentOrignalNode
			);

		if ( NT_SUCCESS(status) )
		{
			hRedirectedKey = out_hKey_Redirected;
			bNeedHandlerKeysVaules = TRUE ;
		}

		if ( STATUS_BAD_INITIAL_PC == status ) { pCurrentRedirectedNode->bFlag_IsIn_WhiteList = TRUE ; }
	}

	if ( bNeedHandlerKeysVaules )
	{
		status = STATUS_SUCCESS ;

		// ����NtEnumerateKey����� 
		if ( bIs_NtEnumerateKey_called && FALSE == pCurrentRedirectedNode->bFlag_Handler_SubKeyNodes_NtEnumerateKey_OK )
		{
			status = Handler_SubKeyNodes_NtEnumerateKey( pCurrentRedirectedNode, pCurrentOrignalNode, hRedirectedKey );
			pCurrentRedirectedNode->bFlag_Handler_SubKeyNodes_NtEnumerateKey_OK = TRUE ;
			hRedirectedKey = out_hKey_Redirected;
		}

		// ����NtEnumerateValueKey�����
		if ( status >= 0 && bIs_NtEnumerateValueKey_called && FALSE == pCurrentRedirectedNode->bFlag_Handler_SubValueNodes_NtEnumerateValueKey_OK )
		{
			status = Handler_SubValueNodes_NtEnumerateValueKey( pCurrentRedirectedNode, pCurrentOrignalNode, hRedirectedKey );
			pCurrentRedirectedNode->bFlag_Handler_SubValueNodes_NtEnumerateValueKey_OK = 1;
			hRedirectedKey = out_hKey_Redirected;	
		}

		if ( hRedirectedKey ) { ZwClose( hRedirectedKey ); }
	}
 
	if ( !NT_SUCCESS(status) ) { LeaveCriticalSection( &g_cs_Regedit ); }
	*out_pCurrentRegNode_Redirected = (PVOID) pCurrentRedirectedNode ;
	return status ;
}


NTSTATUS 
Handler_SubKeyNodes_NtEnumerateKey (
	IN OUT LPXREG_INFORMATION_CLASS RedirectedNode,
	IN LPXREG_INFORMATION_CLASS OrignalNode,
	IN HANDLE hRedirectedKey
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/10/13 [13:10:2010 - 16:06]

Routine Description:
  (1) �����ض����㼴��ǰ������LastWriteTime
  (2) "����"��ǰ�ض��򸸼�+0x20��������
  Ϊʲô��"����"? ������ɳ���в���ԭʼ��ֵ(���AĿ¼),�½���N���Ӽ�,
  ��Щ�½��ļ����ض���ɳ���Լ�ά����ע���Ŀ¼��(���BĿ¼).���ָ�
  �û�����AĿ¼+BĿ¼�����.����ԭʼ��ֵ����,��������Ӧ������õ�����
  ԭʼ��ֵ�µ������Ӽ���Ϣ,����õ��ض���Ŀ¼�������Ӽ���Ϣ,�ٽ�������.

  (3) �����Ϲ�����,�����ֱ����Ϊ"��ɾ��"���,����������Ƴ�֮.
  ��������Ч����,��ɳ����չʾ���û�,�û�������"��ɾ��"�ļ�ֵ
    
Arguments:
  RedirectedNode - Ҫ�����ض��򸸼����+0x20�����Ӽ�����
  OrignalNode - ԭʼ�������
  hRedirectedKey - �ض��򸸼����

Return Value:
  NTSTATUS

--*/
{
	int ret = 0 ;
	BOOL bIsDirtykey = TRUE, bFreeNode = FALSE ;
	NTSTATUS status = STATUS_SUCCESS ;
	LPXREG_INFORMATION_SUBKEY RedirectedNew = NULL, pNode = NULL ;
	ULONG ResultLength, Length, NameLength, Index = 0 ;
	LPKEYINFO KeyInformation = (LPKEYINFO) kmalloc( 0x80 ) ;

	if ( NULL == hRedirectedKey || NULL == RedirectedNode ) { return STATUS_INVALID_PARAMETER ; }

	// 1.1 ���ض����ֵ,��ѯ��Ϣ
	status = g_NtQueryKey_addr( hRedirectedKey, KeyBasicInformation, KeyInformation, 0x80, &ResultLength );
	if ( !NT_SUCCESS(status) && STATUS_BUFFER_OVERFLOW != status )
	{
		kfree( KeyInformation );
		return status ;
	}

	// 1.2 �����ض����㼴��ǰ������LastWriteTime
	RedirectedNode->LastWriteTime.LowPart  = KeyInformation->u.Basic.LastWriteTime.LowPart  ;
	RedirectedNode->LastWriteTime.HighPart = KeyInformation->u.Basic.LastWriteTime.HighPart ;

	// 1.3 ���ڴ���ԭʼ��ֵ��������д���
	if ( OrignalNode )
	{
		// 1.3.1 ��ԭʼ��ֵ�����д��ʱ�������µ�,����µ��ض����ֵ��ȥ
		if ( OrignalNode->LastWriteTime.HighPart > RedirectedNode->LastWriteTime.HighPart || OrignalNode->LastWriteTime.LowPart > RedirectedNode->LastWriteTime.LowPart )
		{
			RedirectedNode->LastWriteTime.LowPart  = KeyInformation->u.Basic.LastWriteTime.LowPart  ;
			RedirectedNode->LastWriteTime.HighPart = KeyInformation->u.Basic.LastWriteTime.HighPart ;
		}
  
		// 1.3.2 ԭʼ��ֵ����,֮ǰ�Ѿ�������һ��ԭʼ�Ӽ�����.�ʽ����ԭʼ������ȡ���Ӽ������е����нڵ�,�������뵽�ض��򸸼��м���
		LPXREG_INFORMATION_SUBKEY CopyedOrignalNode = NULL ;
		LPXREG_INFORMATION_SUBKEY CurrentOrignalNode = OrignalNode->pSubKey_ShowTime_Firstone ;
		
		if ( CurrentOrignalNode )
		{
			while ( TRUE )
			{
				Length = CurrentOrignalNode->NameLength + sizeof(XREG_INFORMATION_SUBKEY) + sizeof(WCHAR) ;
				CopyedOrignalNode = (LPXREG_INFORMATION_SUBKEY) kmalloc( Length ) ; // ������

				CopyedOrignalNode->NameLength = NameLength = CurrentOrignalNode->NameLength ;

				memcpy( CopyedOrignalNode->wszSubKeyName, CurrentOrignalNode->wszSubKeyName, NameLength );
				CopyedOrignalNode->wszSubKeyName[ NameLength / sizeof(WCHAR) ] = UNICODE_NULL ;

				// ���뵽�ض����Ӽ�������ȥ
				InsertListA( (LPLIST_ENTRY_EX)&RedirectedNode->pSubKey_ShowTime_Firstone, 0, (PLIST_ENTRY)CopyedOrignalNode );

				CurrentOrignalNode = CurrentOrignalNode->pFlink ;
				if ( NULL == CurrentOrignalNode ) { break ; }
			}
		}
	}

	// 1.4 "����"��ǰ�ض��򸸼�+0x20��������
	Length = 0x80 ;

	while( TRUE )
	{
		// 1.4.1 ����ԭʼ������ѯ�ض���Ŀ¼ÿ���Ӽ�����Ϣ
		while ( TRUE )
		{
			status = g_NtEnumerateKey_addr( hRedirectedKey, Index, KeyNodeInformation, KeyInformation, Length, &ResultLength );
			
			if ( STATUS_BUFFER_OVERFLOW != status ) { break ; }

			kfree( KeyInformation );
			Length += 0x80 ;
			KeyInformation = (LPKEYINFO) kmalloc( Length );
		}

		if ( STATUS_NO_MORE_ENTRIES == status )
		{
			kfree( KeyInformation );
			return STATUS_SUCCESS ;
		}

		if ( ! NT_SUCCESS(status) ) { break ; }

		// 1.4.2 ����ѯ�õ����Ӽ���Ϣ�����½�� struct _XREG_INFORMATION_SUBKEY_
		ResultLength = KeyInformation->u.Node.NameLength + sizeof(XREG_INFORMATION_SUBKEY) + sizeof(WCHAR) ;
		RedirectedNew = (LPXREG_INFORMATION_SUBKEY) kmalloc( ResultLength );

		RedirectedNew->NameLength = NameLength = KeyInformation->u.Node.NameLength ;

		memcpy( RedirectedNew->wszSubKeyName, KeyInformation->u.Node.Name, NameLength );
		RedirectedNew->wszSubKeyName[ NameLength / sizeof(WCHAR) ] = UNICODE_NULL ;

		RedirectedNew->LastWriteTime.LowPart  = KeyInformation->u.Node.LastWriteTime.LowPart  ;
		RedirectedNew->LastWriteTime.HighPart = KeyInformation->u.Node.LastWriteTime.HighPart ;
		RedirectedNew->bClassName = (KeyInformation->u.Node.TitleIndex || KeyInformation->u.Node.ClassOffset != 0xFFFFFFFF || KeyInformation->u.Node.ClassLength) ;

		bIsDirtykey = IsDirtyKeyEx( (PVOID) &KeyInformation->u.Basic.LastWriteTime );

		bFreeNode = FALSE ;

		// 1.4.3 ��䵱ǰ����+0x20�����Ӽ�����
		pNode = RedirectedNode->pSubKey_ShowTime_Firstone ;
		
		if ( pNode )
		{
			// 1.4.4 ����Subkey�����ֳ��Ƚ�������,Ȼ����뵽��ǰ����+0x20�����Ӽ�������
			while ( TRUE )
			{
				ret = wcsicmp( pNode->wszSubKeyName, RedirectedNew->wszSubKeyName );

				if ( 0 == ret ) { break ; }
				if ( ret > 0 ) { goto _INSERT_ ; }

				pNode = pNode->pFlink ;
				if ( NULL == pNode ) { goto _INSERT_ ; }
			}

			// 1.4.5 ���ڵ�ǰ����+0x20�����Ӽ��������ҵ���ǰ�Ӽ�,��2���������
			if ( bIsDirtykey )
			{
				// (A) �����Ϊ"��ɾ��",����������Ƴ���ǰ���
				RemoveEntryListEx( (LPLIST_ENTRY_EX)&RedirectedNode->pSubKey_ShowTime_Firstone, (PLIST_ENTRY)pNode );
				kfree( pNode );
			}
			else
			{
				// (B) ��������,��������д��ʱ��LastWriteTime
				pNode->LastWriteTime.LowPart	= RedirectedNew->LastWriteTime.LowPart  ;
				pNode->LastWriteTime.HighPart	= RedirectedNew->LastWriteTime.HighPart ;

				if ( RedirectedNew->bClassName ) { pNode->bClassName = RedirectedNew->bClassName ; }
			}

			bFreeNode = TRUE ;
		}
		else
		{
_INSERT_ :
			if ( bIsDirtykey ) 
			{
				bFreeNode = TRUE ;
			}
			else
			{
				// ��ö�ٵõ��ĵ�ǰ�Ӽ������뵽������.
				if ( pNode )
				{
					InsertListB( (LPLIST_ENTRY_EX)&RedirectedNode->pSubKey_ShowTime_Firstone, (PLIST_ENTRY)pNode, (PLIST_ENTRY)RedirectedNew );
				}
				else 
				{
					InsertListA( (LPLIST_ENTRY_EX)&RedirectedNode->pSubKey_ShowTime_Firstone, NULL, (PLIST_ENTRY)RedirectedNew );
				}
			}
		}

		if ( bFreeNode ) { kfree( RedirectedNew ); }
		++ Index ; // ����ö���¸��Ӽ�
		
	} // end-of-while

	kfree( KeyInformation );
	return status ;
}


NTSTATUS 
Handler_SubValueNodes_NtEnumerateValueKey (
	IN LPXREG_INFORMATION_CLASS RedirectedNode,
	IN LPXREG_INFORMATION_CLASS OrignalNode,
	IN HANDLE hRedirectedKey
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/10/13 [13:10:2010 - 16:06]

Routine Description:
  ԭ��ͬ Handler_SubKeyNodes_NtEnumerateKey()
    
--*/
{
	int ret = 0 ;
	BOOL bIsDirtyValuekey = TRUE, bFreeNode = FALSE ;
	NTSTATUS status = STATUS_SUCCESS ;
	LPXREG_INFORMATION_VALUEKEY RedirectedNew = NULL, OrigNode = NULL ;
	ULONG ResultLength, Length, NameLength, ValueDataLength, Index = 0 ;
	PKEY_VALUE_FULL_INFORMATION ValueInfo = (PKEY_VALUE_FULL_INFORMATION) kmalloc( 0x80 ) ;

	if ( NULL == hRedirectedKey || NULL == RedirectedNode ) { return STATUS_INVALID_PARAMETER ; }

	// 1. ���ڴ���ԭʼ��ֵ��������д���
	if ( OrignalNode )
	{
		// ԭʼ��ֵ����,֮ǰ�Ѿ�������һ��ԭʼ��ֵ����.�ʽ����ԭʼ������ȡ����ֵ�����е����нڵ�,�������뵽�ض��򸸼��м���
		LPXREG_INFORMATION_VALUEKEY CopyedOrignalNode = NULL ;
		LPXREG_INFORMATION_VALUEKEY CurrentOrignalNode = OrignalNode->pValueKey_ShowTime_Lastone ;

		if ( CurrentOrignalNode )
		{
			while ( TRUE )
			{
				Length = CurrentOrignalNode->NameLength + CurrentOrignalNode->ValueDataLength + sizeof(XREG_INFORMATION_VALUEKEY) + sizeof(WCHAR) ;
				CopyedOrignalNode = (LPXREG_INFORMATION_VALUEKEY) kmalloc( Length ) ; // ������

				CopyedOrignalNode->NameLength = NameLength = CurrentOrignalNode->NameLength ;

				memcpy( CopyedOrignalNode->wszValueKeyName, CurrentOrignalNode->wszValueKeyName, NameLength );
				CopyedOrignalNode->wszValueKeyName[ NameLength / sizeof(WCHAR) ] = UNICODE_NULL ;

				CopyedOrignalNode->ValueType = CurrentOrignalNode->ValueType ;
				CopyedOrignalNode->ValueData = (char *)&CopyedOrignalNode->wszValueKeyName[0] + CopyedOrignalNode->NameLength + 2 ;
				CopyedOrignalNode->ValueDataLength = ValueDataLength = CurrentOrignalNode->ValueDataLength ;

				memcpy( CopyedOrignalNode->ValueData, CurrentOrignalNode->ValueData, ValueDataLength );

				// ���뵽�ض����ֵ������ȥ
				InsertListA( (LPLIST_ENTRY_EX)&RedirectedNode->pValueKey_ShowTime_Lastone, 0, (PLIST_ENTRY)CopyedOrignalNode );

				CurrentOrignalNode = CurrentOrignalNode->pFlink ;
				if ( NULL == CurrentOrignalNode ) { break ; }
			}
		}
	}

	// 2. "����"����
	Length = 0x80 ;
	while( TRUE )
	{
		// 2.1 ����ԭʼ������ѯ�ض���Ŀ¼ÿ����ֵ����Ϣ
		while ( TRUE )
		{
			status = g_NtEnumerateValueKey_addr( hRedirectedKey, Index, KeyValueFullInformation, ValueInfo, Length, &ResultLength );

			if ( STATUS_BUFFER_OVERFLOW != status ) { break ; }

			kfree( ValueInfo );
			Length += 0x80 ;
			ValueInfo = (PKEY_VALUE_FULL_INFORMATION) kmalloc( Length );
		}

		if ( STATUS_NO_MORE_ENTRIES == status )
		{
			kfree( ValueInfo );
			return STATUS_SUCCESS ;
		}

		if ( ! NT_SUCCESS(status) ) { break ; }

		// 2.2 ����ѯ�õ��ļ�ֵ��Ϣ�����½�� struct _XREG_INFORMATION_VALUEKEY_
		ResultLength = ValueInfo->NameLength + ValueInfo->DataLength + sizeof(XREG_INFORMATION_VALUEKEY) + sizeof(WCHAR) ;
		RedirectedNew = (LPXREG_INFORMATION_VALUEKEY) kmalloc( ResultLength );

		RedirectedNew->NameLength = NameLength = ValueInfo->NameLength ;

		memcpy( RedirectedNew->wszValueKeyName, ValueInfo->Name, NameLength );
		RedirectedNew->wszValueKeyName[ NameLength / sizeof(WCHAR) ] = UNICODE_NULL ;

		RedirectedNew->ValueType = ValueInfo->Type ;
		RedirectedNew->ValueData = (char *)&RedirectedNew->wszValueKeyName[0] + ValueInfo->NameLength + 2 ;
		RedirectedNew->ValueDataLength = ValueInfo->DataLength ;

		memcpy( RedirectedNew->ValueData, (char *)ValueInfo + ValueInfo->DataOffset, ValueInfo->DataLength );

		bFreeNode = FALSE ;

		// 2.3 ��䵱ǰ����+0xXX���ļ�ֵ����
		OrigNode = RedirectedNode->pValueKey_ShowTime_Lastone ;
		bIsDirtyValuekey = ValueInfo->Type ==  _DirtyValueKeyTag_ ;

		if ( OrigNode )
		{
			// ����ValueKey�����ֳ��Ƚ�������,���뵽��ǰ������
			while ( TRUE )
			{
				ret = wcsicmp( OrigNode->wszValueKeyName, RedirectedNew->wszValueKeyName );

				if ( 0 == ret ) { break ; }
				if ( ret > 0 ) { goto _INSERT_ ; }

				OrigNode = OrigNode->pFlink ;
				if ( NULL == OrigNode ) { goto _INSERT_ ; }
			}

			// 1.4.5 �ڵ�ǰ�����ļ�ֵ�������ҵ���ǰ��ֵ
			if ( FALSE == bIsDirtyValuekey )
			{
				InsertListA( (LPLIST_ENTRY_EX)&RedirectedNode->pValueKey_ShowTime_Lastone, (PLIST_ENTRY)OrigNode, (PLIST_ENTRY)RedirectedNew );
				RedirectedNew = NULL ;
			}
			
			RemoveEntryListEx( (LPLIST_ENTRY_EX)&RedirectedNode->pValueKey_ShowTime_Lastone, (PLIST_ENTRY)OrigNode );
			kfree( OrigNode );
			OrigNode = NULL ;
		}

_INSERT_ :
		if ( RedirectedNew )
		{
			if ( bIsDirtyValuekey ) 
			{
				bFreeNode = TRUE ;
			}
			else
			{
				// ��ö�ٵõ��ĵ�ǰ�Ӽ������뵽������.
				if ( OrigNode )
				{
					InsertListB( (LPLIST_ENTRY_EX)&RedirectedNode->pValueKey_ShowTime_Lastone, (PLIST_ENTRY)OrigNode, (PLIST_ENTRY)RedirectedNew );
				}
				else 
				{
					InsertListA( (LPLIST_ENTRY_EX)&RedirectedNode->pValueKey_ShowTime_Lastone, NULL, (PLIST_ENTRY)RedirectedNew );
				}
			}
		}

		if ( bFreeNode ) { kfree( RedirectedNew ); }
		++ Index ; // ����ö���¸��Ӽ�

	} // end-of-while

	kfree( ValueInfo );
	return status ;
}


NTSTATUS 
Handler_RegNode_Orignal (
	OUT PHANDLE phRedirectedKey,
	IN LPWSTR OrignalPath,
	IN LPWSTR RedirectedPath,
	OUT PVOID *pOrignalNode
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/10/13 [13:10:2010 - 16:06]

Routine Description:
  (1) �ڰ���������.����������STATUS_ACCESS_DENIED�ܾ���; ����������STATUS_BAD_INITIAL_PC��Ƿ���
  (2) ��ѯ�ض����(��ǰ����)ֵ,��ʧ�ܷ���STATUS_KEY_DELETED;��Ϊ����ʱ��,��ܾ���
  (3) ���ض����ֵ������,���ѯԭʼ��ֵ,�������ˢ���ؽ��Ӽ�(Subkeys)&��ֵ(ValueKeys)
    
Arguments:
     - 
--*/
{
	NTSTATUS status = STATUS_SUCCESS ; 
	HANDLE hOrignalKey = NULL ;
	ULONG ResultLength ;
	OBJECT_ATTRIBUTES ObjAtr ;
	UNICODE_STRING KeyName ;
	KEY_BASIC_INFORMATION KeyBaseInfo = { 0 } ;
	BOOL bIsWhite = FALSE, bIsBlack = FALSE, bNeedRemoveNode = FALSE ;

	if ( NULL == phRedirectedKey || NULL == OrignalPath || NULL == RedirectedPath || NULL == pOrignalNode ) { return STATUS_INVALID_PARAMETER ; }

	*phRedirectedKey = NULL ;
	*pOrignalNode	 = NULL ;

	InitializeObjectAttributes( &ObjAtr, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	// 1.0 �ڰ���������
	WhiteOrBlack( WhiteOrBlack_Flag_XRegKey, OrignalPath, &bIsWhite, &bIsBlack );

	// 1.1 ������ֱ�ӽ�ֹ
	if ( bIsBlack )
	{
		MYTRACE( L"ko! | Handler_RegNode_Orignal(); | ����������,�ܾ���! szPath=\"%ws\" \n", OrignalPath );
		return STATUS_ACCESS_DENIED ;
	}

	// 1.2 ����������
	if ( bIsWhite ) { return STATUS_BAD_INITIAL_PC ; }

	// 2.1 ��ѯ�ض����(��ǰ����)ֵ�������Ϊ"��ɾ��",��ܾ���
	RtlInitUnicodeString( &KeyName, RedirectedPath );

	status = g_NtOpenKey_addr( phRedirectedKey, KEY_READ, &ObjAtr );
	if ( !NT_SUCCESS(status) )
	{
		if ( STATUS_OBJECT_NAME_NOT_FOUND == status || STATUS_OBJECT_PATH_NOT_FOUND == status )
		{
			status = STATUS_BAD_INITIAL_PC ;
		}

		*phRedirectedKey = NULL ;
		return status ;
	}

	status = g_NtQueryKey_addr( *phRedirectedKey, KeyBasicInformation, &KeyBaseInfo, 0x18, &ResultLength );
	if ( !NT_SUCCESS(status) && status != STATUS_BUFFER_OVERFLOW ) 
	{
		ZwClose( *phRedirectedKey );
		*phRedirectedKey = NULL ;
		return status ;
	}

	if ( IsDirtyKeyEx( (PVOID)&KeyBaseInfo ) )
	{
		ZwClose( *phRedirectedKey );
		*phRedirectedKey = NULL ;
		return STATUS_KEY_DELETED ;
	}

	// 2.2 �ض����ֵ������,��ѯԭʼ��ֵ
	RtlInitUnicodeString( &KeyName, OrignalPath );
	status = g_NtOpenKey_addr( &hOrignalKey, KEY_READ, &ObjAtr );

	if ( !NT_SUCCESS(status) )
	{
		//
		// 2.3.1 ԭʼ��ֵ��ʧ��,statusΪ��2��״̬����:ԭʼ��ֵ�����Ͳ�����! ��Ϊ����ɳ�����½���ֵ,�ᱻ�ض���,
		// �����Ͳ�����ԭʼ��ֵ!������ɳ���ж�ԭʼ��ֵ���в���,������ԭʼ��ֵ!
		//

		if ( STATUS_OBJECT_NAME_NOT_FOUND == status || STATUS_OBJECT_PATH_NOT_FOUND == status ) { return STATUS_SUCCESS ; }
	}
	else
	{
		// 2.3.2 ��ѯԭʼ��ֵ��LastWriteTime
		status = g_NtQueryKey_addr( hOrignalKey, KeyBasicInformation, &KeyBaseInfo, 0x18, &ResultLength );
		status = g_NtQueryKey_addr( hOrignalKey, KeyBasicInformation, &KeyBaseInfo, 0x18, &ResultLength );

		// ����ԭʼ��ֵ�Ľ����Ϣ
		if ( status >= 0 || STATUS_BUFFER_OVERFLOW == status )
		{
			status = Refresh_Orignal_RegNode( hOrignalKey, (PLARGE_INTEGER)&KeyBaseInfo, OrignalPath, pOrignalNode );
		}

		ZwClose( hOrignalKey );
	}

	if ( !NT_SUCCESS(status) )
	{
		ZwClose( *phRedirectedKey );
		*phRedirectedKey = NULL ;
		*pOrignalNode	 = NULL ;
		return status ;
	}

	return STATUS_SUCCESS ;
}


NTSTATUS 
Refresh_Orignal_RegNode (
	IN HANDLE hOrignalKey,
	IN PLARGE_INTEGER LastWriteTime,
	IN LPWSTR OrignalPath,
	OUT PVOID *out_RegNode_Orignal
	)
{
	NTSTATUS status = STATUS_SUCCESS ; 
	ULONG NameLength = 0 ;
	DWORD TickCount = GetTickCount();
	BOOL bNeedRemove = FALSE, bBuildNewNode = FALSE ;
	LPXREG_INFORMATION_CLASS pCurrentOrignalNode = NULL, pNextOrignalNode = NULL ; 

	if ( NULL == hOrignalKey || NULL == LastWriteTime || NULL == OrignalPath || NULL == out_RegNode_Orignal ) { return STATUS_INVALID_PARAMETER ; }

	NameLength = wcslen( OrignalPath ) * sizeof(WCHAR) ;
	pCurrentOrignalNode = (LPXREG_INFORMATION_CLASS) g_XREG_INFORMATION_HEAD_Orignal.Flink ;

	if ( NULL == pCurrentOrignalNode )
	{ 
		bBuildNewNode = TRUE ; 
	}
	else
	{
		// ͨ������ƥ��@hKey��������,�ҵ��򷵻ظý��,�����½�֮
		while ( TRUE )
		{
			bNeedRemove = FALSE ;
			pNextOrignalNode = (LPXREG_INFORMATION_CLASS) pCurrentOrignalNode->ListEntry.Flink ;

			if ( TickCount - pCurrentOrignalNode->TickCount > 30000 ) 
			{ 
				if ( (pCurrentOrignalNode->NameLength == NameLength) && (0 == wcsnicmp(pCurrentOrignalNode->wszOrignalRegPath, OrignalPath, NameLength / sizeof(WCHAR))) )
				{
					// �ҵ���������ƥ��Ľ��,��LastWriteTimeҲ��ͬ,���ظý��
					if ( LastWriteTime->LowPart == pCurrentOrignalNode->LastWriteTime.LowPart && LastWriteTime->HighPart == pCurrentOrignalNode->LastWriteTime.HighPart )
					{
						*out_RegNode_Orignal = (PVOID) pCurrentOrignalNode ;
						return STATUS_SUCCESS ;
					}
					
					// LastWriteTime��ͬ,�����½�����ǰ��pCurrentNode,������LastWriteTime,����@pCurrentNode �е�2��������!
					RemoveRegNodeComEx( (PVOID)pCurrentOrignalNode, FALSE );
					break ;
				}
			}
			else
			{
				bNeedRemove = TRUE ; // ������������н��TickCount�ĺϷ���,���ڷǷ����,�Ƴ���
			}

			if ( bNeedRemove ) { RemoveRegNodeCom( (PVOID)pCurrentOrignalNode, (PVOID)&g_XREG_INFORMATION_HEAD_Orignal ); }

			pCurrentOrignalNode = pNextOrignalNode ;
			if ( NULL == pCurrentOrignalNode ) { bBuildNewNode = TRUE ; break ; }
		}
	}

	// �½����
	if ( bBuildNewNode )
	{
		pCurrentOrignalNode = (LPXREG_INFORMATION_CLASS) kmalloc( NameLength + sizeof(XREG_INFORMATION_CLASS) );

		pCurrentOrignalNode->NameLength	= NameLength ;
		memcpy( pCurrentOrignalNode->wszOrignalRegPath, OrignalPath, NameLength + 2 );

		InsertListA( &g_XREG_INFORMATION_HEAD_Orignal, NULL, (PLIST_ENTRY)pCurrentOrignalNode );
	}

	// ˢ�½��
	pCurrentOrignalNode->LastWriteTime.LowPart	= LastWriteTime->LowPart  ;
	pCurrentOrignalNode->LastWriteTime.HighPart = LastWriteTime->HighPart ;

	// ���µ�ǰԭʼ��ֵ���,��Ҫ�����´洢�������Ӽ�(SubKeys) & ��ֵ(ValueKeys) ��Ϣ 
	status = BuildSubKeysLists( pCurrentOrignalNode, hOrignalKey );
	if ( status < 0 || (status = BuildValueKeysLists( pCurrentOrignalNode, hOrignalKey ), status < 0) )
	{
		RemoveEntryListEx( (LPLIST_ENTRY_EX)&g_XREG_INFORMATION_HEAD_Orignal, (PLIST_ENTRY)pCurrentOrignalNode );
		RemoveRegNodeComEx( (PVOID)pCurrentOrignalNode, TRUE );
	}
	else
	{
		*out_RegNode_Orignal = pCurrentOrignalNode ;
	}

	return status ;
}


NTSTATUS 
BuildSubKeysLists (
	IN LPXREG_INFORMATION_CLASS pCurrentOrignalNode,
	IN HANDLE hOrignalKey
	)
{
	NTSTATUS status = STATUS_SUCCESS ; 
	BOOL bInsertToHead = FALSE ; 
	ULONG ResultLength, Length = 0x80, NameLength, Index = 0 ;
	LPXREG_INFORMATION_SUBKEY NewNode = NULL, pNode = NULL, FuckedNode = NULL ;
	PKEY_NODE_INFORMATION KeyInformation = (PKEY_NODE_INFORMATION) kmalloc( 0x80 ) ;

	if ( NULL == pCurrentOrignalNode || NULL == hOrignalKey ) { return STATUS_INVALID_PARAMETER ; }

	while( TRUE )
	{
		// ����ԭʼ������ѯԭʼĿ¼ÿ���Ӽ�����Ϣ
		while ( TRUE )
		{
			status = g_NtEnumerateKey_addr( hOrignalKey, Index, KeyNodeInformation, KeyInformation, Length, &ResultLength );

			if ( STATUS_BUFFER_OVERFLOW != status ) { break ; }

			kfree( KeyInformation );
			Length += 0x80 ;
			KeyInformation = (PKEY_NODE_INFORMATION) kmalloc( Length );
		}

		if ( STATUS_NO_MORE_ENTRIES == status )
		{
			kfree( KeyInformation );
			return STATUS_SUCCESS ;
		}

		if ( ! NT_SUCCESS(status) ) { break ; }

		// ö�ٵ�ǰĿ¼�µ������Ӽ�,��ÿ���Ӽ�,�����½�����֮,�������
		bInsertToHead = FALSE ; 

		ResultLength = KeyInformation->NameLength + sizeof(XREG_INFORMATION_SUBKEY) + sizeof(WCHAR) ;
		NewNode = (LPXREG_INFORMATION_SUBKEY) kmalloc( ResultLength );

		NewNode->NameLength = NameLength = KeyInformation->NameLength ;

		memcpy( NewNode->wszSubKeyName, KeyInformation->Name, NameLength );
		NewNode->wszSubKeyName[ NameLength / sizeof(WCHAR) ] = UNICODE_NULL ;

		NewNode->LastWriteTime.LowPart	= KeyInformation->LastWriteTime.LowPart  ;
		NewNode->LastWriteTime.HighPart	= KeyInformation->LastWriteTime.HighPart ;
		NewNode->bClassName = (KeyInformation->TitleIndex || KeyInformation->ClassOffset != 0xFFFFFFFF || KeyInformation->ClassLength) ;

		FuckedNode = pCurrentOrignalNode->pSubKey_ShowTime_Lastone  ;
		pNode = pCurrentOrignalNode->pSubKey_ShowTime_Firstone ;

		if ( (FuckedNode && wcsicmp( FuckedNode->wszSubKeyName, NewNode->wszSubKeyName ) < 0) || (NULL == pNode) )
		{
			bInsertToHead = TRUE ; 
		}
		else
		{
			while ( wcsicmp( pNode->wszSubKeyName, NewNode->wszSubKeyName) <= 0 )
			{
				pNode = pNode->pFlink ;
				if ( NULL == pNode )
				{
					bInsertToHead = TRUE ; 
					break ;
				}
			}
		}

		if ( bInsertToHead )
		{
			InsertListA( (LPLIST_ENTRY_EX)&pCurrentOrignalNode->pSubKey_ShowTime_Firstone, NULL, (PLIST_ENTRY)NewNode );
		}
		else
		{
			InsertListB( (LPLIST_ENTRY_EX)&pCurrentOrignalNode->pSubKey_ShowTime_Firstone, (PLIST_ENTRY)pNode, (PLIST_ENTRY)NewNode );
		}

		++ Index ; // ����ö���¸��Ӽ�
	} // end-of-while

	kfree( KeyInformation );
	return status ;
}


NTSTATUS 
BuildValueKeysLists (
	IN LPXREG_INFORMATION_CLASS pCurrentOrignalNode,
	IN HANDLE hOrignalKey
	)
{
	BOOL bInsertToHead = FALSE ; 
	NTSTATUS status = STATUS_SUCCESS ; 
	LPXREG_INFORMATION_VALUEKEY NewNode = NULL, pNode = NULL ;
	ULONG ResultLength, Length  = 0x80, NameLength, Index = 0 ;
	PKEY_VALUE_FULL_INFORMATION ValueInfo = (PKEY_VALUE_FULL_INFORMATION) kmalloc( 0x80 ) ;

	if ( NULL == pCurrentOrignalNode || NULL == hOrignalKey ) { return STATUS_INVALID_PARAMETER ; }

	while( TRUE )
	{
		// 2.1 ����ԭʼ������ѯԭʼĿ¼ÿ����ֵ����Ϣ
		while ( TRUE )
		{
			status = g_NtEnumerateValueKey_addr( hOrignalKey, Index, KeyValueFullInformation, ValueInfo, Length, &ResultLength );

			if ( STATUS_BUFFER_OVERFLOW != status ) { break ; }

			kfree( ValueInfo );
			Length += 0x80 ;
			ValueInfo = (PKEY_VALUE_FULL_INFORMATION) kmalloc( Length );
		}

		if ( STATUS_NO_MORE_ENTRIES == status )
		{
			kfree( ValueInfo );
			return STATUS_SUCCESS ;
		}

		if ( ! NT_SUCCESS(status) ) { break ; }

		// ������ǰ����������ValueKey,Ϊÿ����ֵ�������,���뵽������
		bInsertToHead = FALSE ; 

		ResultLength = ValueInfo->NameLength + ValueInfo->DataLength + sizeof(XREG_INFORMATION_VALUEKEY) + sizeof(WCHAR) ;
		NewNode = (LPXREG_INFORMATION_VALUEKEY) kmalloc( ResultLength );

		NewNode->NameLength = NameLength = ValueInfo->NameLength ;

		memcpy( NewNode->wszValueKeyName, ValueInfo->Name, NameLength );
		NewNode->wszValueKeyName[ NameLength / sizeof(WCHAR) ] = UNICODE_NULL ;

		NewNode->ValueType = ValueInfo->Type ;
		NewNode->ValueData = (char *)&NewNode->wszValueKeyName[0] + ValueInfo->NameLength + 2 ;
		NewNode->ValueDataLength = ValueInfo->DataLength ;

		memcpy( NewNode->ValueData, (char *)ValueInfo + ValueInfo->DataOffset, ValueInfo->DataLength );

		// �ҵ������пɲ����,Insert֮
		pNode = pCurrentOrignalNode->pValueKey_ShowTime_Lastone ;
		if ( pNode )
		{
			while ( wcsicmp( pNode->wszValueKeyName, NewNode->wszValueKeyName ) <= 0 )
			{
				pNode = pNode->pFlink ;
				if ( NULL == pNode )
				{
					bInsertToHead = TRUE ; 
					break ;
				}
			}
		}
		else
		{
			bInsertToHead = TRUE ; 
		}

		if ( bInsertToHead )
		{
			InsertListA( (LPLIST_ENTRY_EX)&pCurrentOrignalNode->pValueKey_ShowTime_Lastone, NULL, (PLIST_ENTRY)NewNode );
		}
		else
		{
			InsertListB( (LPLIST_ENTRY_EX)&pCurrentOrignalNode->pValueKey_ShowTime_Lastone, (PLIST_ENTRY)pNode, (PLIST_ENTRY)NewNode );
		}

		++ Index ; // ����ö���¸��Ӽ�
	} // end-of-while

	kfree( ValueInfo );
	return status ;
}


BOOL 
IsDirtyVauleKey (
	IN PVOID KeyValueInformation,
	IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass
	)
{
	BOOL bRet ;

	switch ( KeyValueInformationClass )
	{
	case KeyValueBasicInformation :
	case KeyValueFullInformation :
	case KeyValuePartialInformation :
		bRet = *((DWORD *)KeyValueInformation + 1) == _DirtyValueKeyTag_ ;
		break ;

	case KeyValueFullInformationAlign64:
		bRet = _DirtyValueKeyTag_ == 0 ;
		break ;

	case KeyValuePartialInformationAlign64:
		bRet = *(DWORD *)KeyValueInformation == _DirtyValueKeyTag_ ;
		break ;
	
	default :
		break ;
	}

	return bRet ;
}


NTSTATUS 
FillValueKeyInfo (
	IN LPXREG_INFORMATION_VALUEKEY pNode ,
	IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass ,
	OUT PVOID KeyValueInformation,
	IN ULONG Length ,
	OUT PULONG ResultLength
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/10/22 [22:10:2010 - 15:07]

Routine Description:
  ����KeyValueInformationClassȷ���ṹ������,��@pNode�еĲ������������@KeyValueInformation   
    
Arguments:
  pNode - ������ǰValueKey������������Ϣ(ɳ���Զ�����)
  KeyValueInformationClass - ö������KEY_VALUE_INFORMATION_CLASS�е�һ��
  KeyValueInformation - ���������ValueKey��Ϣ
    
--*/
{
	NTSTATUS status = STATUS_SUCCESS ; 
	ULONG StructSize = 0, TotalLength = 0 ;

	if ( NULL == pNode || NULL == KeyValueInformation || NULL == ResultLength ) { return STATUS_INVALID_PARAMETER ; }

	// 1. ����_KEY_VALUE_INFORMATION����ȷ���ṹ��Ĵ�С
	switch ( KeyValueInformationClass )
	{
	case KeyValueBasicInformation :
		StructSize = 0xC ;
		TotalLength = pNode->NameLength + StructSize ;
		break ;

	case KeyValueFullInformation :
		StructSize = 0x14 ;
		TotalLength = pNode->NameLength + pNode->ValueDataLength + StructSize ;
		break ;

	case KeyValuePartialInformation :
		StructSize = 0xC ;
		TotalLength = pNode->ValueDataLength + StructSize ;
		break ;

	case KeyValuePartialInformationAlign64:
		StructSize = 8 ;
		TotalLength = pNode->ValueDataLength + StructSize ;
		break ;

	default :
		return STATUS_INVALID_PARAMETER ;
	}

	*ResultLength = TotalLength ;
	if ( Length < StructSize ) { return STATUS_BUFFER_TOO_SMALL ; }

	// 2. �����µĴ����ṹ����ڴ�
	ULONG NameLength = 0 ;
	LPWSTR wszSrc = NULL, wszDest = NULL ;
	LPxx_KEY_VALUE_INFORMATION ptr = (LPxx_KEY_VALUE_INFORMATION) kmalloc( TotalLength );

	if ( KeyValueBasicInformation == KeyValueInformationClass )
	{
		ptr->BasicInfo.Type			= pNode->ValueType  ;
		ptr->BasicInfo.NameLength	= pNode->NameLength ;

		memcpy( ptr->BasicInfo.Name, pNode->wszValueKeyName, pNode->NameLength );
	}
	else if ( KeyValueFullInformation == KeyValueInformationClass )
	{
		ptr->FullInfo.Type		 = pNode->ValueType ;              
		ptr->FullInfo.DataOffset = pNode->NameLength + StructSize ; 
		ptr->FullInfo.DataLength = pNode->ValueDataLength ;  
		ptr->FullInfo.NameLength = pNode->NameLength ;

		memcpy( ptr->FullInfo.Name, pNode->wszValueKeyName, pNode->NameLength );
		memcpy( (LPWSTR)((char *)ptr + ptr->FullInfo.DataOffset), pNode->ValueData, pNode->ValueDataLength );
	}
	else if ( KeyValuePartialInformation == KeyValueInformationClass )
	{
		ptr->PartialInfo.Type		= pNode->ValueType ;
		ptr->PartialInfo.DataLength = pNode->ValueDataLength ;

		memcpy( ptr->PartialInfo.Data, pNode->ValueData, pNode->ValueDataLength );
	}
	else if ( KeyValuePartialInformationAlign64 == KeyValueInformationClass )
	{
		ptr->PartialInfoAlign64.Type		= pNode->ValueType ;
		ptr->PartialInfoAlign64.DataLength  = pNode->ValueDataLength ;

		memcpy( ptr->PartialInfoAlign64.Data, pNode->ValueData, pNode->ValueDataLength );
	}

	// 3. ������ʱ���ݵ������� @KeyValueInformation
	if ( Length < TotalLength )
	{
		status = STATUS_BUFFER_OVERFLOW ;
	}
	else
	{
		Length = TotalLength ;
		status = STATUS_SUCCESS ;
	}

	memcpy( KeyValueInformation, ptr, Length );

	if ( ptr ) { kfree(ptr); }
	return status ;
}


NTSTATUS 
FillSubKeyInfo (
	IN ULONG RegNameLength,
	IN LPWSTR wszRegFullPath,
	IN KEY_INFORMATION_CLASS KeyInformationClass ,
	OUT PVOID KeyInformation,
	IN ULONG Length ,
	OUT PULONG ResultLength,
	IN PLARGE_INTEGER LastWriteTime
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/10/25 [25:10:2010 - 17:35]

Routine Description:
  ����@KeyInformationClass�������ṹ��@KeyInformation,����@LastWriteTime & �Ӽ�����

--*/
{
	NTSTATUS status = STATUS_SUCCESS ; 
	LPKEYINFO KeyInfo = (LPKEYINFO) KeyInformation ;
	LPWSTR shortName_ptr = NULL, Name = NULL ;
	ULONG TotalSize = 0, StructSize = 0, size = 0, *NameLength = 0 ;

	if ( KeyFullInformation != KeyInformationClass )
	{
		shortName_ptr = wcsrchr( wszRegFullPath, '\\' ) + 1 ;
		TotalSize = (RegNameLength - ((ULONG)((char *)shortName_ptr - (char *)wszRegFullPath) / sizeof(WCHAR))) * sizeof(WCHAR) ;
	}

	switch ( KeyInformationClass )
	{
	case KeyBasicInformation :
		StructSize = 0x18 ;
		NameLength = &KeyInfo->u.Basic.NameLength ;
		Name = KeyInfo->u.Basic.Name ;
		break ;

	case KeyNodeInformation :
		StructSize = 0x20 ;
		NameLength = &KeyInfo->u.Node.NameLength ;
		Name = KeyInfo->u.Node.Name ;
		break ;

	case KeyFullInformation :
		StructSize = 0x30 ;
		NameLength = 0 ;
		Name = NULL ;
		break ;
	
	default :
		break ;
	}

	*ResultLength = StructSize + TotalSize ;
	if ( Length < StructSize ) { return STATUS_BUFFER_TOO_SMALL; }

	memset( KeyInformation, 0, StructSize );

	KeyInfo->u.LastWriteTime.LowPart  = LastWriteTime->LowPart  ;
	KeyInfo->u.LastWriteTime.HighPart = LastWriteTime->HighPart ;

	if ( KeyNodeInformation == KeyInformationClass )
	{
		KeyInfo->u.Node.ClassOffset = 0xFFFFFFFF ;
	}

	if ( KeyFullInformation != KeyInformationClass )
	{
		size = Length - StructSize ;

		if ( size < TotalSize )
		{
			status = STATUS_BUFFER_OVERFLOW ;
		}
		else
		{
			size = TotalSize ;
		}

		memcpy( Name, shortName_ptr, size );
		*(ULONG *)NameLength = TotalSize ;
	}

	return status ;
}


///////////////////////////////   END OF FILE   ///////////////////////////////