/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/09/08 [8:9:2010 - 22:13]
* MODULE : d:\Work\Program\Coding\ɳ��\SandBox\Code\Project\ProteinBoxDrv\WhiteOrBlack.c
* 
* Description:
*      
*   �ж���ǰ�ַ����ĺڰ�                      
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "struct.h"
#include "ProcessData.h"
#include "DispatchIoctl.h"
#include "GrayList.h"
#include "WhiteOrBlack.h"

//////////////////////////////////////////////////////////////////////////



NTSTATUS
Ioctl_WhiteOrBlack (
	IN PVOID pNode, 
	IN PVOID pInBuffer
	)
{
	LPIOCTL_WHITEORBLACK_BUFFER Buffer = NULL ;
	LPPDNODE ProcessNode = (LPPDNODE) pNode ;
	LPWSTR lpCompareName = NULL ;
	PERESOURCE	QueueLockList = NULL ;
	PLIST_ENTRY ListHeadWhite = NULL, ListHeadBlack = NULL ;
	BOOL bIsBlack = FALSE, bIsWhite = FALSE ;

//	dprintf( "[ProteinBoxDrv] IOCTL_PROTEINBOX - Ioctl_WhiteOrBlack(); \n" );

	if ( NULL == ProcessNode || NULL == pInBuffer ) { return STATUS_NOT_IMPLEMENTED ; }

	__try
	{
		Buffer = (LPIOCTL_WHITEORBLACK_BUFFER) getIoctlBufferBody( pInBuffer );
		if ( FALSE == MmIsAddressValid( (PVOID)Buffer ) )
		{
			dprintf( "error! | Ioctl_WhiteOrBlack() - getIoctlBufferBody(); | Body��ַ���Ϸ� \n" );
			__leave ;
		}

		ProbeForWrite( Buffer->bIsWhite, 4, sizeof(DWORD) );
		ProbeForWrite( Buffer->bIsBlack, 4, sizeof(DWORD) );

		// ������ݰ��ĺϷ���
		if ( NULL == Buffer->szPath || Buffer->PathLength < 2 || Buffer->PathLength >= 0x800 || FALSE == IsWBFlagOK( Buffer->Flag ) )
		{
			dprintf( "error! | Ioctl_WhiteOrBlack() - IsWBFlagOK(); | ���ݰ����Ϸ� \n" );
			__leave ;
		}

		ProbeForRead( Buffer->szPath, Buffer->PathLength, 2 );

		// ����R3�ַ�����R0�ڴ��,���ڱȽ�
		lpCompareName = (LPWSTR) kmalloc( Buffer->PathLength + 10 );
		if ( NULL == lpCompareName ) 
		{
			dprintf( "error! | Ioctl_WhiteOrBlack(); | malloc Buffer failed,size=0x%x \n", Buffer->PathLength );
			__leave ;
		}

		RtlZeroMemory( lpCompareName, Buffer->PathLength );
		memcpy( lpCompareName, Buffer->szPath, Buffer->PathLength );
		lpCompareName[ Buffer->PathLength / sizeof(WCHAR) ] = UNICODE_NULL ;

		// ��������������
		switch ( Buffer->Flag )
		{
		case WhiteOrBlack_Flag_XFilePath :
			{
				QueueLockList	= ProcessNode->XFilePath.pResource ;
				ListHeadWhite	= &ProcessNode->XFilePath.OpenFilePathListHead ;
				ListHeadBlack	= &ProcessNode->XFilePath.ClosedFilePathListHead ;
			}
			break ;

		case WhiteOrBlack_Flag_XRegKey :
			{
				QueueLockList	= ProcessNode->XRegKey.pResource ;
				ListHeadWhite	= &ProcessNode->XRegKey.DirectListHead ;
				ListHeadBlack	= &ProcessNode->XRegKey.DennyListHead ;
			}
			break ;

		case WhiteOrBlack_Flag_XIpcPath :
			{
				QueueLockList	= ProcessNode->XIpcPath.pResource ;
				ListHeadWhite	= &ProcessNode->XIpcPath.OpenIpcPathListHead ;
				ListHeadBlack	= &ProcessNode->XIpcPath.ClosedIpcPathListHead ;
			}
			break ;

		case WhiteOrBlack_Flag_XClassName :
			{
				QueueLockList	= ProcessNode->XWnd.pResource ;
				ListHeadWhite	= &ProcessNode->XWnd.WndListHead ;
				ListHeadBlack	= NULL;
			}
			break ;
		
		default :
			{
				dprintf( "err! | Ioctl_WhiteOrBlack(); | �Ե�ǰFlag(0x%x)���账�� \n", Buffer->Flag );
				__leave ;
			}
			break ;
		}

		// ��������
		if ( ListHeadBlack )
			bIsBlack = GLFindNodeEx( ListHeadBlack, QueueLockList, lpCompareName, NULL );

		if ( FALSE == bIsBlack && ListHeadWhite )
		{
			// �����Ǻڵ�,������������
			bIsWhite = GLFindNodeEx( ListHeadWhite, QueueLockList, lpCompareName, NULL );
		}

		// ���R3���ݰ�
		*Buffer->bIsBlack = bIsBlack ;
		*Buffer->bIsWhite = bIsWhite ;

		// �ͷ��ڴ�
		kfree( lpCompareName );
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	//	dprintf( "error! | Ioctl_WhiteOrBlack() - __try __except(); | ProbeForXX ��Ӧ�ĵ�ַ���Ϸ� \n" );
		return STATUS_UNSUCCESSFUL ;
	}

	return STATUS_SUCCESS ;
}




///////////////////////////////   END OF FILE   ///////////////////////////////