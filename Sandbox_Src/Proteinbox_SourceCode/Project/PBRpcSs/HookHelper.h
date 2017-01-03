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


#define SE_ASSIGNPRIMARYTOKEN_PRIVILEGE (3L)
#define STATUS_PRIVILEGE_NOT_HELD ((NTSTATUS)0xC0000061L)

#define ARRAYSIZEOF(x) sizeof (x) / sizeof (x[0])

#define MAXLENGTH 0x30

typedef enum _DLLTAG_
{
	Ntdll_TAG = 0,
	Kernel32_TAG,
	KernelBase_TAG,
	ADVAPI32_TAG,
	WS2_32_TAG,

	Nothing_TAG = 0x10,
};

typedef struct _HOOKINFOLittle_rpcss_ 
{
	ULONG_PTR DllTag ;	// ����������ģ����
	ULONG_PTR Tag ; // ��ǵ�ǰ�ڵ㵥Ԫ
	char FunctionName[ MAXLENGTH ]; // ������
	char FunctionNameEx[ MAXLENGTH ]; // WIN7������,���Ϊ�գ�����������ĺ�����һ��
	BOOL bHooked ;	// �Ƿ�Hook
	PVOID OrignalAddress ;	// ԭʼ��ַ
	PVOID FakeAddress ;		// ���˺�����ַ
} HOOKINFOLittleRpcss, *LPHOOKINFOLittleRpcss ;


typedef struct _MODULEINFO_ 
{
/*0x000*/ HMODULE hModuleArrays[10];

} MODULEINFO, *LPMODULEINFO ;

extern LPMODULEINFO __ProcModulesInfo;
extern HMODULE g_hModule_KernelBase;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         ����Ԥ����                        +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


BOOL HookOne( LPHOOKINFOLittleRpcss Info );
VOID GethModule();
PVOID kmalloc ( ULONG length );
VOID kfree ( PVOID ptr );

BOOL
MmIsAddressValid(
	IN PVOID ptr,
	IN ULONG length
	);

///////////////////////////////   END OF FILE   ///////////////////////////////
