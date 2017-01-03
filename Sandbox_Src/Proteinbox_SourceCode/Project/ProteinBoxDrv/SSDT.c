/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/05/19 [19:5:2010 - 10:41]
* MODULE : d:\Work\Program\Coding\ɳ��\SandBox\Code\Project\ProteinBoxDrv\SSDT.c
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
#include "SdtData.h"
#include "SSDT.h"

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


BOOL
HandlerSSDT(
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/19 [19:5:2010 - 16:46]

Routine Description:
  ��ȡ����SSDT������ַ

--*/
{
	BOOL bRet = FALSE ;
	int	 i	  =	0, TotalCounts = 0  ;
	ULONG	MappedFuncAddr	= 0		;
	LPWSTR	wszModuleName	= NULL	;
	LPSTR	szFunctionName	= NULL	;
	LPSSDT_SSSDT_FUNC pArray = NULL ;

	//
	// 1.��ʼ��
	//

	bRet = InitSdtData();
	if ( FALSE == bRet )
	{
		dprintf( "error! | HandlerSSDT() - InitSdtData | ��ʼ��MP�ṹ����ʧ�� \n" );
		return FALSE ;
	}

	//
	// 2.1 ��ȡSSDT��ַ; ��Щ������ַ����Zw*�ĵ�ַ,���ڵ���,������Hook
	//

	if (   FALSE == g_SdtData.bInited 
		|| NULL == g_SdtData.pSdtArray || 0 == g_SdtData.TotalCounts
		|| NULL == g_SdtData.SpecArray || 0 == g_SdtData.SpecCounts
		)
	{
		dprintf( "error! | HandlerSSDT(); | FALSE == g_SdtData.bInited  \n" );
		return FALSE ;
	}

	pArray = g_SdtData.pSdtArray ;
	TotalCounts = g_SdtData.TotalCounts ;

	for( i=0; i<TotalCounts; i++ )
	{
		// ȷ����������SSDT���鲿��
		if ( FALSE == IS_SSDT_TAG( pArray[i].Tag ) ) 
		{
			// �Ѿ�����Shadow SSDT��������,��¼��ʱ�����(Index),������ Handler_ShadowSSDT()����ʹ��
			g_SdtData.ShadowArrayIndex = i ;
			g_SdtData.ShadowSSDTCounts = TotalCounts - i ;
			break ;
		}

		if ( pArray[ i ].SpecailVersion && pArray[ i ].SpecailVersion != g_Version_Info.CurrentVersion )
		{
			// ������ǰ�������ڵ�ǰƽ̨��,�����ע
			continue ;
		}

		// ��ȡÿ��SSDT��Ԫ�ĺ�����ַ,�����ڵ�Ԫ��
		wszModuleName	= pArray[ i ].wszModuleName	 ;
		szFunctionName	= pArray[ i ].szFunctionName ;

		pArray[ i ].MappedFuncAddr = MappedFuncAddr = GetProcAddress( wszModuleName, szFunctionName, TRUE );
		if ( 0 == MappedFuncAddr )
		{
			dprintf( "error! | HandlerSSDT() - GetProcAddress(); | can't get mapped addr: \"%s\" \n", szFunctionName );
			continue ;
		}

		bRet = Get_sdt_function_addr( (PVOID)&pArray[i], _AddressFindMethod_SSDT_, _IndexCheckMethod_SSDT_ );
		if ( FALSE == bRet )
		{
			dprintf( "error! | HandlerSSDT() - Get_ssdt_function_addr(); \n" );
			continue ;
		}
	}

//	SDTWalkNodes( _SDTWalkNodes_SSDT_ ) ;

	//
	// 2.2 ��ȡ�����ssdt������ַ; ��Щ������ַ����Nt*�ĵ�ַ,����Hook,�����ڵ���
	//

	pArray = g_SdtData.SpecArray ;
	TotalCounts = g_SdtData.SpecCounts ;

	for( i=0; i<TotalCounts; i++ )
	{
		if ( pArray[ i ].SpecailVersion && pArray[ i ].SpecailVersion != g_Version_Info.CurrentVersion )
		{
			// ������ǰ�������ڵ�ǰƽ̨��,�����ע
			continue ;
		}

		// ��ȡÿ��SSDT��Ԫ�ĺ�����ַ,�����ڵ�Ԫ��
		wszModuleName	= pArray[ i ].wszModuleName	 ;
		szFunctionName	= pArray[ i ].szFunctionName ;

		pArray[ i ].MappedFuncAddr = MappedFuncAddr = GetProcAddress( wszModuleName, szFunctionName, TRUE );
		if ( 0 == MappedFuncAddr )
		{
			dprintf( "error! | HandlerSSDT() - GetProcAddress(); | can't get mapped addr: \"%s\" \n", szFunctionName );
			continue ;
		}

		bRet = Get_sdt_function_addr( (PVOID)&pArray[i], _AddressFindMethod_Shadow_, _IndexCheckMethod_SSDT_ );
		if ( FALSE == bRet )
		{
			dprintf( "error! | HandlerSSDT() - Get_ssdt_function_addr(); \n" );
			continue ;
		}
	}

	//
	// 3. ������RPC������Hook
	//

	HookSSDT ();

	dprintf( "ok! | HandlerSSDT(); \n" );
	return TRUE ;
}



VOID HookSSDT ()
{
	PatchSSDT( TRUE );
}



VOID UnhookSSDT ()
{
	PatchSSDT( FALSE );
}



VOID
PatchSSDT (
	IN BOOL bFlag
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/12 [12:8:2010 - 18:06]

Routine Description:
  ��ָ����N��ssdt��������Inline(Hook/Unhook)   
    
Arguments:
  bFlag - TRUEΪ����InlineHook; FALSEΪ�ر�InlineHook

--*/
{
	ULONG TotalCounts = 0, i= 0 ;
	LPSSDT_SSSDT_FUNC pArray = NULL ;

	if ( FALSE == g_SdtData.bInited || NULL == g_SdtData.SpecArray || 0 == g_SdtData.SpecCounts )
	{
		dprintf( "error! | PatchSSDT(); | FALSE == g_SdtData.bInited  \n" );
		return ;
	}

	pArray = g_SdtData.SpecArray ;
	TotalCounts = g_SdtData.SpecCounts ;

	for( i=0; i<TotalCounts; i++ )
	{
		if ( pArray[i].SpecailVersion && pArray[i].SpecailVersion != g_Version_Info.CurrentVersion )
		{
			continue ; // ������ǰ�������ڵ�ǰƽ̨��,�����ע
		}

		PatchSDTFunc( (PVOID)&pArray[i], bFlag );
	}

	return ;
}


///////////////////////////////   END OF FILE   ///////////////////////////////