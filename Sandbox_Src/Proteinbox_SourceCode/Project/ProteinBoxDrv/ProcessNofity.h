#pragma once

//////////////////////////////////////////////////////////////////////////

extern NOTIY_INFO g_ProcessNotify_Info ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         ����Ԥ����                        +//      --       -        --  
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
    );

BOOL 
CheckProcessNotifyState (
	);

NTSTATUS
__PsSetCreateProcessNotifyRoutine (
	IN PVOID NotifyRoutine
	);

///////////////////////////////   END OF FILE   ///////////////////////////////