#pragma once

#include <ntimage.h>

//////////////////////////////////////////////////////////////////////////

// �������óɹ��Ƿ�������ʾ
#define __SDT_TRACE__ 0
#define SDTTrace if (__SDT_TRACE__) dprintf


// �����ڴ�ṹ��
#define LPMPHEAD PMAPPED_PE_HEAND
#define LPMPNODE PMAPPED_PE

#define MPHEAD MAPPED_PE_HEAND
#define MPNODE MAPPED_PE


// ������� MappedPE �ڵ�ĺ���
#define kgetaddrMP( _Tag )			MPFindNode  ( (PVOID)g_ListHead__MappedPE, _Tag )
#define kfreeMP()					MPDistroyAll( (PVOID)g_ListHead__MappedPE		)

// �õ�ָ��(Shadow) SSDT ������ַ
#define CONCAT(x, y)		x ## y
#define CONCAT3(x, y, z)	x ## y ## z

#define ZwXXaddr( l )	CONCAT3( g_, l, _addr )
#define ZwTag( l )		CONCAT( Tag_, l )
#define ZwXXDefine( l ) CONCAT3( _, l, _ )

#define  kgetaddrSDT( Name )	Get_sdtfunc_addr( (PULONG)&ZwXXaddr(Name), (ULONG)ZwTag(Name) )


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+							�ṹ��			                  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


#pragma pack(push, 1)
typedef struct _ServiceDescriptorTableEntry 
{
	PVOID   *ServiceTable ;
	PULONG  CounterTable ;
	ULONG   TableSize ;
	PUCHAR  ArgumentTable ;        

} ServiceDescriptorTableEntry, *PServiceDescriptorTableEntry;
#pragma pack(pop)

__declspec(dllimport) ServiceDescriptorTableEntry KeServiceDescriptorTable ;


//
// ZwMapViewOfSectionӳ�����DLL���ڴ�Ľṹ��
//

typedef struct _MAPPED_PE_ // size - 0x64
{
/*0x000*/ struct _MAPPED_PE_* pFlink ; // �ϸ����
/*0x004*/ struct _MAPPED_PE_* pBlink ; // �¸����
/*0x008*/ WCHAR	wszModuleName[ 0x20 ] ;
/*0x048*/ HANDLE	hFile ; 
/*0x04C*/ HANDLE	SectionHandle ;
/*0x050*/ ULONG	ImageBase ;
/*0x054*/ ULONG	SizeOfImage ;
/*0x058*/ ULONG	MappedAddr ;
/*0x05C*/ PIMAGE_NT_HEADERS pinh ;
/*0x060*/ PIMAGE_EXPORT_DIRECTORY pEATAddr ;

} MAPPED_PE, *PMAPPED_PE ; 


typedef struct _MAPPED_PE_HEAND_ 
{
/*0x000*/ int			nTotalCounts ;
/*0x004*/ PERESOURCE	QueueLockList	; // �����������
/*0x008*/ MAPPED_PE	ListHead ;

} MAPPED_PE_HEAND, *PMAPPED_PE_HEAND ;


typedef struct _FAKE_FUNC_INFO_ 
{
	int		RetAddr   ; // ���淵�ص�ַ
	LONG	RefCounts ; 

} FAKE_FUNC_INFO, *LPFAKE_FUNC_INFO ;


//
// SSDT & Shadow SSDT������ؽṹ��
//

typedef struct _SSDT_SSSDT_FUNC_ // size - 0x24
{
/*0x000*/ LPWSTR wszModuleName	;	// ģ���� eg:"NTDLL"
/*0x004*/ LPSTR	szFunctionName	;	// ������ eg: "ZwCreateToken"
/*0x008*/ ULONG	ArgumentNumbers ;	// (Shadow)SSDT�����Ĳ�������
/*0x00C*/
		union 
		{
			ULONG   xxIndex ;
			ULONG	SSDTIndex ;		// SSDT������������
			ULONG	ShadowIndex ;	// Shadow SSDT������������
		} _IndexU_ ;

/*0x010*/ ULONG SpecailVersion ; // ָ����ǰ���˺�����Ӧ�Ĳ���ϵͳƽ̨
// ����SendNotifyMessageW��win2k����NtUserSendNotifyMessage,����ƽ̨����NtUserMessageCall
// ��ô�õ�Ԫ��ֵΪ__win2kʱ��Ӧ��fake������fake_NtUserSendNotifyMessage; ��ֵĬ����Ϊ0

/*0x014*/
		union 
		{
			ULONG fakeFuncAddr ;
			ULONG fake_shadowFunc_addr ; // Shadow ssdt��������ַ
			ULONG fake_ssdtFunc_addr ;   // ssdt��������ַ
		} _AddrU_ ;

/*0x018*/ ULONG Tag ;				// ÿ����Ԫ�����,���ڲ���ָ���ĺ���
/*0x01C*/ ULONG HookType ; // �ҹ����� (Ŀǰ��2��: Header 5 Bytes Jmp / call Hook)
/*0x020*/ ULONG MappedFuncAddr	;	// ������ַ(����ͨ��GetProcAddress���) 
/*0x024*/ ULONG RealFuncAddr	;	// ��������ʵ��ַ
/*0x028*/ BOOL  bHooked ;			// �Ƿ��Ѿ�HOOK��
/*0x02C*/ FAKE_FUNC_INFO FakeFuncInfo ; // ������˺����ķ���ֵ(��ַ) / ���ü��� ����Ϣ

} SSDT_SSSDT_FUNC, *LPSSDT_SSSDT_FUNC ;



typedef struct _SDT_DATA_ // size - 0x14
{
/*0x000*/ BOOL  bInited ;				 // �Ƿ��ʼ���ɹ�
/*0x004*/ int	 TotalCounts ;			 // ������ܸ���
/*0x008*/ LPSSDT_SSSDT_FUNC pSdtArray ; // ������׵�ַ

/*0x00C*/ int SpecCounts ;			 // ����������ܸ���
/*0x010*/ LPSSDT_SSSDT_FUNC SpecArray ; // ����������׵�ַ

/*0x014*/ int	 ShadowArrayIndex ;		 // �ܵ�sdt������,Shadows SSDT��Ԫ����ʼ���(Index)
/*0x018*/ int	 ShadowSSDTCounts ;		 // ������Shadow SSDT����

} SDT_DATA, *PSDT_DATA ;



#define SSDT_TAG_LowerLimit			0	 // ssdt �������кŵ�����	
#define SSDT_TAG_UperLimit			100  // ssdt �������кŵ�����.���Զ����һЩ,�Ժ��������

#define ShadowSSDT_TAG_LowerLimit	199	 // shadow ssdt �������кŵ�����	


// ��ǰTAG�Ƿ�����ssdt������Ԫ��
#define IS_SSDT_TAG( l ) ( l > SSDT_TAG_LowerLimit && l <= SSDT_TAG_UperLimit )

// ��ǰTAG�Ƿ�����shadow ssdt������Ԫ��
#define IS_ShadowSSDT_TAG( l ) ( l > ShadowSSDT_TAG_LowerLimit )

// (shadow)ssdt �������Ƿ�Ƿ�; TRUE��ʾ�Ƿ�
#define IS_INVALID_INDEX( l ) ( l == 0xFFFFFFFF || l >= 0x2000 || (l & 0xFFF) >= 0x600 )


//
// ö������,���ڲ���ָ���ĺ���
//


typedef enum _SPECIAL_SSDT_FUNC_Tag_ 
{
	TagSpec_NtRequestPort = 0,
	TagSpec_NtRequestWaitReplyPort = 1,
	TagSpec_NtAlpcSendWaitReceivePort = 2
};


typedef enum _SSDT_SSSDT_FUNC_Tag_ 
{
	Tag_ZwCreateToken = SSDT_TAG_LowerLimit + 1,// 1
	Tag_ZwSetInformationToken,
	Tag_ZwProtectVirtualMemory,
	Tag_ZwQueryInformationThread,

	// ******* ssdt & shadow ssdt Tag�ķֽ��� *******

	Tag_GetForegroundWindow = ShadowSSDT_TAG_LowerLimit + 1, // 200
	Tag_IsHungAppWindow,
	Tag_GetClassNameW,
	Tag_SetWindowsHookExW,
	Tag_SetWinEventHook,
	Tag_PostMessageW,
	Tag_PostThreadMessageW,
	Tag_EnableWindow,
	Tag_DestroyWindow,
	Tag_SendInput,
	Tag_BlockInput,
	Tag_SetSysColors,
	Tag_SystemParametersInfoW,
	Tag_SendMessageTimeoutW,
	Tag_SendNotifyMessageW_win2k,
	Tag_SendNotifyMessageW,
	Tag_SendMessageCallbackW_win2k,
	Tag_SendMessageCallbackW,
};



typedef enum _sdt_show_info_ 
{
	_SDTWalkNodes_All_		= 0, // 0Ϊ��ӡȫ��;
	_SDTWalkNodes_SSDT_		= 1, // 1Ϊ��ӡSSDT����
	_SDTWalkNodes_Shadow_	= 2, // 2Ϊ��ӡShadow SSDT����

} sdt_show_info ;



enum _AddressFindMethod_
{
	_AddressFindMethod_Shadow_	= 1,
	_AddressFindMethod_SSDT_	= 2,
};


enum _IndexCheckMethod_
{
	_IndexCheckMethod_Shadow_	= 1,
	_IndexCheckMethod_SSDT_		= 2,
};


#define IS_INVALID_METHOD( l ) ( l > 0 && l < 3 )


//////////////////////////////////////////////////////////////////////////

// others
extern BOOL g_MappedPE_Inited_ok ;

extern PMAPPED_PE_HEAND g_ListHead__MappedPE ; 

extern SDT_DATA g_SdtData ;

/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         ����Ԥ����                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


BOOL
InitSdtData (
	);

ULONG
GetProcAddress (
	IN LPWSTR wszModuleName,
	IN LPSTR szFunctionName,
	IN BOOL bReloc
	);

PVOID
LoadPE (
	IN LPWSTR wszModuleName
	);

ULONG 
GetMappedFuncAddr (
	IN PVOID _pMappedInfo,
	IN LPSTR szFunctionName, 
	IN BOOL bReloc
	);

BOOL 
MPCreateTotalHead(
	OUT PVOID* _TotalHead
	);

VOID 
MPDeleteTotalHead(
	IN PVOID* _TotalHead
	);

BOOL 
MPAllocateNode(
	OUT PVOID* _pCurrenList_
	);

PVOID
MPFindNode (
	IN PVOID  _TotalHead,
	IN LPWSTR wszModuleName
	);

PVOID
MPBuildNode (
	IN PVOID _TotalHead ,
	IN LPWSTR wszModuleName
	);

PVOID
MPBuildNodeEx (
	IN PVOID _TotalHead ,
	IN LPWSTR wszModuleName
	);

VOID
MPDistroyAll (
	IN PVOID _TotalHead
	);

VOID
MPWalkNodes (
	IN PVOID _TotalHead
	);

ULONG 
GetRealAddr (
	IN ULONG addr,
	IN PVOID _pMappedInfo
	);

BOOL
IsValidPE (
	ULONG PEAddr
	);



VOID
SDTWalkNodes (
	IN int Index
	);

BOOL
Get_sdt_function_addr (
	OUT PVOID _pNode,
	IN int AddressFindMethod,
	IN int IndexCheckMethod
	);

ULONG
Get_sdt_function_addr_normal (
	IN ULONG Index,
	IN ULONG ArgumentNumbers
	);

ULONG
Get_sdt_function_addr_force (
	IN ULONG Index
	);

ULONG
Get_sdt_function_addr_force_step1 (
	IN ULONG Index
	);

ULONG
Get_sdt_function_addr_force_step2 (
	IN ULONG Index
	);

BOOL
Get_sdtfunc_addr (
	IN PULONG addr,
	IN ULONG Tag
	);

ULONG
Get_KeServiceDescriptorTable_addr(
	) ;

PVOID
Get_sdt_Array (
	IN int Tag
	);

PVOID
Get_sdt_Array_spec (
	IN int Tag
	);

ULONG
Get_sdtfunc_addr_ex (
	IN int Tag
	);

ULONG
GetSDTIndex (
	IN ULONG pMappedAddr,
	IN BOOL bIsShadow
	);

int
GetSDTIndexEx (
	IN ULONG pMappedAddr
	);

int
GetSDTIndexExp (
	IN ULONG pMappedAddr,
	IN ULONG AddressArray,
	IN ULONG DeepCounts,
	IN int CurrentResult
	);

BOOL 
IsFunctionEnd (
	PBYTE CurrentEIP
	);

BOOL 
IsKiFastSystemCall (
	PBYTE CurrentEIP
	);

BOOL
PatchSDTFunc (
	IN PVOID _pNode,
	IN BOOL bHook
	);

///////////////////////////////   END OF FILE   ///////////////////////////////