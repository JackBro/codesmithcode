#pragma once

//////////////////////////////////////////////////////////////////////////


typedef int (__stdcall *_GETPROCESSIDOFTHREAD_)(DWORD);
extern _GETPROCESSIDOFTHREAD_ g_GetProcessIdOfThread_addr ;

extern PVOID g_Buffer_0x1B00 ;


typedef struct _LOADDRVCONTEXT_ 
{
/*0x000*/ ULONG unknown1 ;
/*0x004*/ ULONG unknown2 ;

} LOADDRVCONTEXT, *LPLOADDRVCONTEXT ;


typedef struct _THREAD_PARAMETER_ 
{
/*0x000*/ PVOID This ;
/*0x004*/ LPTOTOAL_DATA pNode ;

} THREAD_PARAMETER, *LPTHREAD_PARAMETER ;


//////////////////////////////////////////////////////////////////////////

class CRPCHandler
{
public:
	static CRPCHandler& GetInstance()
	{
		static CRPCHandler _instance;
		return _instance;
	}

	CRPCHandler(void);
	~CRPCHandler(void);

	DWORD HandlerRPC();

	VOID Handler_ApiNumber_0x1100( PVOID pBuffer, LPTOTOAL_DATA pNode );
	VOID Handler_ApiNumber_0x1300( PVOID pBuffer, LPTOTOAL_DATA pNode );
	VOID Handler_ApiNumber_0x1400( PVOID pBuffer, LPTOTOAL_DATA pNode );
	VOID Handler_ApiNumber_0x1600( PVOID pBuffer, LPTOTOAL_DATA pNode );
	VOID Handler_ApiNumber_0x1800( PVOID pBuffer, LPTOTOAL_DATA pNode );
	PVOID Handler_ApiNumber_0x1B00( PVOID pBuffer, LPTOTOAL_DATA pNode );

	LPPOSTPROCRPCINFO CRPCHandler::PostProcRPC( LPTOTOAL_DATA pTotalNode, int ErrorCode );
	int AllocRpcBuffer( LPTOTOAL_DATA pTotalNode, int StructLength );
	int HeapAllocStub( IN HANDLE hHeap, IN ULONG BufferLength );
	VOID HeapFreeStub( IN HANDLE hHeap, IN ULONG pBuffer );

	//
	// һ�������е��߳�/�ص�����,���������ɾ�̬��.�����൱����ȫ�ֵ��ڴ�,
	// �����ʱ�໹û�������ڴ�,���õ�������ĺ���,��Ȼ���벻��
	//

	static DWORD WINAPI WorkThread( LPVOID lpParameter ); 

protected:
	VOID FreePostProcBuffer( int addr );
	VOID _InsertList( LPTOTOAL_DATA pTotalNode, int ApiNumTag, PVOID pBuffer, _PROCAPINUMBERFUNC_ Func, BOOL Flag );
	BOOL DispatchMsg( LPTOTOAL_DATA pNode );
	VOID _ThreadProc( LPTOTOAL_DATA pNode );
	VOID ThreadProcA( LPTOTOAL_DATA pNode );
	VOID ThreadProcB( LPTOTOAL_DATA pNode );

	LPPOSTPROCRPCINFO CallSpecApiNumFunc( LPTOTOAL_DATA pTotalNode, PRPC_IN_HEADER pRpcHeader, HANDLE PortHandle, PPORT_MESSAGE Msg );
	VOID ProcessConnectionRequest( LPTOTOAL_DATA pTotalNode, PPORT_MESSAGE ReciveMsg );
	LPSIGNALNODELITTLE_EX ProcessLpcRequest( LPTOTOAL_DATA pTotalNode, PPORT_MESSAGE ReciveMsg );
	PPORT_MESSAGE ProcessLpcRequestStep1( LPTOTOAL_DATA pTotalNode, LPSIGNALNODELITTLE_EX pNodeLittleEx, PPORT_MESSAGE ReceiveMsg );
	VOID ProcessLpcRequestStep2( LPSIGNALNODELITTLE_EX pNodeLittleEx, PPORT_MESSAGE ReceiveMsg );
	VOID ProcessLpcPortClosed( LPTOTOAL_DATA pTotalNode, PPORT_MESSAGE ReceiveMsg );
	VOID ProcessLpcPortClosedDep( LPTOTOAL_DATA pTotalNode, PFILETIME pCreationTime );
	VOID CallApiNumFunc( LPTOTOAL_DATA pTotalNode, HANDLE UniqueProcess );

protected:
	RTL_CRITICAL_SECTION m_LoadDrv_cs ;
	RTL_CRITICAL_SECTION m_UnmontRegHive_cs ;

private:
	BOOL m_GetProcessIdOfThread_ok ;
};

extern CRPCHandler g_RpcHandler ;


///////////////////////////////   END OF FILE   ///////////////////////////////
