#pragma once

#include <winioctl.h>


//
// The device driver IOCTLs
//

#define IOCTL_BASE	0x800
#define MY_CTL_CODE(i) CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_BASE+i, METHOD_BUFFERED, FILE_ANY_ACCESS)


#define IOCTL_HELLO				MY_CTL_CODE( 0 )
#define IOCTL_PROTEINBOX		MY_CTL_CODE( 1 )

const WCHAR g_PBLinkName[] = L"\\\\.\\ProteinBoxDrv" ;

/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+							�ṹ��			                  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


#define IOCTL_PROTEINBOX_FLAG__LowerLimit	0x12340000
#define IOCTL_PROTEINBOX_FLAG__UperLimit	IOCTL_PROTEINBOX_FLAG__LowerLimit + 0x100

// �������Ƿ�Ƿ�; TRUE��ʾ�Ƿ�
#define IS_INVALID_IOCTL_FLAG( l ) ( l == 0xFFFFFFFF || l <= IOCTL_PROTEINBOX_FLAG__LowerLimit || l >= IOCTL_PROTEINBOX_FLAG__UperLimit )


typedef enum _IOCTL_PROTEINBOX_FLAG_ 
{
	FLAG_Ioctl_GetSysVersion		= IOCTL_PROTEINBOX_FLAG__LowerLimit + 1, // 0x12340001
	FLAG_Ioctl_GetWork 				= 0x12340002, 
	FLAG_Ioctl_WhiteOrBlack			= 0x12340003, // �ж��ַ����ĺڰ�
	FLAG_Ioctl_GetLicense			= 0x12340004,
	FLAG_Ioctl_SetLicense			= 0x12340005,
	FLAG_Ioctl_StartProcess			= 0x12340006, // ��ɳ������������
	FLAG_Ioctl_QueryProcess			= 0x12340007, 
	FLAG_Ioctl_QueryBoxPath			= 0x12340008,
	FLAG_octl_QueryProcessPath		= 0x12340009,
	FLAG_Ioctl_QueryPathList		= 0x1234000A,
	FLAG_Ioctl_EnumProcessEx		= 0x1234000B,
	FLAG_Ioctl_DisableForceProcess	= 0x1234000C,
	FLAG_Ioctl_HookTramp			= 0x1234000D,
	FLAG_Ioctl_EnumBoxs				= 0x1234000E,
	FLAG_Ioctl_HandlerConf			= 0x1234000F, // ���������ļ� ProteinBox.ini
	FLAG_Ioctl_ReloadConf			= 0x12340010,
	FLAG_Ioctl_CreateDirOrLink		= 0x12340011, // ������������
	FLAG_Ioctl_DuplicateObject		= 0x12340012, // �����ƾ���Ĳ���
	FLAG_Ioctl_GetInjectSaveArea	= 0x12340013, // ��ȡImageNofity�޸ĵ�PE����
	FLAG_Ioctl_RenameFile			= 0x12340014,
	FLAG_Ioctl_SetUserName			= 0x12340015,
	FLAG_Ioctl_HookShadowSSDT		= 0x12340016,
	FLAG_Ioctl_DriverUnload			= 0x12340017,
	FLAG_Ioctl_GetSetDeviceMap		= 0x12340018,
	FLAG_Ioctl_1					= 0x12340019,
	FLAG_Reserved2					= 0x1234001A,
	FLAG_Ioctl_MonitorControl		= 0x1234001B,
	FLAG_Ioctl_MonitorPut			= 0x1234001C,
	FLAG_Ioctl_MonitorGet			= 0x1234001D,
	FLAG_Ioctl_GetUnmountHive		= 0x1234001E,
	FLAG_Ioctl_GetFileName			= 0x1234001F,
	FLAG_Ioctl_ClearUpNode			= 0x12340020,
	FLAG_Ioctl_SetLsaAuthPkg		= 0x12340021,
	FLAG_Ioctl_IoCreateFileSpecifyDeviceObjectHint	= 0x12340022,
	FLAG_Ioctl_2					= 0x12340023,
	FLAG_Ioctl_HookObject			= 0x12340024,
	FLAG_Ioctl_QueryProcessPath		= 0x12340025, 


} IOCTL_PROTEINBOX_FLAG ;




typedef struct _IOCTL_GETSYSVERSION_BUFFER_ // size - 0x4
{
/*0x000 */ LPWSTR wszVersion ;

} IOCTL_GETSYSVERSION_BUFFER, *PIOCTL_GETSYSVERSION_BUFFER ;


typedef enum _IOCTL_ENUMBOXS_FLAG_
{
	_IOCTL_ENUMBOXS_FLAG_GetSandboxsCounts_  = 3,
	_IOCTL_ENUMBOXS_FLAG_GetSandboxsCurName_ = 5,

} IOCTL_ENUMBOXS_FLAG ;


typedef struct _IOCTL_ENUMBOXS_BUFFER_ // size - 0x24
{
/*0x000 */ int Flag ; // 3 - ��һ��ͨ��,����R3ɳ�����; 5 - �ٴ�ͨ��,�ҵ�@CurrentIndex��Ӧ�Ľڵ�,����ɳ����
/*0x004 */ 
		union
		{
			int CurrentIndex ; // ָ����ǰҪ��ȡ�Ľڵ���;
			int* SandboxsCounts ; // [OUT] ���ɳ��ĸ���
		} u ;

/*0x008 */ PVOID lpData ; // [OUT] R3������,������Ż�ȡ����ɳ����[ Length <= MAX_PATH ]
/*0x00C */ int INDataLength ; // R3��������С

} IOCTL_ENUMBOXS_BUFFER, *LPIOCTL_ENUMBOXS_BUFFER ;


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

/*0x020 */  LPWSTR FileRootPath ; 
/*0x024 */  ULONG FileRootPathLength ;

/*0x028 */	LPWSTR KeyRootPath;	
/*0x02C */	ULONG KeyRootPathLength ;

/*0x030 */  LPWSTR LpcRootPath ;
/*0x034 */  ULONG LpcRootPathLength ;

} IOCTL_QUERYPROCESS_BUFFER, *LPIOCTL_QUERYPROCESS_BUFFER ;



//////////////////////////////////////////////////////////////////////////

typedef enum _IOCTL_HANDLERCONF_FUNCTION_TYPE_ 
{
	_Ioctl_Conf_function_InitData_		= 0x1001, // R3���͸�Ioctl,�������ļ��е�������Ϣȫ������R0�洢 
	_Ioctl_Conf_function_VerifyData_	= 0x1002, // R3���͸�Ioctl,��R0���Buffer,���а����˶�/дָ��������Ϣ
	_Ioctl_Conf_function_ReceiveData_	= 0x1003, // R3���͸�Ioctl,����ȡ���������ļ���Ϣ���ݵ�R0��	
	_Ioctl_Conf_function_Reload_		= 0x1004, // ���¼��������ļ�,��R3���¶�ȡ�����ļ���,�����и��º�������׸�����	

} IOCTL_HANDLERCONF_FUNCTION_TYPE ;


typedef enum _IOCTL_HANDLERCONF_BUFFER_TYPE_ 
{
	_Ioctl_Conf_Read_	= 1,
	_Ioctl_Conf_Write_	= 2,

} IOCTL_HANDLERCONF_BUFFER_TYPE ;


typedef struct _IOCTL_HANDLERCONF_BUFFER_IniDataA_ // size - 0x018
{
/*0x000 */ LPSTR SeactionName ;
/*0x004 */ ULONG SeactionNameLength ;

/*0x008 */ LPSTR KeyName ;
/*0x00C */ ULONG KeyNameLength ;

/*0x010 */ LPSTR ValueName ;
/*0x014 */ ULONG ValueNameLength ;

} IOCTL_HANDLERCONF_BUFFER_IniDataA, *LPIOCTL_HANDLERCONF_BUFFER_IniDataA ;


typedef struct _IOCTL_HANDLERCONF_BUFFER_IniDataW_ // size - 0x018
{
/*0x000 */ LPWSTR SeactionName ;
/*0x004 */ ULONG SeactionNameLength ;

/*0x008 */ LPWSTR KeyName ;
/*0x00C */ ULONG KeyNameLength ;

/*0x010 */ LPWSTR ValueName ;
/*0x014 */ ULONG ValueNameLength ;

} IOCTL_HANDLERCONF_BUFFER_IniDataW, *LPIOCTL_HANDLERCONF_BUFFER_IniDataW ;


#define IOCTL_HANDLERCONF_BUFFER_NAMELENGTH 0x50

typedef struct _IOCTL_HANDLERCONF_BUFFER_VerifyData_ // size - 0x24
{
/*0x000 */ char SeactionName[ IOCTL_HANDLERCONF_BUFFER_NAMELENGTH ] ;	// eg: "GlobalSetting"
/*0x004 */ char KeyName[ IOCTL_HANDLERCONF_BUFFER_NAMELENGTH ] ;		// eg: "OpenIpcPath"
/*0x008 */ IOCTL_HANDLERCONF_BUFFER_TYPE Flag ;		// ��/д���
/*0x00C */ PVOID Data ;			// ����R3��ѯ��������

} IOCTL_HANDLERCONF_BUFFER_VerifyData, *LPIOCTL_HANDLERCONF_BUFFER_VerifyData ;


typedef struct _IOCTL_HANDLERCONF_BUFFER_ReceiveData_ // size - 0x04
{
/*0x000 */ ULONG NameLength ;
/*0x004 */ LPSTR szName ; // ansi��ʽ

} IOCTL_HANDLERCONF_BUFFER_ReceiveData, *LPIOCTL_HANDLERCONF_BUFFER_ReceiveData ;


typedef struct _IOCTL_HANDLERCONF_BUFFER_RELAOD_ // size - 0x04
{
/*0x000 */ ULONG Reserved ;

} IOCTL_HANDLERCONF_BUFFER_RELAOD, *LPIOCTL_HANDLERCONF_BUFFER_RELAOD ;


//
// ����Configuration���ܽṹ��
//

typedef struct _IOCTL_HANDLERCONF_BUFFER_ // size - 0x24
{
	union 
	{
		IOCTL_HANDLERCONF_BUFFER_IniDataA		InitData ;
		IOCTL_HANDLERCONF_BUFFER_VerifyData		VerifyData  ; 
		IOCTL_HANDLERCONF_BUFFER_ReceiveData	ReceiveData ;
		IOCTL_HANDLERCONF_BUFFER_RELAOD			ReloadData	;
	} u ;

} IOCTL_HANDLERCONF_BUFFER, *LPIOCTL_HANDLERCONF_BUFFER ;


//////////////////////////////////////////////////////////////////////////

//
// Inline Hook Shadow SSDT��ؽṹ��
//

typedef struct _IOCTL_HOOKSHADOW_BUFFER_ // size - 0x008
{
/*0x000 */ BOOL bHook ; // Shadow ssdt inline hook�Ŀ���
/*0x004*/  ULONG Reserved ;

} IOCTL_HOOKSHADOW_BUFFER, *LPIOCTL_HOOKSHADOW_BUFFER ;


//
// Object Hook ��ؽṹ��
//

typedef struct _IOCTL_HOOKOBJECT_BUFFER_ // size - 0x008
{
/*0x000 */ BOOL bHook ; // hook����
/*0x004*/  ULONG Reserved ;

} IOCTL_HOOKOBJECT_BUFFER, *LPIOCTL_HOOKOBJECT_BUFFER ;


//
// ImageNotify Dll ��ؽṹ��
//

typedef struct _NEW_PE_IAT_INFO_before_ { // size - 0x68

/*0x000 */ ULONG Reserved1[ 4 ] ;

/*0x010 */ PIMAGE_DATA_DIRECTORY pidd ;
/*0x014 */ ULONG Reserved2 ;
/*0x018 */ ULONG IMAGE_DIRECTORY_ENTRY_IMPORT_VirtualAddress ;
/*0x01C */ ULONG IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT_VirtualAddress ;
/*0x020 */ ULONG IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT_Size ;
/*0x024 */ ULONG IMAGE_DIRECTORY_ENTRY_IAT_VirtualAddress ;
/*0x028 */ ULONG IMAGE_DIRECTORY_ENTRY_IAT_Size ;
/*0x02C */ ULONG IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR_VirtualAddress ;
/*0x030 */ ULONG IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR_Size ;
/*0x034 */ ULONG OldProtection ;
/*0x038 */ PIMAGE_IMPORT_DESCRIPTOR pImportTable_mscoree_dll ;
/*0x03C */ PIMAGE_IMPORT_DESCRIPTOR pImportTable_msvcm80_dll ;
/*0x040 */ PIMAGE_IMPORT_DESCRIPTOR pImportTable_msvcm90_dll ;
/*0x044 */ IMAGE_IMPORT_BY_NAME IIN1 ;
/*0x048 */ ULONG Reserved4[ 4 ] ;
/*0x058 */ IMAGE_THUNK_DATA32 itd_1 ;
/*0x05C */ IMAGE_THUNK_DATA32 itd_2 ;
/*0x060 */ ULONG Reserved5[ 2 ] ;

} NEW_PE_IAT_INFO_before, *PNEW_PE_IAT_INFO_before ;


typedef struct _NEW_PE_IAT_INFO_TOTAL_ {

/*0x000 */ NEW_PE_IAT_INFO_before Stub ;
/*0x068 */ IMAGE_IMPORT_DESCRIPTOR IATNew ;
/*0x07C */ IMAGE_IMPORT_DESCRIPTOR IATOld[ 1 ] ;

} NEW_PE_IAT_INFO_TOTAL , *PNEW_PE_IAT_INFO_TOTAL ;


typedef struct _IOCTL_GETINJECTSAVEAREA_BUFFER_ // size - 0x24
{
	ULONG* BeCoveredAddr ;	// [OUT] ���汻���ǵ�PE�ڴ�����ʼ��ַ
	ULONG* MaxLength ;		// [IN OUT] R3׼���Ļ������Ĵ�С,����ʱ����ʵ�ʵ����ݴ�С			
	PVOID lpData ;			// [OUT] R3�Ļ�����,�������ԭʼ��PE�ڴ������
	
} IOCTL_GETINJECTSAVEAREA_BUFFER, *LPIOCTL_GETINJECTSAVEAREA_BUFFER ;


typedef struct _IOCTL_WHITEORBLACK_BUFFER_ // size - 0x24
{
	ULONG Flag ;	 // [IN] ���Ҫƥ�������(File/Reg/IPC ...)
	LPCWSTR szPath ; // [IN] �������ַ���ָ��
	ULONG PathLength ; // [IN] �������ַ�������

	BOOL* bIsWhite ; // [OUT] �Ƿ�Ϊ��
	BOOL* bIsBlack ; // [OUT] �Ƿ�Ϊ��

} IOCTL_WHITEORBLACK_BUFFER, *LPIOCTL_WHITEORBLACK_BUFFER ;


typedef struct _IOCTL_CREATEDIRORLINK_BUFFER_ // size - 0x24
{
	ULONG Reserved ;

	LPWSTR lpcFullPath ; // lpc��ȫ·��
	ULONG lpcFullPathLength ;

	BOOL bFlag ; // ����Ƿ�ʹ�õ������2������
	LPWSTR lpcFatherPath ; // lpc�ĸ�·��
	ULONG lpcFatherPathLength ;

} IOCTL_CREATEDIRORLINK_BUFFER, *LPIOCTL_CREATEDIRORLINK_BUFFER ;


typedef struct _IOCTL_DUPLICATEOBJECT_BUFFER_ // size - 0x14
{
	PHANDLE pTargetHandle ;
	HANDLE FuckedHandle ;
	HANDLE SourceHandle ;
	ACCESS_MASK DesiredAccess ;
	ULONG Options ;

} IOCTL_DUPLICATEOBJECT_BUFFER, *LPIOCTL_DUPLICATEOBJECT_BUFFER ;


typedef struct _IOCTL_GETFILENAME_BUFFER_ // size - 0xC
{
	HANDLE HandleValue ;
	ULONG BufferLength ;
	PVOID lpData ; // [OUT] R3�Ļ�����,�������@HandleValue��Ӧ�Ķ�������·��

} IOCTL_GETFILENAME_BUFFER, *LPIOCTL_GETFILENAME_BUFFER ;


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



typedef struct _IOCTL_QUERYBOXPATH_BUFFER_ // size - 0x38
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


typedef struct _IOCTL_QUERYPROCESSPATH_BUFFER_ // size - 0xC
{
/*0x000 */ ULONG PID ;

/*0x004 */ LPWSTR FileRootPath ; 
/*0x008 */ ULONG FileRootPathLength ;

} IOCTL_QUERYPROCESSPATH_BUFFER, *LPIOCTL_QUERYPROCESSPATH_BUFFER ;



/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


//
// ͨ�ź�IOCTL_PROTEINBOX_BUFFER��Ӧ�����ݽṹ
//

typedef struct _IOCTL_PROTEINBOX_BUFFER_HEAD_ 
{
/*0x000 */ IOCTL_PROTEINBOX_FLAG Flag ; // ��ͬ��ֵ�ֱ��ʾ��Ҫ��ͬ�ĺ�������ǰ����
/*0x004 */ ULONG LittleIoctlCode ;
/*0x008 */ ULONG Reserved ;
	
} IOCTL_PROTEINBOX_BUFFER_HEAD ;


typedef struct _IOCTL_PROTEINBOX_BUFFER_ 
{
/*0x000 */ IOCTL_PROTEINBOX_BUFFER_HEAD Head ;
/*0x008 */ 
/*0x00C */ 
	union 
	{
		IOCTL_GETSYSVERSION_BUFFER GetSysVersion		;
		IOCTL_STARTPROCESS_BUFFER StartProcessBuffer	;
		IOCTL_QUERYPROCESS_BUFFER QueryProcessBuffer	;
		IOCTL_HANDLERCONF_BUFFER  ConfigBuffer			;
		IOCTL_HOOKSHADOW_BUFFER	  HookShadowBuffer		;
		IOCTL_HOOKOBJECT_BUFFER	  HookObjectBuffer		;
		IOCTL_GETINJECTSAVEAREA_BUFFER GetInjectSaveAreaBuffer ;
		IOCTL_WHITEORBLACK_BUFFER	WhiteOrBlackBuffer ;
		IOCTL_CREATEDIRORLINK_BUFFER CreateDirOrLinkBuffer ;
		IOCTL_DUPLICATEOBJECT_BUFFER DuplicateObjectBuffer ;
		IOCTL_GETFILENAME_BUFFER	 GetFileNameBuffer ;
		IOCTL_ENUMBOXS_BUFFER		 EnumBoxsBuffer ;
		IOCTL_DISABLEFORCEPROCESS_BUFFER DisableForceProcessBuffer ;
		IOCTL_QUERYBOXPATH_BUFFER QueryBoxPathBuffer ;
		IOCTL_ENUMPROCESSEX_BUFFER EnumProcessExBuffer ;
		IOCTL_QUERYPROCESSPATH_BUFFER QueryProcessPathBuffer ;

	};
	
} IOCTL_PROTEINBOX_BUFFER, *PIOCTL_PROTEINBOX_BUFFER ;

// �����: ȡ��Ioctl Buffer�ľ�������,����������ͷ
#define getIoctlBufferBody( l )		( (ULONG)l + sizeof( IOCTL_PROTEINBOX_BUFFER_HEAD ) )


//////////////////////////////////////////////////////////////////////////
//
// rpc ���
//
//////////////////////////////////////////////////////////////////////////


typedef enum _SBIESRV_APINUM_
{
	_PBSRV_APINUM_PStoreGetNode_			= 0x1101,
	_PBSRV_APINUM_PStoreGetChildNode_		= 0x1102,
	_PBSRV_APINUM_PStoreGetGrandchildNode_	= 0x1103,
	_PBSRV_APINUM_PStoreEnumSubtypes_		= 0x1104,
	_PBSRV_APINUM_PStoreEnumItems_			= 0x1105,

	_PBSRV_APINUM_StartPBDrv_	= 0x1201,
	_PBSRV_APINUM_KillProcess_  = 0x1203,
	_PBSRV_APINUM_TerminateBox_ = 0x1204,
	_PBSRV_APINUM_StopPBDrv_	= 0x1205, 

	_PBSRV_APINUM_StartService_			= 0x1301,
	_PBSRV_APINUM_EnumServiceStatus_	= 0x1302,
	_PBSRV_APINUM_GetSandboxedServices_ = 0x1303,
	_PBSRV_APINUM_StartBoxedService_	= 0x1304,

	_PBSRV_APINUM_SetupDiGetDevicePropertyW_			= 0x1401,
	_PBSRV_APINUM_CM_Get_Device_Interface_List_filter_	= 0x1402,
	_PBSRV_APINUM_CM_Get_Device_Interface_Alias_ExW_	= 0x1403,
	_PBSRV_APINUM_CM_Get_Device_Interface_Property_ExW_	= 0x1404,
	_PBSRV_APINUM_CM_Get_Class_Property_ExW_Win7_		= 0x1405,
	_PBSRV_APINUM_CM_Get_DevNode_Status_				= 0x1406,
	_PBSRV_APINUM_WinStaQueryInformationW_				= 0x1407,

	_PBSRV_APINUM_PipeCommunication_	= 0x1501,
	_PBSRV_APINUM_CloseLittleHandle_	= 0x1502,
	_PBSRV_APINUM_NtSetInformationFile_ = 0x1503,
	_PBSRV_APINUM_NtReadFile_			= 0x1504,
	_PBSRV_APINUM_NtWriteFile_			= 0x1505,
	_PBSRV_APINUM_ClearUpFile_			= 0x15FF,

	_PBSRV_APINUM_CryptUnprotectData_	= 0x1601,
	_PBSRV_APINUM_CryptProtectData_		= 0x1602,

	_PBSRV_APINUM_MarkFileTime_		= 0x1701,
	_PBSRV_APINUM_SetFileShortName_	= 0x1702,
	_PBSRV_APINUM_NtLoadKey_		= 0x1703,
	_PBSRV_APINUM_NtCloseWinsxs_	= 0x1901,
	_PBSRV_APINUM_NtFsControlFile_	= 0x1903,


	_PBSRV_APINUM_INIGetCurUserSection_ = 0x1801,
	_PBSRV_APINUM_GetAppVersion_		= 0x18AA,

	_PBSRV_APINUM_HandlerWhiteCLSID_	= 0x1B01,
	_PBSRV_APINUM_CoUnmarshalInterface_ = 0x1B06,
	_PBSRV_APINUM_CoMarshalInterface_	= 0x1B07,


} SBIESRV_APINUM ;


#define PB_PORT_MESSAGE_MAXLength	0x200

typedef struct _PB_PORT_MESSAGE_ 
{
/*0x000*/ PORT_MESSAGE Header ;
/*0x018*/ 
	union
	{
		UCHAR Buffer[ PB_PORT_MESSAGE_MAXLength ] ;
	};

} PB_PORT_MESSAGE, *LPPB_PORT_MESSAGE ;


typedef struct _RPC_IN_HEADER_ 
{
/*0x000 */  DWORD	DataLength ;
/*0x004 */  DWORD	Flag ;

} RPC_IN_HEADER, *PRPC_IN_HEADER ;


typedef struct _RPC_OUT_HEADER_ 
{
/*0x000 */  ULONG ReturnLength ;
/*0x004 */  
	union
	{
		NTSTATUS Status ;
		DWORD ErrorCode ;
	} u ;		

} RPC_OUT_HEADER, *LPRPC_OUT_HEADER ;


// RpcTerminateBox()
typedef struct _RPC_PROCTERMINATEBOX_INFO_ 
{
/*0x000*/ RPC_IN_HEADER RpcHeader ;
/*0x008*/ DWORD PID ;
/*0x00C*/ WCHAR szBoxName[ 0x22 ];

} RPC_PROCTERMINATEBOX_INFO, *LPRPC_PROCTERMINATEBOX_INFO ;


// RpcKillProcess
typedef struct _RPC_PROCKILLPROCESS_INFO_ 
{
/*0x000*/ RPC_IN_HEADER RpcHeader ;
/*0x008*/ DWORD PID ;

} RPC_PROCKILLPROCESS_INFO, *LPRPC_PROCKILLPROCESS_INFO ;


typedef struct _PB_BOX_INFO_  
{

	WCHAR BoxName[ MAX_PATH ] ;
	ULONG BoxNameLength ;

	WCHAR ProcessName[ MAX_PATH ] ;
	ULONG ProcessNameLength ;

	WCHAR SID[ MAX_PATH ] ;
	ULONG SIDLength ;

	ULONG SessionId ;
	WCHAR FileRootPath[ MAX_PATH ] ;	// eg:"\Device\HarddiskVolume1\Sandbox\AV\DefaultBox"
	ULONG FileRootPathLength ;

	WCHAR KeyRootPath[ MAX_PATH ] ;		// eg:"\REGISTRY\USER\Sandbox_AV_DefaultBox"
	ULONG KeyRootPathLength ;

	WCHAR LpcRootPath[ MAX_PATH ] ;		// eg:"\Sandbox\AV\DefaultBox\Session_0"
	ULONG LpcRootPathLength ;

} PB_BOX_INFO, *LPPB_BOX_INFO ;



///////////////////////////////   END OF FILE   ///////////////////////////////