/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/05/24 [24:5:2010 - 15:43]
* MODULE : d:\Work\Program\Coding\ɳ��\SandBox\Code\Project\ProteinBoxDrv\Notify.c
* 
* Description:
*      
*   ���ֻص�����ģ��                       
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "Notify.h"
#include "ProcessNofity.h"
#include "ImageNotify.h"
#include "InjectDll.h"


//////////////////////////////////////////////////////////////////////////

BOOL g_bMonitor_Notify = TRUE ; // ��R3��֮R0,�����е��� Ioctl_HookShadow() ����,�ᱻ��TRUE,��������Notify���


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
SetNotify (
	IN BOOL bInstall
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/24 [24:5:2010 - 15:46]

Routine Description:
  �������ֻص�; (ģ��/����)
    
Arguments:
  bInstall - TRUE - ��װ; FALSE - ж�� 

--*/
{
	BOOL bRet = FALSE ;

	if ( TRUE == bInstall )
	{
		// 1. ����ģ��ص�
		bRet = SetImageNotify( TRUE );
		if ( FALSE == bRet )
		{
			dprintf( "error! | SetNotify() - SetImageNotify(); | \n" );
			return FALSE ;
		}

		// 2. �������̻ص�
		bRet = SetProcessNotify( TRUE );
		if ( FALSE == bRet )
		{
			dprintf( "error! | SetNotify() - SetProcessNotify(); | \n" );
			return FALSE ;
		}

		dprintf( "ok! | �������ֻص�; (ģ��/����) \n" );
	}
	else
	{
		// ж�ظ��ֻص�
		bRet = SetImageNotify( FALSE );
		bRet = SetProcessNotify( FALSE );

		dprintf( "ok! | ж�ظ��ֻص�; (ģ��/����) \n" );
	}

	return bRet ;
}



BOOL
SetImageNotify (
	IN BOOL bInstall
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/24 [24:5:2010 - 15:46]

Routine Description:
  ����ģ��ص�    
    
Arguments:
  bInstall - TRUE - ��װ; FALSE - ж�� 

--*/
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ;

	//
	// ע��ģ����ػص�
	//

	if ( TRUE == bInstall )
	{	
		if ( TRUE == CheckImageNotifyState() ) { return TRUE ; }

		// ��ʼ����ȡSSDT������ַ
		if ( FALSE == Get_ZwProtectVirtualMemory_addr() ) { return FALSE ; }	

		// ע��ģ��ص�
		status = PsSetLoadImageNotifyRoutine( g_ImageNotify_Info.NotifyRoutine );
		if( ! NT_SUCCESS( status ) )
		{
			dprintf( "error! | SetImageNotify() - PsSetLoadImageNotifyRoutine(); | (status=0x%08lx) \n", status );
			g_ImageNotify_Info.bNotifyState = FALSE ;
			return FALSE ;
		}

		g_ImageNotify_Info.bNotifyState = TRUE ;
		dprintf( "ok! | Set ImageNotify \n" );
	}

	//
	// ж��ģ����ػص�
	//

	else
	{
		if ( FALSE == CheckImageNotifyState() ) { return TRUE ; }

		PsRemoveLoadImageNotifyRoutine( g_ImageNotify_Info.NotifyRoutine );

		g_ImageNotify_Info.bNotifyState  = FALSE	;
		g_ImageNotify_Info.NotifyRoutine = NULL		;

		dprintf( "ok! | Remove ImageNotify \n" );
	}

	return TRUE ;
}



BOOL
SetProcessNotify (
	IN BOOL bInstall
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/24 [24:5:2010 - 15:46]

Routine Description:
  �������̻ص�    
    
Arguments:
  bInstall - TRUE - ��װ; FALSE - ж�� 

--*/
{

	NTSTATUS status = STATUS_UNSUCCESSFUL ;

	//
	// ע����̼��ػص�
	//

	if ( TRUE == bInstall )
	{	
		if ( TRUE == CheckProcessNotifyState() ) { return TRUE ; }

		// ע��ģ��ص�
		status = PsSetCreateProcessNotifyRoutine( g_ProcessNotify_Info.NotifyRoutine, FALSE );

		if ( STATUS_INVALID_PARAMETER == status )
		{
			status = __PsSetCreateProcessNotifyRoutine( g_ProcessNotify_Info.NotifyRoutine );
		}

		if( ! NT_SUCCESS( status ) )
		{
			dprintf( "error! | SetProcessNotify() - PsSetCreateProcessNotifyRoutine(); | (status=0x%08lx) \n", status );
			g_ProcessNotify_Info.bNotifyState = FALSE ;
			return FALSE ;
		}

		g_ProcessNotify_Info.bNotifyState = TRUE ;
		dprintf( "ok! | Set ProcessNotify \n" );
	}

	//
	// ж�ؽ��̼��ػص�
	//

	else
	{
		if ( FALSE == CheckProcessNotifyState() ) { return TRUE ; }

		PsSetCreateProcessNotifyRoutine( g_ProcessNotify_Info.NotifyRoutine, TRUE );

		g_ProcessNotify_Info.bNotifyState  = FALSE	;
		g_ProcessNotify_Info.NotifyRoutine = NULL	;

		dprintf( "ok! | Remove ProcessNotify \n" );
	}

	return TRUE ;
}



///////////////////////////////   END OF FILE   ///////////////////////////////