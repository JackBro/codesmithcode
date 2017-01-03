#pragma once

#include <ntifs.h>

//////////////////////////////////////////////////////////////////////////


//
// naked �����Ŀ�ƪ & ���
//

#define Prolog() \
{    \
	_asm push ebp    \
	_asm mov ebp, esp    \
	_asm sub esp, __LOCAL_SIZE    \
	_asm pushfd    \
	_asm pushad    \
}

#define FinisDenny()    \
{    \
	_asm popad    \
	_asm popfd    \
	_asm mov esp, ebp    \
	_asm pop ebp    \
}

#define FinisOrig()    \
{    \
	_asm popad    \
	_asm popfd    \
	_asm mov esp, ebp    \
	_asm pop ebp    \
	_asm _emit   0x68	\
	_asm _emit   0x00	\
	_asm _emit   0x00	\
	_asm _emit   0x00	\
	_asm _emit   0x00	\
	_asm ret			\
}

//
// fake ������ʵ��ģ�� (���Header Jmp����; ����call hook����,��ģ�廹�øĸ�)
//

/*
// VOID NtUserBlockInput( IN BOOL fBlockIt );

VOID _declspec(naked) fake_NtUserBlockInput()
{
	BOOL fBlockIt ;

	// 1. ǰ�ô���
	Prolog();
	fBlockIt = FALSE ;

	// 2. �ж��Ƿ��ֹ���ⰴ��,����������ԭ����
	__asm
	{
		mov	eax, dword ptr [ebp+8]
		mov	fBlockIt, eax
	}

	if ( FALSE == fBlockIt ) { goto _Call_Orignal_NtUserBlockInput_ ; }

_Denny_for_NtUserBlockInput_ :
	FinisDenny() ;
	__asm
	{
		mov eax, 0	// �޸ķ���ֵ
		ret 4h
	}

_Call_Orignal_NtUserBlockInput_ :
	FinisOrig();
}
*/


//////////////////////////////////////////////////////////////////////////

BOOL LoadInlineHookEngine ();
VOID UnloadInlineHookEngine () ;

BOOLEAN
HookCode95 (
    IN PVOID  pTarFunction,
    IN PVOID  pNewFunction,
    IN ULONG  HookNumber
    );

VOID
UnhookCode95 (
    IN PVOID  pNewFunction
    );

//////////////////////////////////////////////////////////////////////////

typedef enum _INLINE_ENGINE_TYPES
{
    Automatic,
    CallHookE8,
    CallHookFF15,
    InlineHookPre2,  // ����+����ת
    InlineHookPre1,  // ��ͷ5�ֽ�JMP
    InlineHookDep2,  // push+ret
    InlineHookDep1,  // ���JMP
    InlineCrazyPatch

    //
    // let's do better
    //

} INLINE_ENGINE_TYPES;


//////////////////////////////////////////////////////////////////////////