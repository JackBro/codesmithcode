/***************************************************************************************
* AUTHOR : sudami [sudami@163.com]
* TIME   : 2010/01/05 [5:1:2010 - 15:25]
* MODULE : D:\Program\R0\Coding\ɳ��\SandBox\Code\HandlerReg\main1.cpp
* 
* Description:
*      
*                        
*
***
* Copyright (c) 2008 - 2010 sudami.
*
****************************************************************************************/

#include <iostream.h>
#include "Test.h"

//////////////////////////////////////////////////////////////////////////

int main()
{
	BOOL bOK = TRUE ;
	int Choice = 0 ;
	
	while ( bOK )
	{
		printf (
			"��ѡ�����:	\n"
			"0. ESC			\n"
			"1. �½���ֵ: ZwCreateKey	\n"
			"2. �򿪼�ֵ: ZwOpenKey		\n"
			"3. ����Value: ZwSetValueKey \n"
			"4. ��������ֵ	\n"
			"5. ����ֵ		\n"
			"6. ɾ��ֵ		\n"
			"-------------------------------------------\n"
			);
		
		cin >> Choice;
		switch (Choice)
		{
		case 0:
			bOK = FALSE ;
			break;
			
		case 1:
			TestCreateKey ();
			break;
			
		case 2:
			TestOpenKey ();
			break;
			
		case 3:
			TestSetValueKey ();
			break;
			
		case 4:
			break;

		case 5:
			break;
			
		default:
			break;
		}
	}
	
	//	getchar() ;
	return 0;
}

///////////////////////////////   END OF FILE   ///////////////////////////////