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

typedef enum _IOCTL_DISABLEFORCEPROCESS_FLAG_
{
	_FLAG_IOCTL_DISABLEFORCEPROCESS_EnableForce_	 = 3, // ��ʾ����ǿ�����еĹ���
	_FLAG_IOCTL_DISABLEFORCEPROCESS_DisableForce_	 = 5, // ��ʾ�ر�ǿ�����еĹ���
	_FLAG_IOCTL_DISABLEFORCEPROCESS_DisableForceALL_ = 7, // ��ʾȡ�����е�ǿ������
	_FLAG_IOCTL_DISABLEFORCEPROCESS_EnableForceALL_  = 9, // ��ʾ�������е�ǿ������

} IOCTL_DISABLEFORCEPROCESS_FLAG ;


typedef struct _IOCTL_DISABLEFORCEPROCESS_BUFFER_ // size - 0x24
{
/*0x000 */ LPWSTR szProcName ; // ��ȡ���Ľ���ȫ·��
/*0x008 */ int NameLength ; // ��ȡ���Ľ���ȫ·������
/*0x00C */ int Flag ; // [����] �ɶ�̬�Ŀ���/�ر� ǿ�����еĹ���

} IOCTL_DISABLEFORCEPROCESS_BUFFER, *LPIOCTL_DISABLEFORCEPROCESS_BUFFER ;


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
Ioctl_DisableForceProcess (
	IN PVOID pNode, 
	IN PVOID pInBuffer
	);


///////////////////////////////   END OF FILE   ///////////////////////////////