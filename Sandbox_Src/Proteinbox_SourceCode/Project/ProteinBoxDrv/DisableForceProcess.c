/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/05/25 [25:5:2010 - 17:42]
* MODULE : d:\Work\Program\Coding\ɳ��\SandBox\Code\Project\ProteinBoxDrv\StartProcess.c
* 
* Description:
*      
*   ��ǿ��������� [����д�ĺ��Ѷ���, 0 0!]                
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "Common.h"
#include "DispatchIoctl.h"
#include "ProcessData.h"
#include "SandboxsList.h"
#include "ConfigData.h"
#include "ForceRunList.h"
#include "DisableForceProcess.h"

//////////////////////////////////////////////////////////////////////////

#define MAX_FORPATH_LENGTH 0x2000

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
Ioctl_DisableForceProcess (
	IN PVOID pNode,
	IN PVOID pInBuffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2011/07/11 [11:7:2011 - 16:20]

Routine Description:
  ���� ǿ�Ƴ������н��� �Ŀ���  
    
--*/
{
	BOOL bRet = FALSE, bRunInSandbox = FALSE, bNeedForcePath = FALSE ;
	LPWSTR szFullImageName =  NULL ;
	WCHAR SeactionName[ MAX_PATH ] = L"GlobalSetting" ;
	LPIOCTL_DISABLEFORCEPROCESS_BUFFER Buffer = NULL ;
	LPFORCEPROC_NODE_INFO lpData = NULL ;

	dprintf( "[ProteinBoxDrv] IOCTL_PROTEINBOX - Ioctl_DisableForceProcess(); \n" );

	if ( NULL == pInBuffer ) { return STATUS_NOT_IMPLEMENTED ; }

	__try
	{
		Buffer = (LPIOCTL_DISABLEFORCEPROCESS_BUFFER) getIoctlBufferBody( pInBuffer );
		if ( FALSE == MmIsAddressValid( (PVOID)Buffer ) )
		{
			dprintf( "error! | Ioctl_DisableForceProcess() - getIoctlBufferBody(); | Body��ַ���Ϸ� \n" );
			__leave ;
		}

		// �ж�Flag��ֵ
		if ( _FLAG_IOCTL_DISABLEFORCEPROCESS_EnableForceALL_ == Buffer->Flag )
		{
			g_bForceAll2RunInSandbox = TRUE ;
			g_bAllRunOutofSandbox	 = FALSE ;

			kEnableAllFPL();
		}
		else if ( _FLAG_IOCTL_DISABLEFORCEPROCESS_DisableForceALL_ == Buffer->Flag )
		{
			g_bForceAll2RunInSandbox = FALSE ;
			g_bAllRunOutofSandbox    = TRUE ;

			kDisableAllFPL();
		}
		else if ( _FLAG_IOCTL_DISABLEFORCEPROCESS_EnableForce_ == Buffer->Flag )
		{
			g_bAllRunOutofSandbox	 = FALSE ;
			bNeedForcePath = TRUE;
			bRunInSandbox = TRUE ;
		} 
		else if ( _FLAG_IOCTL_DISABLEFORCEPROCESS_DisableForce_ == Buffer->Flag )
		{
			g_bForceAll2RunInSandbox = FALSE ;
			bNeedForcePath = TRUE;
			bRunInSandbox = FALSE ;
		}
		else
		{
			dprintf( "error! | Ioctl_DisableForceProcess(); | ���Ϸ���Flag:%n \n", Buffer->Flag );
			return STATUS_NOT_IMPLEMENTED;
		}

		if ( FALSE == bNeedForcePath )
		{
			return STATUS_SUCCESS;
		}

		// ��Ҫȡ������ȫ·��
		ProbeForRead( Buffer->szProcName, Buffer->NameLength, 2 );

		if ( Buffer->NameLength < 0 || Buffer->NameLength > MAX_FORPATH_LENGTH )
		{
			dprintf( "error! | Ioctl_DisableForceProcess((); | �ַ������ȳ�������:length=%n \n", Buffer->NameLength );
			__leave ;
		}

		szFullImageName = (LPWSTR) kmalloc( MAX_FORPATH_LENGTH );
		if ( NULL == szFullImageName ) { return STATUS_UNSUCCESSFUL; }

		memcpy( szFullImageName, Buffer->szProcName, Buffer->NameLength );

		// �������ļ���ƥ��·��
		bRet = kIsValueNameExist( SeactionName, L"ForceProcess", szFullImageName );
		if ( FALSE == bRet )
		{
			bRet = kIsValueNameExist( SeactionName, L"ForceFolder", szFullImageName );
		}

		if ( FALSE == bRet )
		{
			dprintf( "error! | Ioctl_DisableForceProcess() | δƥ�䵽ǿ�����е��ļ�·��,������Ч \n" );
			kfree( szFullImageName );
			return STATUS_UNSUCCESSFUL ;
		}

		// �½��ڵ�
		lpData = kbuildnodeFPL( szFullImageName, Buffer->NameLength );
		if ( NULL == lpData ) 
		{
			dprintf( "error! | Ioctl_DisableForceProcess() - kbuildnodeFPL(); | �½��ڵ�ʧ��(%ws) \n", szFullImageName );
			kfree( szFullImageName );
			return STATUS_UNSUCCESSFUL ;
		}

		lpData->bRunInSandbox = bRunInSandbox ;	
		kfree( szFullImageName );
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		dprintf( "error! | Ioctl_DisableForceProcess() - __try __except(); | ProbeForWrite��Ӧ�ĵ�ַ���Ϸ� \n" );
		return STATUS_UNSUCCESSFUL ;
	}

	return STATUS_SUCCESS ;
}



///////////////////////////////   END OF FILE   ///////////////////////////////