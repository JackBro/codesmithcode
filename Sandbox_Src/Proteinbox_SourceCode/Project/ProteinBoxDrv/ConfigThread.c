/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/07/06 [6:7:2010 - 16:04]
* MODULE : d:\Work\Program\Coding\ɳ��\SandBox\Code\Project\ProteinBoxDrv\ConfigThread.c
* 
* Description:
*      
*   ����һ���߳�,���һֱ�ȴ�R3; Ӧ�ò����ù����������һ���¼�������,Ȼ��������Ϣ������������                       
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "ConfigThread.h"
#include "Common.h"
#include "Config.h"
#include "ProcessData.h"

//////////////////////////////////////////////////////////////////////////

#define g_ConfEvent_InitConfigData_wakeup_R0	 L"\\BaseNamedObjects\\Global\\Proteinbox_ConfEvent_InitConfigData_wakeup_R0"
#define g_ConfEvent_InitConfigData_wakeup_R3	 L"\\BaseNamedObjects\\Global\\Proteinbox_ConfEvent_InitConfigData_wakeup_R3"


THREAD_INFO g_tmpThread_info ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+						��ʼϵͳ�߳�                          +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--

VOID 
CreateConfigThread(
	IN BOOL bTerminate
	) 
{
	HANDLE ThreadHandle ;
	BOOL bRet = FALSE ;
	BOOL bOK = FALSE ;
	
	if ( FALSE == bTerminate ) { // ��������
		
		if (!NT_SUCCESS(PsCreateSystemThread (&ThreadHandle,
			THREAD_ALL_ACCESS,
			NULL,
			0L,
			NULL,
			ConfigThread,
			NULL))
		   ) 
		{
			dprintf("CreateConfigThread() -->PsCreateSystemThread. Failed\n");
			return;
		}
		
		ObReferenceObjectByHandle (ThreadHandle,
			THREAD_ALL_ACCESS,
			NULL,
			KernelMode,
			(PVOID *)&g_tmpThread_info.pThreadObj,
			NULL);
		
		//
		// ��ʼ���ý��̵Ľṹ��
		//

		g_tmpThread_info.bIamRunning = TRUE ;
		ZwClose( ThreadHandle );
	} 
	else // ���ٽ���
	{  	
		if ( FALSE == g_tmpThread_info.bIamRunning ) { return ; } 

		// ���߳�û�н���,��ȴ������,����ͷ��ڴ�
		g_tmpThread_info.bIamRunning = FALSE ;

		ZwSetEvent( g_tmpThread_info.hEvent, 0 );  // ����Ϊ"����"״̬,���ѵȴ����߳�
		
		if ( g_tmpThread_info.pThreadObj != NULL ) { // �ȴ��̵߳Ľ���
			KeWaitForSingleObject( g_tmpThread_info.pThreadObj, Executive, KernelMode, FALSE, NULL );	
		}
		
		if ( g_tmpThread_info.pThreadObj != NULL ) { // �� thread object����,�ͷŵ�,Ҫ����BSOD
			ObDereferenceObject( g_tmpThread_info.pThreadObj );
			g_tmpThread_info.pThreadObj = NULL;
		}
	}
	
	return ;
}



VOID
ConfigThread (
	IN PVOID StartContext
	)
{	
	BOOL bRet = FALSE ;

	// �򿪵ȴ��¼�
	bRet = OpenEvent( g_ConfEvent_InitConfigData_wakeup_R0, EVENT_ALL_ACCESS, &g_tmpThread_info.hEvent );
	if( FALSE == bRet )
	{
		dprintf( "error! | ConfigThread() - OpenEvent(); | \"%s\" \n", g_ConfEvent_InitConfigData_wakeup_R0 );
		goto _OVER_ ;
	}

	// �ȴ��¼���һֱ��"����"״̬
	dprintf( "[PB] I'm Waiting for Config Data ... \n" );
	ZwWaitForSingleObject( g_tmpThread_info.hEvent, FALSE, NULL );

	if ( FALSE == g_tmpThread_info.bIamRunning ) { goto _CLEAR_ ; }
	
	// ������δ��ɳ�ʼ������,��ǰ״̬Ϊ����Ӧ�κβ���
	if ( FALSE == g_Driver_Inited_phrase1 )
	{
		dprintf( "error! | ConfigThread(); | ���յ�R3������������,������δ��ɳ�ʼ������,��ǰ״̬Ϊ����Ӧ�κβ���. \n" );
		goto _CLEAR_ ; 
	}

	//
	// ������Ҫ��ȡ�����������ļ���Ϣ,���������¼�����Ӧ�ò�ĵȴ��߳�,
	// R3��ȫ���������׸�R0�������һ�ȴ��¼�,�����������ȴ�,��ɳ�ʼ��.
	//

	bRet = LoadConfig();
	if( FALSE == bRet )
	{
		dprintf( "error! | ConfigThread() - LoadConfig(); | \n" );
		goto _CLEAR_ ;
	}

	if ( NULL == g_ProteinBox_Conf )
	{
		// �������״μ��������ļ�
		g_ProteinBox_Conf = g_ProteinBox_Conf_TranshipmentStation ;
		dprintf( "ok! | ConfigThread(); | �����ļ���ʼ���ɹ�! \n" );
	}

	if ( g_ProteinBox_Conf != g_ProteinBox_Conf_TranshipmentStation )
	{
		// ������Reload�����ļ��Ĳ���, �ͷŵ��ɵ���������
		g_ProteinBox_Conf = g_ProteinBox_Conf_TranshipmentStation ;

		CDDistroyAllEx( (PVOID)g_ProteinBox_Conf_Old );
		dprintf( "ok! | ConfigThread(); | �����ļ�Reload�ɹ�! \n" );
	}

	kWalkCD();
//	GetPBDllPathFromIni( NULL, TRUE ); // Proteinboxdll.dll��ȫ·��ͨ��ע����ȡ,�Ͳ�����INI��������,̫�鷳

_CLEAR_ :
	ZwClose ( g_tmpThread_info.hEvent );

_OVER_ :
	if ( g_ProteinBox_Conf != g_ProteinBox_Conf_TranshipmentStation )
	{
		// ����¼�û�гɹ�,��ô��ʱ�����g_ProteinBox_Conf_TranshipmentStation����ڴ�����ͷ�
		CDDistroyAllEx( (PVOID)g_ProteinBox_Conf_TranshipmentStation );
	}

	g_tmpThread_info.bIamRunning = FALSE ;
	SetEvent( g_ConfEvent_InitConfigData_wakeup_R3, EVENT_MODIFY_STATE ); // �������,��Ҫ����ȴ���R3

	// Ϊ�������н��̽���б����������Ϣ���£������������ɳ����̣�������е��������ݡ�
	kConfResetPD();

	dprintf( "ConfigThread(); | ϵͳ�̹߳������,�˳�֮. \n" );
	PsTerminateSystemThread( STATUS_SUCCESS );
}


/////////////////////////////////////// END OF FILE ///////////////////////////////////////////