#pragma once


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+							�ṹ��			                  +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--



typedef SC_HANDLE (WINAPI* _OpenSCManagerW_) (
    LPCWSTR lpMachineName,
    LPCWSTR lpDatabaseName,
    DWORD   dwDesiredAccess
    );

extern _OpenSCManagerW_	g_OpenSCManagerW_addr ;


typedef SC_HANDLE (WINAPI* _OpenServiceW_) (
    SC_HANDLE   hSCManager,
    LPCWSTR    lpServiceName,
    DWORD       dwDesiredAccess
    );

extern _OpenServiceW_ g_OpenServiceW_addr ;


typedef BOOL (WINAPI* _StartServiceW_) (
    SC_HANDLE            hService,
    DWORD                dwNumServiceArgs,
    LPCWSTR             *lpServiceArgVectors
    );

extern _StartServiceW_ g_StartServiceW_addr ;


typedef BOOL (WINAPI* _CloseServiceHandle_) (
    SC_HANDLE   hSCObject
    );

extern _CloseServiceHandle_ g_CloseServiceHandle_addr ;


typedef SC_HANDLE (WINAPI* _CreateServiceW_)(
	SC_HANDLE hSCManager,
	LPCWSTR lpServiceName,
	LPCWSTR lpDisplayName,
	DWORD dwDesiredAccess,
	DWORD dwServiceType,
	DWORD dwStartType,
	DWORD dwErrorControl,
	LPCWSTR lpBinaryPathName,
	LPCWSTR lpLoadOrderGroup,
	LPDWORD lpdwTagId,
	LPCWSTR lpDependencies,
	LPCWSTR lpServiceStartName,
	LPCWSTR lpPassword
	);

extern _CreateServiceW_ g_CreateServiceW_addr ;


typedef struct _RPCGLOBALINFO_ 
{
/*0x000*/ HANDLE PortHandle ;  
/*0x004*/ ULONG MaxMessageLength ; // ��ȥ LCP Header��С���ֵ
/*0x008*/ ULONG LpcHeaderLength ; // LCP Header�Ĵ�С,һ����0x18�ֽ�,64λ��λ0x28�ֽ� 

} RPCGLOBALINFO, *LPRPCGLOBALINFO ;

extern RPCGLOBALINFO g_RpcGlobalInfo ;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         ����Ԥ����                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


LPWSTR GetRegShellCommandString( LPWSTR lpValueName1, LPWSTR lpValueName2 );

HANDLE OpenSpecRootKey( LPWSTR ValueName );

BOOL ConnectRpcPort();


///////////////////////////////   END OF FILE   ///////////////////////////////
