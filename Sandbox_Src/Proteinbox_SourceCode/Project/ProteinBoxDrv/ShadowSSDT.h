#pragma once

#include <ntimage.h>


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+							�ṹ��			                  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--

//
// Inline Hook Shadow SSDT��ؽṹ��
//

typedef struct _IOCTL_HOOKSHADOW_BUFFER_ // size - 0x008
{
/*0x000 */ BOOL bHook ; // Shadow ssdt inline hook�Ŀ���
/*0x004*/  ULONG Reserved ;

} IOCTL_HOOKSHADOW_BUFFER, *LPIOCTL_HOOKSHADOW_BUFFER ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         ����Ԥ����                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


NTSTATUS
Ioctl_HookShadow (
	IN PVOID pNode, 
	IN PVOID pInBuffer
	);

BOOL
HookShadowSSDT (
	);

VOID
UnhookShadowSSDT (
	);

VOID
UnhookShadowSSDTEx (
	);

BOOL
HandlerEnableWindow (
	);

ULONG
HandlerEnableWindowEx (
	IN int Tag
	);

VOID
HandlerSystemParametersInfoW (
	);

///////////////////////////////   END OF FILE   ///////////////////////////////
