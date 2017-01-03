/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2009/12/08 [8:12:2009 - 18:23]
* MODULE : D:\Program\R0\Coding\ɳ��\SandBox\Code\HandlerFile\main1.cpp
* 
* Description:
*   R3�����ļ�,��������ɳ�������Object Hook���˺��� IopParseFile_IopParseDevice_Filter()     
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include <iostream.h>
#include "Hfile.h"

//////////////////////////////////////////////////////////////////////////

char g_FilePath[MAX_PATH]		= "C:\\Test\\555.txt" ;
char g_FilePathNew[MAX_PATH]	= "C:\\Test\\imadus.txt" ;


//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////

// int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
int main()
{
	BOOL bOK = TRUE ;
	int Choice = 0 ;

// 	printf( 
// 		"��ָ�ɳ��Ա�����IAT:ntdll!NtOpenFile/NtQueryInformationFile/NtSetInformationFile ��InlineHook\n"
// 		"�ٳ��Բ����ļ� ... \n" 
// 		) ;
// 	getchar() ;

	while ( bOK )
	{
		printf (
			"��ѡ�����: \n"
			"0. ESC \n"
			"1. д�ļ�,�������򴴽�						\n"
			"2. �������ļ�								\n"
			"3. ���ļ�\n"
			"4. ɾ�ļ�\n"
			"-------------------------------------------\n"
			);

		cin >> Choice;
		switch (Choice)
		{
		case 0:
			bOK = FALSE ;
			break;

		case 1:
			TestWriteFile ();
			break;

		case 2:
			TestRenameFile();
			break;

		case 3:
			TestReadFile();
			break;

		case 4:
			TestDeleteFile();
			break;

		default:
			break;
		}
	}

//	getchar() ;
	return 0;
}


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                                      +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --  
//                                                                           --    --
//		    																	--

VOID
TestWriteFile(
	) 
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ;
	HANDLE hFile = NULL ;

	// �����ļ�
	status = HFCreateFileEx( g_FilePath, OPEN_ALWAYS );
	if ( !NT_SUCCESS(status) )
	{
		printf( "HFCreateFile(), Error, 0x%08lx \n", status ); 
		return ;
	} else {
		printf( "HFCreateFileEx OK! \"%s\" \n", g_FilePath ) ;
	}
	
	// д�ļ�
	status = HFWriteFile( g_FilePath, "Hello" );
	if ( !NT_SUCCESS(status) )
	{
		printf( "HFWriteFile(), Error, 0x%08lx \n", status ); 
		return ;
	} else {
		printf( "HFWriteFile OK! \"%s\" \n", g_FilePath ) ;
	}

	return ;
}



VOID
TestRenameFile(
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ;
	HANDLE hFile = NULL ;

	// �������ļ�
	status = HFRenameFile( g_FilePath, g_FilePathNew );
	if ( !NT_SUCCESS(status) )
	{
		printf( "HFRenameFile(), Error, 0x%08lx \n", status ); 
	} else {
		printf( "HFRenameFile OK! \"%s\" \n", g_FilePath ) ;
	}

	return ;
}


VOID
TestReadFile(
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ;
	HANDLE hFile = NULL ;
	char Context[MAX_PATH] = "" ;

	// ���ļ�
	status = HFReadFile( g_FilePath, Context );
	if ( !NT_SUCCESS(status) )
	{
		printf( "HFReadFile(), Error, 0x%08lx \n", status ); 
	} else {
		printf( "Context:\n%s \n\n", Context );
	}

	return ;
}


VOID
TestDeleteFile(
	)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL ;

	// ɾ�ļ�
	status = HFDeleteFile( g_FilePath );
	if ( !NT_SUCCESS(status) )
	{
		printf( "HFDeleteFile(), Error, 0x%08lx \n", status ); 
	} else {
		printf( "HFDeleteFile OK! \"%s\" \n", g_FilePath ) ;
	}
		
	return ;
}

//////////////////////////////////////////////////////////////////////////

