///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _ENTRY_CPP
#define _ENTRY_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <ShlObj.H>
#include <Stdio.H>
#include <ShlOBJ.H>
#include <Stdlib.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Entry.H"
#include "Init.H"

#include "../Dialogs/MainDialog.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HINSTANCE ghAppInstance = NULL;

char gsAppCopyRight[64];
char gsFileVersion[10];
char gsTitleCaption[64];
char gsRegistryKey[255];

char gsPath[MAX_PATH];
char gsDataPath[MAX_PATH];
DWORD dwManagementPort = 0;

CMemPool *pMem; //The global memory object.
CService gService;	//The global service.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IsApplicationRunning(void)
{
	bool bResult = false;
	HANDLE hMutex = NULL;
	SECURITY_DESCRIPTOR SD;
	SECURITY_ATTRIBUTES SA;

	if(InitializeSecurityDescriptor(&SD, SECURITY_DESCRIPTOR_REVISION) == FALSE)
	{
		return false;
	}

	if(SetSecurityDescriptorDacl(&SD, TRUE, 0, FALSE) == FALSE)
	{
		return false;
	}

	SA.nLength				= sizeof(SA);
	SA.lpSecurityDescriptor = &SD;
	SA.bInheritHandle		= FALSE;

	char sMutex[255];
	sprintf_s(sMutex, "Global\\%s", gsTitleCaption);
	if((hMutex = CreateMutex(&SA, FALSE, sMutex)) != NULL)
	{
		if(GetLastError() == ERROR_ALREADY_EXISTS)
		{
			bResult = true;
		}

		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
	}

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevious, LPSTR sCmdLine, int iCmdShow)
{
	ghAppInstance = GetModuleHandle(NULL);

	if(!InitializeApp())
    {
        return 1;
    }

	DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, MainDialog);

	UninitializeApp();

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

