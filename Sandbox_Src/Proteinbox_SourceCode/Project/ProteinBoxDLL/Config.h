#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wtypes.h>
#include <windows.h>


//////////////////////////////////////////////////////////////////////////

#define ARRSIZEOF(x) (sizeof(x)/sizeof(x[0]))

#define CONFIG_FILE_NAME_MAIN	"ProteinBox.ini"
#define CONFIG_FILE_NAME_TMPL	"Templates.ini"

#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

//#define kfree( TAG ) if ( TAG ) { free( (PVOID)TAG ); TAG = NULL; }


#define CHECK_MEM(p)                                                         \
{                                                                            \
    if ((p) == NULL)                                                         \
    {                                                                        \
        exit(1);                                                             \
    }                                                                        \
}

//
// �����������ļ���ÿ�ζ�ȡһ�е�����,�ж�������
//

typedef enum _ConfStringType_
{
    T_SECTION,		// eg: [GlobalSetting]
    T_KEY,			// eg: OpenFilePath
	T_KEY_MORE,		// eg: ���к���ַ���sudam*,,imadu*
    T_BLANKLINE,	// �ֺ�';' ��Ϊע����
    T_ERROR			// ����ĸ�ʽ

} ConfStringType ;

/*++

(1) �����ļ�ͼʾ��

    sec1-->sec2-->sec3-->sec4...
      |            |      |  
     key1         key1   key1
      |            |     
     key2         key2   
      |                  
     key3->value1-> ... ->valueN            

(2) �����ļ����ݣ�

[GlobalSetting]
; �ֺ�Ϊע��,����Ϊ�����
OpenFilePath=firefox.exe,%Favorites%
OpenIpcPath=*\BaseNamedObjects*\MAPI-HP*,*\BaseNamedObjects*\OLKCRPC.OBJ=*
sudam*,,imadu*
CloseIpcPath=xx

(3) �����ļ��ṹ��
��ΪSection, Key, Value

--*/

#define KEY_LEN             50
#define SECTION_LEN         30

//
// eg: "firefox.exe,%Favorites%"
//

typedef struct _CONFIG_VALUE_INFO_
{
/*+0x000*/ struct _CONFIG_VALUE_INFO_* next ;
/*+0x004*/ int NameLength ;		// �ַ�������
/*+0x008*/ char* ValueName ;	// �ַ������ݵ�ָ��,��̬����&�ͷ�

} CONFIG_VALUE_INFO, *LPCONFIG_VALUE_INFO ; 


//
// eg: "OpenFilePath"
//

typedef struct _CONFIG_KEY_INFO_
{
/*+0x000*/ struct _CONFIG_KEY_INFO_* next ; // ָ���¸��ڵ�eg:CloseIpcPath
/*+0x004*/ char KeyName[ KEY_LEN + 1 ];	// eg:OpenIpcPath
/*+0x008*/ LPCONFIG_VALUE_INFO ValueHead;		// eg: �ýڵ㽫���һ�������������ڴ洢11,22,33��Щ�ַ���

} CONFIG_KEY_INFO, *LPCONFIG_KEY_INFO ; 


//
// eg: "[GlobalSetting]"
//

typedef struct _CONFIG_SECTION_INFO_
{
/*+0x000*/ struct _CONFIG_SECTION_INFO_* next ;
/*+0x004*/ char SectionName[ SECTION_LEN + 1 ];
/*+0x008*/ LPCONFIG_KEY_INFO KeyHead;

} CONFIG_SECTION_INFO, *LPCONFIG_SECTION_INFO ;


//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif


class CConfig
{

public:
	CConfig();
	virtual ~CConfig();

	LPCONFIG_SECTION_INFO m_pProfile ;

	virtual BOOL Load( IN char* filename );
	virtual	void UnLoad();

	virtual void GetConfString(const char *section, const char *key, LPCONFIG_VALUE_INFO *pszRetStr);
	virtual int GetConfInt(const char *section, const char *key, int defaultValue);
	virtual double GetConfDouble(const char *section, const char *key, double defaultValue);
	
	virtual void WriteConfString( const char *section, const char *key, const char *pszWriteStr);
	virtual void WriteConfInt(const char *section, const char *key, int value);
	virtual void WriteConfDouble(const char *section, const char *key, double value);

protected:

	int GetStringLength( IN const char *pszText );
	LPCONFIG_VALUE_INFO AllocateNodeValue();
	void DistroyNodeValue( IN PVOID ListHead );
	LPCONFIG_VALUE_INFO FixNodeValue( IN const char* szName, IN int Length );
	BOOL BuildNodeValue( IN LPCONFIG_VALUE_INFO ListHead, IN const char* szName, IN int Length );

	int AnalysisTextLine(const char *pszText);
	void GetSectionName(const char *pszText, char *section_name);
	void GetKeyInfo(const char *pszText, char *key_name, LPCONFIG_VALUE_INFO *key_value);
	void GetKeyInfoEx( IN const char *pszText, OUT LPCONFIG_VALUE_INFO *key_value );
	void FreeProfile( LPCONFIG_SECTION_INFO pProfileInfo);
	LPCONFIG_SECTION_INFO LoadProfile(const char *filename);
	void SaveProfile(const char *filename, LPCONFIG_SECTION_INFO pProfileInfo);
	BOOL GetProfileStringPrivate(const char *section,const char *key,char *pszRetStr);

private:
	char m_szName[ MAX_PATH ];	// ��Ҫ�����ļ���ȫ·��
};


class CConfigEx : public CConfig
{
	
public:
	CConfigEx();
	virtual ~CConfigEx();
	
	// ��ʼ����Ϣ
	BOOL Load();
	VOID UnLoad();

	BOOL InitData();
	VOID __InitData( LPSTR lpdest, LPSTR lpsrc );

//	BOOL GetDrvPointer( CDriver* drv );
	BOOL Wakeup_R0_InitConfigData ();
	BOOL Waitfor_R0_InitConfigData ();
	void SendConfigData();

	//
	// һ�������е��߳�/�ص�����,���������ɾ�̬��.�����൱����ȫ�ֵ��ڴ�,
	// �����ʱ�໹û�������ڴ�,���õ�������ĺ���,��Ȼ���벻��
	//

	static DWORD WINAPI WorkThread( LPVOID lpParameter ); 

protected:
	void _ThreadProc();
	BOOL _StartThread();
	void _StopThread();

private:
	BOOL m_bInitOK ;
	BOOL m_bAbortThread ;

	char m_szProcteiBoxMainIni[ MAX_PATH ];		// ��Ҫ�����ļ���ȫ·��
	char m_szProcteiBoxTemplateIni[ MAX_PATH ];	// ���������ļ���ȫ·��

	char m_szEvent_InitConfigData_wakeup_R0[ MAX_PATH ];	// "Global\\Proteinbox_ConfEvent_InitConfigData_wakeup_R0"
	HANDLE m_hEvent_InitConfigData_wakeup_R0 ;  // ��������һ�̻߳�һֱ�ȴ�,��R3���ô��¼��Żἤ��ȴ���

	char m_szEvent_InitConfigData_wakeup_R3[ MAX_PATH ];	// "Global\\Proteinbox_ConfEvent_InitConfigData_wakeup_R3"
	HANDLE m_hEvent_InitConfigData_wakeup_R3 ;  // Ӧ�ò�һֱ�ȴ�,��R0����������ʱ�õ����¼�

	char m_szEvent_waitfor_r0[ MAX_PATH ];	// "Global\\Proteinbox_ConfEvent_waitfor_r0"
	HANDLE m_hEvent_waitfor_r0 ;  // Ӧ�ò�һֱ�ȴ�,��R0����������ʱ�õ����¼�
	
	char m_szEvent_wakeup_r0[ MAX_PATH ];	// "Global\\Proteinbox_ConfEvent_wakeup_r0"
	HANDLE m_hEvent_wakeup_r0 ;  // Ӧ�ò�׼�������ݺ�,֪ͨ�ȴ���ROʱ�õ����¼�

	HANDLE m_hWorkThread ;

//	CDriver* m_drv_ProteinBoxDrv ;
};


//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
#endif // __DRIVER_H__