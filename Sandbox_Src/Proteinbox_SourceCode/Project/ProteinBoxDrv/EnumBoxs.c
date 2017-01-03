/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/05/25 [25:5:2010 - 17:42]
* MODULE : d:\Work\Program\Coding\ɳ��\SandBox\Code\Project\ProteinBoxDrv\StartProcess.c
* 
* Description:
*      
*                        
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
#include "EnumBoxs.h"

//////////////////////////////////////////////////////////////////////////


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
Ioctl_EnumBoxes (
	IN PVOID pNode,
	IN PVOID pInBuffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2011/07/11 [11:7:2011 - 16:20]

Routine Description:
  ö�ٵ�ǰ���е�ɳ��,����������    
    
--*/
{
	ULONG PID = 0, length = 0 ;
	LPSANDBOX_NODE_INFO pData = NULL ;
	LPIOCTL_ENUMBOXS_BUFFER Buffer = NULL ;

	dprintf( "[ProteinBoxDrv] IOCTL_PROTEINBOX - Ioctl_EnumBoxes(); \n" );

	if ( NULL == pInBuffer ) { return STATUS_NOT_IMPLEMENTED ; }

	__try
	{
		Buffer = (LPIOCTL_ENUMBOXS_BUFFER) getIoctlBufferBody( pInBuffer );
		if ( FALSE == MmIsAddressValid( (PVOID)Buffer ) )
		{
			dprintf( "error! | Ioctl_EnumBoxes() - getIoctlBufferBody(); | Body��ַ���Ϸ� \n" );
			__leave ;
		}

		if ( _IOCTL_ENUMBOXS_FLAG_GetSandboxsCounts_ == Buffer->Flag )
		{
			if ( IsWrittenKernelAddr(Buffer->u.SandboxsCounts, 4) ) 
			{
				dprintf( "error! | Ioctl_EnumBoxes() - IsWrittenKernelAddr(); | �������Ϸ�,��д��ĵ�ַBuffer->u.SandboxsCounts:0x%08lx \n", Buffer->u.SandboxsCounts );
				return STATUS_INVALID_PARAMETER_4 ;
			}

			ProbeForWrite( Buffer->u.SandboxsCounts, 4, sizeof(DWORD) );
			*Buffer->u.SandboxsCounts = SBLGetSandboxsCounts();
		} 
		else if ( _IOCTL_ENUMBOXS_FLAG_GetSandboxsCurName_ == Buffer->Flag )
		{
			pData = (LPSANDBOX_NODE_INFO) kgetnodeExSBL( Buffer->u.CurrentIndex );
			if ( NULL == pData ) 
			{ 
				dprintf( "error! | Ioctl_EnumBoxes() - kgetnodeExSBL(); | �޷���ȡ@CurrentIndex=%n ��Ӧ�Ľڵ� \n", Buffer->u.CurrentIndex );
				return STATUS_INVALID_PARAMETER_4 ;
			}

			if ( IsWrittenKernelAddr(Buffer->lpData, (ULONG_PTR)Buffer->INDataLength) || (Buffer->INDataLength < (int)pData->NameLength) ) 
			{
				dprintf( "error! | Ioctl_EnumBoxes() - IsWrittenKernelAddr(); | �������Ϸ�,��д��ĵ�ַBuffer->lpData:0x%08lx, ����:0x%08lx \n", Buffer->lpData, Buffer->INDataLength );
				return STATUS_INVALID_PARAMETER_4 ;
			}

			ProbeForWrite( Buffer->lpData, Buffer->INDataLength, sizeof(DWORD) );
			memcpy( Buffer->lpData, pData->wszName, pData->NameLength );
		}
		else
		{
			dprintf( "error! | Ioctl_EnumBoxes(); | ���Ϸ���Flag:%n \n", Buffer->Flag );
			return STATUS_NOT_IMPLEMENTED;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		dprintf( "error! | Ioctl_EnumBoxes() - __try __except(); | ProbeForWrite��Ӧ�ĵ�ַ���Ϸ� \n" );
		return STATUS_UNSUCCESSFUL ;
	}

	return STATUS_SUCCESS ;
}



///////////////////////////////   END OF FILE   ///////////////////////////////