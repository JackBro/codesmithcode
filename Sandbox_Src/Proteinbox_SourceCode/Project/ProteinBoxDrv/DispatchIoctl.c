/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/05/25 [25:5:2010 - 14:38]
* MODULE : d:\Work\Program\Coding\ɳ��\SandBox\Code\Project\ProteinBoxDrv\DispatchIoctl.c
* 
* Description:
*      
*   �������IOCTL��R3��������ģ��                       
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "ProcessData.h"
#include "DispatchIoctl.h"
#include "GetVersion.h"

//////////////////////////////////////////////////////////////////////////

IOCTL_PROTEINBOX_FUNCTIONS_DATA g_IOCTL_PROTEINBOX_FuncData = { FALSE, 0, NULL }  ;


static IOCTL_PROTEINBOX_FUNCTIONS _ioctl_proteinbox_functions_array_ [] = 
{
	{ FLAG_Ioctl_GetSysVersion,		Ioctl_GetSysVersion		},
	{ FLAG_Ioctl_StartProcess,		Ioctl_StartProcess		},
	{ FLAG_Ioctl_QueryProcess,		Ioctl_QueryProcess		},
	{ FLAG_Ioctl_QueryProcessPath,	Ioctl_QueryProcessPath  },
	{ FLAG_Ioctl_QueryBoxPath,		Ioctl_QueryBoxPath		},
	{ FLAG_Ioctl_HandlerConf,		Ioctl_HandlerConf		},
	{ FLAG_Ioctl_HookShadowSSDT,	Ioctl_HookShadow		},
	{ FLAG_Ioctl_HookObject,		Ioctl_HookObject		},
	{ FLAG_Ioctl_GetInjectSaveArea,	Ioctl_GetInjectSaveArea	},
	{ FLAG_Ioctl_WhiteOrBlack,		Ioctl_WhiteOrBlack		},
	{ FLAG_Ioctl_CreateDirOrLink,	Ioctl_CreateDirOrLink	},
	{ FLAG_Ioctl_DuplicateObject,	Ioctl_DuplicateObject	},
	{ FLAG_Ioctl_GetFileName,		Ioctl_GetFileName		},
	{ FLAG_Ioctl_EnumBoxs,			Ioctl_EnumBoxes			},
	{ FLAG_Ioctl_DisableForceProcess, Ioctl_DisableForceProcess	},
	{ FLAG_Ioctl_EnumProcessEx,		Ioctl_EnumProcessEx		},


};


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                          ������							  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--



NTSTATUS
Handler_DispacthIoctl_PROTEINBOX(
	IN PVOID pInBuffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/25 [25:5:2010 - 14:41]

Routine Description:
  ����R3ͨ�� IOCTL_PROTEINBOX ������������     
    
Arguments:
  pInBuffer - R3����

--*/
{
	LPPDNODE pCurrentNode = NULL ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ; 
	static ULONG Flag = 0, i = 0, TotalCounts = 0 ; 
	static LPIOCTL_PROTEINBOX_FUNCTIONS pArray = NULL ;
	PIOCTL_PROTEINBOX_BUFFER pData = (PIOCTL_PROTEINBOX_BUFFER) pInBuffer ;

	//
	// 1. У������Ϸ���
	//
	
	if ( FALSE == g_Driver_Inited_phrase1 )
	{
		dprintf( "error! | Handler_DispacthIoctl_PROTEINBOX(); | ����δ��ɳ�ʼ������,��ǰ״̬Ϊ����Ӧ�κβ���. \n" );
		return STATUS_UNSUCCESSFUL ;
	}

	if ( NULL == pInBuffer )
	{
		dprintf( "error! | Handler_DispacthIoctl_PROTEINBOX(); | Invalid Paramaters; failed! \n" );
		return STATUS_INVALID_PARAMETER ;
	}

	//
	// 2. ����PID��Ӧ�Ľ����ܽڵ�
	//

	pCurrentNode = (LPPDNODE) kgetnodePD( 0 );
	if ( pCurrentNode && pCurrentNode->bDiscard )
	{
		dprintf( "error! | Handler_DispacthIoctl_PROTEINBOX() - Find_node_from_pid(); | NULL == pCurrentNode \n" );
		return STATUS_PROCESS_IS_TERMINATING ;
	}

	//
	// 3. ����Flagֵ������Ӧ�Ĵ�����
	//

	Init__IOCTL_PROTEINBOX_FuncData() ;

	__try
	{
	//	ProbeForRead( pInBuffer, sizeof(IOCTL_PROTEINBOX_BUFFER), 1 );

		// ȷ��Flag�ĺϷ���
		Flag = pData->Head.Flag ;
		if ( IS_INVALID_IOCTL_FLAG( Flag ) )
		{
			dprintf( "error! | Handler_DispacthIoctl_PROTEINBOX() - IS_INVALID_IOCTL_FLAG(); | (Flag=0x%x) \n", Flag );
			return STATUS_INVALID_PARAMETER ;
		}

		// ��������,�ҵ�Flag��Ӧ�Ĵ�����
		pArray		= g_IOCTL_PROTEINBOX_FuncData.pArray ;
		TotalCounts = g_IOCTL_PROTEINBOX_FuncData.TotalCounts ;

		for( i=0; i<TotalCounts; i++ )
		{
			if ( Flag == pArray[ i ].Flag )
			{
				status = ((_Ioctl_func_)pArray[ i ].FunctionAddr)( pCurrentNode, pInBuffer );
				return status ;
			}
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	    dprintf( "error! | Handler_DispacthIoctl_PROTEINBOX(); | __try __except ; \n" );
	}

	return STATUS_INVALID_DEVICE_REQUEST ;
}



VOID
Init__IOCTL_PROTEINBOX_FuncData (
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/25 [25:5:2010 - 16:50]

Routine Description:
  ��ʼ�� g_IOCTL_PROTEINBOX_FuncData �ṹ��
    
--*/
{
	if ( FALSE == g_IOCTL_PROTEINBOX_FuncData.bInited ) 
	{ 
		g_IOCTL_PROTEINBOX_FuncData.pArray		= _ioctl_proteinbox_functions_array_ ; 
		g_IOCTL_PROTEINBOX_FuncData.TotalCounts	= ARRAYSIZEOF( _ioctl_proteinbox_functions_array_ );
		g_IOCTL_PROTEINBOX_FuncData.bInited		= TRUE ;
	}

	return ;
}



///////////////////////////////   END OF FILE   ///////////////////////////////