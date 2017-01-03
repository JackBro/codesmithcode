#pragma once

//////////////////////////////////////////////////////////////////////////

// �������óɹ��Ƿ�������ʾ
#define __ObjectProcedure_TRACE__ 0
#define OBTrace if (__ObjectProcedure_TRACE__) dprintf

// fake Open Procedure ��ǰ�ô���
#define PreOpenHelper() \
	pNode = (LPPDNODE) kgetnodePD( 0 );	\
	if ( NULL == pNode ) { goto _Call_OrignalFunc_ ; }	\
	if ( pNode->bDiscard || FALSE == pNode->bProcessNodeInitOK ) \
	{	\
		return ;\
	}

// fake Parse Procedure ��ǰ�ô���
#define PreParseHelper() \
	pNode = (LPPDNODE) kgetnodePD( 0 );	\
	if ( NULL == pNode ) { goto _Call_OrignalFunc_ ; }	\
	if ( pNode->bDiscard || FALSE == pNode->bProcessNodeInitOK ) \
	{	\
	return STATUS_PROCESS_IS_TERMINATING ;\
	}



/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+							�ṹ��			                  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--

typedef enum _OB_OPEN_REASON
{
	ObCreateHandle,
	ObOpenHandle,
	ObDuplicateHandle,
	ObInheritHandle,
	ObMaxOpenReason
} OB_OPEN_REASON;


//
// ObjectPAFileDeviceFilter() ���˺��������һ������@pInBufferָ��ýṹ��
//

typedef struct _IopParseFile_Context_ {

/*0x000 */	union{
				struct {
					UCHAR  LimitLow 	;
					UCHAR  AccessMode	; // KernelMode = 0; UserMode = 1
					USHORT LimitHigh	;
				};

				ULONG xx ;
			} u ;

/*0x004 */  ULONG Disposition ;   /* ��/д/��ִ��
#define FILE_SUPERSEDE                    0x00000000
#define FILE_OPEN                         0x00000001
#define FILE_CREATE                       0x00000002
#define FILE_OPEN_IF                      0x00000003
#define FILE_OVERWRITE                    0x00000004
#define FILE_OVERWRITE_IF                 0x00000005
*/
/*0x008 */  ULONG CreateOptions ; /* �⼸����־λ����R3�϶����,����R0,���ñ�ı�־λ������,����μ�
"D:\Program\R0\Others\_Window_Src\_ReactOS\ReactOS-0.3.3\dll\win32\kernel32\file\Create.c" - CreateFileW (Line: 164)

#define FILE_DIRECTORY_FILE		0x00000001		// �򿪵���Ŀ¼
#define FILE_WRITE_THROUGH		0x00000002
#define FILE_SEQUENTIAL_ONLY		0x00000004
#define FILE_NO_INTERMEDIATE_BUFFERING	0x00000008
#define FILE_SYNCHRONOUS_IO_ALERT	0x00000010
#define FILE_SYNCHRONOUS_IO_NONALERT	0x00000020
#define FILE_NON_DIRECTORY_FILE		0x00000040  // �򿪵����ļ� 
#define FILE_CREATE_TREE_CONNECTION	0x00000080
#define FILE_COMPLETE_IF_OPLOCKED	0x00000100
#define FILE_NO_EA_KNOWLEDGE		0x00000200
#define FILE_OPEN_FOR_RECOVERY		0x00000400
#define FILE_RANDOM_ACCESS		0x00000800
#define FILE_DELETE_ON_CLOSE		0x00001000  // �򿪺�ر� 
#define FILE_OPEN_BY_FILE_ID		0x00002000  // ͨ��MFT�Ŵ��ļ�
#define FILE_OPEN_FOR_BACKUP_INTENT	0x00004000  //
#define FILE_NO_COMPRESSION		0x00008000
#define FILE_RESERVE_OPFILTER		0x00100000
#define FILE_OPEN_REPARSE_POINT		0x00200000  //
#define FILE_OPEN_NO_RECALL		0x00400000
#define FILE_OPEN_FOR_FREE_SPACE_QUERY	0x00800000

����ʱ�����´���:
hFile = CreateFile (
            szFileName, 
            GENERIC_ALL ,
            FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE, 
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL );
����˹��˺���,�ػ񵽵�Ȩ��Ϊ:0x204000,������:
FILE_OPEN_REPARSE_POINT | FILE_OPEN_FOR_BACKUP_INTENT

*/

/*0x00C */	ULONG Options ;
/*0x010 */	ULONG OriginalDesiredAccess ;/* ��ӦCreateFileW | NtCreateFile �Ĳ�����:dwDesiredAccess.
����CreateFileW,������ GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE | GENERIC_ALL
����NtCreateFile,������ FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES | SYNCHRONIZE | DELETE

#define FILE_READ_DATA				0x0001	    // file & pipe
#define FILE_LIST_DIRECTORY			0x0001	    // directory
#define FILE_WRITE_DATA				0x0002	    // file & pipe
#define FILE_ADD_FILE				0x0002	    // directory
#define FILE_APPEND_DATA			0x0004	    // file
#define FILE_ADD_SUBDIRECTORY		0x0004	    // directory
#define FILE_CREATE_PIPE_INSTANCE	0x0004	    // named pipe
#define FILE_READ_EA				0x0008		// file & directory
#define FILE_WRITE_EA				0x0010		// file & directory
#define FILE_EXECUTE				0x0020		// file
#define FILE_TRAVERSE				0x0020		// directory
#define FILE_DELETE_CHILD			0x0040		// directory
#define FILE_READ_ATTRIBUTES		0x0080		// all
#define FILE_WRITE_ATTRIBUTES		0x0100		// all
#define DELETE						0x00010000	// ��ע��Ȩ��
#define SYNCHRONIZE					0x00100000	

*/
} IopParseFile_Context, *LPIopParseFile_Context ;



/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         ����Ԥ����                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


typedef VOID (*OB_OPEN_METHOD)(
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN PVOID Object,
	IN ACCESS_MASK GrantedAccess,
	IN ULONG HandleCount
	);

typedef VOID (*OB_OPEN_METHOD_VISTA)(
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN ULONG Reserved1,
	IN PVOID Object,
	IN ACCESS_MASK* pGrantedAccess,
	IN ULONG HandleCount
	);

typedef NTSTATUS (*OB_PARSE_METHOD)(
	IN PVOID ParseObject,
	IN PVOID ObjectType,
	IN OUT PACCESS_STATE AccessState,
	IN KPROCESSOR_MODE AccessMode,
	IN ULONG Attributes,
	IN OUT PUNICODE_STRING CompleteName,
	IN OUT PUNICODE_STRING RemainingName,
	IN OUT PVOID Context OPTIONAL,
	IN PSECURITY_QUALITY_OF_SERVICE SecurityQos OPTIONAL,
	OUT PVOID *Object
	);

//////////////////////////////////////////////////////////////////////////

VOID 
fake_TokenOpen (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN PVOID Object,
	IN ACCESS_MASK GrantedAccess,
	IN ULONG HandleCount
	);

VOID 
fake_TokenOpen_Vista (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN ULONG Reserved1,
	IN PVOID Object,
	IN ACCESS_MASK* pGrantedAccess,
	IN ULONG HandleCount
	);


VOID 
fake_ProcessOpen (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN PVOID Object,
	IN ACCESS_MASK GrantedAccess,
	IN ULONG HandleCount
	);

VOID 
fake_ProcessOpen_Vista (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN ULONG Reserved1,
	IN PVOID Object,
	IN ACCESS_MASK* pGrantedAccess,
	IN ULONG HandleCount
	);


VOID 
fake_ThreadOpen (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN PVOID Object,
	IN ACCESS_MASK GrantedAccess,
	IN ULONG HandleCount
	);

VOID 
fake_ThreadOpen_Vista (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN ULONG Reserved1,
	IN PVOID Object,
	IN ACCESS_MASK* pGrantedAccess,
	IN ULONG HandleCount
	);


VOID 
fake_EventOpen (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN PVOID Object,
	IN ACCESS_MASK GrantedAccess,
	IN ULONG HandleCount
	);

VOID 
fake_EventOpen_Vista (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN ULONG Reserved1,
	IN PVOID Object,
	IN ACCESS_MASK* pGrantedAccess,
	IN ULONG HandleCount
	);


VOID 
fake_MutantOpen (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN PVOID Object,
	IN ACCESS_MASK GrantedAccess,
	IN ULONG HandleCount
	);

VOID 
fake_MutantOpen_Vista (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN ULONG Reserved1,
	IN PVOID Object,
	IN ACCESS_MASK* pGrantedAccess,
	IN ULONG HandleCount
	);


VOID 
fake_SemaphoreOpen (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN PVOID Object,
	IN ACCESS_MASK GrantedAccess,
	IN ULONG HandleCount
	);

VOID 
fake_SemaphoreOpen_Vista (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN ULONG Reserved1,
	IN PVOID Object,
	IN ACCESS_MASK* pGrantedAccess,
	IN ULONG HandleCount
	);

VOID 
fake_SectionOpen (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN PVOID Object,
	IN ACCESS_MASK GrantedAccess,
	IN ULONG HandleCount
	);

VOID 
fake_SectionOpen_Vista (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN ULONG Reserved1,
	IN PVOID Object,
	IN ACCESS_MASK* pGrantedAccess,
	IN ULONG HandleCount
	);


VOID 
fake_LpcPortOpen (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN PVOID Object,
	IN ACCESS_MASK GrantedAccess,
	IN ULONG HandleCount
	);

VOID 
fake_LpcPortOpen_Vista (
	IN OB_OPEN_REASON OpenReason,
	IN PEPROCESS Process OPTIONAL,
	IN ULONG Reserved1,
	IN PVOID Object,
	IN ACCESS_MASK* pGrantedAccess,
	IN ULONG HandleCount
	);


NTSTATUS 
fake_FileParse ( 
	IN PVOID DirectoryObject,
	IN PVOID ObjectType,
	IN OUT PACCESS_STATE AccessState,
	IN KPROCESSOR_MODE AccessMode,
	IN ULONG Attributes,
	IN OUT PUNICODE_STRING CompleteName,
	IN OUT PUNICODE_STRING RemainingName,
	IN OUT PVOID Context OPTIONAL,
	IN PSECURITY_QUALITY_OF_SERVICE SecurityQos OPTIONAL,
	OUT PVOID *Object
	);

NTSTATUS 
fake_DeviceParse ( 
	IN PVOID DirectoryObject,
	IN PVOID ObjectType,
	IN OUT PACCESS_STATE AccessState,
	IN KPROCESSOR_MODE AccessMode,
	IN ULONG Attributes,
	IN OUT PUNICODE_STRING CompleteName,
	IN OUT PUNICODE_STRING RemainingName,
	IN OUT PVOID Context OPTIONAL,
	IN PSECURITY_QUALITY_OF_SERVICE SecurityQos OPTIONAL,
	OUT PVOID *Object
	);


NTSTATUS
fake_CmpParseKey (
	IN PVOID ParseObject,
	IN PVOID ObjectType,
	IN OUT PACCESS_STATE AccessState,
	IN KPROCESSOR_MODE AccessMode,
	IN ULONG Attributes,
	IN OUT PUNICODE_STRING CompleteName,
	IN OUT PUNICODE_STRING RemainingName,
	IN OUT PVOID Context OPTIONAL,
	IN PSECURITY_QUALITY_OF_SERVICE SecurityQos OPTIONAL,
	OUT PVOID *Object
	);


//////////////////////////////////////////////////////////////////////////

BOOL
CheckOpenPactInfo (
	IN PVOID Buffer,
	IN KPROCESSOR_MODE AccessMode,
	IN ULONG Context,
	IN PACCESS_STATE AccessState
	);

NTSTATUS 
ObjectOPProcessFilter (
	IN PVOID _pNode,
	IN PVOID Object,
	IN OB_OPEN_REASON OpenReason,
	IN ACCESS_MASK GrantedAccess
	);

NTSTATUS
ObjectOPThreadFilter(
	IN PVOID _pNode,
	IN PVOID Object,
	IN OB_OPEN_REASON OpenReason,
	IN ACCESS_MASK GrantedAccess
	);

NTSTATUS
ObjectOPFilter (
	IN PVOID _pNode,
	IN PVOID Object,
	IN OB_OPEN_REASON OpenReason,
	IN ACCESS_MASK GrantedAccess
	);

NTSTATUS
ObjectPAFileDeviceFilter (
	IN OUT PUNICODE_STRING RemainingName,
	IN DEVICE_TYPE DeviceType,
	IN PVOID _pNode,
	IN PVOID DirectoryObject,
	IN PVOID pBuffer
	);

NTSTATUS
ObjectKeyFilter (
	IN PVOID _pNode,
	IN PVOID Context,
	IN PVOID ParseObject,
	IN OUT PACCESS_STATE AccessState,
	IN OUT PUNICODE_STRING RemainingName
	);

BOOL
IsApprovedTID (
	IN PVOID _pNode,
	IN HANDLE TID,
	OUT HANDLE *PID
	);

BOOL 
IsApprovedPID (
	IN PVOID _pNode,
	IN ULONG Process,
	IN ULONG GrantedAccess
	);

BOOL
IsApprovedPIDEx (
	IN PVOID _pNode,
	IN ULONG PID
	);

BOOL
__CaptureStackBackTrace (
	IN OB_OPEN_REASON OpenReason,
	IN ULONG __FramesToSkip
	);

NTSTATUS
GetRemainingName (
	IN PVOID Object,
	IN PUNICODE_STRING RemainingName,
	OUT PUNICODE_STRING Name
	);

///////////////////////////////   END OF FILE   ///////////////////////////////