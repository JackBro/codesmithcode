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


#define _WhiteOrBlack_Flag_LowerLimit_	0x60
#define _WhiteOrBlack_Flag_UperLimit_	0x70

#define IsWBFlagOK( l )	  ( l >= _WhiteOrBlack_Flag_LowerLimit_ && l <= _WhiteOrBlack_Flag_UperLimit_ ) 

enum _WhiteOrBlack_Flag_ 
{
	WhiteOrBlack_Flag_XFilePath = _WhiteOrBlack_Flag_LowerLimit_ ,
	WhiteOrBlack_Flag_XRegKey,
	WhiteOrBlack_Flag_XIpcPath,
	WhiteOrBlack_Flag_XClassName,
};


typedef struct _IOCTL_WHITEORBLACK_BUFFER_ // size - 0x24
{
	ULONG Flag ;	 // [IN] ���Ҫƥ�������(File/Reg/IPC ...)
	LPCWSTR szPath ; // [IN] �������ַ���ָ��
	ULONG PathLength ; // [IN] �������ַ�������

	BOOL* bIsWhite ; // [OUT] �Ƿ�Ϊ��
	BOOL* bIsBlack ; // [OUT] �Ƿ�Ϊ��

} IOCTL_WHITEORBLACK_BUFFER, *LPIOCTL_WHITEORBLACK_BUFFER ;

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
Ioctl_WhiteOrBlack (
	IN PVOID pNode, 
	IN PVOID pInBuffer
	);



///////////////////////////////   END OF FILE   ///////////////////////////////