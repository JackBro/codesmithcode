/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/05/24 [24:5:2010 - 16:24]
* MODULE : \SandBox\Code\Project\ProteinBoxDrv\ProcessNofity.c
* 
* Description:
*      
*   ���̻ص����                       
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "Notify.h"
#include "ProcessNofity.h"
#include "ProcessData.h"
#include "Common.h"

//////////////////////////////////////////////////////////////////////////

NOTIY_INFO g_ProcessNotify_Info = { FALSE, NULL } ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                       ���̻ص�����                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


VOID
ProcessNotifyRoutine (
    IN HANDLE   hParentId,
    IN HANDLE   hProcessId,
    IN BOOLEAN  Create
    )
{
	BOOL bRet = FALSE ;
    PEPROCESS  pParent	= NULL ;
    PEPROCESS  pProcess = NULL ;
    NTSTATUS   status	= STATUS_UNSUCCESSFUL ;
	LPPDNODE   pNodeParent = NULL, pNode = NULL ;
	PNODE_INFO_C pNode_c = NULL ;

	if ( FALSE == g_Driver_Inited_phrase1 || FALSE == g_bMonitor_Notify ) { return ; }

    __try
    {
        status = PsLookupProcessByProcessId( hProcessId, &pProcess );

        if ( NT_SUCCESS(status) )
        {
			PsLookupProcessByProcessId( hParentId, &pParent );
        }

		if ( TRUE == Create )
		{
			//
			// ���̴���
			//

			if ( NULL == hParentId || NULL == hProcessId ) { return ; }

			// 1. ����PID��Ӧ�Ľ����ܽڵ�
			pNodeParent = (LPPDNODE) kgetnodePD( hParentId );
			if ( NULL == pNodeParent ) { return ; }
			if ( 1 == pNodeParent->bDiscard ) { return ; }

			// 2.1 �������̽ڵ�
			pNode = (LPPDNODE) kbuildnodePD( hProcessId, FALSE );
			if ( NULL == pNode )
			{
				dprintf( "error! | ProcessNotifyRoutine() - kbuildnodePD(); | �������̽ڵ�ʧ��(PID=%d) \n", hProcessId );
				return ;
			}

			// 2.2 ���C�ڵ�
			pNode->pNode_C = pNode_c = PDCopyNodeC( (PVOID)pNodeParent->pNode_C );
			if ( NULL == pNode_c )
			{
				dprintf( "error! | ProcessNotifyRoutine() - PDCopyNodeC(); | ���ƽ��̽ڵ�Cʧ��(PID=%d) \n", hProcessId );
				kfree( pNode ); // ��δ��������,��ʼ��ʧ�����ͷ��ڴ�
				return ;
			}

			// 2.3 ���������Ԫ
			pNode->XIpcPath.bFlag_Denny_Access_KnownDllsSession = pNodeParent->XIpcPath.bFlag_Denny_Access_KnownDllsSession ;

			kInsertTailPD( pNode );
#if DBG
			{
				LPWSTR lpImageFileShortName = NULL ;
				PUNICODE_STRING lpImageFileName = NULL ;

				// �õ�PID��Ӧ�Ľ���·��,�����߸����ͷ��ڴ�
				bRet = GetProcessImageFileName( hProcessId, &lpImageFileName, &lpImageFileShortName );
				if ( bRet ) 
				{ 
					dprintf( "[ProcessCreate] ��ɳ��������������ڵ�(PID=%d, Name:%ws) \n", hProcessId, lpImageFileShortName );
					kfree( (PVOID)lpImageFileName ); // �ͷ��ڴ�
				}
			}
#endif
		}
		else
		{
			//
			// ��������
			//

			if ( NULL == hProcessId ) { return ; }
			bRet = kRemovePD( hProcessId ); // �ӽ����������Ƴ���ǰ�ڵ�
#if DBG
			if ( TRUE == bRet )
			{
				dprintf( "[ProcessDelete] ��ɳ����������Ƴ��ڵ�(PID=%d) \n", hProcessId );
			}
#endif
			kCheckPD( hProcessId ); // У�������������PID�ĺϷ���
		}
    }
    __finally
    {
        if ( pParent )
        {
            ObDereferenceObject( (PVOID) pParent );

            pParent = NULL;
        }

        if ( pProcess )
        {
            ObDereferenceObject( (PVOID) pProcess );

            pProcess = NULL;
        }
    }

	return ;
}



BOOL 
CheckProcessNotifyState (
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/12 [12:5:2010 - 11:50]

Routine Description:
  ���ģ��ص���״̬

Return Value:
  TRUE - ��ע��; FALSE - δע��
    
--*/
{
	BOOL bNotifyState = g_ProcessNotify_Info.bNotifyState ;

	if ( NULL == g_ProcessNotify_Info.NotifyRoutine )
	{
		g_ProcessNotify_Info.NotifyRoutine = (PVOID) ProcessNotifyRoutine ;
	}

	return bNotifyState ;
}



NTSTATUS
__PsSetCreateProcessNotifyRoutine (
	IN PVOID NotifyRoutine
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 

	dprintf( "call __PsSetCreateProcessNotifyRoutine(); \n" );

	return status ;
}

///////////////////////////////   END OF FILE   ///////////////////////////////