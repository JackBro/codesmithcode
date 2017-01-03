/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2011/07/09 [9:7:2011 - 0:42]
* MODULE : \PBStart\Common.cpp
* 
* Description:
*
*   ���к���ģ��
*                        
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include "stdafx.h"
#include "Common.h"

//////////////////////////////////////////////////////////////////////////


PVOID kmalloc( ULONG length )
{
	PVOID buffer = NULL;
	
	//	buffer = malloc( length );
	buffer = new PVOID[ length ] ;
	if ( NULL == buffer )
	{
		ExitProcess( 0xFFFFFFFF );
	}
	
	memset( buffer, 0, length );
	return buffer ;
}



VOID kfree( PVOID ptr )
{
	if ( ptr && TRUE == MmIsAddressValid(ptr, 1) )
	{
		delete [] ptr ;
		ptr = NULL ;
	}
	
	return ;
}



BOOL
MmIsAddressValid (
	IN PVOID ptr,
	IN ULONG length
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/10/27 [27:10:2010 - 10:46]

Routine Description:
  ��ָ֤����Χ��,�ڴ����ݵĺϷ���
    
Arguments:
  ptr - ����֤��ָ��
  length - ��ַ����

Return Value:
  TRUE / FALSE
    
--*/
{
	if ( NULL == ptr ) { return FALSE ; }

	if ( IsBadReadPtr( (const void *)ptr, length ) ) { return FALSE ; }

	return TRUE ;
}


///////////////////////////////   END OF FILE   ///////////////////////////////
