/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/05/20 [20:5:2010 - 15:42]
* MODULE : d:\Work\Program\Coding\ɳ��\SandBox\Code\Project\ProteinBoxDrv\ShadowSSDT.c
* 
* Description:
*      
*                        
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "Version.h"
#include "Common.h"
#include "SdtData.h"
#include "LDasm.h"
#include "DispatchIoctl.h"
#include "ShadowSSDTProc.h"
#include "ShadowSSDT.h"


//////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


NTSTATUS
Ioctl_HookShadow (
	IN PVOID pNode, 
	IN PVOID pInBuffer
	)
{
	BOOL bRet = TRUE ;
	LPIOCTL_HOOKSHADOW_BUFFER Buffer = NULL ;

	//
	// 1. У������Ϸ���
	//

	dprintf( "[ProteinBoxDrv] IOCTL_PROTEINBOX - Ioctl_HookShadow(); \n" );

	if ( NULL == pInBuffer )
	{
		dprintf( "error! | Ioctl_HookShadow(); | Invalid Paramaters; failed! \n" );
		return STATUS_INVALID_PARAMETER ;
	}

	//
	// 2. ȡ��R3��Buffer����,�����Ƿ�Hook
	//

	__try
	{
		Buffer = (LPIOCTL_HOOKSHADOW_BUFFER) getIoctlBufferBody( pInBuffer );
		if ( FALSE == MmIsAddressValid( (PVOID)Buffer ) )
		{
			dprintf( "error! | Ioctl_HookShadow() - getIoctlBufferBody(); | Body��ַ���Ϸ� \n" );
			__leave ;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		dprintf( "error! | Ioctl_HookShadow() - __try __except(); \n" );
		return STATUS_INVALID_ADDRESS ;
	}

	//
	// 3. ���ݱ�ǲ���Shadow����
	//

	if ( Buffer->bHook )
	{
		// ����Hook
		bRet = HookShadowSSDT() ;
		if ( FALSE == bRet )
		{
			dprintf( "error! | Ioctl_HookShadow() - HookShadowSSDT(); \n" );
			return STATUS_UNSUCCESSFUL ;
		}
	}
	else
	{
		// ֹͣHook
		UnhookShadowSSDT() ;
	}

	return STATUS_SUCCESS ;
}



BOOL
HookShadowSSDT (
	)
{
	BOOL bRet = FALSE, bSkipOnce = FALSE ;
	NTSTATUS status	= STATUS_UNSUCCESSFUL ;
	PEPROCESS	proc		= NULL	;
	LPWSTR	 wszModuleName	= NULL	;
	LPMPNODE pResult		= NULL	;
	LPSTR	 szFunctionName	= NULL	;
	LPSSDT_SSSDT_FUNC pArray = NULL ;
	int	i = 0, j = 0 ;
	ULONG MappedFuncAddr = 0, Tag = 0 ;
	int ShadowArrayIndex = 0, TotalCounts = 0 ;

	//
	// 1.��ʼ��
	//

	if ( KernelMode == ExGetPreviousMode()) { return FALSE ; }

	//
	// 2. ��ȡShadow SSDT��ָ��������ַ
	//

	if ( FALSE == g_SdtData.bInited || NULL == g_SdtData.pSdtArray || 0 == g_SdtData.ShadowSSDTCounts )
	{
		dprintf( "error! | HookShadowSSDT(); | FALSE == g_SdtData.bInited  \n" );
		return FALSE ;
	}

	pArray = g_SdtData.pSdtArray ;
	TotalCounts		 = g_SdtData.TotalCounts	  ;
	ShadowArrayIndex = g_SdtData.ShadowArrayIndex ;

	//
	// 3. Shadow����Ҫ���ŵ�GUI���̿ռ�ſɶ�ȡ��ַ,��Ϊ��ǰ��IOCTLͨ����,������R3������
	//

	for( i = ShadowArrayIndex; i < TotalCounts; i++ )
	{
		// 3.1 ȷ����������Shadow ssdt���鲿��
		Tag = pArray[ i ].Tag ;

		if ( FALSE == IS_ShadowSSDT_TAG( Tag ) ) { continue ; }

		if ( pArray[ i ].SpecailVersion && pArray[ i ].SpecailVersion != g_Version_Info.CurrentVersion )
		{
			continue ; // ������ǰ�������ڵ�ǰƽ̨��,�����ע
		}

		// ��ȡÿ��Shadow SSDT��Ԫ�ĺ�����ַ,�����ڵ�Ԫ��
		wszModuleName	= pArray[ i ].wszModuleName	 ;
		szFunctionName	= pArray[ i ].szFunctionName ;

		// ӳ��һ�����ݵ��ڴ�,�õ�Mapped�ĺ�����ַ
		pArray[ i ].MappedFuncAddr = MappedFuncAddr = GetProcAddress( wszModuleName, szFunctionName, TRUE );
		if ( 0 == MappedFuncAddr )
		{
			dprintf( "error! | HookShadowSSDT() - GetProcAddress(); | can't get mapped addr: \"0x%08lx\" \n", szFunctionName );
			continue ;
		}

		// 3.2 ��ȡ������ʵ��ַ
		bRet = Get_sdt_function_addr( (PVOID)&pArray[i], _AddressFindMethod_Shadow_, _IndexCheckMethod_Shadow_ );
		if ( FALSE == bRet )
		{
			dprintf( "error! | Handler_SSDT() - Get_ssdt_function_addr(); \n" );
			continue ;
		}

		//
		// 3.3 ��Ϊ���user32!�������Զ�Ӧһ��win32k!ntuser*����,�ʲ鿴�����е�ǰλ�õ�Index�����Ϸ��Ƿ�����ͬ��,
		//    û��������ǵ�һ��,����hook,����ͬ��,������ntuser*�����ѱ�Hook��,��Ҫ�����ظ�Hook
		//

		bSkipOnce = FALSE ;

		for ( j=i-1; j>=0; j-- )
		{
			if ( FALSE == IS_ShadowSSDT_TAG( pArray[j].Tag ) ) { break ; }

			if (   pArray[i]._IndexU_.xxIndex == pArray[j]._IndexU_.xxIndex
				&& pArray[i].RealFuncAddr == pArray[j].RealFuncAddr
				&& pArray[j].bHooked 
				)
			{
				bSkipOnce = TRUE ;
				break ;
			}
		}

		if ( bSkipOnce ) { continue ; }

		// 3.4 Inline Hook
		if ( pArray[i]._AddrU_.fakeFuncAddr )
		{
			PatchSDTFunc( (PVOID)&pArray[i], TRUE );
		}
	}

	if ( i >= TotalCounts ) 
	{ 
		bRet = TRUE ;
	//	SDTWalkNodes( _SDTWalkNodes_Shadow_ ) ;
		dprintf( "ok! | HookShadowSSDT(); \n" );
	}

	// 4.1 ��ȡEnableWindow��Ӧ��xpfnProc
	HandlerEnableWindow();

	// 4.2 ����SystemParametersInfoW�����ſ��ܻ�ȡʧ�ܵ����
	HandlerSystemParametersInfoW() ;

	//
	// 5. ж�ص�����ӳ�䵽CSRSS.EXE���̿ռ��user32.dll�ڴ�,��������ж��ʱ�Ż�ͳһж��ӳ��ģ��,�رվ��,
	//    ����ʱ����SYSTEM���,�ر��ļ����ʱ����.����Dettach֮ǰ,���������
	//

	pResult = (LPMPNODE) kgetaddrMP( L"USER32" );
	if ( pResult )
	{
		if ( pResult->MappedAddr )
			ZwUnmapViewOfSection( (HANDLE)0xFFFFFFFF, (PVOID)pResult->MappedAddr );

		if ( pResult->SectionHandle )
			ZwCloseSafe( pResult->SectionHandle, TRUE );

		if ( pResult->hFile )
			ZwCloseSafe( pResult->hFile, TRUE );

		pResult->MappedAddr = 0 ; 
		pResult->SectionHandle = NULL ;
		pResult->hFile = NULL ;
	}

	return bRet ;
}



VOID
UnhookShadowSSDT (
	)
{
	UnhookShadowSSDTEx();
	return ;
}



VOID
UnhookShadowSSDTEx (
	)
{
	BOOL bNeedAttach = FALSE ;
	LPSSDT_SSSDT_FUNC pArray = NULL ;
	int i = 0, ShadowArrayIndex = 0, TotalCounts = 0 ;

	// 1. У������Ϸ���
	if ( FALSE == g_SdtData.bInited || NULL == g_SdtData.pSdtArray || 0 == g_SdtData.ShadowSSDTCounts )
	{
		dprintf( "error! | UnhookShadowSSDT(); | FALSE == g_SdtData.bInited  \n" );
		return ;
	}

	if ( KernelMode == ExGetPreviousMode()) 
	{ 
		// ��������R3����Ҫ��ж��,����������ǿ��ж��,��ʱ��Ҫattach��GUI����
		bNeedAttach = TRUE ;
	}

	if ( bNeedAttach )
	{
		PEPROCESS proc = NULL ;

		NTSTATUS status = GetEProcessByName_QueryInfo( L"csrss.exe", &proc );
		if( !NT_SUCCESS(status) )
		{
			dprintf( "error! | UnhookShadowSSDT() - GetEProcessByName_QueryInfo() | ��ȡcsrss.exe���̶���ʧ��, status=0x%08lx \n", status );
			return ;
		}

		KeAttachProcess ( proc ) ; 
	}

	pArray = g_SdtData.pSdtArray ;
	TotalCounts		 = g_SdtData.TotalCounts	  ;
	ShadowArrayIndex = g_SdtData.ShadowArrayIndex ;

	for( i = ShadowArrayIndex; i < TotalCounts; i++ )
	{
		if ( FALSE == IS_ShadowSSDT_TAG( pArray[ i ].Tag ) ) { continue ; }

		PatchSDTFunc( (PVOID)&pArray[i], FALSE ); // UnHook
	}

	if ( bNeedAttach ) { KeDetachProcess() ; }
	return ;
}



BOOL
HandlerEnableWindow (
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/25 [25:8:2010 - 14:52]

Routine Description:
  ��ȡEnableWindow��Ӧ��xpfnProc.    
    
--*/
{
	ULONG Index = 0 ;

	Index = HandlerEnableWindowEx( Tag_EnableWindow );
	if ( 0 == Index )
	{
		dprintf( "error! | HandlerEnableWindow() - HandlerEnableWindowEx(); | 0 == Index \n" );
		return FALSE ;
	}

	g_Win32k_apfnSimpleCall_Index_Info.SFI_XXXENABLEWINDOW = Index ;
	return TRUE ;
}


//////////////////////////////////////////////////////////////////////////

ULONG
HandlerEnableWindowEx (
	IN int Tag
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/25 [25:8:2010 - 13:40]

Routine Description:
  �õ�Tag��Ӧ�������²���ú����������е�Index;
  eg: EnableWindow���յ��� win32k!xxxEnableWindow, Ϊ�䴫�ݵ�Index=0x60(Vista�Ժ�Ϊ0x61)

  ��User32!EnableWindow�Ŀ�ͷ0xC�ֽ��ڲ���:
  kd> u 77d1c4d4 
  USER32!EnableWindow:
  77d1c4d4 8bff            mov     edi,edi
  77d1c4d6 55              push    ebp
  77d1c4d7 8bec            mov     ebp,esp
  77d1c4d9 6a60            push    60h // �ҵ������ֵ
  77d1c4db ff750c          push    dword ptr [ebp+0Ch]
  77d1c4de ff7508          push    dword ptr [ebp+8]
  77d1c4e1 e8daffffff      call    USER32!NtUserCallHwndParamLock (77d1c4c0)
  77d1c4e6 5d              pop     ebp
  77d1c4e7 c20800          ret     8

  BOOL EnableWindow( HWND hwnd, BOOL bEnable)
  {
      return (BOOL)NtUserCallHwndParamLock(hwnd, bEnable,SFI_XXXENABLEWINDOW);
  }

  �ܶຯ���������NtUserCallHwndParamLock,�������ڲ�����@xpfnProc,����һ���������(Index),
  ϵͳ����ݸ�������������ҵ���Ӧ�ĺ�������֮; apfnSimpleCall[xpfnProc](pwnd, dwParam);

  �����עEnableWindow��Ӧ�����,���ڴ˲���֮;

Arguments:
  Tag - ������Ӧ��Tag,���ڲ��������е�ָ������

Return Value:
  �������к� | 0

--*/
{
	ULONG pAddr = 0, Len = 0, ret = 0 ;
	PUCHAR opcode = NULL ;
	LPSSDT_SSSDT_FUNC pNode = NULL ;

	pNode = (LPSSDT_SSSDT_FUNC) Get_sdt_Array( Tag );
	if ( NULL == pNode ) 
	{
		dprintf( "error! | HandlerEnableWindowEx() - Get_sdt_Array(); | NULL == pNode \n" );
		return 0 ;
	}

	pAddr = pNode->MappedFuncAddr ;
	if ( 0 == pAddr ) 
	{
		dprintf( "error! | HandlerEnableWindowEx(); | NULL == pNode->MappedFuncAddr \n" );
		return 0 ;
	}

	for( ; pAddr<pNode->MappedFuncAddr + 0xC; pAddr += Len )
	{
		Len = SizeOfCode( (PUCHAR)pAddr, &opcode ) ;
		if( !Len ) { Len++; continue ; }

		if (   ( 0x6A == *(PUCHAR)pAddr )
			&& ( *(PUCHAR)(pAddr+1) >= 0x50 && *(PUCHAR)(pAddr+1) <= 0x70 )
			&& ( 0xFF == *(PUCHAR)(pAddr+2) )
			) 
		{
			ret = *(PUCHAR)(opcode + 1) ;
			break ;
		}
	}

	return ret ;
}



static const BYTE g_RealSystemParametersInfoW_Hardcode [] = 
{
	/*
	��1�����ݶ�Ӧ
	77d19cfd c9              leave
	77d19cfe c21000          ret     10h
	*/
	0x0,
	0x10,
	0xC2,
	0xC9,

	/*
	��2�����ݶ�Ӧ
	77d19ceb e89fefffff  call  USER32!NtUserSystemParametersInfo (77d18c8f)
	*/
	0xE8,
	0xFF, // ����ƥ��ı��
	0,
	0
};


VOID
HandlerSystemParametersInfoW (
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/26 [26:8:2010 - 14:11]

Routine Description:
  ����ͨ��ʽ��ȡ SystemParametersInfoW������ʧ��,�ڴ�ͨ�����ⷽʽ�ٴλ�ȡ֮:
  ��SystemParametersInfoW֮����USER32!RealSystemParametersInfoW,�䲻Զ������� 
  call    USER32!NtUserSystemParametersInfo; �ҵ��õ�ַ,��ȡIndex����.

  USER32!RealSystemParametersInfoW+0x28c:
  77d19ce0 8b7d0c          mov     edi,dword ptr [ebp+0Ch]
  77d19ce3 ff7514          push    dword ptr [ebp+14h]
  77d19ce6 56              push    esi
  77d19ce7 57              push    edi
  77d19ce8 ff7508          push    dword ptr [ebp+8]
  77d19ceb e89fefffff      call    USER32!NtUserSystemParametersInfo (77d18c8f) // ******
  77d19cf0 837d0851        cmp     dword ptr [ebp+8],51h
  77d19cf4 0f839a000000    jae     USER32!RealSystemParametersInfoW+0x2a6 (77d19d94)
  77d19cfa 5f              pop     edi
  77d19cfb 5e              pop     esi
  77d19cfc 5b              pop     ebx
  77d19cfd c9              leave
  77d19cfe c21000          ret     10h
  77d19d01 90              nop
  77d19d02 90              nop
  77d19d03 90              nop
  77d19d04 90              nop
  77d19d05 90              nop
  USER32!SystemParametersInfoW:
  77d19d06 6a10            push    10h
    
  USER32!NtUserSystemParametersInfo:
  77d18c8f b82f120000      mov     eax,122Fh // ȡ��Index
  77d18c94 ba0003fe7f      mov     edx,offset SharedUserData!SystemCallStub (7ffe0300)
  77d18c99 ff12            call    dword ptr [edx]
  77d18c9b c21000          ret     10h

--*/
{
	ULONG Index = 0 ;
	BOOL bRet = TRUE ;
	LPSSDT_SSSDT_FUNC pNode = NULL ;

	// 1. ��ȡ SystemParametersInfoW ������Ӧ�ڵ�
	pNode = (LPSSDT_SSSDT_FUNC) Get_sdt_Array( Tag_SystemParametersInfoW );
	if ( NULL == pNode ) 
	{
		dprintf( "error! | HandlerSystemParametersInfoW() - Get_sdt_Array(); | NULL == pNode \n" );
		return ;
	}

	__try
	{
		PBYTE ptr, pHardcode ;
		int n = 0, addr = 0, NtUserSystemParametersInfo_addr = 0 ;

		// 2. ��֤�ڵ�Ϸ���
		if ( pNode->_IndexU_.xxIndex && pNode->RealFuncAddr && pNode->bHooked ) { return ; }

		addr = pNode->MappedFuncAddr ;
		if ( 0 == addr || FALSE == MmIsAddressValid( (PVOID)addr ) )
		{
			dprintf( "error! | HandlerSystemParametersInfoW();| pNode->MappedFuncAddr ��ַ���Ϸ� \n" );
			return ;
		}

		// 3. ����ƥ��֮
		pHardcode = (PBYTE) g_RealSystemParametersInfoW_Hardcode ;

		while ( n++ < 0x30 )
		{
			ptr = (PBYTE) (addr - n) ;
			if ( *pHardcode != *ptr ) { continue ; }
			
			++ pHardcode ;
			if ( 0xFF != *pHardcode ) { continue ; } // ���������0xFF
			
			NtUserSystemParametersInfo_addr = (int) ((int)ptr +  *(int *)(ptr + 1) + 5) ;
			ProbeForRead( (PVOID)NtUserSystemParametersInfo_addr, 0x10, 1 );

			ptr = (PCHAR) NtUserSystemParametersInfo_addr ;

			if (   0xB8 == *(BYTE *)ptr
				&& 0xBA == *(BYTE *)(ptr + 5)
				&& 0xC2 == *(BYTE *)(ptr + 0xC) 
				&& 0x10 == *(BYTE *)(ptr + 0xD) 
				)
			{
				Index = pNode->_IndexU_.ShadowIndex = *(ULONG *)(ptr + 1) ;
				break ;
			}
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		dprintf( "error! | HandlerSystemParametersInfoW() - __try __except(); \n" );
	    return ;
	}

	// 4.1 ��֤�����ŵĺϷ���
	if ( 0 == Index || Index >= 0x2000 || (Index & 0xFFF) >= 0x600 || 0x1000 != (Index & 0xF000) )
	{ 
		dprintf( "error! | HandlerSystemParametersInfoW(); | ͨ��������ʽ�������������Ų��Ϸ�; Index = %d \n", Index );
		return ; 
	}

	// 4.2 ��ͨ��������ʽ�ҵ�Index,��ȡ��Ӧ�ĺ������ǵ�ַ
	pNode->RealFuncAddr = Get_sdt_function_addr_normal( Index, pNode->ArgumentNumbers );
	if ( 0 == pNode->RealFuncAddr )
	{
		dprintf( "error! | HandlerSystemParametersInfoW() - Get_sdt_function_addr_normal(); \n" );
		return ;
	}

	// 4.3 ����Hook
	if ( pNode->_AddrU_.fake_shadowFunc_addr )
	{
		PatchSDTFunc( (PVOID)pNode, TRUE );
	}

	return ;
}

///////////////////////////////   END OF FILE   ///////////////////////////////