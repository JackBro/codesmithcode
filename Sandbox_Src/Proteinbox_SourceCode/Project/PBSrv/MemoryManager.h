#pragma once

//////////////////////////////////////////////////////////////////////////

//
// �ڴ�����Զ���ṹ
//


typedef struct _MMNODETLS_ 
{
/*0x000*/ ULONG Tag ; // _sanbox_owner_tag_ --> 0x61647573 
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

	LPVOID ExAllocatePool( IN LPMMNODETLS pTLSData, IN ULONG Size );
	ULONG MMGetIndex( IN LPMMNODEL pNodeL, IN ULONG Size );
	LPMMNODEL MMAllocateNode( IN LPMMNODETLS pTLSData, IN ULONG Tag );
	LPMMNODETLS MMCreateTLS();
	LPMMNODETLS _MMCreateTLS( IN ULONG Tag );
	VOID FreeTLSData( IN LPMMNODETLS pTLSData );
	VOID ExFreePool( IN int lpAddress, IN int Size);

protected:
	LPVOID ExAllocatePoolL( IN LPMMNODETLS pTLSData, IN ULONG Size );
	LPVOID ExAllocatePoolB( IN LPMMNODETLS pTLSData, IN ULONG Size );
	VOID ExFreePoolL( IN int lpAddress, IN int Size);
	VOID ExFreePoolB( IN int lpAddress, IN int Size);

	

private:
	DWORD m_dwTlsIndex ;

};
