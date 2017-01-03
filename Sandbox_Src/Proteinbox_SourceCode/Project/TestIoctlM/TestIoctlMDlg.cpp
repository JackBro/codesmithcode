// TestIoctlMDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TestIoctlM.h"
#include "TestIoctlMDlg.h"
#include "Work.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////


BOOL g_Inited = FALSE ;
CString  g_szFileName = "D:\\1.exe" ;

TCHAR m_szRootPath[MAX_PATH];


#define LOADERNAME				_T("PBStart.exe")
#define SUBKEY_PROTEINBOX		_T("SOFTWARE\\Proteinbox\\Config")
#define SUBVALUE_ROOTFOLDER		_T("RootFolder")


/////////////////////////////////////////////////////////////////////////////
// CTestIoctlMDlg dialog

CTestIoctlMDlg::CTestIoctlMDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTestIoctlMDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTestIoctlMDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTestIoctlMDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTestIoctlMDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTestIoctlMDlg, CDialog)
	//{{AFX_MSG_MAP(CTestIoctlMDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDTEST, OnLoad)
	//}}AFX_MSG_MAP
	ON_EN_CHANGE(IDC_EDIT_PATH, &CTestIoctlMDlg::OnEnChangeEditPath)
	ON_BN_CLICKED(IDC_BUTTON_RUN, &CTestIoctlMDlg::OnRunPE)
	ON_BN_CLICKED(IDC_BUTTON1, &CTestIoctlMDlg::OnBrowser)
	ON_BN_CLICKED(IDUNLOAD, &CTestIoctlMDlg::OnUnload)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestIoctlMDlg message handlers

BOOL CTestIoctlMDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	m_szProcName = (LPSTR) (LPCTSTR)g_szFileName ;
	GetDlgItem(IDC_EDIT_PATH)->SetWindowText( g_szFileName ); // ��EDITSELECT�ؼ�����ʾ�ļ�·��
	
	GetDlgItem(IDC_BUTTON_RUN)->EnableWindow( FALSE );
	GetDlgItem(IDUNLOAD)->EnableWindow( FALSE );
	GetDlgItem(IDTEST)->EnableWindow( TRUE );

	GetModuleFileName( NULL, m_szRootPath, ARRSIZEOF(m_szRootPath) );
	PathRemoveFileSpec( m_szRootPath );

	// ���������ĸ�Ŀ¼����ע���
	SHSetValue( HKEY_LOCAL_MACHINE, SUBKEY_PROTEINBOX, SUBVALUE_ROOTFOLDER, REG_SZ, (LPCVOID)m_szRootPath, strlen(m_szRootPath) );
	{
		CHAR showinfo[MAX_PATH] = "";
		sprintf( showinfo, "��ǰɳ���Ŀ¼: %s", m_szRootPath );
		ShowLogInfo( showinfo );
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTestIoctlMDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTestIoctlMDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CTestIoctlMDlg::OnCancel() 
{
	OnUnload();
	CDialog::OnCancel();
}



void CTestIoctlMDlg::OnEnChangeEditPath()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ�������������
	// ���͸�֪ͨ��������д CDialog::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	GetDlgItem(IDC_EDIT_PATH)->GetWindowText( g_szFileName );
	m_szProcName = (LPSTR) (LPCTSTR)g_szFileName ;

	return ;
}

void CTestIoctlMDlg::OnRunPE()
{
	char szLoaderPath[ MAX_PATH + 0x20 ] = "";
	PathCombine(szLoaderPath, m_szRootPath, LOADERNAME);

	RunPE( szLoaderPath, m_szProcName );
	return ;
}

void CTestIoctlMDlg::OnBrowser()
{
	CFileDialog sourceFile(TRUE);

	// ��ʾѡ���ļ��Ի���
	if(sourceFile.DoModal() == IDOK)
	{
		g_szFileName = sourceFile.GetPathName() ;
		m_szProcName = (LPSTR) (LPCTSTR)g_szFileName ;

		GetDlgItem(IDC_EDIT_PATH)->SetWindowText(sourceFile.GetPathName()); // ��EDITSELECT�ؼ�����ʾ�ļ�·��
	}

	return ;
}


DWORD WINAPI CTestIoctlMDlg::OnLoadThread(LPVOID lpParameter)
{
	CTestIoctlMDlg* pThis = (CTestIoctlMDlg*)lpParameter;
	if( NULL != pThis )
	{
		pThis->_ThreadProc();
	}

	return 0;
}


BOOL CTestIoctlMDlg::IsDrvLoaded()
{
	HANDLE hFile = CreateFileW (
		g_PBLinkName , 
		FILE_READ_ATTRIBUTES | FILE_READ_DATA | SYNCHRONIZE , 
		FILE_SHARE_READ | FILE_SHARE_WRITE , 
		NULL , 
		OPEN_EXISTING ,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
		0
		);

	if ( INVALID_HANDLE_VALUE != hFile ) 
	{ 
		CloseHandle(hFile);
		return TRUE ; 
	}
	
	return FALSE ; 
}


void CTestIoctlMDlg::_ThreadProc()
{

	BOOL bRet = FALSE ;

	if ( g_Inited ) { return; }

	// ���������ļ����
	ShowLogInfo( "��ʼ�������ļ� \n" );
	bRet = HandlerConf();
	if ( FALSE == bRet )
	{
		ShowLogInfo( "error! | HandlerConf() | ���������ļ�ʧ�� \n" );
		return;
	}

	// ����ProteinBox.sys
	ShowLogInfo( "����ProteinBox.sys \n" );
	CHAR buffer[MAX_PATH];

	GetCurrentDirectory( MAX_PATH, buffer) ;
	sprintf( buffer + strlen(buffer), "\\ProteinBoxDrv.sys" );

	bRet = LoadDriver( buffer, ProteinBoxDrv_LinkName, &g_drv_ProteinBoxDrv );
	if ( FALSE == bRet )
	{
		ShowLogInfo( "error! | LoadDriver() | ����ProteinBox.sysʧ�� \n" );
		goto _error_ ;
	}

	// ����R0�ĵȴ��߳�,R3���������ļ����ݴ�������
	bRet = g_Conf->GetDrvPointer( g_drv_ProteinBoxDrv );
	if ( FALSE == bRet )
	{
		ShowLogInfo( "error! g_Conf->GetDrvPointer() \n" );
		goto _error_ ;
	}

	bRet = g_Conf->Wakeup_R0_InitConfigData() ;
	if ( FALSE == bRet )
	{
		ShowLogInfo( "error! g_Conf->Wakeup_R0_InitConfigData() \n" );
		goto _error_ ;
	}

	//
	// ����Ҫ���޵ȴ�R0����Ϊ��������һ���Ӻ�ἤ�����ǣ��������޵ȴ�������һ���Ӻ���н��
	// ����ȴ�ʱ����̣�������⡣
	//
	ShowLogInfo( "���޵ȴ�R0... \n" );
	bRet = g_Conf->Waitfor_R0_InitConfigData() ;
	if ( FALSE == bRet )
	{
		ShowLogInfo( "error! g_Conf->Waitfor_R0_InitConfigData() \n" );
		goto _error_ ;
	}

	ShowLogInfo( "�ȴ��ɹ���go on \n" );
	// ����������ʼHook
	Ioctl_HookObject( g_drv_ProteinBoxDrv, TRUE ) ;
	Ioctl_HookShadow( g_drv_ProteinBoxDrv, TRUE ) ;

	// ����ɳ��ĸ��
	//	ShowLogInfo("1. Call Ioctl_StartProcess(); ����ɳ��ĸ�� \n");
	//	Ioctl_StartProcess( g_drv_ProteinBoxDrv ) ;

	GetDlgItem(IDC_BUTTON_RUN)->EnableWindow( TRUE );
	GetDlgItem(IDUNLOAD)->EnableWindow( TRUE );

	ShowLogInfo( "��ʼ����� \n" );
	g_Inited = TRUE ;
	if ( g_Conf )
	{
		delete g_Conf ;
		g_Conf = NULL;
	}
	return;

_error_ :
	if ( g_Conf )
	{
		delete g_Conf ;
		g_Conf = NULL;
	}
	g_Inited = FALSE ;
	return;
}


void CTestIoctlMDlg::OnLoad() 
{
	if ( IsDrvLoaded() )
	{
		ShowLogInfo("����δ��ж��,������ϵͳ \n") ;
		return ; 
	}

	GetDlgItem(IDTEST)->EnableWindow( FALSE );

	HANDLE hThread = CreateThread( NULL, 0, OnLoadThread, this, 0, NULL );	
	if ( hThread )
		CloseHandle(hThread);
	
	return;
}


void CTestIoctlMDlg::OnUnload()
{
	if ( FALSE == g_Inited )
	{ 
		ShowLogInfo("����δ����,����ж�� \n") ;
		return ; 
	}

	if ( g_drv_ProteinBoxDrv )
	{
		Ioctl_HookShadow( g_drv_ProteinBoxDrv, FALSE ) ; // ж��Shadow ssdt��Inline Hook
		UnloadDriver( g_drv_ProteinBoxDrv );
		g_drv_ProteinBoxDrv = NULL;
		ShowLogInfo("������ж�� \n") ;
	}

	GetDlgItem(IDTEST)->EnableWindow( TRUE );
	GetDlgItem(IDUNLOAD)->EnableWindow( FALSE );
	GetDlgItem(IDC_BUTTON_RUN)->EnableWindow( FALSE );

	g_Inited = FALSE ;
	return ;
}


void CTestIoctlMDlg::ShowLogInfo( IN LPSTR szInfo )
{
	CString Temp;
	GetDlgItem(IDC_EDIT_SHOWINFO)->GetWindowText(Temp);

	CString LogInfo = szInfo ;
	Temp.Append( "\r\n" );
	Temp += LogInfo;
	
	GetDlgItem(IDC_EDIT_SHOWINFO)->SetWindowText( Temp );
	return ;
}


