#pragma once

//////////////////////////////////////////////////////////////////////////


// �����ڴ�ṹ��
#define LPCDHEAD	LPPB_CONFIG_TOTAL
#define LPCDNODES	LPPB_CONFIG_SECTION
#define LPCDNODEK	LPPB_CONFIG_KEY
#define LPCDNODEV	LPPB_CONFIG_VALUE

#define CDHEAD		PB_CONFIG_TOTAL
#define CDNODES		PB_CONFIG_SECTION
#define CDNODEK		PB_CONFIG_KEY
#define CDNODEV		PB_CONFIG_VALUE


// ������� GrayList �ڵ�ĺ���
#define kGetKeyNode( SeactionName, KeyName, pOutBuffer )		CDFindNodeEx(	  (PVOID)g_ProteinBox_Conf, SeactionName, KeyName, NULL, pOutBuffer )
#define kIsValueNameExist( SeactionName, KeyName, ValueName )	Is_ValueName_Exist( (PVOID)g_ProteinBox_Conf, SeactionName, KeyName, ValueName )
#define kfreeCD()							CDDistroyAll( (PVOID)g_ProteinBox_Conf		)
#define kWalkCD()							CDWalkNodes(  (PVOID)g_ProteinBox_Conf		)


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                  �ܵ������ļ���Ϣ                    +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


enum _CDFINDNODE_TYPE_ 
{
	_NODE_IS_SECTION_ = 1,
	_NODE_IS_KEY_,
	_NODE_IS_VALUE_,
	_NODE_IS_NOTHING_
};

#define IS_VALID_NODETYPE( Tag ) ( Tag >=_NODE_IS_SECTION_ && Tag <=_NODE_IS_VALUE_ )


typedef struct _SEARCH_INFO_ 
{
/*0x000 */ PVOID pNode ;
/*0x004 */ ULONG NodeType ;
} SEARCH_INFO, *LPSEARCH_INFO ;


typedef struct _PB_CONFIG_COMMON_ {

/*0x000 */ LIST_ENTRY List ;
/*+0x008*/ ULONG NameLength ;	// �ַ�������
/*+0x00C*/ LPWSTR wszName ;		// �ַ������ݵ�ָ��.

} PB_CONFIG_COMMON, *LPPB_CONFIG_COMMON ;


typedef struct _PB_CONFIG_COMMONEX_ {

/*0x000 */ LIST_ENTRY List ;
/*+0x008*/ ULONG NameLength ;	// �ַ�������
/*+0x00C*/ LPWSTR wszName ;		// �ַ������ݵ�ָ��.

/*0x010 */ LIST_ENTRY SunList ;

} PB_CONFIG_COMMONEX, *LPPB_CONFIG_COMMONEX ;



typedef struct _PB_CONFIG_VALUE_ {

/*0x000 */ struct _PB_CONFIG_VALUE_* pFlink ; // �ϸ����
/*0x004 */ struct _PB_CONFIG_VALUE_* pBlink ; // �¸����

/*+0x008*/ ULONG NameLength ;	// �ַ�������
/*+0x00C*/ LPWSTR ValueName ;	// �ַ������ݵ�ָ��. eg: c:\Test.exe

} PB_CONFIG_VALUE, *LPPB_CONFIG_VALUE ;



typedef struct _PB_CONFIG_KEY_ {

/*0x000 */ struct _PB_CONFIG_KEY_* pFlink ; // �ϸ����
/*0x004 */ struct _PB_CONFIG_KEY_* pBlink ; // �¸����

/*+0x008*/ ULONG NameLength ;	// �ַ�������
/*+0x00C*/ LPWSTR KeyName ;		// �ַ������ݵ�ָ��. eg: ForceProcess, OpenIpcPath

/*0x010 */ PB_CONFIG_VALUE ValueListHead ;

} PB_CONFIG_KEY, *LPPB_CONFIG_KEY ;



typedef struct _PB_CONFIG_SECTION_ 
{
/*0x000 */ struct _PB_CONFIG_SECTION_* pFlink ; // �ϸ����
/*0x004 */ struct _PB_CONFIG_SECTION_* pBlink ; // �¸����

/*+0x008*/ ULONG NameLength ;		// �ַ�������
/*+0x00C*/ LPWSTR SectionName ;		// �ַ������ݵ�ָ��. eg: [GlobalSetting], [UserSetting] 

/*0x010 */ PB_CONFIG_KEY KeyListHead ;

} PB_CONFIG_SECTION, *LPPB_CONFIG_SECTION ;



typedef struct _PB_CONFIG_TOTAL_ 
{
/*0x000 */ int			SectionCounts ;	// Section������.
/*0x004 */ PERESOURCE	QueueLockList ; // �����������
/*0x008 */ PB_CONFIG_SECTION SectionListHead ;

} PB_CONFIG_TOTAL, *LPPB_CONFIG_TOTAL ;

extern LPPB_CONFIG_TOTAL g_ProteinBox_Conf_TranshipmentStation ;
extern LPPB_CONFIG_TOTAL g_ProteinBox_Conf ;
extern LPPB_CONFIG_TOTAL g_ProteinBox_Conf_Old ;
extern BOOL g_ProteinBox_Conf_Inited_ok ;


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
InitConfigData (
	);

BOOL 
CDCreateTotalHead(
	OUT PVOID* _TotalHead
	);

VOID 
CDDeleteTotalHead(
	IN PVOID* _TotalHead
	);

BOOL 
CDAllocateNode(
	OUT PVOID* pCurrenList,
	IN ULONG StructSize
	);

BOOL
CDBuildNode (
	IN PVOID _TotalHead,
	IN PVOID pInBuffer
	);

PVOID
CDBuildNodeEx (
	IN PERESOURCE QueueLockList,
	IN PVOID _ListHead,
	IN ULONG StructSize,
	IN LPWSTR wszName,
	IN int   NameLength
	);

BOOL
CDFindNode (
	IN PVOID _TotalHead ,
	IN PVOID pInBuffer,
	OUT PVOID pOutBuffer
	);

BOOL
CDFindNodeEx (
	IN PVOID _TotalHead ,
	IN LPWSTR SeactionName,
	IN LPWSTR KeyName,
	IN LPWSTR ValueName,
	OUT PVOID pOutBuffer
	);

VOID
CDDistroyList (
	IN PVOID ListHead,
	IN ULONG DeepCounts
	);

VOID
CDDistroyAll (
	IN PVOID _TotalHead
	);

VOID
CDDistroyAllEx (
	IN PVOID _TotalHead
	);

VOID
CDWalkNodes (
	IN PVOID _TotalHead
	);

BOOL
Is_ValueName_Exist (
	IN PVOID _TotalHead ,
	IN LPWSTR SeactionName,
	IN LPWSTR KeyName,
	IN LPWSTR ValueName
	);

BOOL 
GetPBDllPath(
	IN ULONG uMethodType,
	IN ULONG MaxLength,
	OUT LPSTR* szPath
	);

BOOL 
GetPBDllPathFromReg(
	IN ULONG MaxLength,
	OUT LPSTR szPath
	);

BOOL GetPBDllPathFromIni( OUT LPSTR *lpszPath, IN BOOL bReload );
BOOL GetPBDllPathFromIniW( OUT LPWSTR lpwszPath );


///////////////////////////////   END OF FILE   ///////////////////////////////