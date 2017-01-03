#pragma once

//////////////////////////////////////////////////////////////////////////

#define _NtOpenKey_Tag_		'SUDA'
#define _DirtyValueKeyTag_	'SUDA'	// ���ɳ����"�ѱ�ɾ��"�ļ�ֵ

//
// ��ǰ��ӵ�еļ�ֵ��Ϣ (ValueKey)
//

typedef struct _XREG_INFORMATION_VALUEKEY_
{
	struct _XREG_INFORMATION_VALUEKEY_* pFlink ;		// �ϸ��ڵ�	
	struct _XREG_INFORMATION_VALUEKEY_* pBlink ;		// �¸��ڵ�
	ULONG ValueType ;		// ��ֵ����
	ULONG ValueDataLength ; // ��ֵ�����ݴ�С
	PVOID ValueData ;		// ��ֵ�����ݵ�ַ
	ULONG NameLength ;		// ��ֵ������
	WCHAR wszValueKeyName[1] ; 

} XREG_INFORMATION_VALUEKEY, *LPXREG_INFORMATION_VALUEKEY ;


//
// ��ǰ��ӵ�е��Ӽ���Ϣ (SubKey)
//

typedef struct _XREG_INFORMATION_SUBKEY_
{
	struct _XREG_INFORMATION_SUBKEY_* pFlink ;		// �ϸ��ڵ�	
	struct _XREG_INFORMATION_SUBKEY_* pBlink ;		// �¸��ڵ�  
	ULONG NameLength ;				// ��ֵ������
	ULONG xx1 ;
	LARGE_INTEGER LastWriteTime;    
	BOOL bClassName ;
	BOOL bxx2 ;
	WCHAR wszSubKeyName[1] ;

} XREG_INFORMATION_SUBKEY, *LPXREG_INFORMATION_SUBKEY ;


/*++

���ڵ�ǰ��,������3����Ϣ: 
  (1) ��������Ϣ	  - struct _XREG_INFORMATION_CLASS_
  (2) ����ļ�ֵ��Ϣ  - struct _XREG_INFORMATION_VALUEKEY_
  (3) �����Ӽ���Ϣ	  - struct _XREG_INFORMATION_SUBKEY_

--*/

//
// ��ǰ��ֵ������Ϣ
//

typedef struct _XREG_INFORMATION_CLASS_ {

	LIST_ENTRY_EX ListEntry ;

	HANDLE hKey ; 
	ULONG TickCount ; 

	BOOL bFlag_IsIn_WhiteList ; // Ϊ1������ǰ�������ǰ�������ֵ
	BOOL bFlag_Handler_SubKeyNodes_NtEnumerateKey_OK ;
	BOOL bFlag2 ;
	BOOL bFlag3 ;

	ULONG unknown1 ; 
	LARGE_INTEGER LastWriteTime;  

////////////////////////////////////////////////////////////////////////////

	struct _XREG_INFORMATION_SUBKEY_* pSubKey_ShowTime_Firstone ;	// (���ָ��û���)��һ���Ӽ����
	struct _XREG_INFORMATION_SUBKEY_* pSubKey_ShowTime_Lastone ;	// (���ָ��û���)���һ���Ӽ����
	
	ULONG nCounts_SubKeys_ShowTime ;								// (���ָ��û���)�Ӽ�����
// ����ɳ����ԭ����4���Ӽ�,�����Ѿ���ɾ��3��. ������ʵϵͳ��ɳ����ض�λĿ¼�ﲢδɾ��,�������Ϊ"��ɾ��"״̬.
// ����ʱ������3����Ԫ�м�¼�ľ���ʣ���Ǹ�δ��ɾ���ļ�ֵ. ���������ɳ����Ҫ���ָ��û����ļ�ֵ��Ԫ.
// ���԰�������ľ���,�� pSubKeys_Firstone == pSubKeys_Lastone = 0xxx; nCounts_SubKeys == 1 ;

	ULONG PreviousIndex ; // ö�ټ�ֵʱʼ�ձ����ϴ�ö�ٵ�Index
// �ñ����� үү��\Registry\machine\software ��ǰ����\Registry\machine\software\360 �������ĸ��Ӽ�:
// HKEY_LOCAL_MACHINE\software\360\MJ0011
// HKEY_LOCAL_MACHINE\software\360\PJF
// HKEY_LOCAL_MACHINE\software\360\sudami
// HKEY_LOCAL_MACHINE\software\360\WangYu

////////////////////////////////////////////////////////////////////////////

	struct _XREG_INFORMATION_SUBKEY_* pSubKeys_ShowTime_Previous ; // ö�ټ�ֵʱʼ�ձ����ϴ�ö������ʾ���û���Subkey Node

	BOOL bFlag_Handler_SubValueNodes_NtEnumerateValueKey_OK ;
	BOOL bFlag_b ;
	BOOL bFlag_c ;
	BOOL bFlag_d ;

	struct _XREG_INFORMATION_VALUEKEY_* pValueKey_ShowTime_Lastone ;	// (���ָ��û���)��һ���Ӽ����
	struct _XREG_INFORMATION_VALUEKEY_* pValueKey_ShowTime_Firstone ;	// (���ָ��û���)���һ���Ӽ����
	ULONG nCounts_ValueKeys_ShowTime ;									// (���ָ��û���)��ֵ����(ValueKey)

	ULONG NameLength ;				// ��ֵ������
	WCHAR wszOrignalRegPath[1] ;	// �������ԭʼҪ�����ļ�ֵȫ·��

} XREG_INFORMATION_CLASS, *LPXREG_INFORMATION_CLASS ;

extern LIST_ENTRY_EX g_XREG_INFORMATION_HEAD_Redirected ;
extern LIST_ENTRY_EX g_XREG_INFORMATION_HEAD_Orignal ;


typedef struct _KEYINFO_ 
{
	union
	{
		KEY_FULL_INFORMATION Full ;
		KEY_BASIC_INFORMATION Basic ;
		KEY_NODE_INFORMATION Node ;

		LARGE_INTEGER LastWriteTime ;
	} u ;

} KEYINFO, *LPKEYINFO ;


typedef struct _xx_KEY_VALUE_INFORMATION_
{
	union
	{
		KEY_VALUE_BASIC_INFORMATION BasicInfo ;
		KEY_VALUE_FULL_INFORMATION FullInfo ;
		KEY_VALUE_PARTIAL_INFORMATION PartialInfo ;
		KEY_VALUE_PARTIAL_INFORMATION_ALIGN64 PartialInfoAlign64 ;
	};
} xx_KEY_VALUE_INFORMATION, *LPxx_KEY_VALUE_INFORMATION;

extern CRITICAL_SECTION g_cs_Regedit ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         ����Ԥ����                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


typedef ULONG (WINAPI* _NtQueryKey_)(
    IN HANDLE KeyHandle,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    OUT PVOID KeyInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
	);
extern _NtQueryKey_		g_NtQueryKey_addr ;

typedef ULONG (WINAPI* _NtOpenKey_) (
    OUT PHANDLE KeyHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
	);
extern _NtOpenKey_		g_NtOpenKey_addr ;

typedef ULONG (WINAPI* _NtCreateKey_) (
	OUT PHANDLE KeyHandle,
	IN ACCESS_MASK DesiredAccess,
	IN POBJECT_ATTRIBUTES ObjectAttributes,
	IN ULONG TitleIndex,
	IN PUNICODE_STRING Class OPTIONAL,
	IN ULONG CreateOptions,
	IN PULONG Disposition OPTIONAL
	);
extern _NtCreateKey_	g_NtCreateKey_addr ;

typedef ULONG (WINAPI* _NtDeleteKey_) (
    IN HANDLE KeyHandle
	);
extern _NtDeleteKey_	g_NtDeleteKey_addr ;

typedef ULONG (WINAPI* _NtEnumerateKey_)(
	IN HANDLE KeyHandle,
	IN ULONG Index,
	IN KEY_INFORMATION_CLASS KeyInformationClass,
	OUT PVOID KeyInformation,
	IN ULONG Length,
	OUT PULONG ResultLength
	);
extern _NtEnumerateKey_	g_NtEnumerateKey_addr ;

typedef ULONG (WINAPI* _NtEnumerateValueKey_)(
	IN HANDLE KeyHandle,
	IN ULONG Index,
	IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
	OUT PVOID KeyValueInformation,
	IN ULONG Length,
	OUT PULONG ResultLength
	);
extern _NtEnumerateValueKey_ g_NtEnumerateValueKey_addr ;

typedef ULONG (WINAPI* _NtNotifyChangeMultipleKeys_) (
	IN HANDLE MasterKeyHandle,
	IN ULONG Count,
	IN POBJECT_ATTRIBUTES SlaveObjects,
	IN HANDLE Event,
	IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
	IN PVOID ApcContext OPTIONAL,
	OUT PIO_STATUS_BLOCK IoStatusBlock,
	IN ULONG CompletionFilter,
	IN BOOLEAN WatchTree,
	OUT PVOID Buffer,
	IN ULONG Length,
	IN BOOLEAN Asynchronous
	);

typedef ULONG (WINAPI* _NtLoadKey_) (
    IN POBJECT_ATTRIBUTES KeyObjectAttributes,
    IN POBJECT_ATTRIBUTES FileObjectAttributes
	);

typedef ULONG (WINAPI* _NtRenameKey_) (
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ReplacementName
	);

typedef ULONG (WINAPI* _NtDeleteValueKey_)(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName
	);

typedef ULONG (WINAPI* _NtSetValueKey_)(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName,
    IN ULONG TitleIndex OPTIONAL,
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize
	);

typedef ULONG (WINAPI* _NtQueryValueKey_) (
	IN HANDLE KeyHandle,
	IN PUNICODE_STRING ValueName,
	IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
	OUT PVOID KeyValueInformation,
	IN ULONG Length,
	OUT PULONG ResultLength
	);

typedef ULONG (WINAPI* _NtQueryMultipleValueKey_)(
	IN HANDLE KeyHandle,
	IN OUT PKEY_VALUE_ENTRY ValueList,
	IN ULONG NumberOfValues,
	OUT PVOID Buffer,
	IN OUT PULONG Length,
	OUT PULONG ReturnLength
	);



NTSTATUS
GetRegPath (
	IN HANDLE hRootKey,
	IN PUNICODE_STRING uniObjectName,
	OUT LPWSTR* OrignalPath,
	OUT LPWSTR* RedirectedPath,
	OUT BOOL* bIsHandlerSelfRegPath
	);

BOOL IsRedirectedKeyInvalid ( IN LPWSTR RedirectedPath );

BOOL IsDirtyKey( IN HANDLE KeyHandle );

BOOL IsDirtyKeyEx ( IN PVOID KeyBaseInfo );

NTSTATUS MarkDirtyKey( IN HANDLE KeyHandle );

NTSTATUS MarkKeyTime( IN HANDLE KeyHandle );

NTSTATUS CreateRedirectedSubkeys( IN POBJECT_ATTRIBUTES objectAttributes );

VOID RemoveRegNodeEx ( IN HANDLE KeyHandle, IN BOOL bRemoveFatherNode );

VOID RemoveRegNodeExp( IN LPWSTR szPath, BOOL bRemoveFatherNode );

VOID RemoveRegNodeCom ( IN PVOID pNode, IN PVOID pHead );

VOID RemoveRegNodeComEx ( IN PVOID pNode, IN BOOL bFlag_Free_Current_Node );

NTSTATUS StubNtDeleteKey ( IN HANDLE KeyHandle, IN BOOL bRecursive );

NTSTATUS 
RegKeyFilter (
	IN HANDLE KeyHandle,
	IN LPWSTR OrignalPath,
	IN LPWSTR RedirectedPath,
	IN BOOL bIs_NtEnumerateKey_called,
	IN BOOL bIs_NtEnumerateValueKey_called,
	OUT PVOID * out_pCurrentRegNode_Redirected
	);

NTSTATUS 
Handler_SubKeyNodes_NtEnumerateKey (
	IN LPXREG_INFORMATION_CLASS RedirectedNode,
	IN LPXREG_INFORMATION_CLASS OrignalNode,
	IN HANDLE hRedirectedKey
	);

NTSTATUS 
Handler_SubValueNodes_NtEnumerateValueKey (
	IN LPXREG_INFORMATION_CLASS RedirectedNode,
	IN LPXREG_INFORMATION_CLASS OrignalNode,
	IN HANDLE hRedirectedKey
	);

NTSTATUS 
Handler_RegNode_Orignal (
	OUT PHANDLE phRedirectedKey,
	IN LPWSTR OrignalPath,
	IN LPWSTR RedirectedPath,
	OUT PVOID *pOrignalNode
	);

NTSTATUS 
Refresh_Orignal_RegNode (
	IN HANDLE hOrignalKey,
	IN PLARGE_INTEGER LastWriteTime,
	IN LPWSTR OrignalPath,
	OUT PVOID *out_RegNode_Orignal
	);

NTSTATUS 
BuildSubKeysLists (
	IN LPXREG_INFORMATION_CLASS pCurrentOrignalNode,
	IN HANDLE hOrignalKey
	);

NTSTATUS 
BuildValueKeysLists (
	IN LPXREG_INFORMATION_CLASS pCurrentOrignalNode,
	IN HANDLE hOrignalKey
	);

BOOL 
IsDirtyVauleKey (
	IN PVOID KeyValueInformation,
	IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass
	);

NTSTATUS 
FillValueKeyInfo (
	IN LPXREG_INFORMATION_VALUEKEY pValueKey_ShowTime_Current ,
	IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass ,
	IN PVOID KeyValueInformation,
	IN ULONG Length ,
	OUT PULONG ResultLength
	);

NTSTATUS 
FillSubKeyInfo (
	IN ULONG RegNameLength,
	IN LPWSTR wszRegFullPath,
	IN KEY_INFORMATION_CLASS KeyInformationClass ,
	OUT PVOID KeyInformation,
	IN ULONG Length ,
	OUT PULONG ResultLength,
	IN PLARGE_INTEGER LastWriteTime
	);

///////////////////////////////   END OF FILE   ///////////////////////////////