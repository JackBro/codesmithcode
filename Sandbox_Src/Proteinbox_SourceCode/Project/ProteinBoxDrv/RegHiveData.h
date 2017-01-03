#pragma once

//////////////////////////////////////////////////////////////////////////

//
// �����ڴ�ṹ��
//

#define LPRHHEAD LPREGHIVE_NODE_INFO_HEAND
#define LPRHNODE LPREGHIVE_NODE_INFO

#define RHHEAD REGHIVE_NODE_INFO_HEAND
#define RHNODE REGHIVE_NODE_INFO

//
// ��������ڴ�ĺ���
//

#define kgetnodeRH( Tag1, Tag2, Tag3 )	RHFindNode( (PVOID)g_ListHead__RegHive, Tag1, Tag2, Tag3 )
#define kbuildnodeRH( _Tag )			RHBuildNode( (PVOID)g_ListHead__RegHive, _Tag	)
#define kfreeRH()						RHDistroyAll( (PVOID)g_ListHead__RegHive		)


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
// size - 0x01C
//

typedef struct _REGHIVE_NODE_INFO_ {

/*0x000 */ struct _REGHIVE_NODE_INFO_* pFlink ;	// �ϸ��ڵ�
/*0x004 */ struct _REGHIVE_NODE_INFO_* pBlink ;	// �¸��ڵ�

/*0x008 */ WCHAR HiveFilePath[ MAX_PATH ] ;  // eg:"\Device\HarddiskVolume1\Sandbox\AV\DefaultBox\RegHive"
/*0x00C */ WCHAR HiveRegPath[ MAX_PATH ] ;   // eg:"\REGISTRY\USER\Sandbox_AV_DefaultBox"
/*0x010 */ ULONG PorcessRefCounts ; // ��ǰɳ���н��̼���,��Ϊ����һ������,�ͻ����һ��Ioctl_StartProcess.���л������ǰɳ���Hive�ļ�.������������ü���
/*0x014 */ BOOL bNeedToDistroy ; // ��TRUE,������ǰRegHive���޽���ռ��,����ж�ص�!
/*0x018 */ BOOL ProcessesLock ; // ���ڶ�����̼��ͬ������!��һ��ɳ���п��ܻ���2+������ͬʱ������ʱ����Hive,���Ա���ͬ��,����Ϊ1��ʾ��ռ��,��������״̬!

} REGHIVE_NODE_INFO, *LPREGHIVE_NODE_INFO ;


//
// RH ������ܽڵ�
//

typedef struct _REGHIVE_NODE_INFO_HEAND_ 
{
	/*0x000 */ int			nTotalCounts ;
	/*0x004 */ PERESOURCE	QueueLockList	; // �������Դ��
	/*0x008 */ REGHIVE_NODE_INFO ListHead ;

} REGHIVE_NODE_INFO_HEAND, *LPREGHIVE_NODE_INFO_HEAND ;

extern LPREGHIVE_NODE_INFO_HEAND g_ListHead__RegHive ;
extern BOOL g_RegHive_Inited_ok ;


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
InitRegHive (
	);

BOOL 
RHCreateTotalHead(
	OUT PVOID* _TotalHead
	);

VOID 
RHDeleteTotalHead(
	IN PVOID* _TotalHead
	);

BOOL 
RHAllocateNode(
	OUT PVOID _pCurrenList_
	);

PVOID
RHBuildNode (
	IN PVOID _TotalHead,
	IN PVOID _ProcessNode
	);

PVOID
RHBuildNodeEx (
	IN PVOID _TotalHead,
	IN LPWSTR HiveRegPath,
	IN LPWSTR HiveFilePath
	);

PVOID
RHFindNode (
	IN PVOID _TotalHead ,
	IN LPWSTR HiveRegPath,
	IN LPWSTR HiveFilePath,
	OUT BOOL* bResult
	);

BOOL
RHDeleteNode (
	IN PVOID _TotalHead ,
	IN ULONG pAddress
	);

VOID
RHDistroyAll (
	IN PVOID _TotalHead
	);

VOID
RHWalkNodes (
	IN PVOID _TotalHead
	);

//////////////////////////////////////////////////////////////////////////

BOOL
CheckRegHive (
	IN PVOID _pNode,
	IN PVOID _ProcessNode
	);

BOOL
CmLoadKey(
	IN PVOID _ProcessNode,
	IN POBJECT_ATTRIBUTES KeyObjectAttributes,
	IN POBJECT_ATTRIBUTES FileObjectAttributes
	);

///////////////////////////////   END OF FILE   ///////////////////////////////