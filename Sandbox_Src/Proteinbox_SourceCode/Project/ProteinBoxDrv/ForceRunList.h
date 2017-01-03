#pragma once

//////////////////////////////////////////////////////////////////////////


//
// ��ǿ��������ɳ���еĽ���������ܽڵ�
//

typedef struct _FORCEPROC_NODE_INFO_
{
/*0x000 */ LIST_ENTRY ListEntry ;

/*+0x008*/ int NameLength ;	// ����ȫ·������
/*+0x00C*/ LPWSTR wszProcFullPath ;	// ����ȫ·��
/*+0x00C*/ BOOL bRunInSandbox ; // [����]��Ǹó����Ƿ�Ҫ��ǿ��������ɳ����

} FORCEPROC_NODE_INFO, *LPFORCEPROC_NODE_INFO ;


typedef struct _FORCEPROC_NODE_INFO_HEAND_ 
{
/*0x000 */ int			nTotalCounts ;
/*0x004 */ PERESOURCE	QueueLockList	; // �����������
/*0x008 */ FORCEPROC_NODE_INFO ListHead ;

} FORCEPROC_NODE_INFO_HEAND, *LPFORCEPROC_NODE_INFO_HEAND ;

extern LPFORCEPROC_NODE_INFO_HEAND g_ListHead__ForceProc ;

extern BOOL g_ForceProcLists_Inited_ok ;
extern BOOL g_bForceAll2RunInSandbox ;
extern BOOL g_bAllRunOutofSandbox ; 


// �����:����ڵ�������β
#define kInsertTailFP( _pNode ) \
{								\
	EnterCrit( g_ListHead__ForceProc->QueueLockList );	\
	InsertTailList( (PLIST_ENTRY) &g_ListHead__ForceProc->ListHead, (PLIST_ENTRY) _pNode ); \
	g_ListHead__ForceProc->nTotalCounts ++ ;	\
	LeaveCrit( g_ListHead__ForceProc->QueueLockList );	\
}

#define kfreeFPL()	FPLDistroyAll( (PVOID)g_ListHead__ForceProc )
#define kbuildnodeFPL( _Tag1, _Tag2 ) FPLBuildNode(	(PVOID)g_ListHead__ForceProc, (LPWSTR)_Tag1, (int)_Tag2 )
#define kgetnodeFPL( _Tag )			  FPLFindNode(	(PVOID)g_ListHead__ForceProc, (LPWSTR)_Tag )
#define kDisableAllFPL()			  FPLSetStateAll((PVOID)g_ListHead__ForceProc, FALSE)
#define kEnableAllFPL()				  FPLSetStateAll((PVOID)g_ListHead__ForceProc, TRUE)


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
InitForceProcData (
	);

BOOL 
FPLCreateTotalHead(
	OUT PVOID* _TotalHead
	);

VOID 
FPLDeleteTotalHead(
	IN PVOID* _TotalHead
	);

BOOL 
FPLAllocateNode(
	OUT PVOID* pNode
	);

PVOID
FPLBuildNode (
	IN PVOID _TotalHead,
	IN LPWSTR wszName,
	IN int NameLength
	);

PVOID
FPLFindNode (
	IN PVOID _TotalHead ,
	IN LPWSTR wszName
	);

BOOL
FPLGetState (
	IN LPWSTR wszName,
	IN BOOL* bRunInSandbox
	);

VOID
FPLDistroyAll (
	IN PVOID _TotalHead
	);

VOID
FPLSetStateAll (
	IN PVOID _TotalHead,
	IN BOOL bRunInSandbox
	);

VOID
FPLWalkNodes (
	IN PVOID _TotalHead
	);

///////////////////////////////   END OF FILE   ///////////////////////////////