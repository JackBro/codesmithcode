#define WINVER 0x500
#define _WIN32_WINNT 0x500
#include <windows.h>
#include <Winuser.h>

//////////////////////////////////////////////////////////////////////////

LPSTR szName1 = "�������ع��� V1.3��" ;
LPSTR szName2 = "��" ;


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	INPUT inputs[4] = {0};
	
	HWND hwnd = FindWindow( NULL, szName1 );
	
	if (hwnd)
	{
		SetForegroundWindow(hwnd);
		
// 		inputs[0].type = INPUT_KEYBOARD;
// 		inputs[0].ki.wVk = VK_RETURN;
// 		inputs[1].type = INPUT_KEYBOARD;
// 		inputs[1].ki.wVk = VK_RETURN;
// 		inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

		inputs[0].type = INPUT_KEYBOARD;
		inputs[0].ki.wVk = VK_ESCAPE;
		inputs[1].type = INPUT_KEYBOARD;
		inputs[1].ki.wVk = VK_ESCAPE;
		inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

		SendInput(2,inputs,sizeof(INPUT));
	}
	
	return 0;
}