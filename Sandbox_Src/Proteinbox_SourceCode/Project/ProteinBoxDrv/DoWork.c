/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/05/10 [10:5:2010 - 14:01]
* MODULE : D:\Work\Program\Coding\ɳ��\SandBox\Code\!TestCode\ImageNotifyDll\DoWork.c
* 
* Description:
*   
*   �ɻ��ģ��                   
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "DoWork.h"
#include "Version.h"
#include "Memory.h"
#include "Security.h"
#include "SecurityData.h"
#include "SdtData.h"
#include "SSDT.h"
#include "ShadowSSDT.h"
#include "Notify.h"
#include "ProcessData.h"
#include "RegHiveData.h"
#include "ObjectHook.h"
#include "ObjectData.h"
#include "Config.h"
#include "ShadowSSDT.h"
#include "HookEngine.h"
#include "SandboxsList.h"
#include "ForceRunList.h"

//////////////////////////////////////////////////////////////////////////

BOOL g_Driver_Inited_phrase1 = FALSE ;


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
DoWork (
	BOOL bWork
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/12 [12:5:2010 - 11:50]

Routine Description:
  �ɻ�ĺ���

Arguments:
  bWork - [IN] �Ƿ�ɻ�?

--*/
{
	if ( TRUE == bWork )
	{	
		Starting();
	}
	else
	{
		Stoping();
	}

	return ;
}



VOID
Starting (
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ;
	BOOL	 bRet	= FALSE ;

	// 1. ��ʼ��ϵͳ��Ϣ
	bRet = GetVersion () ;
	if ( FALSE == bRet )
	{
		dprintf( "error! | DoWork() - GetVersion(); \n" );
		return ;
	}

	GetProcessNameOffset();

	// 2.1 ��ʼ���ڴ������
	if ( FALSE == InitMemoryManager() )
	{
		dprintf( "error! | DoWork() - InitMemoryManager(); \n" );
		return ;
	}

	// 2.2 ��ʼ�����̽ڵ������
	if ( FALSE == InitProcessData() )
	{
		dprintf( "error! | DoWork() - InitProcessData(); \n" );
		return ;
	}

	// 2.3 ��ʼ��SID�ڵ������
	if ( FALSE == InitSecurityData() )
	{
		dprintf( "error! | DoWork() - InitSecurityData(); \n" );
		return ;
	}

	// 2.4 ��ʼ��RegHive�ڵ������
	if ( FALSE == InitRegHive() )
	{
		dprintf( "error! | DoWork() - InitRegHive(); \n" );
		return ;
	}

	// 2.5 ��ʼ��ObjectHook�ڵ������
	if ( FALSE == InitObjectData() )
	{
		dprintf( "error! | DoWork() - InitObjectData(); \n" );
		return ;
	}

	// 2.6 ��ʼ�������ļ������Ϣ
	if ( FALSE == InitConfig() )
	{
		dprintf( "error! | DoWork() - InitConfig(); \n" );
		return ;
	}

	// 2.7 ��ʼ��Inline Hook����
	if ( FALSE == LoadInlineHookEngine() )
	{
		dprintf( "error! | DoWork() - LoadInlineHookEngine(); \n" );
		return ;
	}

	// 2.8 ��ʼ��ɳ��'s ����
	if ( FALSE == InitSandboxsData() )
	{
		dprintf( "error! | DoWork() - InitSandboxsData(); \n" );
		return ;
	}

	// 2.9 ��ʼ��ǿ�����й���'s ����
	if ( FALSE == InitForceProcData() )
	{
		dprintf( "error! | DoWork() - InitForceProcData(); \n" );
		return ;
	}

	//
	// 3. ����һ����ȫ������
	//

	bRet = CreateAcl() ;
	if ( FALSE == bRet )
	{
		dprintf( "error! | DoWork() - CreateAcl(); \n" );
		return ;
	}

	//
	// 4. ����ϵͳ���̵ľ��,����һ����Ȩ��
	//

	bRet = AdjustPrivilege() ;
	if ( FALSE == bRet )
	{
		dprintf( "error! | DoWork() - AdjustPrivilege(); \n" );
		return ;
	}

	//
	// 5. ׼��SSDT�������
	//

	bRet = HandlerSSDT() ;
	if ( FALSE == bRet )
	{
		dprintf( "error! | DoWork() - HandlerSSDT(); \n" );
		return ;
	}

	//
	// 6. �������ֻص�; (ģ��/����)
	//

	bRet = SetNotify( TRUE ) ;
	if ( FALSE == bRet )
	{
		dprintf( "error! | DoWork() - SetNotify(); \n" );
		return ;
	}

	//
	// 7. ׼��Object�������
	//

	bRet = HandlerObject () ;
	if ( FALSE == bRet )
	{
		dprintf( "error! | DoWork() - HandlerObject(); \n" );
		return ;
	}

	g_Driver_Inited_phrase1 = TRUE ;
	return ;
}



VOID
Stoping (
	)
{
	dprintf( " --------------- Unload Driver  --------------- \n" );

	SetNotify( FALSE ) ; // ж�ظ��ֻص�; (ģ��/����)
	
	UnhookSSDT();
	UnhookShadowSSDT() ;
	UnloadInlineHookEngine();

	kfreeCD() ; // �ͷ������ļ������Ϣ
	kfreeOD() ; // �ͷ�ObjectHookData�ڵ�����
	kfreeRH() ; // �ͷ�RegHive�ڵ�����
	kfreeSD() ; // �ͷ�SID�ڵ�����
	kfreePD() ; // �ͷŽ��̽ڵ������
	kfreeMP() ; // �ͷ�MappedPE ����ڴ�����
	kfreeSBL() ;
	kfreeFPL() ;
	kfreeMM() ; // �ͷ��ڴ������ [����������һ��]

	return ;
}


///////////////////////////////   END OF FILE   ///////////////////////////////
