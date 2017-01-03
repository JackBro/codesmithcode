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


typedef struct _IOCTL_STARTPROCESS_BUFFER_ // size - 0x24
{
/*0x000 */ LPWSTR wszBoxName ;			// ��ǰɳ����
/*0x004 */ LPWSTR wszRegisterUserID ;	// ��ǰ�û�SID; eg: S-1-5-21-425828121-612454800-1421510826-500_Classes
/*0x008 */ HANDLE* new_hToken ;			// �����¾��,��R3����"��SB"�Ľ���ʱʹ��	
/*0x00C */ HANDLE* new_ImpersonationToken ;

} IOCTL_STARTPROCESS_BUFFER, *LPIOCTL_STARTPROCESS_BUFFER ;



typedef struct _IOCTL_QUERYPROCESS_BUFFER_ // size - 0x38
{
/*0x000 */ ULONG PID ;

/*0x004 */ LPWSTR lpBoxName ; // ���浱ǰ��ɳ����
/*0x008 */ ULONG BoxNameMaxLength ;

/*0x00C */ LPWSTR lpCurrentProcessShortName ; // ���浱ǰ�Ľ��̶���
/*0x010 */ ULONG ProcessShortNameMaxLength ;

/*0x014 */ LPWSTR lpRegisterUserID ; // ��ǰ���̶�Ӧ��SID
/*0x018 */ ULONG RegisterUserIDMaxLength ;

/*0x01C */ PULONG SessionId ; // ��ǰ���̶�Ӧ��SessionId

/*0x020 */ LPWSTR FileRootPath ; 
/*0x024 */ ULONG FileRootPathLength ;

/*0x028 */ LPWSTR KeyRootPath;	
/*0x02C */ ULONG KeyRootPathLength ;

/*0x030 */ LPWSTR LpcRootPath ;
/*0x034 */ ULONG LpcRootPathLength ;

} IOCTL_QUERYPROCESS_BUFFER, *LPIOCTL_QUERYPROCESS_BUFFER ;


typedef struct _IOCTL_QUERYPROCESSPATH_BUFFER_ // size - 0xC
{
/*0x000 */ ULONG PID ;

/*0x004 */ LPWSTR FileRootPath ; 
/*0x008 */ ULONG FileRootPathLength ;

} IOCTL_QUERYPROCESSPATH_BUFFER, *LPIOCTL_QUERYPROCESSPATH_BUFFER ;


typedef struct _IOCTL_QUERYBOXPATH_BUFFER_ // size - 0x10
{
/*0x000 */ LPWSTR lpBoxName ; // [IN] ��ǰ��ɳ����
/*0x004 */ ULONG BoxNamLength ;
/*0x008 */ LPWSTR lpBoxPath ; // [OUT] ���浱ǰɳ������Ӧ��ȫ·��
/*0x00C */ ULONG BoxPathMaxLength ;

} IOCTL_QUERYBOXPATH_BUFFER, *LPIOCTL_QUERYBOXPATH_BUFFER ;


typedef struct _IOCTL_ENUMPROCESSEX_BUFFER_ // size - 0x08
{
/*0x000 */ LPWSTR lpBoxName ; // [IN] ��ǰ��ɳ����
/*0x004 */ int* pArray ; // [OUT] ����PID����

} IOCTL_ENUMPROCESSEX_BUFFER, *LPIOCTL_ENUMPROCESSEX_BUFFER ;


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
Ioctl_StartProcess (
	IN PVOID pNode, 
	IN PVOID pInBuffer
	);

NTSTATUS
Ioctl_QueryProcess (
	IN PVOID pNode,
	IN PVOID pInBuffer
	);

NTSTATUS
Ioctl_QueryProcessPath (
	IN PVOID pNode,
	IN PVOID pInBuffer
	);

NTSTATUS
Ioctl_QueryBoxPath (
	IN PVOID pNode,
	IN PVOID pInBuffer
	);

NTSTATUS
Ioctl_EnumProcessEx (
	IN PVOID pNode,
	IN PVOID pInBuffer
	);

///////////////////////////////   END OF FILE   ///////////////////////////////