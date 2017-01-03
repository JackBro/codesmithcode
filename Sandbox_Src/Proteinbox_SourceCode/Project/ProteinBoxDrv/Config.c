/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/05/28 [28:5:2010 - 14:45]
* MODULE : d:\Work\Program\Coding\ɳ��\SandBox\Code\Project\ProteinBoxDrv\Config.c
* 
* Description:
*      
*   ���������ļ�����ģ��                       
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "Common.h"
#include "DispatchIoctl.h"
#include "Memory.h"
#include "ConfigThread.h"
#include "config.h"

//////////////////////////////////////////////////////////////////////////


#define g_CurrentBox_FullFilePathW	L"\\Device\\HarddiskVolume1\\ProteinBox\\SUDAMI\\DefaultBox"
#define g_CurrentBox_FullRegPathW	L"\\REGISTRY\\USER\\ProteinBox_SUDAMI_DefaultBox"
#define g_CurrentBox_FullIpcPathW	L"\\ProteinBox\\SUDAMI\\DefaultBox\\Session_0"

#define g_ForceProcessPathW	L"C:\\Program Files\\Internet Explorer\\IEXPLORE.EXE"

LPIOCTL_HANDLERCONF_GLOBAL g_Ioctl_HandlerConf_GlobalData = NULL ;


BOOL g_ConfigData_init_ok = FALSE ;

/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                ��R3ͨ��,���������ļ���������              +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


BOOL
InitConfig (
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/07/05 [5:7:2010 - 10:17]

Routine Description:
  ������ʼ��ʱ��ȡ�����ļ�,�����е�ȫ����Ϣ�������ڴ�,��������/д
    
--*/
{
	BOOL bRet = FALSE ;

	// 1. ��ʼ��GrayList����
	bRet = InitConfigData();
	if ( FALSE == bRet )
	{
		dprintf( "error! | InitConfig() - InitConfigData(); | \n" );
		return FALSE ;
	}

#if 0
	// 2. ��ʼ���ñ�������ȫ�ֱ���
	if ( NULL == g_Ioctl_HandlerConf_GlobalData )
	{
		g_Ioctl_HandlerConf_GlobalData = (LPIOCTL_HANDLERCONF_GLOBAL) kmallocMM( sizeof(IOCTL_HANDLERCONF_GLOBAL), MTAG___IOCTL_HANDLERCONF_GLOBAL ) ;
		if ( NULL == g_Ioctl_HandlerConf_GlobalData )
		{
			dprintf( "error! | InitConfig() - kmallocMM | NULL == g_Ioctl_HandlerConf_GlobalData,�����ڴ�ʧ�� \n" );
			return FALSE ;
		}
	}
#endif

	// 3. ����һ���߳������������ļ���Ϣ
	CreateConfigThread( FALSE );
	
	return TRUE ;
}



BOOL
LoadConfig (
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/29 [29:6:2010 - 18:25]

Routine Description:
  ����R3���߳�,������������,�ȴ������������ݰ�

--*/
{
	BOOL bRet = FALSE ;
	BOOL bOK = FALSE ;
	HANDLE hEvent = NULL ;
	LARGE_INTEGER Timeout ;
	ULONG i = 0 ;
	NTSTATUS status = STATUS_UNSUCCESSFUL ;
	
	// 1. �����¼�1ΪTRUE,����R3�ȴ����߳�,���Ὣ����ȫ��������
	bRet = SetEvent( g_ConfEvent_waitfor_r0, EVENT_MODIFY_STATE );
	if( FALSE == bRet )
	{
		dprintf( "error! | LoadConfig() - SetEvent(); | \"%s\" \n", g_ConfEvent_waitfor_r0 );
		return FALSE ;
	}

	// 2. �ȴ�R3���ؽ����ֻ��һ����; R3�������ļ������ݴ����ˣ��ͻὫ���¼���TRUE����������
	bRet = OpenEvent( g_ConfEvent_wakeup_r0, EVENT_ALL_ACCESS, &hEvent );
	if( FALSE == bRet )
	{
		dprintf( "error! | LoadConfig() - OpenEvent(); | \"%ws\" \n", g_ConfEvent_wakeup_r0 );
		return FALSE ;
	}

	dprintf( "LoadConfig() | �ȴ�R3������Ing,�ȴ��ĳ�ʱʱ��Ϊ1���� ... \n" );

	Timeout.QuadPart = -10 * 1000 * 1000 ;   // 1 second
	for ( i = 0; i < 60; i++ ) 
	{
		status = ZwWaitForSingleObject( hEvent, FALSE, &Timeout );
		if ( STATUS_TIMEOUT == status ) { continue ; }

		if( STATUS_SUCCESS == status )
		{
			dprintf( "LoadConfig() | �ѵȵ�R3���صĽ�� \n" );
			bOK = TRUE ;
			break ;
		}
	}

	ZwResetEvent( hEvent, 0 ); // ���ܵȴ��������,�����½����¼��ü�
	ZwClose( hEvent );

	if ( FALSE == bOK )
	{
		dprintf( "Ioctl_HandlerConf() | (R3���ؽ��) �ȴ���ʱ.(status=0x%08lx) \n", status );
		return FALSE ;
	}

	g_ConfigData_init_ok = TRUE ;
	return TRUE ;
}



NTSTATUS
Ioctl_HandlerConf (
	IN PVOID pNode,
	IN PVOID pInBuffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/28 [28:6:2010 - 18:04]

Routine Description:
  ���������¼�1Ϊ��,��R3�ȴ����̼߳���,R3�㷢һ������������,��ȡR0������,���а�������Ϣ�����Ƕ�дָ����ֵ�ĺ�/��������Ϣ. eg:

  [GlobalSetting]
  OpenFilePath=firefox.exe,%Favorites%
  OpenIpcPath=*\BaseNamedObjects*\MAPI-HP*,*\BaseNamedObjects*\OLKCRPC.OBJ=*
  sudam*,imadu*
  CloseIpcPath=

  ����Ϊ�����ļ���Ϣ,����֪��[GlobalSetting]��OpenIpcPath��Ӧ������.��ʱR3���æ�ռ�.
  ͨ���ڼ���Ҫһ���ȴ��¼�,R0������ʱ,�ἤ��õȴ��¼�,������IOCTL��R3,R3���̱߳������
  ��ᴦ��R0�ĸ�����,��R0��Ҫ��������䵽Buffer��,������һ���¼�Ϊ��,��R0�����ȴ�,��������
    
--*/
{
	BOOL bRet = FALSE ;
	PVOID pBody = NULL ;
	PIOCTL_PROTEINBOX_BUFFER pData = (PIOCTL_PROTEINBOX_BUFFER) pInBuffer ;

	dprintf( "[ProteinBoxDrv] IOCTL_PROTEINBOX - Ioctl_HandlerConf(); \n" );

	if ( NULL == pData )
	{
		dprintf( "error! | Ioctl_HandlerConf(); | Invalid Paramaters; failed! \n" );
		return STATUS_INVALID_PARAMETER ;
	}

	pBody = (PVOID) getIoctlBufferBody( pInBuffer );
	if ( NULL == pBody )
	{
		dprintf( "error! | Ioctl_HandlerConf() - getIoctlBufferBody(); | NULL == pBody \n" );
		return STATUS_INVALID_ADDRESS ;
	}

	switch ( pData->Head.LittleIoctlCode )
	{
	case _Ioctl_Conf_function_InitData_ :
		{
		//	dprintf( "Ioctl_HandlerConf() | R3���͸�Ioctl,�������ļ��е�������Ϣȫ������R0�洢  \n" );
			bRet = Ioctl_HandlerConf_InitData( pBody );
		}
		break ;

	case _Ioctl_Conf_function_VerifyData_ : // (����)
		{
			dprintf( "Ioctl_HandlerConf() | R3���͸�Ioctl,��R0���Buffer,���а����˶�/дָ��������Ϣ \n" );
			bRet = Ioctl_HandlerConf_VerifyData( pBody );
		}
		break ;

	case _Ioctl_Conf_function_ReceiveData_ : // (����)
		{
			dprintf( "Ioctl_HandlerConf() | R3���͸�Ioctl,����ȡ���������ļ���Ϣ���ݵ�R0��	 \n" );
			bRet = Ioctl_HandlerConf_ReceiveData( pBody );
		}
		break ;

	case _Ioctl_Conf_function_Reload_ :
		{
			dprintf( "Ioctl_HandlerConf() | Reload Config	 \n" );
			bRet = Ioctl_HandlerConf_Reload();
		}
		break ;

	default :
		dprintf( "error! | Ioctl_HandlerConf() | ���Ϸ������� \n" );
		return STATUS_INVALID_PARAMETER ;
	}

	if ( FALSE == bRet ) { return STATUS_UNSUCCESSFUL ; }
	
	return STATUS_SUCCESS ;
}



BOOL
Ioctl_HandlerConf_InitData (
	IN PVOID pInBuffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/07/02 [2:7:2010 - 17:02]

Routine Description:
  ��������ʱ,�ᷢ��Ioctl��֮R3,R3�㽫�����ļ��е�������Ϣȫ������R0�洢
    
--*/
{
	BOOL bRet = FALSE ;
	PVOID pNode = NULL ;
	WCHAR wszSeactionName[	MAX_PATH ] = L"" ;
	WCHAR wszKeyName[		MAX_PATH ] = L"" ;
	WCHAR wszValueName[		MAX_PATH ] = L"" ;
	IOCTL_HANDLERCONF_BUFFER_IniDataW DataW ;
	LPIOCTL_HANDLERCONF_BUFFER_IniDataA lpData = (LPIOCTL_HANDLERCONF_BUFFER_IniDataA) pInBuffer ;

	// 1. У������Ϸ���
	if ( NULL == lpData || NULL == lpData->SeactionName || NULL == lpData->KeyName || NULL == lpData->ValueName || 0 == lpData->ValueNameLength )
	{
		dprintf( "error! | Ioctl_HandlerConf_InitData(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	// 2.char --> wchar
	bRet = a2w( lpData->SeactionName, wszSeactionName, MAX_PATH );
	if ( FALSE == bRet )
	{
		dprintf( "error! | Ioctl_HandlerConf_InitData() - a2w(); | lpData->SeactionName \n" );
		return FALSE ;
	}

	bRet = a2w( lpData->KeyName, wszKeyName, MAX_PATH );
	if ( FALSE == bRet )
	{
		dprintf( "error! | Ioctl_HandlerConf_InitData() - a2w(); | lpData->KeyName \n" );
		return FALSE ;
	}

	bRet = a2w( lpData->ValueName, wszValueName, MAX_PATH );
	if ( FALSE == bRet )
	{
		dprintf( "error! | Ioctl_HandlerConf_InitData() - a2w(); | lpData->ValueName \n" );
		return FALSE ;
	}
	
	DataW.SeactionName = wszSeactionName ;
	DataW.SeactionNameLength = ( wcslen(wszSeactionName) + 1 ) * sizeof(WCHAR) ; 

	DataW.KeyName = wszKeyName ;
	DataW.KeyNameLength = ( wcslen(wszKeyName) + 1 ) * sizeof(WCHAR) ;

	DataW.ValueName = wszValueName ;
	DataW.ValueNameLength = ( wcslen(wszValueName) + 1 ) * sizeof(WCHAR) ;


	// 3.������Ӧ�ڵ�
	bRet = CDBuildNode( (PVOID)g_ProteinBox_Conf_TranshipmentStation, &DataW );
	if ( FALSE == bRet )
	{
		dprintf( "error! | Ioctl_HandlerConf_InitData() - kBuildNodeCD(); | \n" );
		return FALSE ;
	}

	return TRUE ;
}



BOOL
Ioctl_HandlerConf_VerifyData (
	OUT PVOID pOutBuffer
	)
{	
	return TRUE ;
}



BOOL
Ioctl_HandlerConf_ReceiveData (
	IN PVOID pInBuffer
	)
{
	return TRUE ;
}



BOOL Ioctl_HandlerConf_Reload()
/*++

Author: sudami [sudami@163.com]
Time  : 2011/08/05 [5:8:2011 - 14:34]

Routine Description:
  ���¼��������ļ�,��R3���¶�ȡ�����ļ���,�����и��º�������׸�����
    
--*/
{
	// ��ʼ�������ļ������Ϣ
	if ( FALSE == InitConfig() )
	{
		dprintf( "error! | Ioctl_HandlerConf_Reload() - InitConfig(); \n" );
		return FALSE;
	}

	// ��ʱ��������һ���ȴ��߳�,�ȴ�R3����,����R3�����������׸�����,Reload�ɹ�
	return TRUE;
}



static const LPWSTR g_ConfContextAllowed_Array[ ] = // �����ļ��з��е��ַ�������
{
	L"yes",
	L"y",
	L"ok",
	L"1",
};

static const LPWSTR g_ConfContextDenney_Array[ ] = // �����ļ��н�ֹ���ַ�������
{
	L"no",
	L"n",
	L"0",
};


BOOL
GetConfigurationA (
	IN PCHAR KeyName
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/28 [28:5:2010 - 16:23]

Routine Description:
  ��ѯ�����ļ��ж�Ӧ@KeyName��״̬;���е�BOOL������ö��������,eg:

  [AllowedOrDenney]
  OpenProtectedStorage=y
    
Arguments:
  Context - �����ļ��е���Ŀ

Return Value:
  BOOL
    
--*/
{
	BOOL bRet = TRUE ;
	WCHAR wszKeyName[MAX_PATH] = L"";
	
	if ( NULL == KeyName ) { return FALSE; }

	bRet = a2w( KeyName, wszKeyName, MAX_PATH );
	if ( FALSE == bRet )
	{
		dprintf( "error! | GetConfigurationA() - a2w(); | KeyName=%s \n", KeyName );
		return FALSE ;
	}

	bRet = GetConfigurationW(wszKeyName);
	return bRet ;
}



BOOL
GetConfigurationW (
	IN LPWSTR KeyName
	)
{
	int Index = 0;
	BOOL bRet = FALSE;
	LPWSTR ptr = NULL;
	WCHAR Context[MAX_PATH] = L"";

	bRet = kGetConfSingle( L"AllowedOrDenney", KeyName, Context );
	if ( FALSE == bRet ) { return FALSE; }

	for ( Index = 0; Index < ARRAYSIZEOF(g_ConfContextAllowed_Array); Index++ )
	{
		ptr = g_ConfContextAllowed_Array[ Index ];
		if( 0 == _wcsnicmp(Context, ptr, wcslen(ptr)) )
		{
			return TRUE;
		}
	}

	return FALSE ;
}



BOOL
GetConfigurationSW (
	IN LPWSTR SeactionName,
	IN LPWSTR KeyName,
	IN ULONG MaxLength,
	OUT LPWSTR pOutBuffer
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/07/06 [6:7:2010 - 14:20]

Routine Description:
  ��ѯ�����ļ���key����ĵ����ַ�����Ϣ
    
Arguments:
  SeactionName - eg: [GlobalSetting]
  KeyName - eg: ForceProcess
  MaxLength - �ַ�������󳤶�
  pOutBuffer - �����ѯ���õ����ַ���
    
--*/
{
	ULONG length = 0 ;
	LPPB_CONFIG_KEY pNode = NULL ;
	IOCTL_HANDLERCONF_BUFFER_IniDataW lpData ;

	// 1. У������Ϸ���
	if ( NULL == SeactionName || NULL == KeyName || NULL == pOutBuffer )
	{
		dprintf( "error! | GetConfigurationSW(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	if ( FALSE == g_ConfigData_init_ok )
	{
		dprintf( "error!| GetConfigurationSW(); | �����е��������ļ�����! \n" );
		return FALSE ;
	}

	// 2.����֮
	pNode = (LPPB_CONFIG_KEY) GetConfigurationMW( SeactionName, KeyName );
	if ( NULL == pNode )
	{
// 		dprintf( 
// 			"ko! | GetConfigurationSW() - GetConfigurationMW(); | ��ѯ������Ϣʧ��; Section:%ws, Key;%ws \n",
// 			SeactionName, KeyName
// 			);

		return FALSE ;
	}

	// 3. ���ҵ�,����֮
	length = MaxLength < pNode->ValueListHead.NameLength ? MaxLength : pNode->ValueListHead.NameLength ;
	RtlCopyMemory( pOutBuffer, pNode->ValueListHead.ValueName, length );
	pOutBuffer[ length / sizeof(WCHAR) ] = UNICODE_NULL ;

	return TRUE ;
}



PVOID
GetConfigurationMW (
	IN LPWSTR SeactionName,
	IN LPWSTR KeyName
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/07/06 [6:7:2010 - 14:20]

Routine Description:
  ��ѯ�����ļ���key�����һϵ���ַ�����Ϣ,�����ַ�������ı�ͷ(LPPB_CONFIG_KEY)
    
Arguments:
  SeactionName - eg: [GlobalSetting]
  KeyName - eg: OpenIpcPath
  
--*/
{
	BOOL bRet = FALSE ;
	SEARCH_INFO OutBuffer = { 0 };

	// 1. У������Ϸ���
	if ( NULL == SeactionName || NULL == KeyName )
	{
		dprintf( "error! | GetConfigurationSW(); | Invalid Paramaters; failed! \n" );
		return NULL ;
	}

	if ( FALSE == g_ConfigData_init_ok )
	{
		dprintf( "error!| GetConfigurationMW(); | �����е��������ļ�����! \n" );
		return NULL ;
	}

	// 2.����֮
	bRet = kGetKeyNode( SeactionName, KeyName, &OutBuffer );
	if ( _NODE_IS_KEY_ != OutBuffer.NodeType || NULL == OutBuffer.pNode )
	{
#if 0
		dprintf( 
			"error! | GetConfigurationMW() - kGetKeyNode(); | ��ѯ������Ϣʧ��; Section:%ws, Key;%ws \n",
			SeactionName, KeyName
			);
#endif
		return NULL ;
	}

	return OutBuffer.pNode ;
}



BOOL
ConfMatch (
	IN LPWSTR lpBuffer,
	IN LPWSTR Tag
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/06/10 [10:6:2010 - 17:06]

Routine Description:
  ��@lpBuffer���������; �����в���Tag,��ƥ�䵽�򷵻�TRUE 
    
Arguments:
  lpBuffer - �����ļ��в�ѯ�õ�������,��ʽ����:C:\\Program Files\\Internet Explorer\\IEXPLORE.EXE;C:\\WINDOWS\\NOTEPAD.EXE"; D:\\TestFolder
  Tag - ��ƥ����ַ���

Return Value:
  TRUE - ��ƥ�䵽;
    
--*/
{	
	//
	// 1. У������Ϸ���
	//
	
	if ( NULL == lpBuffer || NULL == Tag )
	{
		dprintf( "error! | ConfMatch(); | Invalid Paramaters; failed! \n" );
		return FALSE ;
	}

	//
	// 2. ��@lpBuffer��������� 
	//

	//
	// 3. ƥ��֮
	//

	if ( 0 == _wcsicmp( Tag, lpBuffer ) )
	{
		return TRUE ;
	}

	return FALSE ;
}



VOID
AbortWait_for_DriverUnload (
	)
{
	BOOL bRet = FALSE ;
	BOOL bOK = FALSE ;
	HANDLE hEvent = NULL ;

	bRet = OpenEvent( g_ConfEvent_wakeup_r0, EVENT_ALL_ACCESS, &hEvent );
	if ( FALSE == bRet ) { return ; }

	ZwSetEvent( hEvent, 0 ); // �����¼���TRUE
	ZwClose( hEvent );

	return ;
}

///////////////////////////////   END OF FILE   ///////////////////////////////