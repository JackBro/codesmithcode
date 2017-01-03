/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2011/07/05 [5:7:2011 - 15:42]
* MODULE : \SandBox\Code\Project\ProteinBoxDLL\HookHelperData.cpp
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
#include "common.h"
#include "Exportfunc.h"
#include "ProteinBoxDLL.h"
#include "PBDynamicData.h"
#include "PBRegsData.h"
#include "HookHelperData.h"

#pragma warning(disable: 4995)

//////////////////////////////////////////////////////////////////////////

HANDLE g_KeyHandle_PBAutoExec = NULL ;

/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


char Handler_RegKey_PBAutoExec( IN char data )
/*++

Author: sudami [sudami@163.com]
Time  : 2011/07/05 [5:7:2011 - 15:44]

Routine Description:
  ����@data�����ݶ� "HKEY_USERS\Sandbox_AV_DefaultBox\user\current\software\PBAutoExec"�����ݽ��в���.
  ��ʵ�ü��൱��һ��ȫ�ֱ��,����SbieDLL.dllע����ֵĳ�ʼ�������Ƿ����!
    
Arguments:
  data - 0 ��ʾҪ��ѯ"HKEY_USERS\Sandbox_AV_DefaultBox\user\current\software\PBAutoExec"��ֵ,�������򴴽�,�����򷵻ز�ѯ����ֵ;
         1 ��ʾҪ����"HKEY_USERS\Sandbox_AV_DefaultBox\user\current\software\PBAutoExec"��ֵ,
                ����Ϊ1,��ʾDLL��ʼ��ע������,�����������ٴ�Load SbieDLL.dllʱ�㲻���ٴγ�ʼ����.

Return Value:
  "*\PBAutoExec"��ֵ
    
--*/
{
	ULONG ResultLength = 0 ;
	UNICODE_STRING KeyName ;
	OBJECT_ATTRIBUTES ObjAtr ;
	NTSTATUS status = STATUS_SUCCESS ;
	WCHAR szKeyName[ MAX_PATH ] = L"" ;
	KEY_VALUE_PARTIAL_INFORMATION Buffer ;

	// 1. ������д��,����ZwSetValueKeyд�뵱ǰ����@data
	if ( data )
	{
		if ( g_KeyHandle_PBAutoExec )
		{
			RtlInitUnicodeString( &KeyName, NULL );
			status = ZwSetValueKey( g_KeyHandle_PBAutoExec, &KeyName, 0, REG_BINARY, &data, 1 );
			if ( ! NT_SUCCESS(status) )
			{
			}
		}
		
		return data;
	}
	
	// 2. û������д��,�򿪼�ֵ
	wcscpy( szKeyName, L"\\registry\\user\\" );
	wcscat( szKeyName, g_BoxInfo.SID );
	wcscat( szKeyName, L"\\software\\PBAutoExec" );

	RtlInitUnicodeString( &KeyName, szKeyName );
	InitializeObjectAttributes( &ObjAtr, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	status = Stub_NtOpenKey( &ObjAtr, &g_KeyHandle_PBAutoExec, 0xF003F );
	if ( status == STATUS_BAD_INITIAL_PC ) { return 0; }

	if ( status == STATUS_OBJECT_NAME_NOT_FOUND )
		status = ZwCreateKey( &g_KeyHandle_PBAutoExec, KEY_ALL_ACCESS, &ObjAtr, 0, 0, 0, 0 );

	if ( ! NT_SUCCESS(status) ) { return 0; }

	RtlInitUnicodeString( &KeyName, NULL );

	status = ZwQueryValueKey(
		g_KeyHandle_PBAutoExec,
		&KeyName,
		KeyValuePartialInformation,
		&Buffer,
		0x10,
		&ResultLength
		);

	if ( status == STATUS_OBJECT_NAME_NOT_FOUND ) { return 0; }
	if ( ! NT_SUCCESS(status) ) { return 0; }

	return data;
}



BOOL Handler_RegKey_RegLink()
/*++

Author: sudami [sudami@163.com]
Time  : 2011/07/05 [5:7:2011 - 16:06]

Routine Description:
  ���� NtCreateKey �� REG_OPTION_CREATE_LINK �ķ�ʽΪ����ļ�ֵ����һ��Ӳ����:
  (��A��ʾ) HKEY_USERS\Sandbox_AV_DefaultBox\user\current\software\classes.
  ���ӵ�����·����ȥ:
  (��B��ʾ)HKEY_USERS\Sandbox_AV_DefaultBox\user\current_classes

  ������ B ���Ӽ��͵��ڲ��� A ��
    
--*/
{
	UNICODE_STRING KeyName ;
	OBJECT_ATTRIBUTES ObjAtr ;
	NTSTATUS status = STATUS_SUCCESS ;
	WCHAR szKeyName[ MAX_PATH ] = L"" ;
	HANDLE hKey = NULL, hKey1 = NULL ;
	BOOL bSetLink = FALSE ;

	// 1.
	wcscpy( szKeyName, g_BoxInfo.KeyRootPath );
	wcscat( szKeyName, L"\\user\\current" );
	wcscat( szKeyName, L"\\software\\classes" );

	RtlInitUnicodeString( &KeyName, szKeyName );
	InitializeObjectAttributes( &ObjAtr, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	status = ZwCreateKey( &hKey, KEY_ALL_ACCESS, &ObjAtr, 0, 0, REG_OPTION_CREATE_LINK, 0 );
	if ( status == STATUS_OBJECT_NAME_COLLISION ) { return TRUE; }
	if ( ! NT_SUCCESS(status) ) { return FALSE; }

	// 2.
	wcscpy( szKeyName, g_BoxInfo.KeyRootPath );
	wcscat( szKeyName, L"\\user\\current" );
	wcscat( szKeyName, L"_classes" );

	RtlInitUnicodeString( &KeyName, szKeyName );

	status = ZwCreateKey( &hKey1, KEY_ALL_ACCESS, &ObjAtr, 0, 0, 0, 0 );
	if ( NT_SUCCESS(status) )
	{
		ZwClose( hKey1 );
		bSetLink = TRUE;
	}

	if ( status == STATUS_OBJECT_NAME_COLLISION ) { bSetLink = TRUE; }

	if ( bSetLink )
	{
		RtlInitUnicodeString( &KeyName, L"SymbolicLinkValue" );

		status = ZwSetValueKey( hKey, &KeyName, 0, REG_LINK, szKeyName, 2 * wcslen(szKeyName) );
		ZwClose( hKey );
		return TRUE ;
	}

	ZwClose( hKey );
	return FALSE ;
}



VOID Handler_RegKey_DisableDCOM()
/*++

Author: sudami [sudami@163.com]
Time  : 2011/07/05 [5:7:2011 - 16:28]

Routine Description:
  ��Զ��Э������ Windows XP �е�һ�������ʹ�� Windows XP �û�����ͨ�� Internet �����ṩ������ʹ�ô˹��ߣ�һ���û�����Ϊ��ר�ҡ���
  ���Բ鿴��һ���û��������û��������档���������û�������ר���������Թ���Գ����û�������Ŀ���Ȩ��Զ�̽�����⡣
  [ɳ��Ҫ���ı��ǽ�ֹ��Զ��Э����]

--*/
{
	UNICODE_STRING KeyName ;
	OBJECT_ATTRIBUTES ObjAtr ;
	NTSTATUS status = STATUS_SUCCESS ;
	WCHAR szKeyName[ MAX_PATH ] = L"" ;
	HANDLE hKey = NULL ;

	InitializeObjectAttributes( &ObjAtr, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	RtlInitUnicodeString( &KeyName, L"\\registry\\machine\\software\\microsoft\\ole" );
	status = Stub_NtOpenKey( &ObjAtr, &hKey, 0xF003F );
	if ( ! NT_SUCCESS(status) ) { return; }

	RtlInitUnicodeString( &KeyName, L"EnableDCOM" );
	status = ZwSetValueKey( hKey, &KeyName, 0, REG_SZ, L"N", 4 );

	ZwClose( hKey );
	return;
}



VOID Handler_RegKey_NukeOnDelete_Recycle()
/*++

Author: sudami [sudami@163.com]
Time  : 2011/07/05 [5:7:2011 - 16:35]

Routine Description:
  �޸�ע����к�"����վ"��Ӧ�ļ�ֵ"NukeOnDelete","UseGlobalSettings"; ��Ϊ1,����ɾ��ʱ����������վ.
  ������ɳ����ɾ���ļ�ʱ,Ϊ�˾������������Ƿ���ʾ"����վ��������ͼ��",ϵͳ���ѯ��ֵ,������ض�����:
  HKEY_USERS\Sandbox_AV_DefaultBox\machine\software\microsoft\Windows\CurrentVersion\Explorer\BitBucket.
  ����@NukeOnDelete,@UseGlobalSettings ������1,��ϵͳ��Ϊɾ�����ļ�����Ҫ��������վ,���ǻ���վͼ�겻��
    
--*/
{
	UNICODE_STRING KeyName ;
	OBJECT_ATTRIBUTES ObjAtr ;	
	NTSTATUS status = STATUS_SUCCESS ;
	KEY_BASIC_INFORMATION KeyInfo = { 0 } ;
	ULONG Flag = 0, Data = 1, Index = 0, ResultLength = 0 ;
	HANDLE hKey1 = NULL, hKey2 = NULL, hRootKey = NULL, hSubKey = NULL ;
	
	hKey1 = Stub_NtCreateKey_Special( HKEY_LOCAL_MACHINE, L"BitBucket", (int*)&Flag );
	if ( hKey1 && (hKey1 != (HANDLE)0xFFFFFFFF) )
	{
		RtlInitUnicodeString( &KeyName, L"NukeOnDelete" );

		status = ZwSetValueKey( hKey1, &KeyName, 0, REG_DWORD, &Data, 4 );
		if ( status >= 0 )
		{
			RtlInitUnicodeString( &KeyName, L"UseGlobalSettings" );
			status = ZwSetValueKey( hKey1, &KeyName, 0, REG_DWORD, &Data, 4 );

			if ( status < 0 ) { Flag = 0x33 ; }
		}
		else
		{
			Flag = 0x22 ;
		}

		ZwClose( hKey1 );
	}

	if ( Flag ) { return; }

	hKey2 = Stub_NtCreateKey_Special( HKEY_CURRENT_USER, L"BitBucket", (int*)&Flag );
	if ( NULL == hKey2 || ((HANDLE)0xFFFFFFFF == hKey2) ) { return; }

	RtlInitUnicodeString( &KeyName, L"Volume" );
	InitializeObjectAttributes( &ObjAtr, &KeyName, OBJ_CASE_INSENSITIVE, hKey2, NULL );

	status = Stub_NtOpenKey( &ObjAtr, &hRootKey, 0xF003F );
	if ( ! NT_SUCCESS(status) )
	{
		ZwClose( hKey2 );
		return;
	}

	ObjAtr.RootDirectory = hRootKey ;
	status = ZwEnumerateKey( hRootKey, 0, KeyBasicInformation, (PVOID)&KeyInfo, 0x100, &ResultLength );
	if ( ! NT_SUCCESS(status) ) { goto _END_ ; }

	while (TRUE)
	{
		KeyInfo.Name[ KeyInfo.NameLength / sizeof(WCHAR) ] = 0 ;
		RtlInitUnicodeString( &KeyName, KeyInfo.Name );

		status = Stub_NtOpenKey( &ObjAtr, &hSubKey, 0xF003F );
		if ( NT_SUCCESS(status) )
		{
			RtlInitUnicodeString( &KeyName, L"NukeOnDelete" );
			ZwSetValueKey( hSubKey, &KeyName, 0, REG_DWORD, &Data, 4 );
			ZwClose( hSubKey );
		}

		++ Index ;

		status = ZwEnumerateKey( hRootKey, 0, KeyBasicInformation, (PVOID)&KeyInfo, 0x100, &ResultLength );
		if ( ! NT_SUCCESS(status) ) { break; }
	}

_END_ :
	ZwClose( hRootKey );
	ZwClose( hKey2 );
	return;
}



VOID Handler_RegKey_BrowseNewProcess()
/*++

Author: sudami [sudami@163.com]
Time  : 2011/07/06 [6:7:2011 - 14:21]

Routine Description:
  HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\BrowseNewProcess �Լ�
  HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\BrowseNewProcess(ǰ�߶�
  �����û��ʻ���Ч,����ֻ�Ե�ǰ�û��ʻ���Ч) �޸Ĵ�ֵBrowseNewProcess����ֵ����Ϊ"yes"��ʾΪIE
  �����ʹ�õ�������;��������Ϊ"no"���ʾ�� Explorer.EXE �� IEXPLORE.EXE ���̺ϲ�.
  [ɳ��Ҫ���ı�������ΪHKEY_CURRENT_USER\*\BrowseNewProcessΪ"yes",��IEʹ�õ�������]

--*/
{
	int Flag = 0 ;
	HANDLE hKey = NULL ;
	UNICODE_STRING KeyName ;
	NTSTATUS status = STATUS_SUCCESS ;

	hKey = Stub_NtCreateKey_Special( HKEY_CURRENT_USER, L"BrowseNewProcess", &Flag );
	if ( NULL == hKey || ((HANDLE)0xFFFFFFFF == hKey) ) { return; }

	RtlInitUnicodeString( &KeyName, L"BrowseNewProcess" );
	status = ZwSetValueKey( hKey, &KeyName, 0, REG_SZ, L"yes", 8 );
	if ( status < 0 ) { Flag = 0x22; }
	
	ZwClose( hKey );
	return;
}



static const LPWSTR g_xxClasses_Arrays[ ] = 
{ 
	L"Machine\\Software\\Classes",
	L"Machine\\Software\\Classes\\Wow64Node",
	L"User\\Current\\Software\\Classes",
	L"User\\Current\\Software\\Classes\\Wow64Node",
};


VOID Handler_RegKey_Elimination()
/*++

Author: sudami [sudami@163.com]
Time  : 2011/07/06 [6:7:2011 - 16:04]

Routine Description:
  DLL��ʼ���ڼ�(�ݹ�)�������ļ�ֵ(�����Ӽ�):
  "HKEY_LOCAL_MACHINE\SOFTWARE\Classes\CompressedFolder\Shell\Open\ddeexec"
  "HKEY_LOCAL_MACHINE\SOFTWARE\Classes\*\shell\sandbox"
  "HKEY_CURRENT_USER\Software\Classes\CompressedFolder\Shell\find\ddeexec"
  "HKEY_CURRENT_USER\Software\Classes\*\shell\sandbox"

--*/
{
	UNICODE_STRING KeyName ;
	OBJECT_ATTRIBUTES ObjAtr ;
	HANDLE hKey = NULL ;
	WCHAR szName[ MAX_PATH ] = L"" ;
	NTSTATUS status = STATUS_SUCCESS ;

	for (int i = 0; i < ARRAYSIZEOF(g_xxClasses_Arrays); i++ )
	{
		// 1.
		if ( wcsstr(g_xxClasses_Arrays[i], L"Wow64") ) { continue; }

		wcscpy( szName, L"\\Registry\\" );
		wcscat( szName, g_xxClasses_Arrays[i] );
		wcscat( szName, L"\\*\\shell\\sandbox" );

		RtlInitUnicodeString( &KeyName, szName );
		InitializeObjectAttributes( &ObjAtr, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

		status = Stub_NtOpenKey( &ObjAtr, &hKey, 0xF003F );
		if ( NT_SUCCESS(status) )
		{
			StubNtDeleteKey( hKey, TRUE );
			ZwClose( hKey );
		}

		// 2.
		if ( wcsstr(g_xxClasses_Arrays[i], L"Wow64") ) { continue; }

		wcscpy( szName, L"\\Registry\\" );
		wcscat( szName, g_xxClasses_Arrays[i] );
		wcscat( szName, L"\\CompressedFolder\\shell\\open\\ddeexec" );

		RtlInitUnicodeString( &KeyName, szName );
		InitializeObjectAttributes( &ObjAtr, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

		status = Stub_NtOpenKey( &ObjAtr, &hKey, 0xF003F );
		if ( NT_SUCCESS(status) )
		{
			StubNtDeleteKey( hKey, TRUE );
			ZwClose( hKey );
		}
	}

	return;
}



static const LPWSTR g_DirtyClSID_Arrays[ ] = 
{ 
	L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\CLSID\\{de1f7eef-1851-11d3-939e-0004ac1abe1f}",
	L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\CLSID\\{761497BB-D6F0-462C-B6EB-D4DAF1D92D43}",
	L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\CLSID\\{DBC80044-A445-435B-BC74-9C25C1C588A9}",
	L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Classes\\CLSID\\{E7E6F031-17CE-4C07-BC86-EABFE594F69C}",
};


VOID Handler_RegKey_clsid_1()
/*++

Author: sudami [sudami@163.com]
Time  : 2011/07/06 [6:7:2011 - 17:14]

Routine Description:
  ��������4����ֵΪ"��ɾ��"״̬:
  "HKEY_LOCAL_MACHINE\SOFTWARE\Classes\CLSID\{de1f7eef-1851-11d3-939e-0004ac1abe1f}"
  "HKEY_LOCAL_MACHINE\SOFTWARE\Classes\CLSID\{761497BB-D6F0-462C-B6EB-D4DAF1D92D43}"
  "HKEY_LOCAL_MACHINE\SOFTWARE\Classes\CLSID\{DBC80044-A445-435B-BC74-9C25C1C588A9}"
  "HKEY_LOCAL_MACHINE\SOFTWARE\Classes\CLSID\{E7E6F031-17CE-4C07-BC86-EABFE594F69C}"
  
--*/
{
	UNICODE_STRING KeyName ;
	OBJECT_ATTRIBUTES ObjAtr ;
	HANDLE hKey = NULL ;
	NTSTATUS status = STATUS_SUCCESS ;

	InitializeObjectAttributes( &ObjAtr, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	for (int i = 0; i < ARRAYSIZEOF(g_DirtyClSID_Arrays); i++ )
	{
		RtlInitUnicodeString( &KeyName, g_DirtyClSID_Arrays[i] );

		status = Stub_NtOpenKey( &ObjAtr, &hKey, 0xF003F );
		if ( NT_SUCCESS(status) )
		{
			MarkDirtyKey( hKey );
		}
	}

	return ;
}



VOID Handler_RegKey_clsid_2()
/*++

Author: sudami [sudami@163.com]
Time  : 2011/07/06 [6:7:2011 - 17:23]

Routine Description:
  ����"HKEY_USERS\Sandbox_AV_DefaultBox\machine\software\microsoft\windows nt\currentversion\winlogon"�ļ�ֵ"Shell"����Ϊ"x",(��Ӧ����"Explorer.exe");
  ɾ��"\\registry\\machine\\software\\classes\\clsid\\{ceff45ee-c862-41de-aee2-a022c81eda92}"�ļ�ֵ"AppId"
    
--*/
{
	UNICODE_STRING KeyName ;
	OBJECT_ATTRIBUTES ObjAtr ;
	HANDLE hKey = NULL ;
	NTSTATUS status = STATUS_SUCCESS ;

	InitializeObjectAttributes( &ObjAtr, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	// 1.
	RtlInitUnicodeString( &KeyName, L"\\registry\\machine\\software\\microsoft\\windows nt\\currentversion\\winlogon" );

	status = Stub_NtOpenKey( &ObjAtr, &hKey, 0xF003F );
	if ( NT_SUCCESS(status) )
	{
		RtlInitUnicodeString( &KeyName, L"Shell" );
		status = ZwSetValueKey( hKey, &KeyName, 0, REG_SZ, L"x", 4 );
	}

	// 2.
	RtlInitUnicodeString( &KeyName, L"\\registry\\machine\\software\\classes\\clsid\\{ceff45ee-c862-41de-aee2-a022c81eda92}" );
	
	status = Stub_NtOpenKey( &ObjAtr, &hKey, 0xF003F );
	if ( NT_SUCCESS(status) )
	{
		RtlInitUnicodeString( &KeyName, L"AppId" );
		status = ZwDeleteValueKey( hKey, &KeyName );
	}

	return;
}



VOID HandlerSelfAutoExec()
{
	NTSTATUS status = STATUS_SUCCESS ;
	ULONG pBuffer = 0, nIndex = 1, dwErrCode = ERROR_NO_TOKEN ;

	return;



	pBuffer = (ULONG) kmalloc( 0x2000 );

	status = PB_EnumProcessEx( g_BoxInfo.BoxName, (int*)pBuffer );
	if ( ! NT_SUCCESS(status) || 1 != *(DWORD *)pBuffer ) { kfree( (PVOID)pBuffer ); return ; }

	// ֻ����ɳ���еĽ�������Ϊ1�����; 

	//
	// �ù���û��ʲô��,����
	//

	return;
}



HANDLE Stub_NtCreateKey_Special( HKEY hRootKey, LPWSTR szKeyName, int *pFlag )
/*++

Author: sudami [sudami@163.com]
Time  : 2011/07/06 [6:7:2011 - 14:30]

Routine Description:
  @hRootKey\Software\Microsoft\Windows\CurrentVersion\Explorer\@szKeyName,���ؾ��.(�������򴴽�֮)
    
Arguments:
  hRootKey - HKEY_LOCAL_MACHINE ��Ӧ"\\Registry\\Machine"; HKEY_CURRENT_USER ��Ӧ"\\Registry\\User\\Current"
  szKeyName - Ҫ�򿪵��Ӽ���

Return Value:
  ָ����ֵ�ľ��

--*/
{
	LPWSTR pszKeyName = NULL ;
	UNICODE_STRING KeyName ;
	OBJECT_ATTRIBUTES ObjAtr ;	
	NTSTATUS status = STATUS_SUCCESS ;
	HANDLE hRootKeyDummy = NULL, hKey_Explorer = NULL, hKey_Target = NULL ;

	if ( hRootKey == HKEY_LOCAL_MACHINE )
	{
		RtlInitUnicodeString( &KeyName, L"\\Registry\\Machine" );
	}
	else if ( hRootKey == HKEY_CURRENT_USER )
	{
		RtlInitUnicodeString( &KeyName, L"\\Registry\\User\\Current" );
	}
	else
	{
		*pFlag = 0xAA ;
		return NULL;
	}

	InitializeObjectAttributes( &ObjAtr, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL );

	status = ZwOpenKey( &hRootKeyDummy, KEY_READ, &ObjAtr );
	if ( ! NT_SUCCESS(status) )
	{
		*pFlag = 0x99;
		return NULL;
	}

	RtlInitUnicodeString( &KeyName, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer" );
	InitializeObjectAttributes( &ObjAtr, &KeyName, OBJ_CASE_INSENSITIVE, hRootKeyDummy, NULL );

	status = ZwOpenKey( &hKey_Explorer, KEY_READ, &ObjAtr );
	ZwClose( hRootKeyDummy );

	if ( status )
	{
		*pFlag = 0x88;
		return NULL;
	}

	RtlInitUnicodeString( &KeyName, szKeyName );
	InitializeObjectAttributes( &ObjAtr, &KeyName, OBJ_CASE_INSENSITIVE, hKey_Explorer, NULL );
	
	status = Stub_NtOpenKey( &ObjAtr, &hKey_Target, 0xF003F );
	if ( status == STATUS_BAD_INITIAL_PC )    // �Ǻڰ�����
	{
		*pFlag = 0;
		return (HANDLE)0xFFFFFFFF ;
	}

	// �����ڵ�ǰ��ֵ�򴴽�֮
	if ( status == STATUS_OBJECT_NAME_NOT_FOUND )
		status = ZwCreateKey( &hKey_Target, KEY_ALL_ACCESS, &ObjAtr, 0, 0, 0, 0 );

	ZwClose( hKey_Explorer );
	if ( status )
	{
		*pFlag = 0x77;
		return NULL;
	}

	return hKey_Target;
}



VOID HandlerIEEmbedding()
{
	if ( FALSE == g_bIs_Inside_iexplore_exe ) { return; }

	LPWSTR ptr = GetCommandLineW();
	if ( NULL == ptr ) { return; }

	if ( (wcsstr(ptr, L"/Embedding") || wcsstr(ptr, L"-Embedding")) && PB_IsOpenCOM() )
	{
		ULONG_PTR BaseAddr = (ULONG_PTR) GetModuleHandleW( NULL );
		ULONG_PTR AddressOfEntryPoint = (ULONG_PTR) SRtlImageNtHeader((PVOID)BaseAddr)->OptionalHeader.AddressOfEntryPoint + BaseAddr ;

		BOOL bRet = Mhook_SetHook( (PVOID*)&AddressOfEntryPoint, fake_entrypoint );
		if ( FALSE == bRet )
		{
			MYTRACE( L"error! | HandlerIEEmbedding() - Mhook_SetHook(); | \"entrypoint\" \n" );
			ExitProcess(0);
		}
	}

	return;
}


VOID fake_entrypoint()
{
	HMODULE hModule = LoadLibraryW(L"ole32.dll");

	//
	// {TODO:}sudami COM����. δ�����!!!
	//

	return;
}


///////////////////////////////   END OF FILE   ///////////////////////////////