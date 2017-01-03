/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/05/17 [17:5:2010 - 14:50]
* MODULE : d:\Work\Program\Coding\ɳ��\SandBox\Code\Project\ProteinBoxDrv\Security.c
* 
* Description:
*      
*   ����Token�����ϵͳȨ�޵���ģ��                         
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "Security.h"
#include "Memory.h"
#include "Version.h"
#include "SdtData.h"
#include "ProcessData.h"
#include "Config.h"

//////////////////////////////////////////////////////////////////////////

_ZwCreateToken_				g_ZwCreateToken_addr			= NULL ;
_RtlAddAccessAllowedAceEx_	g_RtlAddAccessAllowedAceEx_addr = NULL ;

// ��2���������ṩ����ģ�麯��ʹ��
PACL g_DefaultDacl_new = NULL ;
PSECURITY_DESCRIPTOR g_SecurityDescriptor_new = NULL ;


HANDLE	g_NewTokenHandle_system			= NULL	;
BOOL	g_bSepVariableInitialization_ok = FALSE ;

PSID g_SeAuthenticatedUsersSid = NULL, g_SeWorldSid = NULL ;

LUID SeChangeNotifyPrivilege;
LUID SeSystemtimePrivilege ;
LUID SeLoadDriverPrivilege;
LUID SeBackupPrivilege;
LUID SeRestorePrivilege;
LUID SeShutdownPrivilege;
LUID SeDebugPrivilege;

//
// LUID ����������
//

PLUID g_SepFilterPrivileges_Array [ ] =
{
	&SeSystemtimePrivilege,
	&SeLoadDriverPrivilege,
	&SeBackupPrivilege,
	&SeRestorePrivilege,
	&SeShutdownPrivilege,
	&SeDebugPrivilege,
	NULL
};


PRIVILEGE_BLACKLIST g_Privilege_BlackList = { 0, NULL };

PSID SeAliasAdminsSid		= NULL ;
PSID SeAliasPowerUsersSid	= NULL ;

//
// SID ����������
//

PSID g_SepFilterSid_Array [ ] =
{
	&SeAliasAdminsSid,
	&SeAliasPowerUsersSid,
	NULL
};


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+					  ��Ȩ����غ�����Ⱥ					  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


BOOL
DropAdminRights (
	IN PVOID _pNode
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/31 [31:5:2010 - 11:07]

Routine Description:
  ���������ļ��еġ�BlockDrivers����DropAdminRights�����͵�ǰ�߳�
  TokenPrivileges & TokenGroups��Ȩ��,����ACL.�������ZwSetInformationProcess���ø��º��Ȩ��
    
--*/
{
	BOOL   bRet = FALSE ;
	ULONG  ProtectedProcess, pProtectedProcess, OldFlag, offset = 0 ;
	HANDLE hToken = NULL ;
	HANDLE hTokenNew = NULL ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	PROCESS_ACCESS_TOKEN TokenInfo ;
	LPPDNODE pNode = (LPPDNODE) _pNode ;

	//
	// 1. У������Ϸ���
	//
	
	if ( NULL == pNode )
	{
		dprintf( "error! | DropAdminRights(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	//
	// 2. ��ѯ�����ļ�,�����Ƿ�Ȩ
	//

	if ( FALSE == GetConfigurationA( "BlockDrivers" ) )
	{
		dprintf( "ok! DropAdminRights() - GetConfigurationA( \"BlockDrivers\" ) | �����ļ���ָ�������ֹ��������,Ҳ���轵Ȩ \n" );
		return TRUE ;
	}

	pNode->bDropAdminRights = (BYTE) GetConfigurationA( "DropAdminRights" );

	//
	// 3. ��ȡ��ǰ���̵�Token���
	//

	status = ZwOpenProcessToken( NtCurrentProcess(), TOKEN_ALL_ACCESS, &hToken );
	if( ! NT_SUCCESS( status ) )
	{
		dprintf( "error! | DropAdminRights() - ZwOpenProcessToken(); | (status=0x%08lx) \n", status );
		return FALSE ;
	}

	//
	// 4. ���ݾɵ�Token�õ��µ�Ȩ�� - hTokenNew
	//

	hTokenNew = DropAdminRights_phrase1( hToken, pNode->bDropAdminRights );
	if ( NULL == hTokenNew )
	{
		dprintf( "error! | DropAdminRights() - DropAdminRights_phrase1(); | NULL == hTokenNew \n" );
		goto _END_ ;
	}

	if ( hTokenNew == hToken )
	{
		dprintf( "ok! | DropAdminRights() - DropAdminRights_phrase1(); | Token����������Ȼû�б仯:0x%08lx,�ɹ����� \n",hTokenNew );
		goto _OK_ ;
	}

	//
	// 5. ���һ��,Ϊ��ǰ���������µ�hTokenNew
	//

	if ( FALSE == g_Version_Info.IS_before_vista )
	{
		// ��Vista�����Ժ��ϵͳ,EPROCESS������һ�����ƽ���Ȩ�޵ı�־λ:ProtectedProcess, ���ڲ�������Ȩ��ǰ,�����ε�һЩ��Ȩ�޷��ɲ���
		if ( TRUE == g_Version_Info.IS___win7 )
		{
			if ( g_Version_Info.BuildNumber < 0x1DB0 )
				offset = g_Version_Info.BuildNumber < 0x1BBC ? 0x25C : 0x268 ;
			else
				offset = 0x26C ;
		}
		else
		{
			offset = 0x224 ; // vista
		}

		// ȷ��ProtectedProcess��ַ����Ч��
		pProtectedProcess = (ULONG)IoGetCurrentProcess() + offset ;

		if ( FALSE == MmIsAddressValid( (PVOID)pProtectedProcess ) )
		{
			dprintf( "error! | DropAdminRights() - MmIsAddressValid( pProtectedProcess ); | \n" );
			goto _END_ ;
		}

		// ������һЩȨ��,ʹ�����ǿɲ�����ǰ����
		ProtectedProcess = *(PULONG) pProtectedProcess ;

		OldFlag = ProtectedProcess & ABOVE_NORMAL_PRIORITY_CLASS ; 
		*(PULONG) pProtectedProcess = ProtectedProcess & ~(ProtectedProcess & ABOVE_NORMAL_PRIORITY_CLASS) ; 
	}

	// Do it!
	TokenInfo.Token	 = hTokenNew ;
	TokenInfo.Thread = 0 ;

	status = ZwSetInformationProcess( 
		NtCurrentProcess(),
		ProcessAccessToken, 
		&TokenInfo,
		sizeof (PROCESS_ACCESS_TOKEN) 
		);

	// ��ԭ�����ε�Ȩ��
	if ( FALSE == g_Version_Info.IS_before_vista ) { *(PULONG) pProtectedProcess |= OldFlag ; }

	//
	// 6. ������
	//

	ZwClose( hTokenNew );

	if( ! NT_SUCCESS( status ) )
	{
		dprintf( "error! | DropAdminRights() - ZwSetInformationProcess(); | (status=0x%08lx) \n", status );
		goto _END_ ;
	}

_OK_ :
	bRet = TRUE ;
_END_ :
	ZwClose( hToken );
	return bRet ;
}



HANDLE
DropAdminRights_phrase1 (
	IN HANDLE TokenHandle, 
	IN BOOL bDropAdminRights
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/31 [31:5:2010 - 15:13]

Routine Description:
  ���͵�ǰ�߳� TokenPrivileges & TokenGroups ��Ȩ��,����ACL    
    
Arguments:
  TokenHandle - �������Ľ���Token���
  bDropAdminRights - �Ƿ�Ȩ

Return Value:
  ����Ȩ�޺�õ���TokenHandle
    
--*/
{
	BOOL bRet = FALSE ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	ULONG DefaultDacl_Length = 0 ;
	PACL  Dacl = NULL ;
	PSID  Sid  = NULL ;
	PTOKEN_STATISTICS		LocalStatistics		= NULL	;
	PTOKEN_USER				LocalUser			= NULL	;
	PTOKEN_GROUPS			LocalGroups			= NULL	;
	PTOKEN_PRIVILEGES		LocalPrivileges		= NULL	;
	PTOKEN_OWNER			LocalOwner			= NULL	;
	PTOKEN_PRIMARY_GROUP	LocalPrimaryGroup	= NULL	;
	PTOKEN_DEFAULT_DACL		LocalDefaultDacl	= NULL	;
	PTOKEN_DEFAULT_DACL		NewDefaultDacl		= NULL	;
	PTOKEN_SOURCE			LocalSource			= NULL	;
	OBJECT_ATTRIBUTES ObjectAttributes ; 
	SECURITY_QUALITY_OF_SERVICE SecurityQos ;

	//
	// 1. ����@TokenHandle,��ѯ���Ӧ�ĸ�����Ϣ
	//
	
	if (	FALSE == QueryInformationToken( TokenHandle, TokenStatistics, &LocalStatistics, 0 )
		||	FALSE == QueryInformationToken( TokenHandle, TokenUser, &LocalUser, 0 )
		||	FALSE == QueryInformationToken( TokenHandle, TokenGroups, &LocalGroups, 0 )
		||	FALSE == QueryInformationToken( TokenHandle, TokenPrivileges, &LocalPrivileges, 0 )
		||	FALSE == QueryInformationToken( TokenHandle, TokenOwner, &LocalOwner, 0 )
		||	FALSE == QueryInformationToken( TokenHandle, TokenPrimaryGroup, &LocalPrimaryGroup,0 )
		||	FALSE == QueryInformationToken( TokenHandle, TokenDefaultDacl, &LocalDefaultDacl, &DefaultDacl_Length )
		||	FALSE == QueryInformationToken( TokenHandle, TokenSource, &LocalSource, 0 )
		)
	{
		dprintf( "error! | DropAdminRights_phrase1() - QueryInformationToken(); | \n" );
		goto _CLEAR_UP_ ;
	}

	//
	// 2. ����LocalPrivileges & LocalGroups ����
	//

	bRet = DropAdminRights_phrase2( LocalPrivileges, LocalGroups, bDropAdminRights );
	if ( FALSE == bRet )
	{
		dprintf( "error! | DropAdminRights_phrase1() - DropAdminRights_phrase2(); | \n" );
		goto _CLEAR_UP_ ;
	}

	SecurityQos.Length					= sizeof( SecurityQos );
	SecurityQos.ImpersonationLevel		= LocalStatistics->ImpersonationLevel ;
	SecurityQos.ContextTrackingMode	= SECURITY_STATIC_TRACKING ;
	SecurityQos.EffectiveOnly			= FALSE ;

	ObjectAttributes.SecurityQualityOfService = &SecurityQos ;

	InitializeObjectAttributes (
		&ObjectAttributes,
		NULL,
		OBJ_CASE_INSENSITIVE,
		NULL,
		NULL
		);

	//
	// 3. ��ȡ ZwCreateToken ��ԭʼ��ַ,����֮
	//

	bRet = kgetaddrSDT( ZwCreateToken );
	if ( FALSE == bRet )
	{
		dprintf( "error! | DropAdminRights_phrase1() - DropAdminRights_phrase2(); | �޷���ȡZwCreateToken�ĵ�ַ \n" );
		goto _CLEAR_UP_ ;
	}

	status = g_ZwCreateToken_addr(
		&TokenHandle,
		TOKEN_ALL_ACCESS,
		&ObjectAttributes,
		LocalStatistics->TokenType ,
		&LocalStatistics->AuthenticationId,
		&LocalStatistics->ExpirationTime,
		LocalUser,
		LocalGroups,
		LocalPrivileges,
		LocalOwner,
		LocalPrimaryGroup,
		LocalDefaultDacl,
		LocalSource
		);

	//
	// 3.1 �����������һ������
	//

	if ( bDropAdminRights &&  STATUS_INVALID_OWNER == status )
	{
		// ����һ���µ�ACL
		NewDefaultDacl = (PTOKEN_DEFAULT_DACL) kmallocMM( DefaultDacl_Length + 0x80, MTAG___NewDefaultDacl );
		if ( NULL == NewDefaultDacl )
		{
			dprintf( "error! | DropAdminRights_phrase1() - kmallocMM( MTAG___NewDefaultDacl ); | NULL == NewDefaultDacl \n" );
			goto _CLEAR_UP_ ;
		}

		memcpy( NewDefaultDacl, LocalDefaultDacl, DefaultDacl_Length );

		NewDefaultDacl->DefaultDacl = Dacl = (PACL)( (ULONG)NewDefaultDacl + 4 );
		NewDefaultDacl->DefaultDacl->AclSize += 0x80 ;
		Sid = LocalUser->User.Sid ;

		RtlAddAccessAllowedAce( Dacl, ACL_REVISION2, GENERIC_ALL, Sid );

		// �����µ�ACL�ٴδ���Token
		status = g_ZwCreateToken_addr (
			&TokenHandle,
			TOKEN_ALL_ACCESS,
			&ObjectAttributes,
			LocalStatistics->TokenType ,
			&LocalStatistics->AuthenticationId,
			&LocalStatistics->ExpirationTime,
			LocalUser,
			LocalGroups,
			LocalPrivileges,
			(PTOKEN_OWNER)&Sid,
			LocalPrimaryGroup,
			NewDefaultDacl,
			LocalSource
			);

		if ( !NT_SUCCESS(status) )
		{
			dprintf( "error! | DropAdminRights_phrase1() - ZwCreateToken(); | ��STATUS_INVALID_OWNER == status��ǰ�����ٴε���ZwCreateToken()ʧ�� - (status=0x%08lx) \n", status );
			goto _CLEAR_UP_ ;
		}

		SetSecurityObject( (HANDLE)0xFFFFFFFF, Dacl );
		SetSecurityObject( (HANDLE)0xFFFFFFFE, Dacl );
		SetSecurityObject( TokenHandle, Dacl		 );
	}

	if ( !NT_SUCCESS(status) )
	{			 
		dprintf( "error! | DropAdminRights_phrase1() - ZwCreateToken(); | (status=0x%08lx) \n", status );
	}

_CLEAR_UP_ :
	kfree( (PVOID)LocalStatistics );
	kfree( (PVOID)LocalUser );
	kfree( (PVOID)LocalGroups );
	kfree( (PVOID)LocalPrivileges );
	kfree( (PVOID)LocalOwner );
	kfree( (PVOID)LocalPrimaryGroup );
	kfree( (PVOID)LocalDefaultDacl );
	kfree( (PVOID)LocalSource );
	return TokenHandle ;
}



BOOL
DropAdminRights_phrase2 (
	IN PTOKEN_PRIVILEGES LocalPrivileges, 
	IN PTOKEN_GROUPS LocalGroups, 
	IN BOOL bDropAdminRights
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/31 [31:5:2010 - 16:15]

Routine Description:
  1. ����TokenPrivileges������,��@bDropAdminRightsΪTRUE���򽫸������nCounts����Ϊ1��
  ��������0x17��SE_CHANGE_NOTIFY_PRIVILEGEȨ�ޣ�����ҲҪȥ������Ȩ�ޣ�
  #define SE_SYSTEMTIME_PRIVILEGE           0x0c
  #define SE_LOAD_DRIVER_PRIVILEGE          0x10
  #define SE_BACKUP_PRIVILEGE               0x11
  #define SE_RESTORE_PRIVILEGE              0x12
  #define SE_SHUTDOWN_PRIVILEGE             0x13
  #define SE_DEBUG_PRIVILEGE                0x14

  2.����TokenGroups������,��@bDropAdminRightsΪTRUE�������е�SID������ʾ��ѵ�ǰSIDĨȥ
  0x201,0x050000000,0x20,0x220 ���� 0x201,0x050000000,0x20,0x223
  
  #define DOMAIN_ALIAS_RID_ADMINS        (0x00000220L)
  #define DOMAIN_ALIAS_RID_POWER_USERS   (0x00000223L)
    
Arguments:
  LocalPrivileges -
  LocalGroups -
  bDropAdminRights - �Ƿ�Ȩ    
    
--*/
{
	BOOL  bIsWhite = FALSE ;
	ULONG BlackListPrivilegeCount = 0, PrivilegeCount = 0, RealPrivilegeCount = 0 ;
	ULONG BlackIndex = 0, PrivilegeIndex = 0 ; 
	ULONG GroupCount = 0, RealGroupCount = 0 ;
	ULONG GroupIndex = 0, BlackGroupIndex = 0 ;
	PSID_AND_ATTRIBUTES		Groups = NULL ;
	SID_AND_ATTRIBUTES		GroupsCurrent ;
	PLUID_AND_ATTRIBUTES	Privileges = NULL ;
	LUID_AND_ATTRIBUTES		PrivilegesCurrent ;
	LPPRIVILEGE_SET_EX		BlackListPrivilege = NULL ;
	PSID *pCurrentSID = NULL ;

	//
	// 1. ��ʼ�����ֱ��������
	//

	if ( FALSE == SepVariableInitialization() )
	{
		dprintf( "error! | DropAdminRights_phrase2() - SepVariableInitialization(); | ��ʼ�����ֱ��������ʧ��! \n" );
		return FALSE ;
	}

	//
	// 2. ���� TokenPrivileges ����
	//

	Privileges		= LocalPrivileges->Privileges		;
	PrivilegeCount	= LocalPrivileges->PrivilegeCount	;

	BlackListPrivilege		= g_Privilege_BlackList.BlackListPrivilege	;
	BlackListPrivilegeCount	= BlackListPrivilege->PrivilegeCount		;

	if ( PrivilegeCount && BlackListPrivilegeCount )
	{
		//
		// 1.1 �Ե�ǰ�����е�ÿ����Ԫ�����Ų�,һ�������Ǻ�������Ա,���������
		//

		for ( PrivilegeIndex=0; PrivilegeIndex < PrivilegeCount; PrivilegeIndex++ )
		{
			bIsWhite = FALSE ; // ÿ��ѭ����ʼ��Ĭ���ú�,��Ϊһ�ж���а���
			PrivilegesCurrent = Privileges[ PrivilegeIndex ] ;

			if ( bDropAdminRights )
			{
				// 1.1.1 ��@bDropAdminRightsΪTRUE���򽫸������nCounts����Ϊ1�������� SE_CHANGE_NOTIFY_PRIVILEGE Ȩ��
				if ( RtlEqualLuid( &PrivilegesCurrent.Luid, &SeChangeNotifyPrivilege) ) 
				{
					bIsWhite = TRUE ; // �����ᱣ���� SeChangeNotifyPrivilege Ȩ��
				}
			}
			else
			{
				// 1.1.2 @bDropAdminRightsΪFALSEʱ�����ں������в鿴�Ƿ��е�ǰ��Ԫ,�����Ƴ���
				do
				{
					if ( RtlEqualLuid( &BlackListPrivilege->Privilege[ BlackIndex ].Luid, &PrivilegesCurrent.Luid ) ) 
					{
						break ;
					}

					++ BlackIndex ; // Сѭ���ļ���
				}
				while ( BlackIndex < BlackListPrivilegeCount );

				// 1.1.3 ��Цѭ������ʱ,�������������������δ�ҵ���ǰ��Ԫ,������ǰ��Ԫ���ں�������,�ð�
				if ( BlackIndex >= BlackListPrivilegeCount ) 
				{
					bIsWhite = TRUE ;
				}
			}

			if ( TRUE == bIsWhite ) { continue ; }

			//
			// 1.2 ȥ����ǰ��Privileges��Ԫ,��Ϊ���ں�������
			//
			
			RealPrivilegeCount = LocalPrivileges->PrivilegeCount - 1 ;
			if ( RealPrivilegeCount )
			{
				// 1.2.1 �����������һ����Ԫ �滻�� ��ǰλ�õĺڵ�Ԫ
				PrivilegesCurrent.Luid		 = LocalPrivileges->Privileges[ RealPrivilegeCount ].Luid		;
				PrivilegesCurrent.Attributes = LocalPrivileges->Privileges[ RealPrivilegeCount ].Attributes	;
			}

			LocalPrivileges->PrivilegeCount = RealPrivilegeCount ;
		}
	}

	//
	// 2. ���� TokenGroups ����
	//

	GroupCount	= LocalGroups->GroupCount	;
	Groups		= LocalGroups->Groups		;

	if ( 0 == GroupCount ) { return TRUE; }
	
	for ( GroupIndex=0; GroupIndex < GroupCount; GroupIndex++ )
	{
		GroupsCurrent = Groups[ GroupIndex ] ;

		for( BlackGroupIndex = 0; BlackGroupIndex < ARRAYSIZEOF( g_SepFilterSid_Array ); ++ BlackGroupIndex )
		{
			pCurrentSID = g_SepFilterSid_Array[ BlackGroupIndex ] ;
			if ( NULL == pCurrentSID ) { break ; }

			if ( RtlEqualSid( GroupsCurrent.Sid, *pCurrentSID ) )
			{
				RealGroupCount = LocalGroups->GroupCount - 1 ;
				if ( RealGroupCount )
				{
					GroupsCurrent.Sid		 = LocalGroups->Groups[ RealGroupCount ].Sid		;
					GroupsCurrent.Attributes = LocalGroups->Groups[ RealGroupCount ].Attributes	;
				}

				LocalGroups->GroupCount = RealGroupCount;
			}
		}
	}

	return TRUE ;
}



BOOL
SepVariableInitialization(
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/01 [1:6:2010 - 15:15]

Routine Description:
  ��ʼ������,��Ҫ�ǵõ�LUID & SID�������б�,�����ڹ�����
    
--*/
{
	ULONG Size = 0, i = 0 ;
	ULONG SidWithTwoSubAuthorities = 0 ;
	PLUID Temp = NULL ;
	LPPRIVILEGE_SET_EX BlackListPrivilege = NULL ;
	static SID_IDENTIFIER_AUTHORITY  SepNtAuthority = SECURITY_NT_AUTHORITY ;

	if ( TRUE == g_bSepVariableInitialization_ok ) { return TRUE ; }

	//
	// 1. ��ʼ������LUID
	//

	SeChangeNotifyPrivilege		 = RtlConvertLongToLuid( SE_CHANGE_NOTIFY_PRIVILEGE		 );
	SeSystemtimePrivilege		 = RtlConvertLongToLuid( SE_SYSTEMTIME_PRIVILEGE		 );
	SeLoadDriverPrivilege		 = RtlConvertLongToLuid( SE_LOAD_DRIVER_PRIVILEGE		 );
	SeBackupPrivilege			 = RtlConvertLongToLuid( SE_BACKUP_PRIVILEGE			 );
	SeRestorePrivilege			 = RtlConvertLongToLuid( SE_RESTORE_PRIVILEGE			 );
	SeShutdownPrivilege			 = RtlConvertLongToLuid( SE_SHUTDOWN_PRIVILEGE			 );
	SeDebugPrivilege			 = RtlConvertLongToLuid( SE_DEBUG_PRIVILEGE				 );

	//
	// 2. ��ʼ�� LUID ������
	//

	g_Privilege_BlackList.BlackListPrivilegeSetSize = Size = sizeof( PRIVILEGE_SET_EX ) + 5 * (ULONG)sizeof( LUID_AND_ATTRIBUTES );

	g_Privilege_BlackList.BlackListPrivilege = BlackListPrivilege = (LPPRIVILEGE_SET_EX) kmallocMM( Size, MTAG___Privilege_BlackList );
	if ( NULL == BlackListPrivilege )
	{
		dprintf( "error! | SepVariableInitialization() -kmallocMM(MTAG___Privilege_BlackList); | �����ڴ�ʧ�� \n" );
		return FALSE ;
	}

	BlackListPrivilege->PrivilegeCount = 6 ;

	for ( i=0; i<6; i++ )
	{
		Temp = g_SepFilterPrivileges_Array[ i ] ;

		BlackListPrivilege->Privilege[ i ].Luid = *Temp ;
		BlackListPrivilege->Privilege[ i ].Attributes = 0 ;
	}

	//
	// 3. ��ʼ�� SID ������ 
	//

	SidWithTwoSubAuthorities = RtlLengthRequiredSid( 2 );

	SeAliasAdminsSid	 = (PSID) kmallocMM( SidWithTwoSubAuthorities, MTAG___SeAliasAdminsSid		);
	SeAliasPowerUsersSid = (PSID) kmallocMM( SidWithTwoSubAuthorities, MTAG___SeAliasPowerUsersSid	);
	if ( NULL == SeAliasAdminsSid || NULL == SeAliasPowerUsersSid )
	{
		dprintf( "error! | SepVariableInitialization() -kmallocMM(MTAG___SeAliasAdminsSid); | �����ڴ�ʧ�� \n" );
		return FALSE ;
	}

	RtlInitializeSid( SeAliasAdminsSid,     &SepNtAuthority, 2 );
	RtlInitializeSid( SeAliasPowerUsersSid, &SepNtAuthority, 2 );

	*(RtlSubAuthoritySid( SeAliasAdminsSid,     1 )) = DOMAIN_ALIAS_RID_ADMINS ;
	*(RtlSubAuthoritySid( SeAliasPowerUsersSid, 1 )) = DOMAIN_ALIAS_RID_POWER_USERS ;

	g_bSepVariableInitialization_ok = TRUE ;
	return TRUE ;
}



BOOL
QueryInformationToken(
	IN HANDLE TokenHandle,
	IN int TokenInformationClass,
	OUT PVOID* out_TokenInformation,
	OUT PULONG out_TokenInformationLength OPTIONAL
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/31 [31:5:2010 - 15:26]

Routine Description:
  �Ե���ZwQueryInformationToken�����İ�װ,����ɵ������ͷ��ڴ�

--*/
{
	ULONG ReturnLength = 0 ;
	PVOID TokenInformation = NULL ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 

	//
	// 1. У������Ϸ���
	//
	
	if ( NULL == out_TokenInformation )
	{
		dprintf( "error! | QueryInformationToken(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	//
	// 2.���������ڴ��С,�����ڴ�,��ȡ��Ϣ
	//

	status = ZwQueryInformationToken (
		TokenHandle,
		(TOKEN_INFORMATION_CLASS)TokenInformationClass,
		0,
		0,
		&ReturnLength
		);

	if ( STATUS_BUFFER_TOO_SMALL != status ) 
	{ 
		dprintf( "error! | QueryInformationToken() - ZwQueryInformationToken(); | (status=0x%08lx), TokenInformationClass=%d \n", status, TokenInformationClass );
		return FALSE ;
	}

	TokenInformation = (PVOID) kmalloc( ReturnLength );
	if ( NULL == TokenInformation )
	{
		dprintf( "error! | QueryInformationToken() - kmallocMM(); | �����ڴ�ʧ��,TokenInformationClass=%d \n", TokenInformationClass );
		return FALSE ;
	}

	status = ZwQueryInformationToken (
		TokenHandle,
		(TOKEN_INFORMATION_CLASS)TokenInformationClass,
		TokenInformation,
		ReturnLength,
		&ReturnLength
		);

	if( ! NT_SUCCESS( status ) )
	{
		dprintf( "error! | QueryInformationToken() - ZwQueryInformationToken(); | (status=0x%08lx),TokenInformationClass=%d \n", status, TokenInformationClass );
		return FALSE ;
	}

	*out_TokenInformation = (PVOID) TokenInformation ;
	if ( out_TokenInformationLength ) { *out_TokenInformationLength = (ULONG) ReturnLength ; }
	 
	return TRUE ;
}



NTSTATUS
SetSecurityObject (
	IN HANDLE Handle,
	IN PACL Dacl
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/01 [1:6:2010 - 11:46]

Routine Description:
  �Ե���ZwSetSecurityObject�����İ�װ
        
--*/
{
	NTSTATUS status = STATUS_UNSUCCESSFUL  ; 
	SECURITY_DESCRIPTOR SecurityDescriptor ;

	status = RtlCreateSecurityDescriptor( &SecurityDescriptor, SECURITY_DESCRIPTOR_REVISION );
	if( ! NT_SUCCESS( status ) )
	{
		dprintf( "error! | SetSecurityObject() - RtlCreateSecurityDescriptor(); | (status=0x%08lx) \n", status );
		return status ;
	}

	status = RtlSetDaclSecurityDescriptor( &SecurityDescriptor, TRUE, Dacl, FALSE );
	if( ! NT_SUCCESS( status ) )
	{
		dprintf( "error! | SetSecurityObject() - RtlSetDaclSecurityDescriptor(); | (status=0x%08lx) \n", status );
		return status ;
	}

	status = ZwSetSecurityObject( Handle, DACL_SECURITY_INFORMATION, &SecurityDescriptor );
	return status ;
}



/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+           ������ʼ���׶δ�����ȫ�������Ⱥ�����Ⱥ          +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


BOOL CreateAcl()
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/17 [17:5:2010 - 22:05]

Routine Description:
  ����һ����ȫ������,������ʹ��

Return Value:
  BOOL
    
--*/
{
	NTSTATUS			 status = STATUS_UNSUCCESSFUL ;
	ULONG				 AclSize			 = 0x80	 ;
	BOOL				 bRet				 = FALSE ;
	PACL				 pDefaultAcl		 = NULL	 ;
	PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL  ;

	//
	// 1. ����ACL�ṹ�� & ��ʼ��
	//

	g_DefaultDacl_new = pDefaultAcl = (PACL) kmallocMM( AclSize, MTAG___TokenInformation_DefaultDacl_New );
	if ( NULL == pDefaultAcl )
	{
		dprintf( "error! | CreateAcl() - kmallocMM(); NULL == pDefaultAcl \n" );
		return FALSE ;
	}

	if ( FALSE == InitializeAcl( pDefaultAcl, AclSize, ACL_REVISION ) )
	{
		dprintf( "error! | CreateAcl() - InitializeAcl() \n" );
		return FALSE ;
	}

	//
	// 2. ��ʼ��SID
	//

	bRet = InitializeSid () ;
	if ( FALSE == bRet )
	{
		dprintf( "error! | CreateAcl() - __InitializeSid() \n" );
		return FALSE ;
	}

	//
	// 3. ��ACL���ָ����Ȩ��
	//

	Call_RtlAddAccessAllowedAce( pDefaultAcl, g_SeAuthenticatedUsersSid );
	Call_RtlAddAccessAllowedAce( pDefaultAcl, g_SeWorldSid	);

	//
	// 4. ������ȫ������ & ���֮
	//

	g_SecurityDescriptor_new = pSecurityDescriptor = (PSECURITY_DESCRIPTOR) kmallocMM( 0x40, MTAG___SecurityDescriptor );
	if ( NULL == pSecurityDescriptor )
	{
		dprintf( "error! | CreateAcl() - kmallocMM(); NULL == pSecurityDescriptor \n" );
		return FALSE ;
	}

	status = RtlCreateSecurityDescriptor( pSecurityDescriptor, SECURITY_DESCRIPTOR_REVISION );
	if ( !NT_SUCCESS(status) )
	{
		dprintf( "error! | CreateAcl() - RtlCreateSecurityDescriptor() | status == 0x%08lx \n", status );
		return FALSE;
	}
	
	status = RtlSetDaclSecurityDescriptor(
		pSecurityDescriptor,
		TRUE, 
		pDefaultAcl,
		FALSE
		);

	if ( !NT_SUCCESS(status) )
	{
		dprintf( "error! | CreateAcl() - RtlSetDaclSecurityDescriptor() | status == 0x%08lx \n", status );
		return FALSE;
	}

	dprintf( "ok! | CreateAcl(); | ����һ����ȫ������\n" );
	return TRUE ;
}

//////////////////////////////////////////////////////////////////////////


BOOL
InitializeAcl (
	IN PACL		pAcl,
	IN DWORD	nAclLength,
	IN DWORD	dwAclRevision
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/17 [17:5:2010 - 14:56]

Routine Description:
  ��ʼ��ACL�ṹ��

--*/
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ;

	status = RtlCreateAcl ( pAcl, nAclLength, dwAclRevision );
	if ( !NT_SUCCESS(status) )
	{
		dprintf( "error! | InitializeAcl() - RtlCreateAcl(); status == 0x%08lx \n", status );
		return FALSE;
	}

	return TRUE;
}



BOOL
__InitializeSid (
	IN PSID Sid,
	IN PSID_IDENTIFIER_AUTHORITY pIdentifierAuthority,
	IN BYTE SubAuthorityCount,
	IN ULONG SubAuthority
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/17 [17:5:2010 - 21:54]

Routine Description:
  ����SID�ṹ��

--*/
{
	PULONG SubAuthority__ = NULL ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ;

	status = RtlInitializeSid( Sid, pIdentifierAuthority, SubAuthorityCount );
	if ( !NT_SUCCESS (status) )
	{
		dprintf( "error! | InitializeSid() - RtlInitializeSid(); status == 0x%08lx \n", status );
		return FALSE;
	}

	if ( SubAuthority )
	{
		SubAuthority__ = RtlSubAuthoritySid( Sid, 0 );
		*SubAuthority__ = SubAuthority ;
	}

	return TRUE;
}



BOOL InitializeSid ()
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/17 [17:5:2010 - 21:53]

Routine Description:
  ��ʼ��2��SID
    
--*/
{
	BOOL bRet = FALSE ;
	ULONG SidLength1 = 0 ;
	SID_IDENTIFIER_AUTHORITY SeNtSidAuthority	 = { SECURITY_NT_AUTHORITY } ; 
	SID_IDENTIFIER_AUTHORITY SeWorldSidAuthority = { SECURITY_WORLD_SID_AUTHORITY	 } ;

	SidLength1 = RtlLengthRequiredSid( 1 );
	g_SeAuthenticatedUsersSid	= (PSID) kmallocMM( SidLength1, MTAG___SeAuthenticatedUsersSid );
	g_SeWorldSid				= (PSID) kmallocMM( SidLength1, MTAG___SeWorldSid );
	
	if ( NULL == g_SeAuthenticatedUsersSid || NULL == g_SeWorldSid )
	{
		dprintf( "error! | InitializeSid() -kmallocMM(); | �����ڴ�ʧ�� \n" );
		return FALSE ;
	}

	bRet = __InitializeSid( g_SeAuthenticatedUsersSid, &SeNtSidAuthority, 1, SECURITY_AUTHENTICATED_USER_RID );
	bRet = __InitializeSid( g_SeWorldSid,	&SeWorldSidAuthority, 1, 0 );

	return bRet ;
}



NTSTATUS
Call_RtlAddAccessAllowedAce (
	IN OUT PACL Acl,
	IN PSID Sid
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ;
	UNICODE_STRING uniBuffer ;
	ULONG Flags = OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERITED_ACE ;

	if ( NULL == g_RtlAddAccessAllowedAceEx_addr )
	{
		RtlInitUnicodeString( &uniBuffer, L"RtlAddAccessAllowedAceEx" );
		g_RtlAddAccessAllowedAceEx_addr = ( _RtlAddAccessAllowedAceEx_ ) MmGetSystemRoutineAddress( &uniBuffer );
	}

	if ( g_RtlAddAccessAllowedAceEx_addr )
	{
		status = g_RtlAddAccessAllowedAceEx_addr ( 
			Acl,
			ACL_REVISION,
			Flags, // 0x13
			0x10000000,
			Sid
			);
	}
	else
	{
		status = RtlAddAccessAllowedAce( Acl, ACL_REVISION, 0x10000000, Sid );
	}

	return status ;
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


BOOL AdjustPrivilege ()
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/18 [18:5:2010 - 20:58]

Routine Description:
  ����ϵͳ���̵ľ��,����һ����Ȩ��,����þ����ȫ�ֱ�����,���Ժ�ʹ��
    
--*/
{
	BOOL		bRet		= FALSE				;
	HANDLE		hProcess	= NULL				;
	HANDLE		TokenHandle	= NULL				;
	CLIENT_ID	ClientId	= { NULL, NULL }	;
	NTSTATUS	status		= STATUS_UNSUCCESSFUL ;
	OBJECT_ATTRIBUTES ObjectAttributes ;
	SECURITY_QUALITY_OF_SERVICE Qos ;
	TOKEN_PRIVILEGES NewState ;

	//
	// 1.��ϵͳ����,��þ��
	//

	ClientId.UniqueThread	= 0;
	ClientId.UniqueProcess	= (HANDLE) 4 ;

	InitializeObjectAttributes(
		&ObjectAttributes,
		NULL,
		OBJ_KERNEL_HANDLE;,
		NULL,
		NULL
		);

	status = ZwOpenProcess( 
		&hProcess ,
		PROCESS_QUERY_INFORMATION ,
		&ObjectAttributes ,
		&ClientId 
		);

	if ( !NT_SUCCESS (status) )
	{
		ClientId.UniqueProcess	= (HANDLE) 8 ;

		status = ZwOpenProcess( 
			&hProcess ,
			PROCESS_QUERY_INFORMATION ,
			&ObjectAttributes ,
			&ClientId 
			);

		if ( !NT_SUCCESS (status) )
		{
			dprintf( "error! | AdjustPrivilege() - ZwOpenProcess() | status == 0x%08lx \n", status );
			return FALSE ;
		}
	}

	//
	// 2. ����ϵͳ���̵ľ��
	//

	status = ZwOpenProcessToken( hProcess, 0xF01FF, &TokenHandle );
	ZwClose( hProcess );

	if ( !NT_SUCCESS (status) )
	{
		dprintf( "error! | AdjustPrivilege() - ZwOpenProcessToken() | status == 0x%08lx \n", status );
		return FALSE ;
	}

	Qos.Length				= sizeof( SECURITY_QUALITY_OF_SERVICE ) ;
	Qos.ImpersonationLevel	= SecurityImpersonation ;
	Qos.ContextTrackingMode = 0 ;
	Qos.EffectiveOnly		= FALSE ;

	ObjectAttributes.SecurityQualityOfService = &Qos;

	status = ZwDuplicateToken (
		TokenHandle, 
		0, 
		&ObjectAttributes, 
		0,
		TokenImpersonation,
		&g_NewTokenHandle_system
		);

	ZwClose( TokenHandle );

	if ( !NT_SUCCESS (status) )
	{
		dprintf( "error! | AdjustPrivilege() - ZwDuplicateToken() | status == 0x%08lx \n", status );
		return FALSE ;
	}

	//
	// 3. �����¡�ľ���µ�Ȩ��
	//

	NewState.PrivilegeCount = 3 ;
	NewState.Privileges[ 0 ].Luid.LowPart	= SE_RESTORE_PRIVILEGE ;
	NewState.Privileges[ 0 ].Luid.HighPart	= 0;
	NewState.Privileges[ 0 ].Attributes		= SE_PRIVILEGE_ENABLED ;

	status = ZwAdjustPrivilegesToken (
		g_NewTokenHandle_system ,
		FALSE ,
		&NewState,
		0 ,
		NULL ,
		NULL
		);

	if ( !NT_SUCCESS (status) )
	{
		dprintf( "error! | AdjustPrivilege() - ZwAdjustPrivilegesToken() | status == 0x%08lx \n", status );
		ZwClose( g_NewTokenHandle_system );
		g_NewTokenHandle_system = NULL ;
		return FALSE ;
	}

	dprintf( "ok! | AdjustPrivilege(); | ����ϵͳ���̵ľ�� \n" );
	return TRUE ;
}



NTSTATUS 
RtlGetUserSid(
	IN HANDLE ProcessHandle,
	OUT LPWSTR RegisterUserID
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/02 [2:6:2010 - 15:22]

Routine Description:
  �õ���ǰ�û���SID    
    
Arguments:
  RegisterUserID - ����õ���SID

--*/
{
	HANDLE TokenHandle;
	UCHAR Buffer[256];
	PSID_AND_ATTRIBUTES SidBuffer;
	ULONG ReturnLength;
	UNICODE_STRING SidString;
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 

	//
	// 1. У������Ϸ���
	//
	
	if ( NULL == RegisterUserID )
	{
		dprintf( "error! | RtlGetUserSid(); | Invalid Paramaters; failed! \n" );
		return status ;
	}

	//
	// 2. �򿪽��̻��߳�,��ȡ��Ӧ��Token
	//

	if ( (HANDLE)0xFFFFFFFE != ProcessHandle )
	{
		status = ZwOpenProcessToken ( ProcessHandle, TOKEN_QUERY, &TokenHandle );
	}
	else
	{
		// Open the thread token
		status = ZwOpenThreadToken ( (HANDLE)0xFFFFFFFE, TOKEN_QUERY, TRUE, &TokenHandle );
		if ( ! NT_SUCCESS(status) )
		{
			// We failed, is it because we don't have a thread token? 
			if (status != STATUS_NO_TOKEN) 
			{
				dprintf( "error! | RtlGetUserSid() - ZwOpenThreadToken() | status = 0x%08lx \n", status );
				return status ;
			}

			// It is, so use the process token
			status = ZwOpenProcessToken ( NtCurrentProcess(), TOKEN_QUERY, &TokenHandle );
		}
	}

	if ( ! NT_SUCCESS(status) ) 
	{
		dprintf( "error! | RtlGetUserSid() - ZwOpenProcessToken() | status = 0x%08lx \n", status );
		return status ;
	}

	//
	// 3. ��ѯToken�����ľ�����Ϣ
	//

	SidBuffer = (PSID_AND_ATTRIBUTES)Buffer ;
	status = ZwQueryInformationToken (
		TokenHandle,
		TokenUser,
		(PVOID)SidBuffer,
		sizeof(Buffer),
		&ReturnLength
		);

	// Close the handle and handle failure
	ZwClose( TokenHandle );
	if ( ! NT_SUCCESS(status) )
	{
		dprintf( "error! | RtlGetUserSid() - ZwQueryInformationToken() | status = 0x%08lx \n", status );
		return status ;
	}

	//
	// 4. ת����SID��ʽ
	//

	status = RtlConvertSidToUnicodeString( &SidString, SidBuffer[0].Sid, TRUE );
	if ( ! NT_SUCCESS(status) )
	{
		dprintf( "error! | RtlGetUserSid() - RtlConvertSidToUnicodeString() | status = 0x%08lx \n", status );
		return status ;
	}

	wcscpy( RegisterUserID, SidString.Buffer );
	RtlFreeUnicodeString( &SidString ); // �������һ��Ҫʧ���ڴ�

	return STATUS_SUCCESS ;
}



BOOL 
ProcessIdToSessionId(
	IN DWORD dwProcessId OPTIONAL,
	OUT PULONG pSessionId
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/02 [2:6:2010 - 17:00]

Routine Description:
  �õ�ָ�����̶�Ӧ��SessionId
    
Arguments:
  dwProcessId - ָ���Ľ���,Ϊ����Ϊ�������
  pSessionId - �������ȡ��SessionId

--*/
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	PROCESS_SESSION_INFORMATION SessionInformation;
	OBJECT_ATTRIBUTES ObjectAttributes;
	CLIENT_ID ClientId;
	HANDLE ProcessHandle;
	ULONG ReturnLength = 4 ; 

	//
	// 1. У������Ϸ���
	//
	
	if ( NULL == pSessionId )
	{
		dprintf( "error! | ProcessIdToSessionId(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	//
	// 2. ȷ�����̾��
	//

	if ( 0 == dwProcessId )
	{
		ProcessHandle = NtCurrentProcess() ;
	}
	else
	{
		ClientId.UniqueProcess	= (HANDLE)dwProcessId ;
		ClientId.UniqueThread	= 0 ;

		InitializeObjectAttributes( &ObjectAttributes, NULL, 0, NULL, NULL );

		status = ZwOpenProcess(
			&ProcessHandle,
			PROCESS_QUERY_INFORMATION,
			&ObjectAttributes,
			&ClientId
			);

		if( ! NT_SUCCESS(status) )
		{
			dprintf( "error! | ProcessIdToSessionId() - ZwOpenProcess() | status = 0x%08lx \n", status );
			return FALSE ;
		}
	}

	//
	// 3. ��ѯ���̾����Ӧ��SessionId
	//

	status = ZwQueryInformationProcess(
		ProcessHandle,
		ProcessSessionInformation,
		&SessionInformation,
		sizeof(SessionInformation),
		&ReturnLength
		);

	if( NT_SUCCESS(status) )
	{
		*pSessionId = SessionInformation.SessionId;
		return TRUE;
	}

	return FALSE ;
}



NTSTATUS
Reduce_TokenPrivilegeGroups (
	IN PVOID _pNode,
	IN HANDLE Token,
	IN ACCESS_MASK GrantedAccess
	)
{
	NTSTATUS status = STATUS_SUCCESS ; 
	PTOKEN_GROUPS		LocalGroups		= NULL ;
	PTOKEN_PRIVILEGES	LocalPrivileges = NULL ;
	LPPDNODE pNode = (LPPDNODE) _pNode ;

	//
	// 1. У������Ϸ���
	//
	
	if ( NULL == Token || NULL == pNode )
	{
		dprintf( "error! | Reduce_TokenPrivilegeGroups(); | Invalid Paramaters; failed! \n" );
		return status ;
	}

	//
	// 2. ��Ȩ
	//
	
	if ( GrantedAccess & 4 )
	{
		status = SeQueryInformationToken( Token, TokenGroups, &LocalGroups );
		if ( NT_SUCCESS(status) )
		{
			status = SeQueryInformationToken( Token, TokenPrivileges, &LocalPrivileges );
			if ( NT_SUCCESS(status) )
			{
				if ( TRUE == DropAdminRights_phrase2( LocalPrivileges, LocalGroups, pNode->bDropAdminRights ) ) 
				{
					status = STATUS_ACCESS_DENIED;
				}
			}
			else
			{	
				dprintf( "error! | Reduce_TokenPrivilegeGroups() - SeQueryInformationToken(); | (status=0x%08lx) \n", status );
			}
		} 
		else 
		{	
			dprintf( "error! | Reduce_TokenPrivilegeGroups() - SeQueryInformationToken(); | (status=0x%08lx) \n", status );
		}

		kfree( LocalGroups );
		kfree( LocalPrivileges );
	}

	return status ;
}


///////////////////////////////   END OF FILE   ///////////////////////////////