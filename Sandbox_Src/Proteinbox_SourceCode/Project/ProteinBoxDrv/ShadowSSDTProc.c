/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/08/05 [5:8:2010 - 11:30]
* MODULE : d:\Work\Program\Coding\ɳ��\SandBox\Code\Project\ProteinBoxDrv\ShadowSSDTProc.c
* 
* Description:
*      
*   fake Shadow SSDT ������,���𴰿�/��Ϣ����Ȳ���                       
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "Version.h"
#include "Common.h"
#include "HookEngine.h"
#include "ProcessData.h"
#include "GrayList.h"
#include "SdtData.h"
#include "ObjectProc.h"
#include "ShadowSSDTProc.h"

//////////////////////////////////////////////////////////////////////////

_NtUserSendInput_			g_NtUserSendInput_addr				= NULL ;
_NtUserQueryWindow_			g_NtUserQueryWindow_addr			= NULL ;
_NtUserGetClassName_		g_NtUserGetClassName_addr			= NULL ;
_NtUserGetForegroundWindow_ g_NtUserGetForegroundWindow_addr	= NULL ;


ULONG g_LsaAuthPkg = -1 ;
ULONG g_SendInput_RetValue = 0 ;

Win32k_apfnSimpleCall_Index g_Win32k_apfnSimpleCall_Index_Info = { 0 } ;


// ��ֹ��ɳ����İ��������̷���������Ϣ
static const UINT g_forbidden_Message_Array[] = 
{
	WM_DESTROY ,
	WM_SETREDRAW ,
	WM_CLOSE ,
	WM_QUERYENDSESSION ,
	WM_QUIT ,
	WM_ENDSESSION ,
	WM_NOTIFY ,
	WM_NCDESTROY ,
	WM_COMMAND ,
	WM_SYSCOMMAND ,
	0x319 ,
};


// ����ͨ��NtUserSystemParametersInfo����������������
static const UINT g_SystemParametersInfo_WhiteList_Array [] = 
{
	1 ,		// SPI_GETBEEP
	3 ,		// SPI_GETMOUSE
	5 ,		// SPI_GETBORDER
	0x0A ,	// SPI_GETKEYBOARDSPEED
	0x0D ,	// SPI_ICONHORIZONTALSPACING
	0x0E ,	// SPI_GETSCREENSAVETIMEOUT
	0x10 ,	// SPI_GETSCREENSAVEACTIVE
	0x12 ,	// SPI_GETGRIDGRANULARITY
	0x16 ,	// SPI_GETKEYBOARDDELAY
	0x18 ,	// SPI_ICONVERTICALSPACING
	0x19 ,	// SPI_GETICONTITLEWRAP
	0x1B ,	// SPI_GETMENUDROPALIGNMENT
	0x1F ,	// SPI_GETICONTITLELOGFONT
	0x23 ,	// SPI_GETFASTTASKSWITCH
	0x26 ,	// SPI_GETDRAGFULLWINDOWS
	0x29 ,	// SPI_GETNONCLIENTMETRICS
	0x2B ,	// SPI_GETMINIMIZEDMETRICS
	0x2D ,	// SPI_GETICONMETRICS
	0x30 ,	// SPI_GETWORKAREA
	0x42 ,	// SPI_GETHIGHCONTRAST
	0x44 ,	// SPI_GETKEYBOARDPREF
	0x46 ,	// SPI_GETSCREENREADER
	0x48 ,	// SPI_GETANIMATION
	0x4A ,	// SPI_GETFONTSMOOTHING
	0x4F ,	// SPI_GETLOWPOWERTIMEOUT
	0x50 ,	// SPI_GETPOWEROFFTIMEOUT
	0x53 ,	// SPI_GETLOWPOWERACTIVE
	0x54 ,  // SPI_GETPOWEROFFACTIVE
	0x59 ,	// SPI_GETDEFAULTINPUTLANG
	0x5C ,	// SPI_GETWINDOWSEXTENSION
	0x5E ,	// SPI_GETMOUSETRAILS
	0x32 ,	// SPI_GETFILTERKEYS
	0x34 ,	// SPI_GETTOGGLEKEYS
	0x36 ,	// SPI_GETMOUSEKEYS
	0x38 ,	// SPI_GETSHOWSOUNDS
	0x3A ,	// SPI_GETSTICKYKEYS
	0x3C ,	// SPI_GETACCESSTIMEOUT
	0x3E ,	// SPI_GETSERIALKEYS
	0x40 ,	// SPI_GETSOUNDSENTRY
	0x5F ,	// SPI_GETSNAPTODEFBUTTON
	0x62 ,	// SPI_GETMOUSEHOVERWIDTH
	0x64 ,	// SPI_GETMOUSEHOVERHEIGHT
	0x66 ,	// SPI_GETMOUSEHOVERTIME
	0x68 ,	// SPI_GETWHEELSCROLLLINES
	0x6A ,	// SPI_GETMENUSHOWDELAY
	0x6C ,	// SPI_GETWHEELSCROLLCHARS
	0x6E ,	// SPI_GETSHOWIMEUI
	0x70 ,	// SPI_GETMOUSESPEED
	0x72 ,	// SPI_GETSCREENSAVERRUNNING
	0x73 ,	// SPI_GETDESKWALLPAPER
	0x74 ,	// SPI_GETAUDIODESCRIPTION
	0x76 ,	// SPI_GETSCREENSAVESECURE
	0x1000 ,	// SPI_GETACTIVEWINDOWTRACKING
	0x1002 ,	// SPI_GETMENUANIMATION
	0x1004 ,	// SPI_GETCOMBOBOXANIMATION
	0x1006 ,	// SPI_GETLISTBOXSMOOTHSCROLLING
	0x1008 ,	// SPI_GETGRADIENTCAPTIONS
	0x100A ,	// SPI_GETKEYBOARDCUES
	0x100C ,	// SPI_GETACTIVEWNDTRKZORDER
	0x100E ,	// SPI_GETHOTTRACKING
	0x1012 ,	// SPI_GETMENUFADE
	0x1014 ,	// SPI_GETSELECTIONFADE
	0x1016 ,	// SPI_GETTOOLTIPANIMATION
	0x1018 ,	// SPI_GETTOOLTIPFADE
	0x101A ,	// SPI_GETCURSORSHADOW
	0x101C ,	// SPI_GETMOUSESONAR
	0x101E ,	// SPI_GETMOUSECLICKLOCK
	0x1020 ,	// SPI_GETMOUSEVANISH
	0x1022 ,	// SPI_GETFLATMENU
	0x1024 ,	// SPI_GETDROPSHADOW
	0x1026 ,	// SPI_GETBLOCKSENDINPUTRESETS
	0x103E ,	// SPI_GETUIEFFECTS
	0x1040 ,	// SPI_GETDISABLEOVERLAPPEDCONTENT
	0x1042 ,	// SPI_GETCLIENTAREAANIMATION
	0x1048 ,	// SPI_GETCLEARTYPE
	0x104A ,	// SPI_GETSPEECHRECOGNITION
	0x2000 ,	// SPI_GETFOREGROUNDLOCKTIMEOUT
	0x2002 ,	// SPI_GETACTIVEWNDTRKTIMEOUT
	0x2004 ,	// SPI_GETFOREGROUNDFLASHCOUNT
	0x2006 ,	// SPI_GETCARETWIDTH
	0x2008 ,	// SPI_GETMOUSECLICKLOCKTIME
	0x200A ,	// SPI_GETFONTSMOOTHINGTYPE
	0x200C ,	// SPI_GETFONTSMOOTHINGCONTRAST
	0x200E ,	// SPI_GETFOCUSBORDERWIDTH
	0x2010 ,	// SPI_GETFOCUSBORDERHEIGHT
	0x2012 ,	// SPI_GETFONTSMOOTHINGORIENTATION
	0x2014 ,	// 
	0x2016 ,	// 
};

/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


/*
VOID
fake_NtUserMessageCall(
    IN HWND hwnd,
    IN UINT msg,
    IN WPARAM wParam,
    IN LPARAM lParam,
    IN ULONG_PTR xParam,
    IN DWORD xpfnProc,
    IN BOOL bAnsi
	)
*/
VOID _declspec(naked) fake_NtUserMessageCall()
{
	int ret, msg, hwnd ;
	LPPDNODE pNode ;
	LPSSDT_SSSDT_FUNC FuncInfo ;

	// 1. ǰ�ô���
	PreHelper( Tag_SendMessageTimeoutW );
	
	// 2. ����֮
	__asm 
	{
		mov	    eax, dword ptr [ebp+8h] // ȡ����
		mov		hwnd, eax

		mov	    eax, dword ptr [ebp+0Ch]
		mov		msg, eax
	}

	if ( MsgFilter( (PVOID)pNode, _MSG_TYPE_SEND_, (HWND)hwnd, msg ) ) { goto _Pass_ ; }

_Denny_ :
	FinisDenny() ;
	__asm
	{
		mov eax, 0	// �޸ķ���ֵ
		ret 1Ch
	}

_Pass_ :
	FinisOrig();
}



// VOID
// fake_NtUserSendMessageCallback(
//     IN HWND hwnd,
//     IN UINT wMsg,
//     IN WPARAM wParam,
//     IN LPARAM lParam,
//     IN /*SENDASYNCPROC*/PVOID lpResultCallBack,
//     IN ULONG_PTR dwData
// 	)
VOID _declspec(naked) fake_NtUserSendMessageCallback()
{
	int ret, msg ;
	HWND hwnd;
	LPPDNODE pNode ;
	LPSSDT_SSSDT_FUNC FuncInfo ;

	// 1. ǰ�ô���
	PreHelper( Tag_SendMessageCallbackW );

	// 2. ����֮
	__asm 
	{
		mov	    eax, dword ptr [ebp+8h] // ȡ����
		mov		hwnd, eax

		mov	    eax, dword ptr [ebp+0Ch]
		mov		msg, eax
	}

	if ( MsgFilter( (PVOID)pNode, _MSG_TYPE_SEND_, hwnd, msg ) ) { goto _Pass_ ; }

_Denny_ :
	FinisDenny() ;
	__asm
	{
		mov eax, 0	// �޸ķ���ֵ
		ret 18h
	}

_Pass_ :
	FinisOrig();
}



/*
VOID
fake_NtUserSendNotifyMessage(
    IN HWND hwnd,
    IN UINT Msg,
    IN WPARAM wParam,
    IN LPARAM lParam OPTIONAL
	)
*/
VOID _declspec(naked) fake_NtUserSendNotifyMessage()
{
	int ret, msg ;
	HWND hwnd;
	LPPDNODE pNode ;
	LPSSDT_SSSDT_FUNC FuncInfo ;

	// 1. ǰ�ô���
	PreHelper( Tag_SendNotifyMessageW_win2k );

	// 2. ����֮
	__asm 
	{
		mov	    eax, dword ptr [ebp+8h] // ȡ����
		mov		hwnd, eax

		mov	    eax, dword ptr [ebp+0Ch]
		mov		msg, eax
	}

	if ( MsgFilter( (PVOID)pNode, _MSG_TYPE_SEND_, hwnd, msg ) ) { goto _Pass_ ; }

_Denny_ :
	FinisDenny() ;
	__asm
	{
		mov eax, 0	// �޸ķ���ֵ
		ret 10h
	}

_Pass_ :
	FinisOrig();
}



/*
VOID
fake_NtUserPostMessage(
    IN HWND hwnd,
    IN UINT msg,
    IN WPARAM wParam,
    IN LPARAM lParam
	)
*/
VOID _declspec(naked) fake_NtUserPostMessage()
{
	int ret, msg ;
	HWND hwnd;
	LPPDNODE pNode ;
	LPSSDT_SSSDT_FUNC FuncInfo ;

	// 1. ǰ�ô���
	PreHelper( Tag_PostMessageW );

	// 2. ����֮
	__asm 
	{
		mov	    eax, dword ptr [ebp+8h] // ȡ����
		mov		hwnd, eax

		mov	    eax, dword ptr [ebp+0Ch]
		mov		msg, eax
	}

	if ( MsgFilter( (PVOID)pNode, _MSG_TYPE_POST_, hwnd, msg ) ) { goto _Pass_ ; }

_Denny_ :
	FinisDenny() ;
	__asm
	{
		mov eax, 0	// �޸ķ���ֵ
		ret 10h
	}

_Pass_ :
	FinisOrig();
}



/*
VOID
fake_NtUserPostThreadMessage(
    IN DWORD id,
    IN UINT msg,
    IN WPARAM wParam,
    IN LPARAM lParam
	)
*/
VOID _declspec(naked) fake_NtUserPostThreadMessage()
{
	UINT ret, msg ;
	HANDLE TID, PID ;
	LPPDNODE pNode ;
	LPSSDT_SSSDT_FUNC FuncInfo ;

	// 1. ǰ�ô���
	PreHelper( Tag_PostThreadMessageW );

	// 2. ����֮
	__asm 
	{
		mov	    eax, dword ptr [ebp+8h] // ȡ����
		mov		TID, eax

		mov	    eax, dword ptr [ebp+0Ch]
		mov		msg, eax
	}

	if ( NULL == TID ) { TID = PsGetCurrentThreadId(); }

	if ( pNode->XWnd.bFlag_SendPostMessage_allowed ) { goto _Pass_ ; } // ����NtUser**������Ϣ,����

	if ( ! (msg & 0xFFFF0000) && (msg > 0xC000) ) { goto _Pass_ ; }

	if ( IsApprovedTID( (PVOID)pNode, TID, &PID ) ) { goto _Pass_ ; } // ��������ɳ���еĽ���,����

	if ( IsWhiteProcess( (PVOID)pNode, PID ) ) { goto _Pass_ ; } // ��������ɳ����İ׽���,����

_Denny_ :
	FinisDenny() ;
	__asm
	{
		mov eax, 0	// �޸ķ���ֵ
		ret 10h
	}

_Pass_ :
	FinisOrig();
}



/*
VOID
fake_NtUserCallHwndParamLock(
    IN HWND hwnd,
    IN ULONG_PTR dwParam,
    IN DWORD xpfnProc
	)
*/
VOID _declspec(naked) fake_NtUserCallHwndParamLock()
{
	UINT ret, xpfnProc ;
	HANDLE PID ;
	HWND hWnd ;
	LPPDNODE pNode ;
	LPSSDT_SSSDT_FUNC FuncInfo ;

	// 1. ǰ�ô���
	PreHelper( Tag_EnableWindow );
	PID = hWnd = NULL ;
	
	// 2. ����֮
	__asm 
	{
		mov	    eax, dword ptr [ebp+8h] // ȡ����
		mov		hWnd, eax

		mov	    eax, dword ptr [ebp+10h]
		mov		xpfnProc, eax
	}

	// ֻ��עEnableWindow��Ӧ��Index
	if ( xpfnProc != g_Win32k_apfnSimpleCall_Index_Info.SFI_XXXENABLEWINDOW ) { goto _Pass_ ; } 

	// hWnd --> PID ; ת��ʧ����ܾ���
	PID = g_NtUserQueryWindow_addr( hWnd, QUERY_WINDOW_UNIQUE_PROCESS_ID );
	if ( NULL == PID ) { goto _Denny_ ; }

	// �Լ����Լ�����Ϣ �� ��ɳ���еĽ��̷���Ϣ,����
	if ( PsGetCurrentProcessId() == PID || IsApprovedPIDEx( (PVOID)pNode, (ULONG)PID) ) { goto _Pass_ ; }

_Denny_ :
	FinisDenny() ;
	__asm
	{
		mov eax, 0	// �޸ķ���ֵ
		ret 0Ch
	}

_Pass_ :
	FinisOrig();
}



/*
VOID
fake_NtUserDestroyWindow(
    IN HWND hwnd
	)
*/
VOID _declspec(naked) fake_NtUserDestroyWindow()
{
	int ret ;
	HWND hwnd;
	HANDLE PID ;
	LPPDNODE pNode ;
	LPSSDT_SSSDT_FUNC FuncInfo ;

	// 1. ǰ�ô���
	PreHelper( Tag_DestroyWindow );
	hwnd = PID = NULL ;

	__asm // ȡ����
	{
		mov	    eax, dword ptr [ebp+8h]
		mov		hwnd, eax
	}

	// 2. ����֮
	PID = g_NtUserQueryWindow_addr( hwnd, QUERY_WINDOW_UNIQUE_PROCESS_ID );
	if ( PID )
	{
		// ���ڵ���������,���ܲ���
		if ( PsGetCurrentProcessId() == PID ) { goto _Pass_ ; } 
		
		// ��������ͬһɳ���еĳ���, ����֮
		if ( IsApprovedPIDEx( (PVOID)pNode, (ULONG)PID ) )  { goto _Pass_ ; } 

		// ���������ֹ��,����Ϣ��ʾ�û�
		ShowWarningInfo( _DestroyWindow_Type_, (HANDLE)PID );
	}

_Denny_ :
	FinisDenny() ;
	__asm
	{
		mov eax, 0	// �޸ķ���ֵ
		ret 4h
	}

_Pass_ :
	FinisOrig();
}



/*
VOID
fake_NtUserSendInput(
	IN UINT    cInputs,
	IN int pInputs,
	IN int     cbSize
	);
*/

VOID _declspec(naked) fake_NtUserSendInput()
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/17 [17:8:2010 - 18:26]

Routine Description:
  ��ֹ��ɳ����ĳ�����ģ�ⰴ��(��� & ������Ϣ). ����NtUserGetForegroundWindow,NtUserQueryWindow
  �õ��ܺ��ߵ�PID,ͨ��Find_Node_from_PID()�鿴���Ƿ�Ϊɳ���ڲ�����.�������ֹ��,���ڲ����������֮.

  ���Գ�������:
  HWND hwnd = FindWindow(NULL,"�������ع��� V1.3��");
  SetForegroundWindow(hwnd);
  inputs[0].type = INPUT_KEYBOARD;
  inputs[0].ki.wVk = VK_ESCAPE;
  inputs[1].type = INPUT_KEYBOARD;
  inputs[1].ki.wVk = VK_ESCAPE;
  inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

  SendInput(2,inputs,sizeof(INPUT)); // ���ڲ������NtUserSendInput,�ú�����һ������ò����Զ��1(��ȷ��)
  // һ���ڲ�����:NtUserSendInput(1, &kbd, sizeof(INPUT)); kbd������鱣��������2��С���鵥Ԫ������
    
--*/
{
	int cInputs, pInputs, cbSize, n, ret ;
	BOOL bJumpOut ;
	HWND hwndActive;
	HANDLE PID ;
	LPPDNODE pNode ;
	LPSSDT_SSSDT_FUNC FuncInfo ;

	// 1. ǰ�ô���
	PreHelper( Tag_SendInput );	
	n = ret = cInputs = 0 ;
	hwndActive = PID = NULL ;

	/*
	kd> u win32k!NtUserSendInput
	win32k!NtUserSendInput:
	bf849b03 6a18            push    18h
	bf849b05 6858b398bf      push    offset win32k!`string'+0x5b8 (bf98b358)
	bf849b0a e8d7d448c2      call    81cd6fe6 // ���ǵ�Hook,�ʷ��ص�ַӦ��NtUserSendInput�ڲ�

	��ջ�������:
	esp     <-- ��Ϊ��ǰ����û��ebp,��ebp����������KiFastCallEntry�ռ�
	retAddr <-- ���ص�  call    81cd6fe6��������ַ
	arg1    <-- 18h
	arg2    <-- offset win32k!`string'+0x5b8 (bf98b358)
	retAddr <-- nt!KiFastCallEntry�����ڲ�
	arg1    <-- cInputs  (NtUserSendInput�Ĳ���1)
	arg2    <-- pInputs  (NtUserSendInput�Ĳ���2)
	arg3    <-- cbSize   (NtUserSendInput�Ĳ���3)

	// 1.1 ��fake��������ʱ��call NtUserSendInput,Ҫ��֤����ʱfake����ֱ�ӷ���.���жϷ��ص�ַ�Ƿ��ڵ�ǰ�����ڲ�
	*/
	__asm
	{
		// ȡ����
		mov		eax, dword ptr [ebp+14h]
		mov		cInputs, eax

		mov		eax, dword ptr [ebp+18h]
		mov		pInputs, eax

		mov		eax, dword ptr [ebp+1Ch]
		mov		cbSize, eax
	}

	// 2. ������1����������,��������Ҫ����ԭʼ����,��ԭʼ����
	if ( cInputs & _SimulationButton_Flag_ ) 
	{
		ClearFlag( cInputs, _SimulationButton_Flag_ );
		__asm
		{ 
			mov   eax, cInputs
			mov   dword ptr [ebp+14h], eax
		}

		goto _Pass_ ;
	}
	
	// 3.1 �ж��Ƿ��ֹ���ⰴ��,����������ԭ����
	if ( 0 == pNode->XWnd.bFlag_BlockFakeInput ) { goto _Pass_ ; }

	// 3.2 ��Ҫ��ֹ,���߹�������
	while ( cInputs && cInputs < 0x10 )
	{
		// �õ���ǰ���� --> PID --> ��֤�Ƿ���ɳ����
		bJumpOut = FALSE ;
		hwndActive = g_NtUserGetForegroundWindow_addr();
		if ( hwndActive )
		{
			PID = g_NtUserQueryWindow_addr( hwndActive, QUERY_WINDOW_UNIQUE_PROCESS_ID );
			if ( NULL == kgetnodePD( PID ) )
			{
				bJumpOut = TRUE ; // ��ǰ�����Ĵ��ڲ���ɳ����,Ҫ��ֹ��
			}
		}
		else
		{
			bJumpOut = TRUE ;
		}

		if ( bJumpOut )
		{
			if ( pNode->XWnd.CurrentHwnd != hwndActive ) { pNode->XWnd.CurrentHwnd = hwndActive ; }
			break ;
		}

		// ����������,����ԭʼ����
		if ( 1 != g_NtUserSendInput_addr( 1 | _SimulationButton_Flag_ , pInputs, cbSize ) ) { break ; }

		// cInputs ָ���˵�ǰָ������ĸ���; pInputs ��ָ�������ͷָ��; cbSize��ÿ��ָ�Ԫ�Ĵ�С
		-- cInputs ;		// ��������ݼ�
		pInputs += cbSize ; // ��ָ���ƶ����¸����鵥Ԫ
		++ n ;
	}

_Denny_ :
	if ( bJumpOut ) { ShowWarningInfo( _SendInput_Type_, (HANDLE)PID ); }
	g_SendInput_RetValue = n ;

	FinisDenny() ;
	__asm
	{
		add esp, 0Ch // ������ص�ַ (���ص�call 81cd6fe6��������ַ ) + �ֲ����� 2��,����SHE����
		mov eax, g_SendInput_RetValue	// �޸ķ���ֵ
		ret 0Ch
	}

_Pass_ :
	FinisOrig();
}



/*
VOID
fake_NtUserBlockInput(
    IN BOOL fBlockIt
	)
*/
VOID _declspec(naked) fake_NtUserBlockInput()
{
	int ret ;
	BOOL fBlockIt ;
	HWND hwndActive;
	LPPDNODE pNode ;
	LPSSDT_SSSDT_FUNC FuncInfo ;

	// 1. ǰ�ô���
	PreHelper( Tag_BlockInput );

	fBlockIt = FALSE ;

	// 2. �ж��Ƿ��ֹ���ⰴ��,����������ԭ����
	__asm
	{
		mov	eax, dword ptr [ebp+8]
		mov	fBlockIt, eax
	}
	
	if ( 0 == pNode->XWnd.bFlag_BlockFakeInput || FALSE == fBlockIt ) { goto _Pass_ ; }

	// 3. ��ʾ���ص�����Ϣ
	hwndActive = g_NtUserGetForegroundWindow_addr();
	if ( pNode->XWnd.CurrentHwnd != hwndActive ) { pNode->XWnd.CurrentHwnd = hwndActive ; }

	ShowWarningInfo( _BlockInput_Type_, NULL );

_Denny_ :
	FinisDenny() ;
	__asm
	{
		mov eax, 0	// �޸ķ���ֵ
		ret 4h
	}

_Pass_ :
	FinisOrig();
}



/*
VOID
fake_NtUserSetSysColors(
    IN int cElements,
    IN CONST INT * lpaElements,
    IN CONST COLORREF * lpaRgbValues,
    IN UINT  uOptions
	)
*/
VOID _declspec(naked) fake_NtUserSetSysColors()
{
	int ret ;
	LPPDNODE pNode ;
	LPSSDT_SSSDT_FUNC FuncInfo ;

	// 1. ǰ�ô���
	PreHelper( Tag_SetSysColors );

	if ( 0 == pNode->XWnd.bFlag_BlockSysParam ) { goto _Pass_ ; }

	ShowWarningInfo( _SetSysColors_Type_, NULL );

_Denny_ :
	FinisDenny() ;
	__asm
	{
		mov eax, 0	// �޸ķ���ֵ
		ret 10h
	}

_Pass_ :
	FinisOrig();
}



/*
BOOL
APIENTRY
NtUserSystemParametersInfo (
    UINT uiAction,
    UINT uiParam,
    PVOID pvParam,
    UINT fWinIni
	);
*/
VOID _declspec(naked) fake_NtUserSystemParametersInfo(VOID)
{
	int ret, Index ;
	UINT uiAction, pvParam ;
	LPPDNODE pNode ;
	LPSSDT_SSSDT_FUNC FuncInfo ;

	// 1. ǰ�ô���
	PreHelper( Tag_SystemParametersInfoW );
	uiAction = pvParam = 0 ;

	if ( 0 == pNode->XWnd.bFlag_BlockSysParam ) { goto _Pass_ ; }

	/*
	kd> u win32k!NtUserSystemParametersInfo
    win32k!NtUserSystemParametersInfo:
	bf843888 6850020000      push    250h
	bf84388d 6880ae98bf      push    offset win32k!`string'+0xe0 (bf98ae80)
	bf843892 e84fd7bfc2      call    82440fe6 // ���ǵ�Hook��

	��ջ�������:
	esp     <-- ��Ϊ��ǰ����û��ebp,��ebp����������KiFastCallEntry�ռ�
	retAddr <-- ���ص�  call    81cd6fe6��������ַ
	arg1    <-- 250h
	arg2    <-- offset win32k!`string'+0xe0 (bf98ae80)
	retAddr <-- nt!KiFastCallEntry�����ڲ�
	arg1    <-- cInputs  (NtUserSystemParametersInfo�Ĳ���1)
	...

	*/
	__asm 
	{
		mov	    eax, dword ptr [ebp+14h] // ȡ����
		mov		uiAction, eax

		mov	    eax, dword ptr [ebp+1Ch]
		mov		pvParam, eax
	}

	// ��������������
	for ( Index=0; Index<ARRAYSIZEOF(g_SystemParametersInfo_WhiteList_Array); Index++ )
	{
		if ( g_SystemParametersInfo_WhiteList_Array[ Index ] == uiAction ) 
		{
			// �ڰ�������; ��ʹ�ǰ׵�,ҲҪ��ֹ�������
			if ( (SPI_ICONHORIZONTALSPACING == uiAction || SPI_ICONVERTICALSPACING == uiAction) && (0 == pvParam) )
			{
				break ;
			}
			else
			{
				goto _Pass_ ;
			}
		}
	}

	// ���ڰ�������,��ֹ����~
	ShowWarningInfo( _SystemParametersInfoW_Type_, NULL );

_Denny_ :
	FinisDenny() ;
	__asm
	{
		add esp, 0Ch // ������ص�ַ (���ص�call 82440fe6 // ���ǵ�Hook���������ַ ) + �ֲ����� 2��,����SHE����
		mov eax, 0	 // �޸ķ���ֵ
		ret 10h
	}

_Pass_ :
	FinisOrig();
}



/*
VOID
fake_NtUserSetWindowsHookEx(
    IN HANDLE hmod,
    IN PUNICODE_STRING pstrLib OPTIONAL,
    IN DWORD idThread,
    IN int nFilterType,
    IN PROC pfnFilterProc,
    IN DWORD dwFlags
	)
*/
VOID _declspec(naked) fake_NtUserSetWindowsHookEx()
{
	int ret, idThread, nFilterType ;
	LPPDNODE pNode ;
	LPSSDT_SSSDT_FUNC FuncInfo ;

	// 1. ǰ�ô���
	PreHelper( Tag_SetWindowsHookExW );
	idThread = nFilterType = 0 ;

	if ( 0 == pNode->XWnd.bFlag_BlockWinHooks ) { goto _Pass_ ; }

	__asm
	{
		mov	eax, dword ptr [ebp+10h]
		mov	idThread, eax

		mov	eax, dword ptr [ebp+14h]
		mov	nFilterType, eax
	}

	// ����3�����͵���Ϣ����:1,13,14
	if ( 0 == nFilterType || WH_JOURNALPLAYBACK == nFilterType || WH_KEYBOARD_LL == nFilterType || WH_MOUSE_LL == nFilterType ) { goto _Pass_ ; }

	// ��ֹ WH_SYSMSGFILTER ��Ϣ
	if ( WH_SYSMSGFILTER == nFilterType )  { goto _Denny_ ; }
	
	// ��������ɳ���еĽ���,����
	if ( idThread && IsApprovedTID((PVOID)pNode, (HANDLE)idThread, NULL) )  { goto _Pass_ ; } 
	
	ShowWarningInfo( _SetWindowsHookEx_Type_, NULL );

_Denny_ :
	FinisDenny() ;
	__asm
	{
		mov eax, 0	// �޸ķ���ֵ
		ret 18h
	}

_Pass_ :
	FinisOrig();
}



/*
VOID
fake_NtUserSetWinEventHook(
    IN DWORD           eventMin,
    IN DWORD           eventMax,
    IN HMODULE         hmodWinEventProc,
    IN PUNICODE_STRING pstrLib OPTIONAL,
    IN PVOID    pfnWinEventProc,
    IN DWORD           idEventProcess,
    IN DWORD           idEventThread,
    IN DWORD           dwFlags
	)
*/
VOID _declspec(naked) fake_NtUserSetWinEventHook()
{
	int ret, idEventProcess, idEventThread, dwFlags ;
	LPPDNODE pNode ;
	LPSSDT_SSSDT_FUNC FuncInfo ;

	// 1. ǰ�ô���
	PreHelper( Tag_SetWinEventHook );

	if ( 0 == pNode->XWnd.bFlag_BlockWinHooks ) { goto _Pass_ ; }

	__asm
	{
		mov	eax, dword ptr [ebp+1Ch]
		mov	idEventProcess, eax

		mov	eax, dword ptr [ebp+20h]
		mov	idEventThread, eax

		mov	eax, dword ptr [ebp+24h]
		mov	dwFlags, eax
	}

	if ( !(dwFlags & WINEVENT_INCONTEXT) ) { goto _Pass_ ; }

	if ( idEventThread && IsApprovedTID((PVOID)pNode, (HANDLE)idEventThread, NULL) )  { goto _Pass_ ; } // ��������ɳ���еĽ���,����
	
	if ( idEventProcess && IsApprovedPIDEx((PVOID)pNode, (ULONG)idEventProcess) ) { goto _Pass_ ; } // ��������ɳ���еĽ���,����
	
	ShowWarningInfo( _SetWinEventHook_Type_, (HANDLE)idEventProcess );

_Denny_ :
	FinisDenny() ;
	__asm
	{
		mov eax, 0	// �޸ķ���ֵ
		ret 20h
	}

_Pass_ :
	FinisOrig();
}



/*
VOID
fake_NtRequestPort (
    IN HANDLE PortHandle,
    IN PPORT_MESSAGE RequestMessage
    )
*/
VOID _declspec(naked) fake_NtRequestPort()
{
	NTSTATUS status ; 
	int ret, PortHandle, RequestMessage ;
	LPPDNODE pNode ;
	LPSSDT_SSSDT_FUNC FuncInfo ;

	// 1. ǰ�ô���
	PreRpcHelper( TagSpec_NtRequestPort );

	/*
	kd> u nt!NtRequestPort
	nt!NtRequestPort:
	805978b4 6a3c            push    3Ch
	805978b6 68a0a04d80      push    offset nt!_real+0xb0 (804da0a0)
	805978bb e82657d801      call    8231cfe6 // ���ǵ�Hook
	805978c0 33db            xor     ebx,ebx

	��ջ�������:
	esp     <-- ��Ϊ��ǰ����û��ebp,��ebp����������KiFastCallEntry�ռ�
	retAddr <-- ���ص�  call    81cd6fe6��������ַ
	arg1    <-- 3Ch
	arg2    <-- offset nt!_real+0xb0 (804da0a0)
	retAddr <-- nt!KiFastCallEntry�����ڲ�
	arg1    <-- PortHandle		(NtRequestPort�Ĳ���1)
	arg2    <-- RequestMessage  (NtRequestPort�Ĳ���2)

	*/
	__asm
	{
		mov		eax, dword ptr [ebp+14h] // ȡ����1
		mov		PortHandle, eax

		mov		eax, dword ptr [ebp+18h] // ȡ����2
		mov		RequestMessage, eax
	}

	status = RpcFilter( (HANDLE)PortHandle, (PVOID)RequestMessage );
	if ( NT_SUCCESS(status) ) { goto _Pass_ ; }

	ShowWarningInfo( _NtRequestPort_Type_, NULL );

_Denny_ :
	FinisDenny() ;
	__asm
	{
		add esp, 0Ch  // ������ص�ַ (���ص�call 8231cfe6 // ���ǵ�Hook���������ַ ) + �ֲ����� 2��,����SHE����
		mov eax, 0C000010Ah		// �޸ķ���ֵ STATUS_PROCESS_IS_TERMINATING;
		ret 8h
	}

_Pass_ :
	FinisOrig();
}



/*
VOID
fake_NtRequestWaitReplyPort (
    IN HANDLE PortHandle,
    IN PPORT_MESSAGE RequestMessage,
    OUT PPORT_MESSAGE ReplyMessage
    )
*/
VOID _declspec(naked) fake_NtRequestWaitReplyPort()
{
	NTSTATUS status ; 
	int ret, PortHandle, RequestMessage ;
	LPPDNODE pNode ;
	LPSSDT_SSSDT_FUNC FuncInfo ;

	// 1. ǰ�ô���
	PreRpcHelper( TagSpec_NtRequestWaitReplyPort );

	/*
	kd> u nt!NtRequestWaitReplyPort
	nt!NtRequestWaitReplyPort:
	80597be0 6a7c            push    7Ch
	80597be2 68b8a04d80      push    offset nt!_real+0xc8 (804da0b8)
	80597be7 e8fa733102      call    828aefe6 // ���ǵ�Hook
	80597bec 33f6            xor     esi,esi

	��ջ�������:
	esp     <-- ��Ϊ��ǰ����û��ebp,��ebp����������KiFastCallEntry�ռ�
	retAddr <-- ���ص�  call    828aefe6��������ַ
	arg1    <-- 7Ch
	arg2    <-- offset nt!_real+0xc8 (804da0b8)
	retAddr <-- nt!KiFastCallEntry�����ڲ�
	arg1    <-- PortHandle		(NtRequestWaitReplyPort�Ĳ���1)
	arg2    <-- RequestMessage  (NtRequestWaitReplyPort�Ĳ���2)

	*/
	__asm
	{
		mov		eax, dword ptr [ebp+14h] // ȡ����1
		mov		PortHandle, eax

		mov		eax, dword ptr [ebp+18h] // ȡ����2
		mov		RequestMessage, eax
	}

	status = RpcFilter( (HANDLE)PortHandle, (PVOID)RequestMessage );
	if ( NT_SUCCESS(status) ) { goto _Pass_ ; }

	ShowWarningInfo( _NtRequestWaitReplyPort_Type_, NULL );

_Denny_ :
	FinisDenny() ;
	__asm
	{
		add esp, 0Ch  // ������ص�ַ (���ص�call 828aefe6 // ���ǵ�Hook���������ַ ) + �ֲ����� 2��,����SHE����
		mov eax, 0C000010Ah		// �޸ķ���ֵ STATUS_PROCESS_IS_TERMINATING;
		ret 0Ch
	}

_Pass_ :
	FinisOrig();
}



/*
VOID
fake_NtAlpcSendWaitReceivePort(
	IN HANDLE PortHandle,
	IN int a2,
	IN void *RequestMessage,
	int a4, int a5, int a6, int a7, int a8
	)
*/
VOID _declspec(naked) fake_NtAlpcSendWaitReceivePort()
{
	NTSTATUS status ; 
	int ret, PortHandle, RequestMessage ;
	LPPDNODE pNode ;
	LPSSDT_SSSDT_FUNC FuncInfo ;

	// 1. ǰ�ô���
	PreRpcHelper( TagSpec_NtAlpcSendWaitReceivePort );

	__asm
	{
		mov		eax, dword ptr [ebp+14h] // ȡ����1
		mov		PortHandle, eax

		mov		eax, dword ptr [ebp+1Ch] // ȡ����3
		mov		RequestMessage, eax
	}

	status = RpcFilter( (HANDLE)PortHandle, (PPORT_MESSAGE)RequestMessage );
	if ( NT_SUCCESS(status) ) { goto _Pass_ ; }

	ShowWarningInfo( _NtAlpcSendWaitReceivePort_Type_, NULL );

_Denny_ :
	FinisDenny() ;
	__asm
	{
		add esp, 0Ch  // ������ص�ַ (���ص�call 828aefe6 // ���ǵ�Hook���������ַ ) + �ֲ����� 2��,����SHE����
		mov eax, 0C000010Ah		// �޸ķ���ֵ STATUS_PROCESS_IS_TERMINATING;
		ret 20h
	}

_Pass_ :
	FinisOrig();
}






/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                         Others                            +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--


int
Prepare_for_RPC (
	OUT PVOID* _pNode
	)
{
	BOOL bRet = TRUE ;
	LPPDNODE pNode = NULL ;
	
	// 1. ����PID��Ӧ�Ľ����ܽڵ�,��δ�ҵ�,������ɳ���еĳ���,����
	pNode = (LPPDNODE) kgetnodePD( 0 );
	if ( NULL == pNode ) { return _PREPARE_RETURN_TYPE_PASS_ ; }

	// 2. �����������Ľڵ�,���ؾܾ���־λ
	if ( pNode->bDiscard ) { return _PREPARE_RETURN_TYPE_DENNY_ ; }
	
	// 3. һ������,���ؼ�����־λ
	*_pNode = (PVOID) pNode ;
	return _PREPARE_RETURN_TYPE_GOON_ ;
}



int
Prepare_for_NtUser (
	OUT PVOID* _pNode
	)
{
	BOOL bRet = TRUE ;
	LPPDNODE pNode = NULL ;

	// 1. ��ȡ����shaodw ssdt��ԭʼ��ַ,������ȫ�ֱ�����; ��ȡ��ַ�쳣Ĭ�Ϸ���
	bRet = Prepare_for_NtUserEx ();
	if ( FALSE == bRet ) { return _PREPARE_RETURN_TYPE_PASS_ ; }
	
	// 2. ����PID��Ӧ�Ľ����ܽڵ�,��δ�ҵ�,������ɳ���еĳ���,����
	pNode = (LPPDNODE) kgetnodePD( 0 );
	if ( NULL == pNode ) { return _PREPARE_RETURN_TYPE_PASS_ ; }

	// 3. �����������Ľڵ�,���ؾܾ���־λ
	if ( pNode->bDiscard ) { return _PREPARE_RETURN_TYPE_DENNY_ ; }
	
	// 4. ������Ӧ�ĺڰ�����,����ʧ���򽫸ýڵ���Ϊ"������",���ؾܾ���־λ
	if ( FALSE == pNode->XWnd.bFlagInited ) 
	{
		bRet = BuildGrayList_for_Wnd( (PVOID)pNode );
		if ( FALSE == bRet )
		{
			dprintf( "error! | fake_NtUser*() - BuildGrayList_for_Wnd(); | ����������ʧ��. \n" );
			pNode->bDiscard = 1 ;
			return _PREPARE_RETURN_TYPE_DENNY_ ;
		}
	}

	// 5. һ������,���ؼ�����־λ
	*_pNode = (PVOID) pNode ;
	return _PREPARE_RETURN_TYPE_GOON_ ;
}



BOOL Prepare_for_NtUserEx ()
{
	BOOL bRet = FALSE ;

	if ( NULL == g_NtUserSendInput_addr )
	{
		bRet = Get_sdtfunc_addr( (PULONG)&g_NtUserSendInput_addr, Tag_SendInput );
		if ( FALSE == bRet || 0 == g_NtUserSendInput_addr )
		{
			return FALSE ;
		}
	}

	if ( NULL == g_NtUserGetForegroundWindow_addr )
	{
		bRet = Get_sdtfunc_addr( (PULONG)&g_NtUserGetForegroundWindow_addr, Tag_GetForegroundWindow );
		if ( FALSE == bRet || 0 == g_NtUserGetForegroundWindow_addr )
		{
			return FALSE ;
		}
	}

	if ( NULL == g_NtUserQueryWindow_addr )
	{
		bRet = Get_sdtfunc_addr( (PULONG)&g_NtUserQueryWindow_addr, Tag_IsHungAppWindow );
		if ( FALSE == bRet || 0 == g_NtUserQueryWindow_addr )
		{
			return FALSE ;
		}
	}

	if ( NULL == g_NtUserGetClassName_addr )
	{
		bRet = Get_sdtfunc_addr( (PULONG)&g_NtUserGetClassName_addr, Tag_GetClassNameW );
		if ( FALSE == bRet || 0 == g_NtUserGetClassName_addr )
		{
			return FALSE ;
		}
	}

	return TRUE ;
}



BOOL
MsgFilter (
	IN PVOID _pNode,
	IN int Flag,
	IN HWND hWnd,
	IN UINT msg
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/22 [22:8:2010 - 0:58]

Routine Description:
  NtUserMessageCall,NtUserMessageCallback,NtUserMessageNotify,NtUserPostMessage
  ��Ҫ����Ϣ���˺���MsgFilter()�Ĺ��˹�������
  1) 0x15F BYTE bFlag_SendPostMessage_allowed ; // �Ƿ�����NtUser**������Ϣ.������ֱ�ӷ���
  2) 0x15D BYTE bFlag_BlockFakeInput ;// �Ƿ��ֹ���ⰴ��,TRUEΪ��ֹ; �����ֹ,�����3
     ��������ļ�δ�Դ˴�������,Ҫ����һ����Msg,ʣ�����Ϣ������3�Ĺ��˹���
  3) ������Լ����Լ�����Ϣ ���߸�����һ��ɳ���еĳ�����Ϣ,����֮
  4) ���PID��ɳ��Ľ�������,ֱ����7�Ĺ��˹���
  5) ���PID����ɳ��Ľ�������,��Msg == 0x3E4,����֮
  6) ���PID����ɳ��Ľ�������,��Msg != 0x3E4,����IsWhiteProcess(),������TRUE,�����֮
  7) ����Is_Name_in_NodeWnd_normal()�жϽ�����Ϣ�Ľ��̴��������Ƿ�����ڰ�������. ��������ֱ�ӽ�ֹ��
     (ֻ����ɳ���еĳ�����Ϣ�����ָ���ĳ���,������������)
  8) ����Ϣ�Ƿ��͵�����������,Ҳͬ����������������е���Щ���巢��g_forbidden_Message_Array�����е���Ϣ
  9) ����ɳ���еĳ�����explorer.exe�е��ര��CicMarshalWndClass����0x400���ϵ���Ϣ
    
Arguments:
  _pNode - ��ǰ���̶�Ӧ�Ľ��
  Flag  - �����SendMessage ���� PostMessage
  hWnd - eg: NtUserMessageCall�Ĳ���
  msg - eg: NtUserMessageCall�Ĳ���

Return Value:
  TRUE - ��������Ϣ; FALSE - ��ֹ��ǰ��Ϣ�ķ���
    
--*/
{
	BOOL bIsWhite = FALSE ;
	ULONG ClassNameLength = 0, Index = 0 ;
	SIZE_T RegionSize = 0x1000 ;
	NTSTATUS status = STATUS_SUCCESS ; 
	HANDLE PID = NULL ;
	PERESOURCE QueueLockList = NULL ;
	LPName_Info lpNameInfo = NULL ;
	LPPDNODE pNode = (LPPDNODE) _pNode ;

	// 1. У������Ϸ���
	if ( NULL == pNode ){ return TRUE ; } // �������Ϸ�,Ĭ�Ϸ��е�
	if ( pNode->XWnd.bFlag_SendPostMessage_allowed ) { return TRUE ; } // ����NtUser**������Ϣ,����
	
	// 2. �������ֹ���ⰴ��,Ҫ����һ����Msg,ʣ�����Ϣ�����߹��˹���
	if ( 0 == pNode->XWnd.bFlag_BlockFakeInput )
	{
		if (   0xFF == msg 
			|| (msg >= WM_KEYFIRST   && msg <= WM_KEYLAST)
			|| (msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST)
			|| (msg >= 0x2A0 && msg <= 0x2A3)
			|| (msg >= WM_TABLET_FIRST && msg <= WM_TABLET_LAST)
			)
		{
			return TRUE ;
		}
	}

	// 3. �����Լ����Լ�����Ϣ ���߸�ɳ���е�����������Ϣ,����
	PID = g_NtUserQueryWindow_addr( hWnd, QUERY_WINDOW_UNIQUE_PROCESS_ID );

	if ( NULL == PID ) { return FALSE ; }
	if ( PsGetCurrentProcessId() == PID || IsApprovedPIDEx(_pNode, (ULONG)PID) ) { return TRUE ; }

	// 4. �����������: ��ɳ����Ľ��̷���Ϣ
	if ( NULL == kgetnodePD( PID ) )
	{
		if ( 0x3E4 == msg ) { return TRUE ; }
		if ( IsWhiteProcess( _pNode, PID ) ) { return TRUE ; }
	}

	// 5. �ڵ�ǰ����������һ���㹻�����ʱ�ڴ�
	if ( NULL == pNode->XWnd.TranshipmentStation )
	{
		status = ZwAllocateVirtualMemory (
			NtCurrentProcess() ,
			 (PVOID *)&pNode->XWnd.TranshipmentStation ,
			0,
			&RegionSize,
			MEM_COMMIT,
			PAGE_READWRITE
			);

		if ( ! NT_SUCCESS(status) )
		{
			dprintf( "error! | MsgFilter() - ZwAllocateVirtualMemory(); | status=0x8lx \n", status );
			pNode->bDiscard = 1 ;
			return FALSE ;
		}
	}

	if ( NULL == pNode->XWnd.VictimClassName )
	{
		pNode->XWnd.VictimClassName = kmalloc( 0x100 );
		if ( NULL == pNode->XWnd.VictimClassName )
		{
			dprintf( "error! | MsgFilter() - kmalloc(); | (Length=0x100) \n" );
			pNode->bDiscard = 1 ;
			return FALSE ;
		}
	}

	// 6. hWnd-->Name,���ַ���������pNode->XWnd.VictimClassName
	__try
	{
		lpNameInfo = (LPName_Info) pNode->XWnd.TranshipmentStation ;
		RtlZeroMemory( lpNameInfo, 0x1000 );

		lpNameInfo->NameInfo.Name.Length			= 0 ;
		lpNameInfo->NameInfo.Name.MaximumLength	= 0xFE ;
		lpNameInfo->NameInfo.Name.Buffer			= (LPWSTR)((int)lpNameInfo + 8) ;

		ProbeForRead( lpNameInfo->pBuffer, 0x100, 2 );

		// ��ѯ�õ�hwnd��Ӧ�Ĵ�������
		ClassNameLength = g_NtUserGetClassName_addr( hWnd, FALSE, (PUNICODE_STRING)lpNameInfo );
		if ( 0 == ClassNameLength || ClassNameLength >= 0x80 )
		{
			dprintf( "error! | MsgFilter() - g_NtUserGetClassName_addr(); | ClassNameLength=%d \n", ClassNameLength );
			return FALSE ;
		}

		// ���� & ���Ͻ�����
		RtlCopyMemory( pNode->XWnd.VictimClassName, lpNameInfo->pBuffer, ClassNameLength * sizeof(WCHAR) );
		*(WORD*)( (ULONG)pNode->XWnd.VictimClassName + ClassNameLength * sizeof(WCHAR) ) = UNICODE_NULL ;
		RtlZeroMemory( lpNameInfo, 0x1000 );

	}
	 __except(EXCEPTION_EXECUTE_HANDLER)
	{
		dprintf( "error! | MsgFilter() - __try __except (); | \n" );
	    return FALSE ;
	}

	//
	// �����Ǵ�����ɳ����Ĵ��ڷ���Ϣ
	//

	// 7. VictimClassName��������
	QueueLockList = pNode->XWnd.pResource ;
	bIsWhite = GLFindNodeEx( &pNode->XWnd.WndListHead, QueueLockList, pNode->XWnd.VictimClassName, NULL );

	// 7.1 ��ɳ����� "��" ���������ڷ���Ϣ,�ܾ���
	if ( FALSE == bIsWhite ) { return FALSE ; }
	
	if ( msg < 0x400 )
	{
		// 7.2 ��������ɳ����İ��������̷���������Ϣ
		for ( Index=0; Index<ARRAYSIZEOF(g_forbidden_Message_Array); Index++ )
		{
			// ��ǰ���̵���Ϣ�����Ƿ�,�ܾ���!
			if ( g_forbidden_Message_Array[ Index ] == msg ) { return FALSE ; }
		}

		return TRUE ;
	}
	else
	{
		// 7.3 ��ɳ����İ��������ڷ���0x400���ϵ���Ϣ,���ܾ���; ֻ����explorer.exe������ӵ�е�����Ϊ"CicMarshalWndClass"�Ĵ���
		BOOL bRet = TRUE, bPass = FALSE ;
		LPWSTR lpImageFileShortName = NULL ;
		PUNICODE_STRING lpImageFileName = NULL ;

		// �õ�PID��Ӧ�Ľ���·��
		bRet = GetProcessImageFileName( PID, &lpImageFileName, &lpImageFileShortName ); // �����߸����ͷ��ڴ�
		if ( FALSE == bRet ) { return TRUE ; } // δ�õ���������Ĭ�Ϸ���

		if (   (0 == _wcsicmp( lpImageFileShortName, L"explorer.exe" ))
			&& (0x12 == ClassNameLength)
			&& (0 == _wcsicmp( pNode->XWnd.VictimClassName, L"CicMarshalWndClass" ))
			)
		{
			bPass = TRUE ; // Allowed SendMessage to explorer's CicMarshalWndClass
		}

		kfree( (PVOID)lpImageFileName ); // �ͷ��ڴ�
		return bPass ;
	}

	return FALSE ;
}



NTSTATUS
RpcFilter (
	IN HANDLE PortHandle,
	IN PPORT_MESSAGE RequestMessage
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/26 [26:8:2010 - 18:48]

Routine Description:
  RPC���˺���,������� NtRequestProt / NtRequestWaitReplyPort / NtAlpcSendWaitReceivePort
    
--*/
{
	BOOL bHasNoName = FALSE ;
	NTSTATUS status = STATUS_SUCCESS ; 
	PVOID Object = NULL, NewObject = NULL ;
	POBJECT_NAME_INFORMATION ObjectNameInfo = NULL ;

	// 1. У������Ϸ���
	if ( NULL == PortHandle || NULL == RequestMessage )
	{
	//	dprintf( "error! | RpcFilter(); | Invalid Paramaters; failed! \n" );
		return STATUS_INVALID_PARAMETER ;
	}

	// 2. ��ö�����; Handle --> Object --> Object Name
	status = ObReferenceObjectByHandle( PortHandle, 0, NULL, KernelMode, &Object, NULL );
	if ( ! NT_SUCCESS(status) )
	{
	//	dprintf( "error! | RpcFilter() - ObReferenceObjectByHandle(); | status=0x8lx \n", status );
		return status ;
	}

	NewObject = GetFileObject( Object );
	if ( NULL == NewObject )
	{
	//	dprintf( "error! | RpcFilter() - GetFileObject(); | NULL == NewObject \n" );
		status = STATUS_ACCESS_DENIED ;
		goto _OVER_ ;
	}

	ObjectNameInfo = SepQueryNameString( NewObject, &bHasNoName ); // �ɵ������ͷ��ڴ�
	if ( NULL == ObjectNameInfo )
	{
		if ( bHasNoName ) {
			status = STATUS_SUCCESS ;
		} else {
		//	dprintf( "error! | RpcFilter() - SepQueryNameString(); | NULL == ObjectNameInfo \n" );
			status = STATUS_UNSUCCESSFUL ;
		}

		status = STATUS_SUCCESS ;
		goto _OVER_ ;
	}

	// 3. ���˶�����
	status = RpcFilterEx( RequestMessage, (PUNICODE_STRING)ObjectNameInfo );
	if ( STATUS_BAD_INITIAL_PC == status )
	{
		status = RpcFilterExp( RequestMessage, (PUNICODE_STRING)ObjectNameInfo );
		if ( STATUS_BAD_INITIAL_PC == status )
			status = STATUS_SUCCESS ;
	}

	// 4. Clear Up
	kfree( (PVOID)ObjectNameInfo );
_OVER_ :
	if ( Object ) { ObfDereferenceObject(Object); }
	return status ;
}



NTSTATUS
RpcFilterEx (
	IN PPORT_MESSAGE RequestMessage,
	IN PUNICODE_STRING ObjectNameInfo
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/31 [31:8:2010 - 15:17]

Routine Description:
  ��һ�����ҪĿ������������� L"\\Sessions\\*\\Windows\\ApiPort" ��������RPC��Ϣ:
  UserpExitWindowsEx / UserpEndTask
    
--*/
{
	ULONG ApiNumber = 0 ;
	PUSER_API_MSG Msg = (PUSER_API_MSG) RequestMessage ;

	// 1. У������Ϸ���
	if ( NULL == RequestMessage || NULL == ObjectNameInfo ) { return STATUS_UNSUCCESSFUL ; }

	// 2. ���˶�����
	if (   ObjectNameInfo->Length < 0x20
		|| _wcsicmp( &ObjectNameInfo->Buffer[ ObjectNameInfo->Length / sizeof(WCHAR) - 0x10 ], L"\\Windows\\ApiPort" )
		|| ( ObjectNameInfo->Length >= 0x20 && _wcsnicmp( ObjectNameInfo->Buffer, L"\\Sessions\\", 0xA ) )
		)
	{
		return STATUS_BAD_INITIAL_PC ;
	}

	// 3. ������,�������ĳ��ȿ϶�>=20 �����ݷ���L"\\Sessions\\*\\Windows\\ApiPort"
	__try
	{
		ProbeForRead( RequestMessage, sizeof(PORT_MESSAGE), sizeof(ULONG) );
		if ( RequestMessage->u1.s1.TotalLength < 0x20 ) { return STATUS_SUCCESS ; } // ��Ϣ���С����0x20,ֱ�ӷ���

		ProbeForRead( Msg, 0x28, sizeof(ULONG) );
		ApiNumber = Msg->ApiNumber ;
		
		// ��ֹ�ػ���Ϣ
		if (   ApiNumber == CSR_MAKE_API_NUMBER( USERSRV_SERVERDLL_INDEX,UserpExitWindowsEx)
			|| ApiNumber == CSR_MAKE_API_NUMBER( USERSRV_SERVERDLL_INDEX,UserpEndTask)
			)
		{
			return STATUS_ACCESS_DENIED ;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	    return STATUS_UNSUCCESSFUL ;
	}

	return STATUS_SUCCESS ;
}



NTSTATUS
RpcFilterExp (
	IN PPORT_MESSAGE RequestMessage,
	IN PUNICODE_STRING ObjectNameInfo
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/31 [31:8:2010 - 15:17]

Routine Description:
  ��һ�����⴦����RPC��Ϣ
    
--*/
{
	ULONG ApiNumber = 0 ;
	PUSER_API_MSG Msg = (PUSER_API_MSG) RequestMessage ;

	// 1. У������Ϸ���
	if ( NULL == RequestMessage || NULL == ObjectNameInfo ) { return STATUS_UNSUCCESSFUL ; }

	// 2. ֻ��ע������Ϊ "\\LsaAuthenticationPort" ���� "\\RPC Control\\lsasspirpc" �����
	if ( 0x2C == ObjectNameInfo->Length )
	{
		if ( _wcsicmp(ObjectNameInfo->Buffer, L"\\LsaAuthenticationPort") ) { return STATUS_BAD_INITIAL_PC ; }		
	}
	else if ( 0x2E == ObjectNameInfo->Length )
	{
		if ( _wcsicmp(ObjectNameInfo->Buffer, L"\\RPC Control\\lsasspirpc") ) { return STATUS_BAD_INITIAL_PC ; }
	}
	else 
	{
		return STATUS_BAD_INITIAL_PC ;
	}

	// 3. ������,��������Ϊ����2�����
	__try
	{
		ProbeForRead( RequestMessage, sizeof(PORT_MESSAGE), sizeof(ULONG) );
		
		if ( g_Version_Info.IS_before_vista )
		{
			// 3.1 Vista��ǰ��ϵͳ
			if ( RequestMessage->u1.s1.TotalLength < 0x30 ) { return STATUS_SUCCESS ; } // ��Ϣ���С����0x20,ֱ�ӷ���

			ProbeForRead( Msg, 0x30, sizeof(ULONG) );
			
			if (   2 == (ULONG) Msg->CaptureBuffer 
				&& g_LsaAuthPkg == (ULONG) Msg->ReturnValue
				&& *(DWORD *)((ULONG)Msg + 0x28) >= 4
			   )
			{
				ULONG Reserved = Msg->Reserved ;
				ProbeForRead( (PVOID)Reserved, 4, sizeof(ULONG) );

				if ( 5 == *(PULONG) Reserved )
				{
					return STATUS_ACCESS_DENIED ;
				}
			}
		}
		else
		{
			// 3.2 Vista / Win7�µĴ���
			PVOID Buffer = (PVOID) ((ULONG_PTR)Msg + sizeof(PORT_MESSAGE)) ;
			ULONG DataLength = Msg->h.u1.s1.DataLength ;

			ProbeForRead( Buffer, DataLength, sizeof(SHORT) );

			if ( DataLength >= 0x12 && wcsstr( (WCHAR*)Buffer, L"Negotiate" ) )
			{
				return STATUS_ACCESS_DENIED ;
			}
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return STATUS_UNSUCCESSFUL ;
	}

	return STATUS_SUCCESS ;
}



BOOL
IsWhiteProcess (
	IN PVOID _ProcessNode,
	IN HANDLE PID
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/23 [23:8:2010 - 14:45]

Routine Description:
  ͨ��@PID�õ�������Ϣ�Ľ�����(�ܺ���),��������@pNode�õ�WndListHead,�����ð���������. 
  ������Ϊ"*:"�Ľڵ㵥Ԫ,ȡ����ֵ���������,���ܺ��߽���������ƥ��. ����ͬ,����������
  �����Ķ������ǲ��ù�,������ܺ��߷�ʲô��Ϣ�ͷ�ʲô��Ϣ!Ҳ����˵,���Ǹ�����: 
  "ɳ���еĽ��̿�����ɳ������ض����̷����κ���Ϣ"
  eg: �����ļ���ָ��OpenWinClass=*:nmSvc.exe; ��ɳ���е��κν��̿���ɳ�����nmSvc.exe���κ���Ϣ

Arguments:
  _ProcessNode - ��ǰ���̶�Ӧ�Ľ��(��������Ϣ��������)
  PID - ������Ϣ�Ľ���ID(�����Ϊ�ܺ���); ͨ��2��;����ȡ,һ����NtUserPostThreadMessage�Ĳ���; һ���ɵ���NtUserQueryWindow��hWndת������
    
--*/
{
	BOOL bRet = FALSE ;
	LPWSTR wszName = NULL, lpImageFileShortName = NULL ;
	PUNICODE_STRING lpImageFileName = NULL ;
	PLIST_ENTRY ListHead = NULL, Next = NULL ;	
	PERESOURCE QueueLockList = NULL ;
	LPGRAYLIST_INFO pNode = NULL ;
	LPPDNODE ProcessNode = (LPPDNODE) _ProcessNode ;

	// 1. У������Ϸ���
	if ( NULL == ProcessNode || NULL == PID ) { return FALSE ; }
	
	QueueLockList	= ProcessNode->XWnd.pResource ;
	ListHead		= &ProcessNode->XWnd.WndListHead ;
	if ( NULL == QueueLockList || NULL == ListHead ) { return FALSE ; }

	// 2. �õ�PID��Ӧ�Ľ���·��
	bRet = GetProcessImageFileName( PID, &lpImageFileName, &lpImageFileShortName ); // �����߸����ͷ��ڴ�
	if ( FALSE == bRet )
	{
		dprintf( "error! | IsWhiteProcess() - GetProcessImageFileName(); | (PID=0x%08lx)\n", PID );
		return FALSE ;
	}
	
	// 3. ��������
	bRet = FALSE ;
	EnterCrit( QueueLockList );	// ��������
	Next = ListHead->Flink;

	while ( Next != ListHead )
	{
		pNode = (LPGRAYLIST_INFO)(CONTAINING_RECORD( Next, GRAYLIST_INFO, ListEntry ));

		wszName = pNode->wszName ; 
		if (   ( pNode->NameLength > 3 )
			&& ( L'*' == *wszName )
			&& ( L':' == wszName[1] )
			)
		{
			if ( 0 == _wcsicmp( lpImageFileShortName, (LPWSTR)((int)wszName+4) ) )
			{
				bRet = TRUE ;
				break ;
			}
		}

		Next = Next->Flink;
	}

	LeaveCrit( QueueLockList );	// �ͷ���
	kfree( (PVOID)lpImageFileName ); // �ͷ��ڴ�
	return bRet ;
}



VOID 
ShowWarningInfo (
	IN int InfoType,
	IN HANDLE VictimHandle
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/08/21 [21:8:2010 - 15:54]

Routine Description:
  ��ʾ��fake shadow ssdt �������ص��ĸ��ִ���ֹ����Ϣ    
    
Arguments:
  InfoType - ��ӡ��Ϣ������
  VictimHandle - ��@InfoType == _SendInput_Type_,�ò�����ֵ,Ϊ������PID
    
--*/
{
	NTSTATUS status = STATUS_SUCCESS ; 
	PEPROCESS VictimProcess = NULL;
	ULONG InvaderProcess = (ULONG)PsGetCurrentProcess() ;

	switch ( InfoType )
	{
	case _SendInput_Type_ :
		dprintf( "[Warining] ��⵽ģ�ⰴ�� (SendInput),�ѳɹ���ֹ \n" );
		break ;

	case _BlockInput_Type_ :
		dprintf( "[Warining] ��⵽ģ�ⰴ�� (BlockInput),�ѳɹ���ֹ \n" );
		break;

	case _DestroyWindow_Type_ :
		dprintf( "[Warining] ��⵽���ٴ��� (DestroyWindow),�ѳɹ���ֹ \n" );
		break;

	case _SetSysColors_Type_ :
		dprintf( "[Warining] ��⵽���ô�����ɫ (SetSysColors),�ѳɹ���ֹ \n" );
		break;

	case _SystemParametersInfoW_Type_ :
		dprintf( "[Warining] ��⵽��������������� (SystemParametersInfoW),�ѳɹ���ֹ \n" );
		break;

	case _SetWinEventHook_Type_ :
		dprintf( "[Warining] ��⵽����ȫ�ֹ��� (SetWinEventHook),�ѳɹ���ֹ \n" );
		break;

	case _SetWindowsHookEx_Type_ :
		dprintf( "[Warining] ��⵽����ȫ�ֹ��� (SetWindowsHookEx),�ѳɹ���ֹ \n" );
		break;

	case _NtRequestPort_Type_ :
	case _NtRequestWaitReplyPort_Type_ :
	case _NtAlpcSendWaitReceivePort_Type_ :
		dprintf( "[Warining] ��⵽���Ϸ���RPCͨ�� (Nt(Alpc)Request*),�ѳɹ���ֹ \n" );
		break;
	
	default :
		return ;
	}	

	if ( VictimHandle )
	{
		// ȡ���ܺ����̶���
		status = PsLookupProcessByProcessId( (HANDLE)VictimHandle, &VictimProcess);
		if (!NT_SUCCESS( status )) { return ; }

		ObDereferenceObject( (PVOID)VictimProcess ) ;

		dprintf( 
			"��Դ����(������): %s \n"
			"Ŀ�����(�ܺ���): %s \n\n",
			(PCHAR)(InvaderProcess + g_ImageFileName_Offset),
			(PCHAR)((ULONG)VictimProcess + g_ImageFileName_Offset)
			);
	}
	else
	{
		dprintf( 
			"��Դ����(������): %s \n",
			(PCHAR)(InvaderProcess + g_ImageFileName_Offset)
			);
	}

	return ;
}


///////////////////////////////   END OF FILE   ///////////////////////////////