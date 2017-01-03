#pragma once

//////////////////////////////////////////////////////////////////////////


//
// �����ڴ�ṹ��
//

#define LPMMHEAD PMEMORY_HEAD_INFO
#define LPMMNODE PMEMORY_INFO

#define MMHEAD MEMORY_HEAD_INFO
#define MMNODE MEMORY_INFO

//
// ��������ڴ�ĺ���
//

#define kmallocMM( _nSize, _Tag )	MMBuildNode( (PVOID)g_ListHead__MemoryManager, _nSize, _Tag	)
#define kfreeMM()					MMDistroyAll( (PVOID)g_ListHead__MemoryManager				)
#define kgetaddrMM( _Tag )			MMFindNode  ( (PVOID)g_ListHead__MemoryManager, _Tag		)

//
// others
//

extern BOOL g_MemoryManager_Inited_ok ;


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
// �ڴ�ڵ�
//

typedef struct _MEMORY_INFO_ 
{
/*0x000 */ LIST_ENTRY MemoryListEntry ; // ָ���¸��ڵ�

/*0x004 */ ULONG nSize		; // �ڴ��С
/*0x008 */ ULONG pAddress	; // �ڴ��ַ

/*0x00C */ ULONG Tag		; // �ڴ����

} MEMORY_INFO, *PMEMORY_INFO ;


//
// �ڴ������ܽڵ�
//

typedef struct _MEMORY_HEAD_INFO_ 
{
/*0x000 */ PERESOURCE	QueueLockList	; // �����������
/*0x004 */ ULONG		nTotalCounts    ; // �ӽڵ�����
/*0x004 */ MEMORY_INFO  ListHead		;

} MEMORY_HEAD_INFO, *PMEMORY_HEAD_INFO ;

extern PMEMORY_HEAD_INFO g_ListHead__MemoryManager ;


//
// �ڴ���Ӧ��TAG����
//

//
// ��ЩTag��ǵ��ڴ�ᱻƵ���õ�,һ�㱣���������֮������ݽṹ; ���ɹ���ʱ�ڴ�ʹ��
//

typedef enum _MEMORY_TAG_ 
{
	MTAG___TokenInformation_DefaultDacl_New = 1,
	MTAG___SecurityDescriptor = 2 ,
	MTAG___NewDefaultDacl = 3 ,
	MTAG___Privilege_BlackList = 4 ,
	MTAG___SeAliasAdminsSid = 5 ,
	MTAG___SeAliasPowerUsersSid = 6 ,
	MTAG___SeAuthenticatedUsersSid = 7 ,
	MTAG___SeWorldSid = 8 ,
	MTAG___Context_InfoTip = 9 ,
	MTAG___ADDR_FOR_CaptureStackBackTrace = 10 ,
	MTAG___IOCTL_HANDLERCONF_GLOBAL = 11 ,
	MTAG___InjectDllPath = 12 ,

};


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
InitMemoryManager (
	);

BOOL 
MMCreateTotalHead(
	OUT PVOID* _TotalHead
	);

VOID 
MMDeleteTotalHead(
	IN PVOID* _TotalHead
	);

BOOL 
MMAllocateNode(
	OUT PVOID _pCurrenList_
	);

ULONG
MMBuildNode (
	IN PVOID _TotalHead ,
	IN ULONG nSize,
	IN ULONG Tag
	);

PVOID
MMBuildNodeEx (
	IN PVOID _TotalHead ,
	IN ULONG nSize,
	IN ULONG pAddress,
	IN ULONG Tag
	);

ULONG
MMFindNode (
	IN PVOID _TotalHead ,
	IN ULONG Tag,
	IN ULONG nSize OPTIONAL
	);

PVOID
MMFindNodeEx (
	IN PVOID _TotalHead ,
	IN ULONG Tag,
	IN ULONG nSize OPTIONAL
	);

BOOL
MMDeleteNode (
	IN PVOID _TotalHead ,
	IN ULONG pAddress
	);

VOID
MMDistroyAll (
	IN PVOID _TotalHead
	);

VOID
MMWalkNodes (
	IN PVOID _TotalHead
	);


///////////////////////////////   END OF FILE   ///////////////////////////////