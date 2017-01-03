// Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


//////////////////////////////////////////////////////////////////////////

void SampleCreateDoc()
{
	::CoInitialize(NULL);	// COM ��ʼ��
				// �����MFC���򣬿���ʹ��AfxOleInit()���
	
	HRESULT hr;		// ����ִ�з���ֵ
	IStorage *pStg = NULL;	// ���洢�ӿ�ָ��
	IStorage *pSub = NULL;	// �Ӵ洢�ӿ�ָ��
	IStream *pStm = NULL;	// ���ӿ�ָ��
	
	hr = ::StgCreateDocfile(	// ���������ļ�
		L"c:\\360.stg",	// �ļ�����
		STGM_CREATE | STGM_WRITE | STGM_SHARE_EXCLUSIVE,	// �򿪷�ʽ
		0,		// ��������
		&pStg);		// ȡ�ø��洢�ӿ�ָ��

	if ( !SUCCEEDED ( hr ) ) 
	{
		printf( "StgCreateDocfile() failed \n" );
		goto _END_ ;
	}
	
	hr = pStg->CreateStorage(	// �����Ӵ洢
		L"SubStg",	// �Ӵ洢����
		STGM_CREATE | STGM_WRITE | STGM_SHARE_EXCLUSIVE,
		0,0,
		&pSub);		// ȡ���Ӵ洢�ӿ�ָ��
	if ( !SUCCEEDED ( hr ) ) 
	{ 
		printf( "pStg->CreateStorage() failed \n" );
		goto _END_ ; 
	}
	
	hr = pSub->CreateStream(	// ������
		L"Stm",		// ������
		STGM_CREATE | STGM_WRITE | STGM_SHARE_EXCLUSIVE,
		0,0,
		&pStm);		// ȡ�����ӿ�ָ��
	if ( !SUCCEEDED ( hr ) )
	{ 
		printf( "pSub->CreateStream() failed \n" );
		goto _END_ ; 
	}

	hr = pStm->Write(		// ������д������
		"Hello",		// ���ݵ�ַ
		5,		// �ֽڳ���(ע�⣬û��д���ַ�����β��\0)
		NULL);		// ����Ҫ�õ�ʵ��д����ֽڳ���
	if ( !SUCCEEDED ( hr ) ) 
	{ 
		printf( "pStm->Write() failed \n" );
		goto _END_ ; 
	}

	printf( "ok!!! \n\n" );
	
_END_ :
	if( pStm )	pStm->Release();// �ͷ���ָ��
	if( pSub )	pSub->Release();// �ͷ��Ӵ洢ָ��
	if( pStg )	pStg->Release();// �ͷŸ��洢ָ��

	::CoUninitialize() ;		// COM �ͷ�
		// ���ʹ�� AfxOleInit(),�򲻵��øú���
}



void GetWallpaper()
{
	WCHAR   wszWallpaper [MAX_PATH];
	HRESULT hr;
	IActiveDesktop* pIAD;
	
	// 1. ��ʼ��COM�⣨��Windows����DLLs����ͨ�����ڳ����InitInstance()�е���
	// CoInitialize ( NULL )�������������롣MFC����ʹ��AfxOleInit()��
	
	CoInitialize ( NULL );
	
	// 2. ʹ������ṩ�Ļ������������ഴ��COM����
	// ���ĸ�����֪ͨCOM��Ҫʲô�ӿ�(������IActiveDesktop).
	
	hr = CoCreateInstance (
		CLSID_ActiveDesktop,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IActiveDesktop,
		(void**) &pIAD
		);
	
	if ( SUCCEEDED(hr) )
	{
		// 3. ���COM���󱻴����ɹ����������������GetWallpaper() ������
		hr = pIAD->GetWallpaper ( wszWallpaper, MAX_PATH, 0 );
		
		if ( SUCCEEDED(hr) )
		{
            // 4. ��� GetWallpaper() �ɹ�������������ص��ļ����֡�
            // ע������ʹ��wcout ����ʾUnicode ��wszWallpaper.  wcout ��
            // Unicode ר�ã�������cout.��ͬ��

			printf( "Wallpaper path is:\n %ws \n\n", wszWallpaper );
		//	wcout << L"Wallpaper path is:\n    " << wszWallpaper << endl << endl;
        }
        else
        {
            cout << "GetWallpaper() failed." << endl;
        }
		
        // 5. �ͷŽӿڡ�
        pIAD->Release();
    }
    else
    {
        cout << "CoCreateInstance() failed." << endl;
    }
	
    // 6. �ջ�COM�⡣MFC ��������һ�������Զ���ɡ�
	::CoUninitialize() ;		// COM �ͷ�
	
	return ;
}



void CreateWallpaperLnk ()
{
	LPCSTR sWallpaper = "C:\\WINDOWS\\Web\\Wallpaper\\Bliss.bmp" ;
	HRESULT hr ;
	IShellLink*   pISL;
	IPersistFile* pIPF;
	
    // 1. ��ʼ��COM��(��Windows ����DLLs). ͨ����InitInstance()�е���
    // CoInitialize ( NULL )�������������롣MFC ����ʹ��AfxOleInit() ��
	
    CoInitialize ( NULL );
	
    // 2. ʹ������ṩ��Shell Link��������ഴ��COM����.
    // ���ĸ�����֪ͨCOM ��Ҫʲô�ӿ�(������IShellLink)��
	
    hr = CoCreateInstance ( 
		CLSID_ShellLink,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IShellLink,
		(void**) &pISL 
		);
	
    if ( SUCCEEDED(hr) )
    {
        // 3. ���ÿ�ݷ�ʽĿ��(ǽֽ�ļ�)��·����
        hr = pISL->SetPath ( sWallpaper );
		
        if ( SUCCEEDED(hr) )
        {
            // 4. ��ȡ�������ĵڶ����ӿ�(IPersistFile)��
            hr = pISL->QueryInterface ( IID_IPersistFile, (void**) &pIPF );
			
            if ( SUCCEEDED(hr) )
            {
                // 5. ����Save() ��������ĳ���ļ��ÿ�ݷ�ʽ����һ��������
                // Unicode ����
                hr = pIPF->Save ( L"C:\\wallpaper.lnk", FALSE );
				if ( SUCCEEDED(hr) )
				{
					printf( "ok!!! \n\n" );
				}
				
                // 6a. �ͷ�IPersistFile �ӿڡ�
                pIPF->Release();
            }
        }
		
        // 6. �ͷ�IShellLink �ӿڡ�
        pISL->Release();
    }
	
    // ���������Ϣ��������ʡ�ԡ�
	
    // 7. �ջ�COM �⡣MFC ��������һ�������Զ���ɡ�
    CoUninitialize();


	return ;
}


void CLSID2ProgID()
{
	::CoInitialize( NULL );
	
	HRESULT hr;
	CLSID clsid = {0x06BE7323,0xEF34,0x11d1,{0xAC,0xD8,0,0xC0,0x4F,0xA3,0x10,0x9}}; 
	LPOLESTR lpwProgID = NULL;
	IMalloc * pMalloc = NULL;

	hr = ::ProgIDFromCLSID( clsid, &lpwProgID );
	if ( ! SUCCEEDED(hr) )
	{
		printf( "::ProgIDFromCLSID() failed: 0x%08lx \n\n", hr );
		goto _end_ ;
	}
	
	printf( "lpwProgID: %ws \n\n", lpwProgID );
	
	hr = ::CoGetMalloc( 1, &pMalloc );  // ȡ�� IMalloc
	if ( SUCCEEDED(hr) )
	{
		pMalloc->Free( lpwProgID );  // �ͷ�ProgID�ڴ�
		pMalloc->Release();          // �ͷ�IMalloc
	}

_end_ :
	::CoUninitialize();
}



//////////////////////////////////////////////////////////////////////////


int main()
{
	BOOL bOK = TRUE ;
	int Choice = 0 ;

	system( "title=Test for xx:COM" );

	while ( bOK )
	{
		printf (
			"��ѡ�����: \n"
			"0. ESC \n"
			"1. ����COM�ӿ�д�ļ���c:\\360.stg			\n"
			"2. �õ�����ǽֽ��·��						\n"
			"3. ��������ǽֽ��ݷ�ʽ��c:\\wallpaper.lnk \n"
			"4. clsid to progid							\n"
			"-------------------------------------------\n"
			);
		
		cin >> Choice;
		switch (Choice)
		{
		case 0:
			bOK = FALSE ;
			break;
			
		case 1 :
			SampleCreateDoc() ;
			break;

		case 2 :
			GetWallpaper() ;
			break;
		
		case 3 :
			CreateWallpaperLnk() ;
			break;

		case 4 :
			CLSID2ProgID() ;
			break;
			
		default:
			break;
		}
	}
	
	//	getchar() ;
	return 0;
}
