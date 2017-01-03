#pragma once

//////////////////////////////////////////////////////////////////////////

//
// �����ڴ�ṹ��
//

#define LPSDHEAD LPREGISTRY_USERSID_INFO_HEAND
#define LPSDNODE LPREGISTRY_USERSID_INFO

#define SDHEAD REGISTRY_USERSID_INFO_HEAND
#define SDNODE REGISTRY_USERSID_INFO

//
// ��������ڴ�ĺ���
//

#define kgetnodeSD( _Tag )			SDFindNode( (PVOID)g_ListHead__RegistryUserSID, _Tag	)
#define kbuildnodeSD( _Tag )		SDBuildNode( (PVOID)g_ListHead__RegistryUserSID, _Tag	)
#define kfreeSD()					SDDistroyAll( (PVOID)g_ListHead__RegistryUserSID		)



/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+							�ṹ��			                  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


typedef struct _REGISTRY_USERSID_INFO_ {

/*0x000 */ struct _REGISTRY_USERSID_INFO_* pFlink ; // �ϸ����
/*0x004 */ struct _REGISTRY_USERSID_INFO_* pBlink ; // �¸����

/*0x008 */ ULONG StructSize ;  // �ýṹ����ܴ�С
/*0x00C */ ULONG Length_RegitryUserSID ; // �ýṹ������ַ���1�ĳ���
/*0x010 */ ULONG Length_CurrentUserName ; // �ýṹ������ַ���2�ĳ���
/*0x014 */ WCHAR RegitryUserSID[ MAX_PATH ] ; // �ýṹ������ַ���1�ĵ�ַ
/*0x018 */ WCHAR CurrentUserName[ MAX_PATH ] ; // �ýṹ������ַ���2�ĵ�ַ, eg:"AV"

} REGISTRY_USERSID_INFO, *LPREGISTRY_USERSID_INFO ;


//
// SID������ܽڵ�
//

typedef struct _REGISTRY_USERSID_INFO_HEAND_ 
{
/*0x000 */ int			nTotalCounts ;
/*0x004 */ PERESOURCE	QueueLockList	; // �����������
/*0x008 */ REGISTRY_USERSID_INFO ListHead ;

} REGISTRY_USERSID_INFO_HEAND, *LPREGISTRY_USERSID_INFO_HEAND ;

extern LPREGISTRY_USERSID_INFO_HEAND g_ListHead__RegistryUserSID ;
extern BOOL g_SecurityDataManager_Inited_ok ;

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
InitSecurityData (
	);

BOOL 
SDCreateTotalHead(
	OUT PVOID* _TotalHead
	);

VOID 
SDDeleteTotalHead(
	IN PVOID* _TotalHead
	);

BOOL 
SDAllocateNode(
	OUT PVOID _pCurrenList_
	);

PVOID
SDBuildNode (
	IN PVOID _TotalHead,
	IN LPWSTR RegisterUserID
	);

PVOID
SDBuildNodeEx (
	IN PVOID _TotalHead,
	IN LPWSTR RegisterUserID
	);

PVOID
SDFindNode (
	IN PVOID _TotalHead ,
	IN LPWSTR RegisterUserID
	);

BOOL
SDDeleteNode (
	IN PVOID _TotalHead ,
	IN ULONG pAddress
	);

VOID
SDDistroyAll (
	IN PVOID _TotalHead
	);

VOID
SDWalkNodes (
	IN PVOID _TotalHead
	);



///////////////////////////////   END OF FILE   ///////////////////////////////