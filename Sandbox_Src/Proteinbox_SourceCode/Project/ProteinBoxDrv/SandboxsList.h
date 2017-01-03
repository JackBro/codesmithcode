#pragma once

//////////////////////////////////////////////////////////////////////////


//
// ɳ��������ܽڵ�
//

typedef struct _SANDBOX_NODE_INFO_
{
/*0x000 */ LIST_ENTRY ListEntry ;

/*+0x008*/ ULONG NameLength ;	// ɳ��������
/*+0x00C*/ LPWSTR wszName ;	// ɳ����ָ��. eg: L"sudami"
/*+0x00C*/ ULONG Index ; // ��R3ѭ��ö��ɳ����ʱʹ��

} SANDBOX_NODE_INFO, *LPSANDBOX_NODE_INFO ;


typedef struct _SANDBOX_NODE_INFO_HEAND_ 
{
/*0x000 */ ULONG			nTotalCounts ;
/*0x004 */ PERESOURCE	QueueLockList	; // �����������
/*0x008 */ SANDBOX_NODE_INFO ListHead ;

} SANDBOX_NODE_INFO_HEAND, *LPSANDBOX_NODE_INFO_HEAND ;

extern LPSANDBOX_NODE_INFO_HEAND g_ListHead__Sandboxs ;

extern BOOL g_SandboxLists_Inited_ok ;



// �����:����ڵ�������β
#define kInsertTailSB( _pNode ) \
{								\
	EnterCrit( g_ListHead__Sandboxs->QueueLockList );	\
	InsertTailList( (PLIST_ENTRY) &g_ListHead__Sandboxs->ListHead, (PLIST_ENTRY) _pNode ); \
	g_ListHead__Sandboxs->nTotalCounts ++ ;	\
	LeaveCrit( g_ListHead__Sandboxs->QueueLockList );	\
}

#define kfreeSBL()	SBLDistroyAll( (PVOID)g_ListHead__Sandboxs )
#define kbuildnodeSBL( _Tag1, _Tag2, _Tag3 ) SBLBuildNode(	(PVOID)g_ListHead__Sandboxs, (LPWSTR)_Tag1, (ULONG)_Tag2, (BOOL*)_Tag3 )
#define kgetnodeSBL( _Tag1, _Tag2 )			 SBLFindNode(	(PVOID)g_ListHead__Sandboxs, (LPWSTR)_Tag1, (ULONG)_Tag2 )
#define kgetnodeExSBL( _Tag )				 SBLFindNodeEx(	(PVOID)g_ListHead__Sandboxs, (ULONG)_Tag )


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
InitSandboxsData (
	);

int SBLGetSandboxsCounts();

BOOL 
SBLCreateTotalHead(
	OUT PVOID* _TotalHead
	);

VOID 
SBLDeleteTotalHead(
	IN PVOID* _TotalHead
	);

BOOL 
SBLAllocateNode(
	OUT PVOID* pNode
	);

PVOID
SBLBuildNode (
	IN PVOID _TotalHead,
	IN LPWSTR wszName,
	IN ULONG NameLength,
	OUT BOOL* bOverload
	);

PVOID
SBLFindNode (
	IN PVOID _TotalHead ,
	IN LPWSTR wszName,
	IN ULONG NameLength
	);

PVOID
SBLFindNodeEx (
	IN PVOID _TotalHead ,
	IN ULONG Index
	);

VOID
SBLDistroyAll (
	IN PVOID _TotalHead
	);

VOID
SBLWalkNodes (
	IN PVOID _TotalHead
	);

///////////////////////////////   END OF FILE   ///////////////////////////////