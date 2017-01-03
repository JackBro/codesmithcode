/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/05/10 [10:5:2010 - 15:27]
* MODULE : d:\Work\Program\Coding\ɳ��\SandBox\Code\!TestCode\ImageNotifyDll\InjectDll.c
* 
* Description:
*      
*   ������ͨ���޸�PE�ڴ�ṹע��DLL����Ҫ������                       
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include <stdio.h>
#include <ntddk.h>

#include "struct.h"
#include "InjectDll.h"

//////////////////////////////////////////////////////////////////////////

// static const char g_szDllPath [] = "C:\\sudami.dll" ;

_ZwProtectVirtualMemory_ g_ZwProtectVirtualMemory_addr = NULL ;


//////////////////////////////////////////////////////////////////////////


NTSTATUS
InjectDll_by_reconstruct_pe (
	IN ULONG PEAddr
	) 
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/10 [10:5:2010 - 15:29]

Routine Description:
  ����Sandboxie���ֶ�,ͨ���ع��ڴ���PE��IAT,�ﵽ����ע��DLL��ָ�����̵�Ч��.
    
Arguments:
  ImageBaseAddress - [IN] ģ���ַ

--*/
{
	char   bFlag ;
	USHORT Magic = 0 ;
	ULONG  SectionTableIndex, SectionTableCurrentAddr, ImportTableAddr, NumberOfBytesToProtect ; 
	ULONG  out_pbNewIid_offset, OldAccessProtection, out_cbNew, NumberOfRvaAndSizes ;

	NTSTATUS status = STATUS_UNSUCCESSFUL ;
	PVOID					pbNewIid				= NULL	; 
	PIMAGE_NT_HEADERS32		pinh					= NULL	; 
	PIMAGE_SECTION_HEADER	pish					= NULL	;
	PIMAGE_DATA_DIRECTORY	pidd					= NULL	;
	PNEW_PE_IAT_INFO_TOTAL	pNew					= NULL ;
	PIMAGE_SECTION_HEADER   pSectionHeaderCurrent	= NULL	;
	PIMAGE_DOS_HEADER		ImageBaseAddress		= (PIMAGE_DOS_HEADER) PEAddr ;

	if ( 0 == ImageBaseAddress )
	{
		dprintf( "error! | InjectDll_by_reconstruct_pe(); Invalid Paramaters; 0 == ImageBaseAddress. \n" );
		return STATUS_INVALID_PARAMETER ;
	}

	ProbeForRead( (LPVOID)ImageBaseAddress, 0x40, 1 );

	//  
	// 1. У��PE�Ϸ���
	//  

	if ( ImageBaseAddress->e_magic != 'MZ' && ImageBaseAddress->e_magic != 'ZM' )
	{
		dprintf( "error! | InjectDll_by_reconstruct_pe(); Invalid PE; ImageBaseAddress->e_magic; \n" );
		return STATUS_INVALID_IMAGE_NOT_MZ ;
	}

	pinh = (PIMAGE_NT_HEADERS)( (PCHAR)ImageBaseAddress + ImageBaseAddress->e_lfanew );
	ProbeForRead( pinh, 0x108, 1 );

	if ( pinh->Signature != 'EP' )
	{
		dprintf( "error! | InjectDll_by_reconstruct_pe(); Invalid PE; pinh->Signature; \n" );
		return STATUS_INVALID_IMAGE_FORMAT ;
	}

	Magic = pinh->OptionalHeader.Magic;

	switch ( Magic )
	{
	case 0x10B : // Ӧ������0x10B
		{
			pidd = pinh->OptionalHeader.DataDirectory;
			NumberOfRvaAndSizes = pinh->OptionalHeader.NumberOfRvaAndSizes;
			bFlag = 0;
		}
		break ;

	case 0x20B :
		{
			pidd = &pinh->OptionalHeader.DataDirectory[2];
			NumberOfRvaAndSizes = pinh->OptionalHeader.DataDirectory[1].Size;
			bFlag = 1;
		}
		break ;

	default :
		pidd = 0;
		NumberOfRvaAndSizes = 0;
		break ;
	}

	if ( (0 == pidd) || (NumberOfRvaAndSizes < 15) )
	{
		dprintf( "error! | InjectDll_by_reconstruct_pe(); 0 == pidd; Failed! \n" );
		return STATUS_NOT_A_DIRECTORY ;
	}

	pish = (PIMAGE_SECTION_HEADER) ( (PCHAR) &pinh->OptionalHeader + pinh->FileHeader.SizeOfOptionalHeader ) ;

	ProbeForRead( pish, 0x28 * pinh->FileHeader.NumberOfSections, 1 );

	//  
	// 2. �ع�PE�ļ�,�õ������齨��PE���ֿ������,���а���sbiedll.dll��·��
	//  

	pNew = InjectDll_by_reconstruct_pe_dep (
		bFlag,
		ImageBaseAddress,
		pish,
		pinh->FileHeader.NumberOfSections,
		pidd,
		&out_cbNew,
		&out_pbNewIid_offset
		);

	//  
	// 2.1 �����¹�����PE�ڴ��Ĳ�������
	//  

	if ( NULL == pNew )
	{
		dprintf( "error! | InjectDll_by_reconstruct_pe(); 0 == pNew; Failed! \n" );
		return STATUS_UNSUCCESSFUL ;
	}
	
	pNew->Stub.pidd = pidd;
	pNew->Stub.Reserved2 = 0;
	pNew->Stub.IMAGE_DIRECTORY_ENTRY_IMPORT_VirtualAddress			= pidd[ IMAGE_DIRECTORY_ENTRY_IMPORT ].VirtualAddress ;
	pNew->Stub.IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT_VirtualAddress	= pidd[ IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT ].VirtualAddress ;
	pNew->Stub.IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT_Size				= pidd[ IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT ].Size ;
	pNew->Stub.IMAGE_DIRECTORY_ENTRY_IAT_VirtualAddress				= pidd[ IMAGE_DIRECTORY_ENTRY_IAT ].VirtualAddress ;
	pNew->Stub.IMAGE_DIRECTORY_ENTRY_IAT_Size						= pidd[ IMAGE_DIRECTORY_ENTRY_IAT ].Size ;
	pNew->Stub.IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR_VirtualAddress	= pidd[ IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR ].VirtualAddress ;
	pNew->Stub.IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR_Size			= pidd[ IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR ].Size ;

	pbNewIid = (char *)ImageBaseAddress + out_pbNewIid_offset;
	ProbeForRead( pbNewIid, out_cbNew, 1 );

	NumberOfBytesToProtect = 0x80 ;

	//  
	// 3.����ԭPE��Ĳ�������
	//

	if( FALSE == Make_address_writable( (PVOID)pidd, 0x80 ) )
	{
		status = STATUS_UNSUCCESSFUL ;
		goto _end_ ;
	}

	//  
	// 3.1 ���Ŀ¼���е�IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT �� IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR
	//  

	SectionTableIndex = 0;

	pidd[ IMAGE_DIRECTORY_ENTRY_IMPORT ].VirtualAddress			= out_pbNewIid_offset + 0x68 ; // ��������IAT��ַ
	pidd[ IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT ].VirtualAddress	= 0;
	pidd[ IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT ].Size				= 0;
	pidd[ IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR ].VirtualAddress = 0;
	pidd[ IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR ].Size			= 0;

	//  
	// 3.2 If the file didn't have an IAT_DIRECTORY, we create one...
	//     ��Ŀ¼����С��13,�Ҳ�����Import Address Table(�����ַ��Ŀ¼)�����:
	//     �������нڱ�,�ҵ���ַλ��IAT��Χ�ڵĽ�,�����ַ & ��С���� 12��Ŀ¼��
	//  
	if ( NumberOfRvaAndSizes <= 0xC || !pidd[ IMAGE_DIRECTORY_ENTRY_IAT ].VirtualAddress )
	{
		while ( 1 )
		{
			if ( SectionTableIndex >= pinh->FileHeader.NumberOfSections )
				break;

			pSectionHeaderCurrent	= &pish[ SectionTableIndex ] ;
			SectionTableCurrentAddr = pSectionHeaderCurrent->VirtualAddress ;
			ImportTableAddr			= pNew->Stub.IMAGE_DIRECTORY_ENTRY_IMPORT_VirtualAddress ;

			if (   (ImportTableAddr >= SectionTableCurrentAddr)
				&& (ImportTableAddr < SectionTableCurrentAddr + pSectionHeaderCurrent->SizeOfRawData)
				)
			{
				pidd[ IMAGE_DIRECTORY_ENTRY_IAT ].VirtualAddress = SectionTableCurrentAddr ;

				if ( pSectionHeaderCurrent->Misc.VirtualSize ) {
					pidd[ IMAGE_DIRECTORY_ENTRY_IAT ].Size = pSectionHeaderCurrent->Misc.VirtualSize ;
				} else {
					pidd[ IMAGE_DIRECTORY_ENTRY_IAT ].Size = pSectionHeaderCurrent->SizeOfRawData ;
				}

				break;
			}

			++ SectionTableIndex;
		}
	}

	//  
	// 4. ���ع���PE�����ԭ����PE��,�ﵽ�ں���ע��ָ��DLL��ָ�����̵�Ч��
	//  

	NumberOfBytesToProtect = out_cbNew;

	if( FALSE == Make_address_writable( pbNewIid, out_cbNew ) )
	{
		status = STATUS_UNSUCCESSFUL ;
		goto _end_ ;
	}

	// �ؼ�һ��Code
	memcpy( pbNewIid, pNew, out_cbNew );
	status = STATUS_SUCCESS ;

_end_ :
	if ( NULL != pNew ) { kfree( (PVOID)pNew ); }

	return status;
}



PVOID 
InjectDll_by_reconstruct_pe_dep(
	IN char bFlag,
	IN PIMAGE_DOS_HEADER PEAddr,
	IN PIMAGE_SECTION_HEADER pSectionHeader,
	IN ULONG NumberOfSections,
	IN PIMAGE_DATA_DIRECTORY pImageDataDirectory,
	OUT PULONG out_cbNew,
	OUT PVOID out_pbNewIid_offset
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/12 [12:5:2010 - 11:19]

Routine Description:
  �����µ�IATb��,����ע���DLL·��,����֮ 
    
Arguments:
  bFlag - [IN] 
  PEAddr - [IN] PEͷ
  pSectionHeader - [IN] �ڱ�ͷ
  NumberOfSections - [IN] �ڵ�����
  pImageDataDirectory - [IN] �����ַ
  out_cbNew - [OUT] �����¹����IAT����ܴ�С
  out_pbNewIid_offset - [OUT] �����¹����IAT���ƫ��(VirtualAddress)

Return Value:
  �¹����IAT��ָ�� [��Ҫ�������ͷ��ڴ�]
    
--*/
{
	ULONG NumberOfDll = 0 ;
	ULONG DllPathLength, cbNew, nTotalCounts_for_IAT, nCounts, pbNewIid_offset ;
	PCHAR ptr1, ptr2, DllName ;

	PIMAGE_IMPORT_DESCRIPTOR pImportTable			= NULL ;
	PIMAGE_IMPORT_DESCRIPTOR pCurrentIAT			= NULL ; 
	PIMAGE_IMPORT_DESCRIPTOR pImportTable_current	= NULL ;
	PNEW_PE_IAT_INFO_TOTAL	 lpBuffer				= NULL ;
	char szName[ MAX_PATH ] = "" ;

	//  
	// 1. ��ȡ�����ĵ�ַ
	//  

	pImportTable = (PIMAGE_IMPORT_DESCRIPTOR) ( (PCHAR)PEAddr + pImageDataDirectory[ IMAGE_DIRECTORY_ENTRY_IMPORT ].VirtualAddress );
	if ( NULL == pImportTable ) { return NULL ; }

	//  
	// 2. �������������������,����������ܸ���(ÿ����Ԫ����һ�������DLL); ��������Ϊ0xC8
	//  

	ProbeForRead( pImportTable, 0x14, 1 );
	if ( pImportTable->Name )
	{
		pCurrentIAT = pImportTable;

		do
		{
			if ( !pCurrentIAT->FirstThunk )
				break;

			if ( NumberOfDll >= 0xC8 )
				return NULL ;

			++ pCurrentIAT ;
			++ NumberOfDll ;

			ProbeForRead( pCurrentIAT, 0x14, 1 );
		}
		while ( pCurrentIAT->Name );
	}

	//  
	// 3. �����ܴ�С�����ڴ�
	//  

	DllPathLength = strlen( g_szDllPathA );
	cbNew = DllPathLength + 20 * NumberOfDll + 0xA8 ; // �µ��ڴ��С = Sbiedll.dll·������ + ��ǰ����IAT����ܴ�С + 0xA8

	if ( cbNew >= 0x1000 )
	{
		dprintf( "error! | InjectDll_by_reconstruct_pe_dep(); cbNew >= 0x1000; \n" );
		return NULL ;
	}

	//  
	// 4. �ҵ��µĺ��ʴ�С�Ŀ�϶��ַ,���պ󸲸����
	//  

	pbNewIid_offset = FindNearBase( pSectionHeader, NumberOfSections, pImageDataDirectory, cbNew );
	if ( !pbNewIid_offset )
	{
		dprintf( "error! | InjectDll_by_reconstruct_pe_dep(); Can't find interspace in PE; \n" );
		return NULL ;
	}

	//  
	// 5. �����µĴ�С�����ڴ�,�����µ�PE�ṹ
	//  

	lpBuffer = kmalloc( cbNew );
	if ( !lpBuffer )
	{
		dprintf( "error! | InjectDll_by_reconstruct_pe_dep(); Can't allocate memory; \n" );
		return NULL ;
	}

	lpBuffer->Stub.pImportTable_mscoree_dll = 0 ;
	lpBuffer->Stub.pImportTable_msvcm80_dll = 0 ;
	lpBuffer->Stub.pImportTable_msvcm90_dll = 0 ;

	//  
	// 5.1 �ӽṹ��+0x7C��ʼ,�洢���е�IAT��Ԫ,��������3��DLL��Ӧ��IAT
	//  

	nCounts = 1;
	pImportTable_current = lpBuffer->IATOld ;
	while ( 1 )
	{
		// �õ���ǰ������Ӧ��DLL����,�ų�������3���ļ�¼,ֻ�ǽ����Ƕ�Ӧ�ĵ�����ַ����һ��
		while ( 1 )
		{
			if ( 0 == pImportTable->Name )
				break;

			DllName = (PCHAR)PEAddr + pImportTable->Name ;
			RtlZeroMemory( szName, MAX_PATH );
			strcpy( szName, DllName );

			if ( 0 == _stricmp(szName, "mscoree.dll") )
			{
				lpBuffer->Stub.pImportTable_mscoree_dll = pImportTable ;// ������Ӧ��DLL��Ϊ"mscoree.dll"
				++pImportTable;
			}
			else if ( 0 == _stricmp(szName, "msvcm80.dll") )
			{
				lpBuffer->Stub.pImportTable_msvcm80_dll = pImportTable ;// ������Ӧ��DLL��Ϊ"msvcm80.dll"
				++pImportTable;
			}
			else if ( 0 == _stricmp(szName, "msvcm90.dll") )
			{
				lpBuffer->Stub.pImportTable_msvcm90_dll = pImportTable ;// ������Ӧ��DLL��Ϊ"msvcm90.dll"
				++pImportTable;
			}
			else 
			{
				break ;
			}
		}

		pImportTable_current->Characteristics	= pImportTable->Characteristics;
		pImportTable_current->TimeDateStamp		= pImportTable->TimeDateStamp;
		pImportTable_current->ForwarderChain	= pImportTable->ForwarderChain;
		pImportTable_current->Name				= pImportTable->Name;
		pImportTable_current->FirstThunk		= pImportTable->FirstThunk;

		++ nCounts ;
		++ pImportTable_current ;

		if ( pImportTable->Name == 0 )
			break;

		if ( pImportTable->FirstThunk == 0 )
			break;

		if ( nCounts >= 0xC8 )
		{
			dprintf( "error! | InjectDll_by_reconstruct_pe_dep(); Invalid IAT counts > 0xC8; \n" );
			return NULL ;
		}

		++pImportTable ;  // ����ÿ�������
	}

	//
	// 5.2 �����µ�IAT��
	//

	ptr2 = (PCHAR) ((ULONG)lpBuffer + 0x14 * nCounts + 0x68) ;
	ptr1 = (PCHAR) g_szDllPathA;
	if ( 0 ==strncmp( g_szDllPathA, "\\??\\", 4 ) ) { ptr1 += 4; }

	strcpy( ptr2, ptr1 ); // ����DLLȫ·������ "C:\sudami.dll"

	lpBuffer->Stub.IIN1.Hint = 0; 
	strcpy( lpBuffer->Stub.IIN1.Name, "Test" ); // sudami.dll�ĵ�������
	
	lpBuffer->Stub.itd_1.u1.AddressOfData = pbNewIid_offset + 0x44 ;
	lpBuffer->Stub.itd_2.u1.AddressOfData = 0;

	if ( bFlag )
	{
		lpBuffer->Stub.Reserved5[0] = 0 ;
		lpBuffer->Stub.Reserved5[1] = 0 ;
	}

	//
	// ������һ��IAT ( ƫ����+0x68~+0x7c) size - 0x14
	//

	lpBuffer->IATNew.OriginalFirstThunk = pbNewIid_offset + 0x58 ;
	lpBuffer->IATNew.TimeDateStamp		= 0 ;
	lpBuffer->IATNew.ForwarderChain		= 0xFFFFFFFF ;
	lpBuffer->IATNew.Name				= pbNewIid_offset + (ULONG)ptr2 - (ULONG)lpBuffer ;
	lpBuffer->IATNew.FirstThunk			= pbNewIid_offset + 0x58 ;

	lpBuffer->Stub.Reserved1[0] = 0 ;
	lpBuffer->Stub.Reserved1[1] = 0 ;
	lpBuffer->Stub.Reserved1[2] = 0 ;
	lpBuffer->Stub.Reserved1[3] = 0 ;


	*(DWORD *)out_pbNewIid_offset = pbNewIid_offset;
	*((DWORD *)out_pbNewIid_offset + 1) = 0 ;
	*out_cbNew = cbNew;

	return lpBuffer;
}



ULONG 
FindNearBase (
	IN PIMAGE_SECTION_HEADER pSectionHeader,
	IN ULONG NumberOfSections,
	IN PIMAGE_DATA_DIRECTORY pImageDataDirectory,
	IN ULONG cbNew
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/12 [12:5:2010 - 11:22]

Routine Description:
  �ҵ�PE�к��ʴ�С�ĵط�,�����洢�¹����IAT��
    
Arguments:
  pSectionHeader - [IN] �ڱ�ͷ
  NumberOfSections - [IN] �ڵ�����
  pImageDataDirectory - [IN] �����ַ
  cbNew - [IN] �������IAT����ܴ�С

Return Value:
  ���ʵ�PE��϶��ַ
    
--*/
{
	ULONG SectionIndex, ArrayIndex, ImageDataDirectoryIndex, ConcernedIDDIndex ;
	ULONG ResourceDirectorySize, ResourceDirectorySize_High8bit, ResourceDirectorySize_new, pider_start, pider_end ;
	ULONG IDD_current_VirtualAddress, IDD_current_VirtualAddress_start, IDD_current_VirtualAddress_end, IDD_current_Size ;
	ULONG Section_current_PhysicalAddress, Section_current_SizeOfRawData, Section_current_VirtualAddress ;
	ULONG Section_current_VirtualSize, Section_current_VirtualSize_High8bit, Section_current_VirtualSize_new ;
	ULONG cbNew_aligned_8bit ;
	PIMAGE_SECTION_HEADER pish = NULL ;

	pider_start			  = pImageDataDirectory[ IMAGE_DIRECTORY_ENTRY_RESOURCE ].VirtualAddress ;
	ResourceDirectorySize = pImageDataDirectory[ IMAGE_DIRECTORY_ENTRY_RESOURCE ].Size			 ;
	cbNew_aligned_8bit	  = PadToDwordPtr( cbNew );

	//  
	// (A) ����"������ԴĿ¼"��"��С�Ϸ�"�����
	//

	if ( pider_start && ResourceDirectorySize )
	{
		ResourceDirectorySize_High8bit = ResourceDirectorySize >> 12 ;
		ResourceDirectorySize_new = 0xFFFFF000 * (ResourceDirectorySize >> 12) + ResourceDirectorySize ;

		if ( 0 == ResourceDirectorySize_new ) { -- ResourceDirectorySize_High8bit ; }

		if ( 0x1000 - cbNew_aligned_8bit >= ResourceDirectorySize_new )
		{
			pider_start += (ResourceDirectorySize_High8bit << 12) - cbNew_aligned_8bit + 0x1000 ;
		}

		pider_end = pider_start + cbNew_aligned_8bit - 1 ;

		//  
		// 1. ����ȫ��������ָ����Ŀ¼��Index,��������ImageDataDirectory
		//  

		ArrayIndex = ConcernedIDDIndex = 0 ;

		while ( 1 )
		{
			ConcernedIDDIndex = g_ImageDataDirectoryIndex_concerned_array[ ArrayIndex ++ ] ;
			if ( IDD_ARRAY_END == ConcernedIDDIndex ) { return pider_start ; }

			//  
			// ��ȡ"��ǰĿ¼��" & "��ԴĿ¼��" �ڴ��ַ�����ֵ
			//  

			IDD_current_VirtualAddress_start = pImageDataDirectory[ ConcernedIDDIndex ].VirtualAddress	;
			IDD_current_Size				 = pImageDataDirectory[ ConcernedIDDIndex ].Size			;	
			IDD_current_VirtualAddress_end   = IDD_current_VirtualAddress_start + IDD_current_Size		;

			if ( 0 == IDD_current_VirtualAddress_start ) { continue ; }

			//  
			// 3. ��"��ԴĿ¼" �� "��ǰĿ¼��"�ĵ�ַ��Χ �ǻ��������Ĺ�ϵ,����ȫ�����,�����������һ��Ŀ¼��
			// 

			if (   (pider_start < IDD_current_VirtualAddress_start || pider_start > IDD_current_VirtualAddress_end)
				&& (pider_end   < IDD_current_VirtualAddress_start || pider_end   > IDD_current_VirtualAddress_end) 
				)
			{
				continue ;
			}  

			break ;
		}
	}

	//  
	// (B) ����������ԴĿ¼���СΪ�յ����,���д����Ĵ������
	//

	if ( 0 == NumberOfSections ) { return 0; }

	SectionIndex = 0 ;
	pish = pSectionHeader ;

	//  
	// 1. У�� "�Ǵ����Ľ�"
	//  

	while( TRUE )
	{
		if ( pish->Characteristics & IMAGE_SCN_CNT_CODE )  { goto _get_special_section_addr_A_next_ ; }

		Section_current_PhysicalAddress	= pish->Misc.PhysicalAddress ;
		Section_current_SizeOfRawData	= pish->SizeOfRawData		 ;
		Section_current_VirtualAddress  = pish->VirtualAddress		 ;

		if ( (0 == Section_current_PhysicalAddress) && (0 == Section_current_SizeOfRawData) ) { goto _get_special_section_addr_A_next_ ; }

		//  
		// 1.1 ����Ŀ¼��ImageDataDirectory,�鿴�Ƿ��ڴ�����Ŀ¼���ַλ�ڵ�ǰ�ڵĵ�ַ��Χ��; ����,��ýڲ�����Ҫ��,������ǰСѭ��,������ѭ��.
		//  

		ArrayIndex = ImageDataDirectoryIndex = 0 ;

		while( TRUE )
		{
			ImageDataDirectoryIndex		= g_ImageDataDirectoryIndex_concerned_array[ ArrayIndex ++ ] ;
			IDD_current_VirtualAddress	= pImageDataDirectory[ ImageDataDirectoryIndex ].VirtualAddress ;

			if (   IDD_current_VirtualAddress
				&& IDD_current_VirtualAddress >= Section_current_VirtualAddress
				&& IDD_current_VirtualAddress <= Section_current_VirtualAddress + Section_current_SizeOfRawData 
				)
			{
				break ;
			}			

			//  
			// 1.2 �����е�Ŀ¼���еĵ�Ԫ��ַ����λ�ڵ�ǰ�ڵ��ڴ淶Χ,���ҵľ��������.����֮
			// 

			if ( IDD_ARRAY_END == ImageDataDirectoryIndex ) { return Section_current_VirtualAddress ; }
		}

_get_special_section_addr_A_next_ :
		++ pish  ;
		++ SectionIndex ;

		if ( SectionIndex >= NumberOfSections ) { break ; }
	}

	//  
	// 2. У�� "��ִ�п� && �����Ľ�"
	//

	SectionIndex = 0;
	pish = pSectionHeader ;

	while( TRUE )
	{
		if ( (!pish->Characteristics & IMAGE_SCN_MEM_EXECUTE) || (!pish->Characteristics & IMAGE_SCN_CNT_CODE) )  { goto _get_special_section_addr_B_next_ ; }

		Section_current_VirtualSize		= pish->Misc.VirtualSize		;
		Section_current_VirtualAddress  = pish->VirtualAddress	;

		if ( 0 == Section_current_VirtualSize ) { goto _get_special_section_addr_B_next_ ; }

		Section_current_VirtualSize_High8bit = Section_current_VirtualSize >> 12 ;
		Section_current_VirtualSize_new		 = Section_current_VirtualSize + 0xFFFFF000 * Section_current_VirtualSize_High8bit ;

		if ( Section_current_VirtualSize_new && (0x1000 - cbNew_aligned_8bit >= Section_current_VirtualSize_new) )
		{
			return ((Section_current_VirtualSize_High8bit + 1) << 12) + Section_current_VirtualAddress - cbNew_aligned_8bit ;
		}

_get_special_section_addr_B_next_ :
		++ pish  ;
		++ SectionIndex ;

		if ( SectionIndex >= NumberOfSections ) { break ; }
	}

	//  
	// 3. �ҵ�����"��ִ�п� && �����"�ĵ�һ����
	//  

	SectionIndex = 0 ;
	pish = pSectionHeader ;

	while( TRUE )
	{
		//  
		// 3.1 ����ǰ�ڵ����� ������ "��ִ�п� && �����",����ѭ��,֪���ҵ�������
		// 

		if ( (pish->Characteristics & IMAGE_SCN_MEM_EXECUTE) && (pish->Characteristics & IMAGE_SCN_CNT_CODE) )  
		{
			Section_current_VirtualAddress  = pish->VirtualAddress ;
			return Section_current_VirtualAddress ;
		}

		++ pish  ;
		++ SectionIndex ;

		if ( SectionIndex >= NumberOfSections ) { break ; }
	}

	return 0 ;
}



DWORD PadToDword( IN DWORD dw )
{
	return (dw + 3) & ~3;
}



DWORD PadToDwordPtr( IN DWORD dw )
{
	return (dw + 7) & ~7 ;
}



BOOL InitSSDT ()
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/12 [12:5:2010 - 11:12]

Routine Description:
  �õ�ZwProtectVirtualMemory�ĵ�ַ; ����δ������,�ʱ������ж�λ֮!
  ͨ���������ZwPulseEvent,�õ����ϵ�ZwProtectVirtualMemory������ַ

  ZwPowerInformation@20   --> ����
  ZwPrivilegeCheck@12
  ZwPrivilegedServiceAuditAlarm@20
  ZwPrivilegeObjectAuditAlarm@24
  ZwProtectVirtualMemory@20      ID
  ZwPulseEvent@8          --> ����

Return Value:
  BOOL
    
--*/
{
	if ( g_ZwProtectVirtualMemory_addr ) { return TRUE ; }

	g_ZwProtectVirtualMemory_addr = FindUnresolved( ZwPulseEvent );

	if( NULL == g_ZwProtectVirtualMemory_addr )
	{
		dprintf( "error! | InitSSDT() - FindUnresolved(); NULL == g_ZwProtectVirtualMemory_addr; failed! \n" );
		return FALSE ;
	}

	dprintf( "ZwProtectVirtualMemory: 0x%08lx \n", g_ZwProtectVirtualMemory_addr );
	return TRUE ;
}



PVOID 
FindUnresolved ( 
	IN PVOID pFunc 
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/12 [12:5:2010 - 11:05]

Routine Description:
  �ҵ�SSDT(Zwϵ��)�������ϸ������ĵ�ַ 
    
Arguments:
  pFunc - [IN] �ѵ�����Zw������ַ 

--*/
{
	UCHAR	pattern[5] = { 0 };
	PUCHAR	bytePtr = NULL;
	PULONG  oldStart = 0;
	ULONG	newStart = 0;

	memcpy( pattern, pFunc, 5 );

	oldStart = (PULONG)&(pattern[1]);
	newStart = *oldStart - 1;
	*oldStart = newStart;

	for( bytePtr = (PUCHAR)pFunc - 5; bytePtr >= (PUCHAR)pFunc - 0x800; bytePtr-- )
	{
		if( TRUE == CheckPattern( bytePtr, pattern, 5 ) )
		{
			return (PVOID) bytePtr ;
		}
	}

	return NULL ;
}



BOOL 
CheckPattern( 
	IN unsigned char* pattern1,
	IN unsigned char* pattern2,
	IN size_t size 
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/12 [12:5:2010 - 11:07]

Routine Description:
  ���Ա�2�������е������Ƿ���ͬ    

Return Value:
  TRUE - ��ͬ; FALSE - ��ͬ
    
--*/
{
	register unsigned char* p1 = pattern1;
	register unsigned char* p2 = pattern2;

	while( size-- > 0 )
	{
		if( *p1++ != *p2++ )
			return FALSE ;
	}

	return TRUE ;
}



BOOL
Make_address_writable(
	IN PVOID address,
	IN ULONG size 
	)
/*++

Author: sudami [sudami@163.com]
Time  : 2010/05/12 [12:5:2010 - 17:38]

Routine Description:
  ����ָ�����ȵĽ��������ַ�ռ��ڴ�ɶ�д
    
Arguments:
  address - [IN] �����õĽ��̿ռ������ַ
  size - [IN] ��ַ�ĳ���

Return Value:
  TRUE - ���óɹ�; FALSE - ����ʧ��
    
--*/
{
	NTSTATUS	status				= STATUS_UNSUCCESSFUL	;
	ULONG		pageAccess			= 0						;
	ULONG		ZwProtectArray[ 3 ] = { 0 }					;

	pageAccess = PAGE_EXECUTE_READWRITE ;
	ZwProtectArray[0] = (ULONG) address ;
	ZwProtectArray[1] = size			;
	ZwProtectArray[2] = 0				;

	status = g_ZwProtectVirtualMemory_addr (
		(HANDLE) -1 ,
		(PVOID *)(&(ZwProtectArray[0])) ,
		&(ZwProtectArray[1]),
		pageAccess,
		&(ZwProtectArray[2]) 
		);

	if( !NT_SUCCESS( status ) )
	{
		dprintf ( "error! | Make_address_writable() - ZwProtectVirtualMemory(); handler address:%d failed (Status %lx) \n", address, status );
		return FALSE;
	}

	return TRUE;
}

///////////////////////////////   END OF FILE   ///////////////////////////////