#pragma once


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+							�ṹ��			                  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--

typedef struct _IOCTL_GETINJECTSAVEAREA_BUFFER_ // size - 0x24
{
	ULONG* BeCoveredAddr ;	// [OUT] ���汻���ǵ�PE�ڴ�����ʼ��ַ
	ULONG* MaxLength ;		// [IN OUT] R3׼���Ļ������Ĵ�С,����ʱ����ʵ�ʵ����ݴ�С			
	PVOID lpData ;			// [OUT] R3�Ļ�����,�������ԭʼ��PE�ڴ������

} IOCTL_GETINJECTSAVEAREA_BUFFER, *LPIOCTL_GETINJECTSAVEAREA_BUFFER ;

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
Ioctl_GetInjectSaveArea (
	IN PVOID pNode, 
	IN PVOID pInBuffer
	);



///////////////////////////////   END OF FILE   ///////////////////////////////