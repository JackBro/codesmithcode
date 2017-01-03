#pragma once

//////////////////////////////////////////////////////////////////////////

//
// �ڴ�����Զ���ṹ
//

typedef struct _MMNODETLS_ 
{
/*0x000*/ ULONG Tag ; // _sanbox_owner_tag_ --> 0x747a756b 
/*0x004*/ CRITICAL_SECTION cs ;
/*0x01C*/ LIST_ENTRY_EX ListEntryExA ;	  // ָ�� struct _MMNODEL_ �ڵ�
/*0x028*/ LIST_ENTRY_EX ListEntryExB ;	  // ָ�� struct _MMNODEL_ �ڵ�
/*0x034*/ LIST_ENTRY_EX ListEntryExFuck ; // ָ�� struct _MMNODEL_ �ڵ�
/*0x040*/ BYTE MemoryInUseFlag[0x80] ;
/*0x0C0*/

} MMNODETLS, *LPMMNODETLS ;


typedef struct _MMNODEL_ 
{
/*0x000*/ struct _MMNODEL_ *Flink ;
/*0x004*/ struct _MMNODEL_ *Blink ;
/*0x008*/ ULONG Tag ; // _sanbox_owner_tag_ --> 0x747a756b 
/*0x00C*/ LPMMNODETLS pTLSDataGlobal ;
/*0x010*/ LPMMNODETLS pTLSDataCur ; // ����TLSData�ĵ�ַ,���ڱ��ṹ��+0x100��
/*0x014*/ ULONG Size ; // ��ʼֵΪ0x1FE
/*0x018*/ ULONG Reserved[26];
/*0x080*/ BYTE MemoryInUseFlag[0x80] ;
/*0x100*/ MMNODETLS TLSDataCur ;

} MMNODEL, *LPMMNODEL ;


typedef struct _PROCESS_GLOBALMEMORY_NODE_ 
{
/*0x000*/ ULONG Reserved[0x30] ; 
/*0x0C0*/ ULONG kmalloc_for_frequently_used_memory_BaseAddrOffset; // ��kmalloc_for_frequently_used_memory()����ʹ��
/*0x0C4*/
	struct
	{
/*0x0C4*/ BYTE nLockNtCreateKey ;
/*0x0C5*/ BYTE nLockNtCreateFile ;
/*0x0C6*/ BYTE nLockNtClose ;
/*0x0C7*/ BYTE nLockRtlGetCurrentDirectory ;

/*0x0C8*/ BYTE nLockNtDeleteFile ;
/*0x0C9*/ BYTE nLockUnkown1[3] ;
	}sFileLock;

/*0x0CC*/ 
	struct
	{
/*0x0CC*/ ULONG Flag ;

/*0x0D0*/ LPUNICODE_STRING_EX ImagePathName ;
/*0x0D4*/ LPUNICODE_STRING_EX DllPath ;
/*0x0D8*/ LPUNICODE_STRING_EX CurrentDirectory ;
/*0x0DC*/ LPUNICODE_STRING_EX CommandLine ;
/*0x0E0*/ LPUNICODE_STRING_EX Environment ;

/*0x0E4*/ BYTE bIsDirectory ;
/*0x0E5*/ BYTE nLockUnkown1[3] ;
	} sRCP ; // RtlCreateProcessParameters

/*0x0E8*/ 
	struct
	{
		ULONG nLockCreateProcessAsUser;
	}sProcessLock;

/*0x0EC*/ 
	struct
	{
/*0x0EC*/ PVOID ServiceDatas ; // ��ɳ���е����з���
/*0x0F0*/ DWORD OldTickCount ;
/*0x0F4*/ LPWSTR ServiceName; // д��־��,�ɷ���
	}sSandboxServices;

/*0x0F8*/
	struct
	{
		HANDLE PortHandle ;  
		ULONG MaxMessageLength ; // ��ȥ LCP Header��С���ֵ
		ULONG LpcHeaderLength ; // LCP Header�Ĵ�С,һ����0x18�ֽ�,64λ��λ0x28�ֽ� 
	}sLPC;

/*0x104*/ LPVOID lpDialogFunc;
} PROCESS_GLOBALMEMORY_NODE, *LPPROCESS_GLOBALMEMORY_NODE ;
#pragma pack()


#define TLS_OUT_OF_INDEXES ((DWORD)0xFFFFFFFF)

//////////////////////////////////////////////////////////////////////////

class CMemoryManager
{
public:
	static CMemoryManager& GetInstance()
	{
		static CMemoryManager _instance;
		return _instance;
	}

	CMemoryManager(void);
	~CMemoryManager(void);

	BOOL InitTLS();
	LPVOID GetData(int* dwErrCode);
	LPVOID ExAllocatePool( IN LPMMNODETLS pTLSData, IN ULONG Size );
	ULONG MMGetIndex( IN LPMMNODEL pNodeL, IN ULONG Size );
	LPMMNODEL MMAllocateNode( IN LPMMNODETLS pTLSData, IN ULONG Tag );
	LPMMNODETLS MMCreateTLS();
	LPMMNODETLS _MMCreateTLS( IN ULONG Tag );
	VOID FreeTLSData( IN LPMMNODETLS pTLSData );
	VOID ExFreePool( IN int lpAddress, IN int Size);
	PVOID kmallocEx( int nSize );
	void kfreeEx(int lpAddress);
	void AntiDebuggerEnter(PVOID _pNode);
	void AntiDebuggerLeave(PVOID _pNode);

	PVOID 
	kmalloc_for_frequently_used_memory(
		LPPROCESS_GLOBALMEMORY_NODE pNode, 
		int nIndex, 
		int Length
		);

protected:
	LPVOID ExAllocatePoolL( IN LPMMNODETLS pTLSData, IN ULONG Size );
	LPVOID ExAllocatePoolB( IN LPMMNODETLS pTLSData, IN ULONG Size );
	VOID ExFreePoolL( IN int lpAddress, IN int Size);
	VOID ExFreePoolB( IN int lpAddress, IN int Size);


private:
	DWORD m_dwTlsIndex ;
	DWORD m_dwTlsIndex_tlsdata;
	LPMMNODETLS m_pTLSData;

};
