#pragma once

#include "GetInjectSaveArea.h"
#include "StartProcess.h"
#include "Config.h"
#include "ShadowSSDT.h"
#include "ObjectHook.h"
#include "WhiteOrBlack.h"
#include "CreateDirOrLink.h"
#include "DuplicateObject.h"
#include "GetFileName.h"
#include "EnumBoxs.h"
#include "DisableForceProcess.h"

//////////////////////////////////////////////////////////////////////////



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
		IOCTL_STARTPROCESS_BUFFER	StartProcessBuffer	;
		IOCTL_QUERYPROCESS_BUFFER	QueryProcessBuffer	;
		IOCTL_HANDLERCONF_BUFFER	ConfigBuffer		;	
		IOCTL_HOOKOBJECT_BUFFER		HookObjectBuffer	;
		IOCTL_GETINJECTSAVEAREA_BUFFER GetInjectSaveAreaBuffer ;
		IOCTL_WHITEORBLACK_BUFFER	WhiteOrBlackBuffer ;
		IOCTL_CREATEDIRORLINK_BUFFER CreateDirOrLinkBuffer ;
		IOCTL_DUPLICATEOBJECT_BUFFER DuplicateObjectBuffer ;
		IOCTL_GETFILENAME_BUFFER	 GetFileNameBuffer ;
		IOCTL_ENUMBOXS_BUFFER		 EnumBoxsBuffer ;
		IOCTL_QUERYPROCESSPATH_BUFFER QueryProcessPathBuffer ;

	};

} IOCTL_PROTEINBOX_BUFFER, *PIOCTL_PROTEINBOX_BUFFER ;

// �����: ȡ��Ioctl Buffer�ľ�������,����������ͷ
#define getIoctlBufferBody( l )		( (ULONG)l + sizeof( IOCTL_PROTEINBOX_BUFFER_HEAD ) )


//
// �����е�ÿ����������Ԫ
//

typedef struct _IOCTL_PROTEINBOX_FUNCTIONS_  // size - 0x8
{
/*0x000 */ IOCTL_PROTEINBOX_FLAG Flag ; // ��ͬ��ֵ�ֱ��ʾ��Ҫ��ͬ�ĺ�������ǰ����
/*0x004 */ PVOID FunctionAddr ; // ��Ӧ�Ĵ�������ַ

} IOCTL_PROTEINBOX_FUNCTIONS, *LPIOCTL_PROTEINBOX_FUNCTIONS ;


//
// ������������ܽṹ��
//

typedef struct _IOCTL_PROTEINBOX_FUNCTIONS_DATA_ // size - 0xC
{
/*0x000 */ BOOL  bInited ;				 // �Ƿ��ʼ���ɹ�
/*0x004 */ int	 TotalCounts ;			 // ������ܸ���
/*0x008 */ LPIOCTL_PROTEINBOX_FUNCTIONS pArray ; // ������׵�ַ

} IOCTL_PROTEINBOX_FUNCTIONS_DATA, *PIOCTL_PROTEINBOX_FUNCTIONS_DATA ;

extern IOCTL_PROTEINBOX_FUNCTIONS_DATA g_IOCTL_PROTEINBOX_FuncData ;


typedef NTSTATUS (__stdcall *_Ioctl_func_)( PVOID, PVOID );

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
Handler_DispacthIoctl_PROTEINBOX(
	IN PVOID pInBuffer
	);

VOID
Init__IOCTL_PROTEINBOX_FuncData (
	);

///////////////////////////////   END OF FILE   ///////////////////////////////