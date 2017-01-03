#if !defined(_outputdebug_H_)
#define _outputdebug_H_

// ������Release�汾�е����򿪴˿��أ���ʱע�����йر�

#ifdef _DEBUG
#define _MYDEBUG
#endif


#ifdef _MYDEBUG /////////////

#define _MYDEBUGFLAG_A "[PBSRV] "
#define _MYDEBUGFLAG_W L"[PBSRV] "

void _cdecl MyAtlTraceW(LPCWSTR lpszFormat, ...);
void _cdecl MyAtlTraceA(LPCSTR lpszFormat, ...);


#ifdef _UNICODE
    #define MYTRACE MyAtlTraceW
#else
    #define MYTRACE MyAtlTraceA
#endif


#else//////////////

#define MyAtlTraceA __noop /*(void)0*/
#define MyAtlTraceW __noop /*(void)0*/
#define MYTRACE		__noop /*(void)0*/

#endif

#endif // !defined(_outputdebug_H_)