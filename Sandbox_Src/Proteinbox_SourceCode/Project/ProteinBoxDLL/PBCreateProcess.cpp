/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2011/06/07 [7:6:2011 - 16:23]
* MODULE : e:\Data\Work\Program\Coding\ɳ��\SandBox\Code\Project\ProteinBoxDLL\PBCreateProcess.cpp
* 
* Description:
*
*   
*                        
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "stdafx.h"
#include "Common.h"
#include "HookHelper.h"
#include "Exportfunc.h"
#include "ProteinBoxDLL.h"
#include "MemoryManager.h"
#include "PBToken.h"
#include "PBLoadData.h"
#include "PBFilesData.h"
#include "PBCreateProcess.h"

#pragma warning(disable: 4995)

//////////////////////////////////////////////////////////////////////////

_CreateProcessA_ g_CreateProcessA_addr = NULL ;
_CreateProcessW_ g_CreateProcessW_addr = NULL ;
_CreateProcessAsUserA_ g_CreateProcessAsUserA_addr = NULL ;
_CreateProcessAsUserW_ g_CreateProcessAsUserW_addr = NULL ;
_RtlCreateProcessParameters_	g_RtlCreateProcessParameters_addr	= NULL ;
_RtlCreateProcessParametersEx_	g_RtlCreateProcessParametersEx_addr = NULL ;
_ExitProcess_ g_ExitProcess_addr = NULL ;
_WinExec_ g_WinExec_addr = NULL ;



static const LPWSTR g_MailName_Arrays[ ] = 
{ 
	L"thunderbird.exe",
	L"msimn.exe",
	L"outlook.exe",
	L"winmail.exe",
	L"wlmail.exe",
	L"IncMail.exe",
	L"eudora.exe",
	L"thebat.exe",
};



/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--



BOOL Hook_CreateProcessAsUser( IN HMODULE hModule )
{
	BOOL bRet = FALSE ;

	g_CreateProcessAsUserA_addr = (_CreateProcessAsUserA_) GetProcAddress( hModule, "CreateProcessAsUserA"   );
	g_CreateProcessAsUserW_addr = (_CreateProcessAsUserW_) GetProcAddress( hModule, "CreateProcessAsUserW" );

	bRet = Mhook_SetHook( (PVOID*)&g_CreateProcessAsUserA_addr, fake_CreateProcessAsUserA );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_CreateProcessAsUser() - Mhook_SetHook(); | \"CreateProcessAsUserA\" \n" );
		return FALSE ;
	}

	bRet = Mhook_SetHook( (PVOID*)&g_CreateProcessAsUserW_addr, fake_CreateProcessAsUserW );
	if ( FALSE == bRet )
	{
		MYTRACE( L"error! | Hook_CreateProcessAsUser() - Mhook_SetHook(); | \"CreateProcessAsUserW\" \n" );
		return FALSE ;
	}

	return TRUE;
}



NTSTATUS 
RtlCreateProcessParameters_Filter (
	PVOID *ProcessParameters,
	PUNICODE_STRING ImagePathName, 
	PUNICODE_STRING DllPath, 
	PUNICODE_STRING CurrentDirectory, 
	PUNICODE_STRING CommandLine, 
	PVOID Environment, 
	PUNICODE_STRING WindowTitle, 
	PUNICODE_STRING DesktopInfo, 
	PUNICODE_STRING ShellInfo, 
	PUNICODE_STRING RuntimeData, 
	int a11, 
	BOOL bIsCallEx
	)
{
	NTSTATUS status = STATUS_SUCCESS ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	if ( 0 == wcsicmp(ImagePathName->Buffer, g_szPath_ntvdm) )
	{
		pNode->sRCP.Flag = 9 ;
	}

	if ( pNode->sRCP.Flag )
	{
		if ( 9 != pNode->sRCP.Flag && pNode->sRCP.DllPath->uniBuffer.Length )
		{
			DllPath = &pNode->sRCP.DllPath->uniBuffer ;
		}

		if ( bIsCallEx )
		{
			status = g_RtlCreateProcessParametersEx_addr( ProcessParameters, ImagePathName, DllPath, CurrentDirectory, CommandLine,
						Environment, WindowTitle, DesktopInfo, ShellInfo, RuntimeData, a11 );
		}
		else
		{
			status = g_RtlCreateProcessParameters_addr( ProcessParameters, ImagePathName, DllPath, CurrentDirectory, CommandLine,
						Environment, WindowTitle, DesktopInfo, ShellInfo, RuntimeData );
		}
	} 
	else
	{
		FixupProcessNodeL( &pNode->sRCP.ImagePathName, ImagePathName );
		FixupProcessNodeL( &pNode->sRCP.DllPath, DllPath );
		FixupProcessNodeL( &pNode->sRCP.CurrentDirectory, CurrentDirectory );
		FixupProcessNodeL( &pNode->sRCP.CommandLine, CommandLine );
		FixupProcessNodeEnv( &pNode->sRCP.Environment, Environment );

		pNode->sRCP.bIsDirectory = 1;
		++ pNode->sRCP.Flag ;

		status = STATUS_BAD_INITIAL_PC;
	}

	return status;
}



VOID FixupProcessNode( PVOID _pNode )
{
	BOOL bNeedFree = FALSE ;
	UNICODE_STRING uniBuffer ;
	LPWSTR lpDllPath = NULL, lpEnvironment = NULL, lpEnvironmentDummy = NULL ;
	LPWSTR lpImagePathName = NULL, szCurrentDirectory = NULL, lpCurrentDirectory = NULL ;
	LPUNICODE_STRING_EX pImagePathName = NULL, pCurrentDirectory = NULL, DllPath = NULL, Environment = NULL ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) _pNode ;

	if ( NULL == pNode ) { return; }

	// redirect procname
	pImagePathName = pNode->sRCP.ImagePathName ;
	if ( pImagePathName && pImagePathName->uniBuffer.Length )
	{
		lpImagePathName = GetProcFullPathL( pImagePathName->uniBuffer.Buffer, TRUE );
		if ( lpImagePathName )
		{
			RtlInitUnicodeString( &uniBuffer, lpImagePathName );
			FixupProcessNodeL( &pNode->sRCP.ImagePathName, &uniBuffer );

			kfree(lpImagePathName);
		}
	}
	else
	{
		RtlInitUnicodeString( &uniBuffer, NULL );
		FixupProcessNodeL( &pNode->sRCP.ImagePathName, &uniBuffer );
	}

	// redirect directory
	pCurrentDirectory = pNode->sRCP.CurrentDirectory ;
	if ( pCurrentDirectory && pCurrentDirectory->uniBuffer.Length )
	{
		bNeedFree = FALSE ;
		szCurrentDirectory = pCurrentDirectory->uniBuffer.Buffer ;
	}
	else
	{
		ULONG length = GetCurrentDirectoryW( 0, NULL );
		szCurrentDirectory = (LPWSTR) kmalloc( 2 * length + 0x10 );
		
		GetCurrentDirectoryW( length + 8, szCurrentDirectory );
		bNeedFree = TRUE ;
	}

	lpCurrentDirectory = GetProcFullPathL( szCurrentDirectory, TRUE );
	if ( lpCurrentDirectory )
	{
		RtlInitUnicodeString( &uniBuffer, lpCurrentDirectory );
		FixupProcessNodeL( &pNode->sRCP.CurrentDirectory, &uniBuffer );
		kfree( lpCurrentDirectory );
	}
	else
	{
		if ( 0 == pCurrentDirectory->uniBuffer.Length )
		{
			pNode->sRCP.bIsDirectory = 0 ;
		}

		RtlInitUnicodeString( &uniBuffer, szCurrentDirectory );
		FixupProcessNodeL( &pNode->sRCP.CurrentDirectory, &uniBuffer );
	}

	if ( bNeedFree ) { kfree( szCurrentDirectory ); }

	// redirect search path
	DllPath = pNode->sRCP.DllPath ;
	if ( DllPath && DllPath->uniBuffer.Length )
	{
		lpDllPath = GetRedirectedSearchPath( pNode, DllPath->uniBuffer.Buffer, FALSE );
		if ( lpDllPath )
		{
			RtlInitUnicodeString( &uniBuffer, lpDllPath );
			FixupProcessNodeL( &pNode->sRCP.DllPath, &uniBuffer );
			kfree( lpDllPath );
		}
	}
	else
	{
		RtlInitUnicodeString( &uniBuffer, NULL );
		FixupProcessNodeL( &pNode->sRCP.DllPath, &uniBuffer );
	}
	
	// redirect enviroment
	Environment = pNode->sRCP.Environment ;
	if ( Environment && Environment->uniBuffer.Length )
	{
		lpEnvironment = Environment->uniBuffer.Buffer;
	}
	else
	{
		lpEnvironment = GetEnvironmentStringsW();
	}

	lpEnvironmentDummy = SetSBEnv( pNode, lpEnvironment );
	if ( lpEnvironmentDummy )
	{
		// "00000000_SBIE_ALL_USERS=\Device\HarddiskVolume1\Documents and Settings\All Users"
		FixupProcessNodeEnv( &pNode->sRCP.Environment, lpEnvironmentDummy );
		kfree( lpEnvironmentDummy );
	}

	return;
}



VOID 
FixupProcessNodeL (
	OUT LPUNICODE_STRING_EX* _pNode,
	IN PUNICODE_STRING puniBuffer 
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2011/06/20 [20:6:2011 - 11:26]

Routine Description:
  ��@puniBuffer�����ݿ����������ӽṹ����(������ǰ���̶�Ӧ���ļ�ȫ·������Ϣ) 
    
Arguments:
  pNode - �����Ľṹ��ָ��
  puniBuffer - Ҫ��������

--*/
{
	ULONG Length = 0 ;
	LPWSTR szBuffer = NULL ;
	LPUNICODE_STRING_EX pNode = *_pNode ;

	if ( puniBuffer && puniBuffer->Length && puniBuffer->Buffer )
	{
		Length = puniBuffer->Length & 0xFFFE ;
		szBuffer = puniBuffer->Buffer ;
	}

	// �����ȳ���,�ͷŵ��ɵĽṹ��,���������ڴ�
	if ( pNode && pNode->uniBuffer.MaximumLength < Length )
	{
		kfree( pNode );
		pNode = NULL ;
	}

	if ( NULL == pNode )
	{
		pNode = (LPUNICODE_STRING_EX) kmalloc( Length + 0x10 );
		pNode->uniBuffer.Buffer = pNode->Data ;
		pNode->uniBuffer.MaximumLength = (USHORT)(Length + 4 );
		
		*_pNode = pNode ;
	}

	pNode->uniBuffer.Length = (USHORT)Length ;

	if ( Length ) { memcpy( pNode->uniBuffer.Buffer, szBuffer, Length ); }
	pNode->uniBuffer.Buffer[Length / sizeof(WCHAR) ] = 0;

	return;
}



VOID
FixupProcessNodeEnv (
	OUT LPUNICODE_STRING_EX* _pNode, 
	IN PVOID lpBuffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2012/03/08 [8:3:2012 - 15:08]

Routine Description:
  ��ʹ *@_pNode �� @lpBuffer Ϊ�գ�ҲҪΪ������ڴ�
    
--*/
{
	WORD *ptr = NULL ;
	ULONG Length = 0 ;
	LPUNICODE_STRING_EX pNode = *_pNode ;

	if ( lpBuffer )
	{
		ptr = (WORD *)lpBuffer ;
		while ( *ptr || ptr[1] )
		{
			++ ptr ;
			++ Length ;
		}

		Length = (Length + 2) * sizeof(WCHAR) ;
	}

	if ( pNode && pNode->uniBuffer.MaximumLength < Length )
	{
		kfree( pNode );
		pNode = NULL ;
	}

	if ( NULL == pNode )
	{
		pNode = (LPUNICODE_STRING_EX) kmalloc( Length + 0x10 );
		pNode->uniBuffer.Buffer = pNode->Data ;
		pNode->uniBuffer.MaximumLength = Length;

		*_pNode = pNode ;
	}

	pNode->uniBuffer.Length = (USHORT)Length ;

	if ( Length )
	{
		memcpy( pNode->uniBuffer.Buffer, lpBuffer, Length ); 
	}
	else
	{
		pNode->uniBuffer.Buffer[0] = 0;
		pNode->uniBuffer.Buffer[1] = 0;
	}

	return;
}



LPWSTR 
GetProcFullPathL( 
	IN LPCWSTR lpFileName, 
	IN BOOLEAN bFlag_TranslateNtToDosPath 
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2011/06/22 [22:6:2011 - 11:29]

Routine Description:
  �ú���ò��Ϲ����,����Ŀ����У�����ȫ·���Ƿ�Ϸ�. ����˵��,��������:
  ����ɳ���еĳ��򴴽����ӽ���@lpFileName ="C:\Program Files\Mozilla Thunderbird\thunderbird.exe",�ú�������
  SbieDll_GetHandlePath()����ת��Ϊ"\Device\HarddiskVolume1\Program Files\Mozilla Thunderbird\thunderbird.exe",
  ��@bFlag_TranslateNtToDosPathΪ1�����������SbieDll_TranslateNtToDosPath()��������ַ�����ת����ȥ,���
  "C:\Program Files\Mozilla Thunderbird\thunderbird.exe".
    
Arguments:
  lpFileName -
  bFlag_TranslateNtToDosPath  -

Return Value:
  �ɹ��򷵻�ת������ַ��� | ʧ���򷵻� 0
    
--*/
{
	HANDLE hFile = NULL ; 
	LPWSTR szPath = NULL ;
	NTSTATUS status = STATUS_SUCCESS ;
	BOOL bIsHandlerSelfFilePath = FALSE ;

	hFile = CreateFileW( lpFileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0 );
	if ( hFile == (HANDLE)INVALID_HANDLE_VALUE ) { return NULL; }

	szPath = (LPWSTR) kmalloc( 0x4000 );
	status = PB_GetHandlePath( hFile, szPath, &bIsHandlerSelfFilePath );
	CloseHandle( hFile );

	if ( ! NT_SUCCESS(status) ) { kfree(szPath); return NULL; }

	if ( bFlag_TranslateNtToDosPath )
	{
		if ( FALSE == PB_TranslateNtToDosPath(szPath) )
		{
			kfree( szPath );
			return NULL;
		}
	}
	else
	{
		if ( FALSE == bIsHandlerSelfFilePath )
		{
			kfree( szPath );
			return NULL;
		}
	}

	return szPath;
}



LPWSTR GetProcFullPathK( LPWSTR lpFileName )
/*++

Author: sudami [sudami@163.com]
Time  : 2011/06/22 [22:6:2011 - 18:23]

Routine Description:
  �ú���ò��Ϲ����,����Ŀ����У�����ȫ·���Ƿ�Ϸ�. ����˵��,��������:
  ����ɳ���еĳ��򴴽����ӽ���@lpFileName ="C:\Program Files\Mozilla Thunderbird\thunderbird.exe",�ú�������
  PB_GetHandlePath()����ת��Ϊ"\Device\HarddiskVolume1\Program Files\Mozilla Thunderbird\thunderbird.exe",
  ��������PB_TranslateNtToDosPath()��������ַ�����ת����ȥ,���"C:\Program Files\Mozilla Thunderbird\thunderbird.exe".
    
Arguments:
  lpFileName - [IN]

Return Value:
  �ɹ��򷵻�ת������ַ��� | ʧ���򷵻� 0

--*/
{
	HANDLE hFile = NULL ; 
	LPWSTR szPath = NULL ;
	NTSTATUS status = STATUS_SUCCESS ;
	BOOL bIsHandlerSelfFilePath = FALSE ;

	hFile = CreateFileW( lpFileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0 );
	if ( hFile == (HANDLE)INVALID_HANDLE_VALUE ) { return NULL; }

	szPath = (LPWSTR) kmalloc( 0x4000 );
	status = PB_GetHandlePath( hFile, szPath, NULL );
	CloseHandle( hFile );

	if ( ! NT_SUCCESS(status) ) { kfree(szPath); return NULL; }

	if ( FALSE == PB_TranslateNtToDosPath(szPath) )
	{
		kfree( szPath );
		return NULL;
	}

	return szPath;
}


LPWSTR
GetRedirectedSearchPath (
	PVOID _pNode, 
	LPWSTR lpBuffer,
	BOOL bFlag
	)
{
	int Array[ 0x400 ] = { 0 } ;
	ULONG length = 0, TotalLength = 0 ;
	LPWSTR lpImagePathName = NULL, ptr = NULL, pBufferTmp = NULL ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) _pNode ;

	if ( NULL == pNode ) { return NULL; }

	// 1.
	RtlZeroMemory( Array, 0x400 );
	if ( bFlag )
	{
		TotalLength = 4 + GetRedirectedSearchPathEx( Array, pNode->sRCP.CurrentDirectory->uniBuffer.Buffer );

		lpImagePathName = pNode->sRCP.ImagePathName->uniBuffer.Buffer ;
		if ( lpImagePathName )
		{
			ptr = wcsrchr( lpImagePathName, '\\' );
			if ( ptr )
			{
				*ptr = 0;
				TotalLength = TotalLength + 4 + GetRedirectedSearchPathEx( Array, lpImagePathName );
				*ptr = '\\';
			}
		}
	}

	//  
	// 2. �������ÿ���ַ���,��ʽ����:"A; B; C; ... ;"
	//    ����A�ַ����õ���Ӧ���ض���ֵA+, �� A �� A+ ���뵽pArray������,Ȼ���ٲ���B�ַ���...
	//

	while ( *lpBuffer )
	{
		ptr = wcschr( lpBuffer, ';' );
		if ( ptr )
		{
			length = (ULONG)((char *)ptr - (char *)lpBuffer) / sizeof(WCHAR) ;
			++ ptr ;
		}
		else
		{
			length = wcslen( lpBuffer );
			ptr = &lpBuffer[ length ];
		}

		if ( *lpBuffer == '\"' )
		{
			++ lpBuffer ;
			length -= 2 ;
		}

		if ( length )
		{
			pBufferTmp = (LPWSTR) kmalloc( 2 * length + 2 );
			memcpy( pBufferTmp, lpBuffer, 2 * length );
			pBufferTmp[ length ] = 0;

			TotalLength = TotalLength +  4 + GetRedirectedSearchPathEx( Array, pBufferTmp );
			kfree( pBufferTmp );
		}

		lpBuffer = ptr;
	}

	// 3. ��ȫ�����ض����ַ��������,�����һ��,����֮
	ptr = pBufferTmp = (LPWSTR) kmalloc( 2 * TotalLength );
	for ( int i = 0; Array[i]; ++i )
	{
		if ( i )
		{
			*ptr = ';' ;
			++ ptr ;
		}

		wcscpy( ptr, (LPWSTR)Array[i] );
		ptr += wcslen(ptr);
		
		kfree( (PVOID)Array[i] );
	}

	*ptr = 0;
	return pBufferTmp;	
}


ULONG 
GetRedirectedSearchPathEx (
	int* pArray, 
	LPWSTR lpFileName
	)
{
	int i = 0;
	ULONG size1 = 0, size2 = 0, TotalLength = 0;
	BOOL bOrignal = FALSE, bRedirected = FALSE;
	LPWSTR szPathOrig = NULL, szPathRedirected = NULL, ptr = NULL ;

	// 1. �õ�ԭʼȫ·��
	szPathOrig = GetProcFullPathK(lpFileName);    // eg: pPath_Orignal = "C:\Program Files\Mozilla Thunderbird"
	if ( PB_IsWow64() )
	{
		ptr = wcsrchr( szPathOrig, '\\' );
		if ( ptr && 0 == wcsicmp(ptr + 1, L"SysWOW64") )
		{
			wcscpy( (LPWSTR)(ptr + 1), L"system32" );
		}
	}

	// 2. �õ��ض�����ȫ·��
	szPathRedirected = GetProcFullPathL(lpFileName, TRUE);// eg: pPath_Redirected = "C:\Sandbox\AV\DefaultBox\drive\C\Program Files\Mozilla Thunderbird"
	if ( szPathOrig )
		bOrignal = TRUE;
	else
		bOrignal = FALSE;

	if ( szPathRedirected )
		bRedirected = TRUE;
	else
		bRedirected = FALSE;

	// 3. �Ƚ� "ԭʼֵ" �� "�ض���ֵ"�Ƿ����
	if ( szPathOrig && szPathRedirected && 0 == wcsicmp(szPathOrig, szPathRedirected) ) { bRedirected = FALSE; }

	// 4. �������в���
	for ( i = 0; ; ++i )
	{
		if ( pArray[i] )
		{
			if ( (FALSE == bOrignal) || wcsicmp(szPathOrig, (LPWSTR)pArray[i]) || (bOrignal = FALSE, bRedirected) )
			{
				if ( FALSE == bRedirected )
					continue;

				if ( wcsicmp(szPathRedirected, (LPWSTR)pArray[i]) )
					continue;

				bRedirected = FALSE;

				if ( bOrignal )
					continue;
			}
		}

		break;
	}

	if ( bRedirected )
	{
		size1 = 2 * wcslen(szPathRedirected) + 2;
		pArray[i] = (int) kmalloc(size1);
		memcpy( (LPWSTR)pArray[i++], szPathRedirected, size1 );
		TotalLength = size1;
	}

	if ( bOrignal )
	{
		size2 = 2 * wcslen(szPathOrig) + 2;
		pArray[i] = (int) kmalloc(size2);
		memcpy( (LPWSTR)pArray[i], szPathOrig, size2 );
		TotalLength += size2;
	}

	if ( szPathOrig && szPathOrig != lpFileName ) { kfree(szPathOrig); }
	if ( szPathRedirected && szPathRedirected != lpFileName ) { kfree(szPathRedirected); }

	return TotalLength ;
}


LPWSTR 
SetSBEnv (
	PVOID _pNode, 
	LPWSTR lpEnvironment 
	)
{
	ULONG size1 = 0, size2 = 0, Length = 0 ;
	LPWSTR ptr = NULL, pBuffer = NULL, pOutBuffer = NULL, pIn = NULL, pout = NULL ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) _pNode ;

	if ( NULL == pNode ) { return NULL; }

	ptr = lpEnvironment;
	while ( *ptr )
	{
		size1 = wcslen(ptr) + 1;
		if ( 0 == wcsnicmp(ptr, L"PATH=", 5) )
		{
			pBuffer = GetRedirectedSearchPath( pNode, ptr + 5, TRUE );
		}

		ptr += size1;
		Length += size1;
	}

	if ( pBuffer ) { Length += wcslen(pBuffer) + 1; }
	Length = 2 * Length + 0x10 ;
	pOutBuffer = SetSBEnvEx( Length, &pout );

	for ( ptr = lpEnvironment; *ptr; ptr += size1 )
	{
		size2 = wcslen(ptr);
		size1 = size2 + 1;
		pIn = ptr;

		memcpy( pout, ptr, 2 * (size1+1) );

		if ( wcsnicmp(pout, L"PATH=", 5) || NULL == pBuffer )
		{
			pout += size1;
		}
		else
		{
			wcscpy( (LPWSTR)(pout + 5), pBuffer );
			pout += wcslen(pout) + 1;
		}
	}

	*pout = 0;
	if ( pBuffer ) { kfree((PVOID)pBuffer); }

	return pOutBuffer;
}



LPWSTR 
SetSBEnvEx (
	int Length, 
	LPWSTR *pOutBuffer
	)
{
	ULONG TotalLength = 0 ;
	LPWSTR pBuffer = NULL, ptr1 = NULL, ptr2 = NULL, ptr3 = NULL, ptr4 = NULL ;

	TotalLength = Length + 8 + 
		( wcslen(L"00000000_SBIE_CURRENT_USER") 
		+ wcslen(L"00000000_SBIE_ALL_USERS") 
		+ g_lpszEnv_00000000_SBIE_ALL_USERS_Length 
		+ g_lpszEnv_00000000_SBIE_CURRENT_USER_Length
		) * sizeof(WCHAR) ;

	pBuffer = (LPWSTR) kmalloc( TotalLength );
	wcscpy( pBuffer, L"00000000_SBIE_ALL_USERS" );
	ptr1 = &pBuffer[ wcslen(pBuffer) ];
	*ptr1 = '=' ;
	wcscpy( ptr1 + 1, g_lpszEnv_00000000_SBIE_ALL_USERS );

	ptr2 = &ptr1[ wcslen(ptr1) ];
	*ptr2 = 0;
	++ ptr2;
	wcscpy( ptr2, L"00000000_SBIE_CURRENT_USER" );

	ptr3 = &ptr2[ wcslen(ptr2) ];
	*ptr3 = '=';
	wcscpy( ptr3 + 1, g_lpszEnv_00000000_SBIE_CURRENT_USER );
	
	ptr4 = &ptr3[ wcslen(ptr3) ];
	*ptr4 = 0;
	*pOutBuffer = ptr4 + 1;

	return pBuffer ;
}



BOOL IsExeNeedRedirect( LPWSTR lpApplicationName )
{
	BOOL bIs_outlook_etc = FALSE ;
	LPWSTR lpApplicationNameDummy = NULL, lpShortName = NULL ;
	
	lpApplicationNameDummy = GetProcFullPathL( lpApplicationName, FALSE );
	if ( lpApplicationNameDummy )
	{
		kfree( lpApplicationNameDummy );
		return TRUE;
	}

	// 2.0 ��ȡ���̶��� eg:����ȫ·��Ϊ"c:\windows\system32\test.exe",������ȡ��"test.exe"
	lpShortName = wcsrchr( lpApplicationName, '\\' );
	if ( lpShortName )
	{
		++ lpShortName ;
	}
	else
	{
		lpShortName = lpApplicationName;
	}

	// 2.1 �жϽ��̶����Ƿ�Ϊ��֪�ʼ���
	for (int Index = 0; Index < ARRAYSIZEOF(g_MailName_Arrays); Index++ )
	{
		if ( 0 == wcsicmp( lpShortName, g_MailName_Arrays[Index] ) ) 
		{
			bIs_outlook_etc = TRUE;
			break ;
		}
	}

	//  
	// 2.2 ����δ֪���ʼ�����,�û���Ҫ�Լ���ӵ������ļ���,��֤������.
	// �������ѯ�����ļ�,�ҵ�"mailto"����ʼ�������
	//
	if ( FALSE == bIs_outlook_etc )
	{
		LPWSTR pUnknowMailProc = NULL, pUnknowMailProc_shortName = NULL ;
		
		pUnknowMailProc = PB_AssocQueryProgram( L"mailto" );
		if ( pUnknowMailProc )
		{
			pUnknowMailProc_shortName = wcsrchr( pUnknowMailProc, '\\' );
			if ( pUnknowMailProc_shortName )
				++pUnknowMailProc_shortName;
			else
				pUnknowMailProc_shortName = pUnknowMailProc;

			if ( 0 == wcsicmp(lpShortName, pUnknowMailProc_shortName) )
				bIs_outlook_etc = TRUE;

			kfree( pUnknowMailProc );
		}
	}

	if ( 0 == wcsicmp(lpShortName, L"rundll32.exe") )
		bIs_outlook_etc = FALSE;
	
	// 2.3 ����ǰ���̲����ʼ���صĽ���,ֱ�ӷ���TRUE����Ҫ�ض���; �����ʼ���س���,�����һ���ж�
	if ( FALSE == bIs_outlook_etc ) { return TRUE; }

	// 2.4.��ʹ���ʼ�����Ľ���,Ҳ����ֱ�ӷ���.���ò�ѯ�����ļ���Ϣ.�������Ƿ���Ҫ�ض���
	int nIndex = 0 ;
	BOOL bNeedToRedirect = FALSE ;

	//
	// ������Ҫ��ѯ�����ļ�,����PB_QueryConf����; �����ȼ�һ�¶�,���ڵ���ʱ���޲�  - -|
	//

	// 2.4.1 ��ѯ�����ļ���Ϣ: "OpenFilePath";������ƥ���,˵��������Ҫ�ض���

	// 2.4.2 ��ѯ�����ļ���Ϣ: "OpenPipePath";������ƥ���,˵��������Ҫ�ض���

	// OVER
	return bNeedToRedirect ;
}


BOOL 
Call_Orignal_CreateProcessAsUser (
	BOOL bIsUnicode,
	HANDLE hToken, 
	PVOID lpApplicationName,
	PVOID lpCommandLine, 
	LPSECURITY_ATTRIBUTES lpProcessAttributes, 
	LPSECURITY_ATTRIBUTES lpThreadAttributes, 
	BOOL bInheritHandles, DWORD dwCreationFlags, 
	LPVOID lpEnvironment,
	PVOID lpCurrentDirectory, 
	PVOID lpStartupInfo, 
	LPPROCESS_INFORMATION lpProcessInformation
	)
{
	BOOL bRet = FALSE ;

	if ( bIsUnicode )
	{
		if ( hToken )
		{
			bRet = g_CreateProcessAsUserW_addr ( 
				hToken,(LPCWSTR)lpApplicationName,(LPWSTR)lpCommandLine,
				lpProcessAttributes,lpThreadAttributes,bInheritHandles,dwCreationFlags,
				lpEnvironment,(LPCWSTR)lpCurrentDirectory,(LPSTARTUPINFOW)lpStartupInfo,lpProcessInformation
				);
		} 
		else 
		{
			bRet = g_CreateProcessW_addr (
				(LPCWSTR)lpApplicationName,(LPWSTR)lpCommandLine,
				lpProcessAttributes,lpThreadAttributes,bInheritHandles,dwCreationFlags,
				lpEnvironment,(LPCWSTR)lpCurrentDirectory,(LPSTARTUPINFOW)lpStartupInfo,lpProcessInformation
				);
		}
	}
	else
	{
		if ( hToken )
		{
			bRet = g_CreateProcessAsUserA_addr (
				hToken,(LPCSTR)lpApplicationName,(LPSTR)lpCommandLine,
				lpProcessAttributes,lpThreadAttributes,bInheritHandles,dwCreationFlags,
				lpEnvironment,(LPCSTR)lpCurrentDirectory,(LPSTARTUPINFOA)lpStartupInfo,lpProcessInformation
				);
		}
		else
		{
			bRet = g_CreateProcessA_addr (
				(LPCSTR)lpApplicationName,(LPSTR)lpCommandLine,
				lpProcessAttributes,lpThreadAttributes,bInheritHandles,dwCreationFlags,
				lpEnvironment,(LPCSTR)lpCurrentDirectory,(LPSTARTUPINFOA)lpStartupInfo,lpProcessInformation
				);
		}
	}

	if ( (FALSE == bRet) && (ERROR_PRIVILEGE_NOT_HELD == GetLastError()) && hToken )
	{
		
		bRet = Call_Orignal_CreateProcessAsUser (
			bIsUnicode,
			0,
			lpApplicationName,
			lpCommandLine,
			lpProcessAttributes,
			lpThreadAttributes,
			bInheritHandles,
			dwCreationFlags,
			lpEnvironment,
			lpCurrentDirectory,
			lpStartupInfo,
			lpProcessInformation
			);
	}

	return bRet ;
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


ULONG WINAPI 
fake_CreateProcessA (
	LPCSTR lpApplicationName, 
	LPSTR lpCommandLine, 
	LPSECURITY_ATTRIBUTES lpProcessAttributes, 
	LPSECURITY_ATTRIBUTES lpThreadAttributes, 
	BOOL bInheritHandles, 
	DWORD dwCreationFlags, 
	LPVOID lpEnvironment, 
	LPCSTR lpCurrentDirectory, 
	LPSTARTUPINFOA lpStartupInfo, 
	LPPROCESS_INFORMATION lpProcessInformation
	)
{
	return fake_CreateProcessAsUserA (
		NULL,
		lpApplicationName,
		lpCommandLine,
		lpProcessAttributes,
		lpThreadAttributes,
		bInheritHandles,
		dwCreationFlags,
		lpEnvironment,
		lpCurrentDirectory,
		lpStartupInfo,
		lpProcessInformation
		);
}



ULONG WINAPI
fake_CreateProcessW(
	LPWSTR lpApplicationName, 
	LPWSTR lpCommandLine, 
	LPSECURITY_ATTRIBUTES lpProcessAttributes, 
	LPSECURITY_ATTRIBUTES lpThreadAttributes, 
	BOOL bInheritHandles, 
	DWORD dwCreationFlags, 
	LPVOID lpEnvironment, 
	LPCWSTR lpCurrentDirectory, 
	LPSTARTUPINFOW lpStartupInfo, 
	LPPROCESS_INFORMATION lpProcessInformation
	)
{
	return fake_CreateProcessAsUserW (
		NULL,
		lpApplicationName,
		lpCommandLine,
		lpProcessAttributes,
		lpThreadAttributes,
		bInheritHandles,
		dwCreationFlags,
		lpEnvironment,
		lpCurrentDirectory,
		lpStartupInfo,
		lpProcessInformation
		);
}



ULONG WINAPI
fake_CreateProcessAsUserA (
	  HANDLE  hToken,
	  LPCSTR lpApplicationName,
	  LPSTR lpCommandLine,
	  LPSECURITY_ATTRIBUTES lpProcessAttributes,
	  LPSECURITY_ATTRIBUTES lpThreadAttributes,
	  BOOL bInheritHandles,
	  DWORD dwCreationFlags,
	  LPVOID lpEnvironment,
	  LPCSTR lpCurrentDirectory,
	  LPSTARTUPINFOA lpStartupInfo,
	  LPPROCESS_INFORMATION lpProcessInformation
	  )
{
	BOOL bRet = FALSE ;
	DWORD dwErrorCode = 0, dwCreationFlagsDummy = 0 ;
	PVOID lpEnvironmentDummy = NULL ;
	LPSTR lpApplicationNameDummy = NULL, lpCurrentDirectoryDummy = NULL ;
	ANSI_STRING ansiApplicationName, ansiCurrentDirectory, ansiCommandLine ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	++pNode->sProcessLock.nLockCreateProcessAsUser;
	pNode->sRCP.Flag = 0;

	// 1. �ȵ���ԭʼ����
	bRet = Call_Orignal_CreateProcessAsUser (
		FALSE,
		hToken,
		(PVOID)lpApplicationName,
		lpCommandLine,
		lpProcessAttributes,
		lpThreadAttributes,
		bInheritHandles,
		dwCreationFlags,
		lpEnvironment,
		(PVOID)lpCurrentDirectory,
		lpStartupInfo,
		lpProcessInformation
		);

	// 2. ��ԭʼ��������ʧ��,ȥ����ӦȨ�޺��ٴε���
	dwErrorCode = GetLastError();
	if ( (dwErrorCode == ERROR_ACCESS_DENIED) && (dwCreationFlags & CREATE_BREAKAWAY_FROM_JOB) )
	{
		OutputDebugStringW( L"**** BREAKAWAY ***** A \n" );
		ClearFlag( dwCreationFlags, CREATE_BREAKAWAY_FROM_JOB ); // ����ñ�־λ: CREATE_BREAKAWAY_FROM_JOB
	//	dwCreationFlags &= 0xFEFFFFFF ; 
		pNode->sRCP.Flag = 0;

		bRet = Call_Orignal_CreateProcessAsUser (
			FALSE,
			hToken,
			(PVOID)lpApplicationName,
			lpCommandLine,
			lpProcessAttributes,
			lpThreadAttributes,
			bInheritHandles,
			dwCreationFlags,
			lpEnvironment,
			(PVOID)lpCurrentDirectory,
			lpStartupInfo,
			lpProcessInformation
			);

		dwErrorCode = GetLastError();
	}

	// 3. �������ܽṹ��
	if ( pNode->sRCP.Flag && (9 != pNode->sRCP.Flag) )
	{
		FixupProcessNode( pNode );

		// 4. �Ƿ��б�Ҫ�ض�����̴�������
		if ( IsExeNeedRedirect( pNode->sRCP.ImagePathName->uniBuffer.Buffer ) )
		{
			// 4.1 �ض����������
			RtlUnicodeStringToAnsiString( &ansiApplicationName, &pNode->sRCP.ImagePathName->uniBuffer, TRUE );
			lpApplicationNameDummy = ansiApplicationName.Buffer ;

			RtlUnicodeStringToAnsiString( &ansiCurrentDirectory, &pNode->sRCP.CurrentDirectory->uniBuffer, TRUE );
			lpCurrentDirectoryDummy = ansiCurrentDirectory.Buffer;
			
			if ( lpCommandLine )
			{
				RtlUnicodeStringToAnsiString( &ansiCommandLine, &pNode->sRCP.CommandLine->uniBuffer, TRUE);
				lpCommandLine = ansiCommandLine.Buffer;
			}
			else
			{
				ansiCommandLine.Buffer = NULL;
			}

			lpEnvironmentDummy = (PVOID) pNode->sRCP.Environment->uniBuffer.Buffer ;
			if ( lpEnvironmentDummy && *(WORD *)lpEnvironmentDummy )
			{
				dwCreationFlagsDummy = dwCreationFlags | CREATE_UNICODE_ENVIRONMENT; // ���CREATE_UNICODE_ENVIRONMENT��־λ
			}
			else
			{
				dwCreationFlagsDummy = dwCreationFlags &~ CREATE_UNICODE_ENVIRONMENT ;// ���CREATE_UNICODE_ENVIRONMENT��־λ
				lpEnvironmentDummy = NULL;
			}

			if ( ! pNode->sRCP.bIsDirectory ) { lpCurrentDirectoryDummy = NULL; }

			Walk_c_windows_winsxs_Total ( 
				pNode->sRCP.ImagePathName->uniBuffer.Buffer,
				pNode->sRCP.DllPath->uniBuffer.Buffer,
				0
				);

			// 4.2 �ض���󴴽�����
			bRet = Call_Orignal_CreateProcessAsUser (
				FALSE,
				hToken,
				lpApplicationNameDummy,
				lpCommandLine,
				lpProcessAttributes,
				lpThreadAttributes,
				bInheritHandles,
				dwCreationFlagsDummy,
				lpEnvironmentDummy,
				lpCurrentDirectoryDummy,
				lpStartupInfo,
				lpProcessInformation
				);

			dwErrorCode = GetLastError();
			RtlFreeAnsiString( &ansiApplicationName );
			RtlFreeAnsiString( &ansiCurrentDirectory );
			if ( ansiCommandLine.Buffer ) { RtlFreeAnsiString( &ansiCommandLine ); }
		}
		else
		{
			dwErrorCode = ERROR_ACCESS_DENIED;
			bRet = FALSE;
		}
	}
	else
	{
		if ( 0 == pNode->sRCP.Flag )
		{
			if ( bRet ) { dwErrorCode = ERROR_FILE_NOT_FOUND; }
			bRet = FALSE ;
		}
	}

	// ��β����
	pNode->sRCP.Flag = 0 ;
	-- pNode->sProcessLock.nLockCreateProcessAsUser;
	SetLastError( dwErrorCode );

	return bRet;
}



ULONG WINAPI
fake_CreateProcessAsUserW (
	HANDLE  hToken,
	LPCWSTR lpApplicationName,
	LPWSTR lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	LPVOID lpEnvironment,
	LPCWSTR lpCurrentDirectory,
	LPSTARTUPINFOW lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation
	)
{
	BOOL bRet = FALSE ;
	DWORD dwErrorCode = 0, dwCreationFlagsDummy = 0 ;
	PVOID lpEnvironmentDummy = NULL ;
	LPWSTR lpApplicationNameDummy = NULL, lpCurrentDirectoryDummy = NULL ;
	LPPROCESS_GLOBALMEMORY_NODE pNode = (LPPROCESS_GLOBALMEMORY_NODE) CMemoryManager::GetInstance().GetData(NULL);

	++pNode->sProcessLock.nLockCreateProcessAsUser;
	pNode->sRCP.Flag = 0;

	// 1. �ȵ���ԭʼ����
	bRet = Call_Orignal_CreateProcessAsUser (
		TRUE,
		hToken,
		(PVOID)lpApplicationName,
		lpCommandLine,
		lpProcessAttributes,
		lpThreadAttributes,
		bInheritHandles,
		dwCreationFlags,
		lpEnvironment,
		(PVOID)lpCurrentDirectory,
		lpStartupInfo,
		lpProcessInformation
		);

	// 2. ��ԭʼ��������ʧ��,ȥ����ӦȨ�޺��ٴε���
	dwErrorCode = GetLastError();
	if ( (dwErrorCode == ERROR_ACCESS_DENIED) && (dwCreationFlags & CREATE_BREAKAWAY_FROM_JOB) )
	{
		OutputDebugStringW( L"**** BREAKAWAY ***** A \n" );
		ClearFlag( dwCreationFlags, CREATE_BREAKAWAY_FROM_JOB ); // ����ñ�־λ: CREATE_BREAKAWAY_FROM_JOB
		//	dwCreationFlags &= 0xFEFFFFFF ; 
		pNode->sRCP.Flag = 0;

		bRet = Call_Orignal_CreateProcessAsUser (
			TRUE,
			hToken,
			(PVOID)lpApplicationName,
			lpCommandLine,
			lpProcessAttributes,
			lpThreadAttributes,
			bInheritHandles,
			dwCreationFlags,
			lpEnvironment,
			(PVOID)lpCurrentDirectory,
			lpStartupInfo,
			lpProcessInformation
			);

		dwErrorCode = GetLastError();
	}

	// 3. �������ܽṹ��
	if ( pNode->sRCP.Flag && (9 != pNode->sRCP.Flag) )
	{
		FixupProcessNode( pNode );

		// 4. �Ƿ��б�Ҫ�ض�����̴�������
		if ( IsExeNeedRedirect( pNode->sRCP.ImagePathName->uniBuffer.Buffer ) )
		{
			// 4.1 �ض����������
			lpApplicationNameDummy = pNode->sRCP.ImagePathName->uniBuffer.Buffer ;
			lpCurrentDirectoryDummy = pNode->sRCP.CurrentDirectory->uniBuffer.Buffer ;
			if ( lpCommandLine ) { lpCommandLine = pNode->sRCP.CommandLine->uniBuffer.Buffer ; }
			
			lpEnvironmentDummy = (PVOID) pNode->sRCP.Environment->uniBuffer.Buffer ;
			if ( lpEnvironmentDummy && *(WORD *)lpEnvironmentDummy )
			{
				dwCreationFlagsDummy = dwCreationFlags | CREATE_UNICODE_ENVIRONMENT; // ���CREATE_UNICODE_ENVIRONMENT��־λ
			}
			else
			{
				dwCreationFlagsDummy = dwCreationFlags &~ CREATE_UNICODE_ENVIRONMENT ;// ���CREATE_UNICODE_ENVIRONMENT��־λ
				lpEnvironmentDummy = NULL;
			}

			if ( !pNode->sRCP.bIsDirectory ) { lpCurrentDirectoryDummy = NULL; }

			Walk_c_windows_winsxs_Total ( 
				pNode->sRCP.ImagePathName->uniBuffer.Buffer,
				pNode->sRCP.DllPath->uniBuffer.Buffer,
				0
				);

			// 4.2 �ض���󴴽�����
			bRet = Call_Orignal_CreateProcessAsUser (
				TRUE,
				hToken,
				lpApplicationNameDummy,
				lpCommandLine,
				lpProcessAttributes,
				lpThreadAttributes,
				bInheritHandles,
				dwCreationFlagsDummy,
				lpEnvironmentDummy,
				lpCurrentDirectoryDummy,
				lpStartupInfo,
				lpProcessInformation
				);

			dwErrorCode = GetLastError();
		}
		else
		{
			dwErrorCode = ERROR_ACCESS_DENIED;
			bRet = FALSE;
		}
	}
	else
	{
		if ( 0 == pNode->sRCP.Flag )
		{
			if ( bRet ) { dwErrorCode = ERROR_FILE_NOT_FOUND; }
			bRet = FALSE ;
		}
	}

	// ��β����
	pNode->sRCP.Flag = 0 ;
	-- pNode->sProcessLock.nLockCreateProcessAsUser;
	SetLastError( dwErrorCode );

	return bRet;
}



ULONG WINAPI 
fake_RtlCreateProcessParameters (
    OUT /*PRTL_USER_PROCESS_PARAMETERS*/PVOID *ProcessParameters,
    IN PUNICODE_STRING ImagePathName,
    IN PUNICODE_STRING DllPath OPTIONAL,
    IN PUNICODE_STRING CurrentDirectory OPTIONAL,
    IN PUNICODE_STRING CommandLine OPTIONAL,
    IN PVOID Environment OPTIONAL,
    IN PUNICODE_STRING WindowTitle OPTIONAL,
    IN PUNICODE_STRING DesktopInfo OPTIONAL,
    IN PUNICODE_STRING ShellInfo OPTIONAL,
    IN PUNICODE_STRING RuntimeData OPTIONAL
    )
{
	return RtlCreateProcessParameters_Filter( ProcessParameters, ImagePathName, DllPath, CurrentDirectory, CommandLine,
		Environment, WindowTitle, DesktopInfo, ShellInfo, RuntimeData, 0, FALSE );
}



ULONG WINAPI 
fake_RtlCreateProcessParametersEx (
    OUT /*PRTL_USER_PROCESS_PARAMETERS*/PVOID *ProcessParameters,
    IN PUNICODE_STRING ImagePathName,
    IN PUNICODE_STRING DllPath OPTIONAL,
    IN PUNICODE_STRING CurrentDirectory OPTIONAL,
    IN PUNICODE_STRING CommandLine OPTIONAL,
    IN PVOID Environment OPTIONAL,
    IN PUNICODE_STRING WindowTitle OPTIONAL,
    IN PUNICODE_STRING DesktopInfo OPTIONAL,
    IN PUNICODE_STRING ShellInfo OPTIONAL,
    IN PUNICODE_STRING RuntimeData OPTIONAL,
	IN ULONG Reserved
    )
{
	return RtlCreateProcessParameters_Filter( ProcessParameters, ImagePathName, DllPath, CurrentDirectory, CommandLine,
		Environment, WindowTitle, DesktopInfo, ShellInfo, RuntimeData, Reserved, TRUE );
}



ULONG WINAPI 
fake_ExitProcess(
    UINT uExitCode
    )
{
	g_ExitProcess_addr( 0 );
	return 0; 
}



ULONG WINAPI 
fake_WinExec(
    LPSTR lpCmdLine,
    UINT uCmdShow
    )
{
	BOOL bRet = FALSE ;
	HANDLE hProcess = NULL ;
	STARTUPINFOA StartupInfo ;
	PROCESS_INFORMATION ProcInfo ;

	RtlZeroMemory( &StartupInfo, sizeof(StartupInfo) );
	RtlZeroMemory( &ProcInfo, sizeof(ProcInfo) );

	StartupInfo.cb = sizeof(StartupInfo) ;
	StartupInfo.dwFlags = STARTF_USESHOWWINDOW ;
	StartupInfo.wShowWindow = uCmdShow ;

	bRet = CreateProcessA( NULL, lpCmdLine, 0, 0, 0, 0, 0, 0, &StartupInfo, &ProcInfo );
	if ( bRet )
	{
		CloseHandle( ProcInfo.hThread );
		CloseHandle( ProcInfo.hProcess );
		hProcess = ProcInfo.hProcess;
	}
	else
	{
		hProcess = (HANDLE)HANDLE_FLAG_PROTECT_FROM_CLOSE;
	}

	return (ULONG)hProcess;
}


///////////////////////////////   END OF FILE   ///////////////////////////////