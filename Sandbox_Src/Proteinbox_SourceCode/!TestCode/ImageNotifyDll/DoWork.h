#pragma once

#include <ntddk.h>


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+							�ṹ��			                  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--

typedef struct _IMAGENOTIY_INFO_ {

/*0x000 */ BOOL bNotifyState ;		// �Ƿ�ע��ɹ�
/*0x004 */ PVOID NotifyRoutine ;    // �ص�������ַ

} IMAGENOTIY_INFO, *PIMAGENOTIY_INFO ;

extern IMAGENOTIY_INFO g_ImageNotify_Info ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         ����Ԥ����                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


void
DoWork (
	);

void
ImageNotifyRoutine (
  IN PUNICODE_STRING  FullImageName,
  IN HANDLE  ProcessId,
  IN PIMAGE_INFO  ImageInfo
  );

BOOL 
CheckNotifyState (
	);

BOOL
Is_special_process (
	IN PUNICODE_STRING  FullImageName
	);

///////////////////////////////   END OF FILE   ///////////////////////////////