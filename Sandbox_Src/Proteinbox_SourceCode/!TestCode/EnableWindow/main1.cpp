#define WINVER 0x500
#define _WIN32_WINNT 0x500
#include <windows.h>
#include <Winuser.h>

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	HWND hwnd = FindWindow(NULL,"�������ع��� V1.3��");

	if (hwnd)
	{
		SetForegroundWindow(hwnd);

		EnableWindow( hwnd, FALSE ) ; // ��ֹ��ǰ����
	}

	return 0;
}
