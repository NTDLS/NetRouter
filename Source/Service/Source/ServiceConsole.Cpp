///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _ServiceConsole_Cpp
#define _ServiceConsole_Cpp
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <IO.H>
#include <fcntl.h>
#include <Process.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Entry.H"
#include "CManagement.H"
#include "CCore.H"
#include "ServiceConsole.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::Windows;
using namespace NSWFL::KeyGeneration;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CManagement *gManage = NULL;
_ConIO ConIO;
HANDLE Console_Thread_Handle = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProcessConsoleCommand(const char *sCommand, const char **sParams, int iParams)
{
	if(_strcmpi(sCommand, "Help") == 0)
	{
		WriteCon("Help\n");
		WriteCon("Pumps\n");
		WriteCon("Routes\n");
		WriteCon("Threads\n");
		WriteCon("Cls\n");
		WriteCon("Exit\n");
	}
	else if(_strcmpi(sCommand, "cls") == 0)
	{
		system("cls");
	}
	else if(_strcmpi(sCommand, "Pumps") == 0)
	{
		for(int iPump = 0; iPump < pCore->Pumps.Count; iPump++)
		{
			PUMP *pPump = &pCore->Pumps.Items[iPump];
			if(pPump->Used)
			{
				WriteCon("Pump# %d\n", iPump);
				WriteCon("   - UID              : %s\n", pPump->RouteGUID);
				WriteCon("   - Port             : %d\n", pPump->Server->ListenPort());
				WriteCon("   - MaxConnections   : %d\n", pPump->Server->MaxConnections());
				WriteCon("   - Load             : %.2f%%\n",
					(((float)pPump->Server->CurrentConnections()) / ((float)pPump->Server->MaxConnections()) * 100.0f));
				WriteCon("   - Pump Threads     : %d of %d\n", pPump->Server->p_iDataPumpThreads, pPump->Server->p_iMaxDataPumpThreads);
				WriteCon("   - Resolver Threads : %d of %d\n", pPump->Server->p_iResolverThreads, pPump->Server->p_iMaxResolverThreads);
				WriteCon("   - Pump Spawn       : %d%%\n", pPump->Server->p_iDataPumpSpawnThreshold);
				WriteCon("   - Resolver Spawn   : %d%%\n", pPump->Server->p_iResolverSpawnThreshold);
			}
		}
	}
	else if(_strcmpi(sCommand, "Routes") == 0)
	{
		for(int iRoute = 0; iRoute < pCore->Routes->Collection.Count; iRoute++)
		{
			ROUTE *pRoute = &pCore->Routes->Collection.Items[iRoute];
			if(pRoute->GUID)
			{
				WriteCon("Route# %d\n", iRoute);
				WriteCon("   - UID                    : %s\n", pRoute->GUID);
				WriteCon("   - Name                   : %s\n", pRoute->Name);
				//WriteCon("   -   : %s\n", pRoute->Description);
				WriteCon("   - LocalPort              : %d\n", pRoute->LocalPort);
				WriteCon("   - MaxConnections         : %d\n", pRoute->MaxConnections);
				WriteCon("   - DataPumpThreads        : %d\n", pRoute->DataPumpThreads);
				WriteCon("   - ResolverThreads        : %d\n", pRoute->ResolverThreads);
				WriteCon("   - InitialBufferSize      : %d\n", pRoute->InitialBufferSize);
				WriteCon("   - MaxBufferSize          : %d\n", pRoute->MaxBufferSize);
				WriteCon("   - IdleDisconnectTimeout  : %d\n", pRoute->IdleDisconnectTimeout);
				WriteCon("   - DataPumpSpawnThreshold : %d\n", pRoute->DataPumpSpawnThreshold);
				WriteCon("   - ResolverSpawnThreshold : %d\n", pRoute->ResolverSpawnThreshold);
				WriteCon("   - PersistenceTime        : %d\n", pRoute->PersistenceTime);
				WriteCon("   - AutoStart              : %d\n", pRoute->AutoStart);
				//CListenIPs *ListenIPs;
				//CConnectHosts *ConnectHosts;
				//CHTTPHeaders *HTTPHeaders;
			}
		}
	}
	else if(_strcmpi(sCommand, "Threads") == 0)
	{
		for(int iPump = 0; iPump < pCore->Pumps.Count; iPump++)
		{
			PUMP *pPump = &pCore->Pumps.Items[iPump];
			if(pPump->Used)
			{
				WriteCon("Pump# %d\n", iPump);

				WriteCon("   - MaxConnections   : %d\n", pPump->Server->MaxConnections());
				WriteCon("   - Pump Threads     : %d of %d\n", pPump->Server->p_iDataPumpThreads, pPump->Server->p_iMaxDataPumpThreads);
				WriteCon("   - Pump Spawn       : %d%%\n", pPump->Server->p_iDataPumpSpawnThreshold);
				WriteCon("   - Resolver Threads : %d of %d\n", pPump->Server->p_iResolverThreads, pPump->Server->p_iMaxResolverThreads);
				WriteCon("   - Resolver Spawn   : %d%%\n", pPump->Server->p_iResolverSpawnThreshold);
				WriteCon("   - Resolver Queue   : %d\n", pPump->Server->p_ResolverStack->StackSize());
				WriteCon("   - Overall Load     : %.2f%%\n",
					(((float)pPump->Server->CurrentConnections()) / ((float)pPump->Server->MaxConnections()) * 100.0f));

				WriteCon("   - Thread Connection Details\n");

				for(int iThread = 0; iThread < pPump->Server->p_iDataPumpThreads; iThread++)
				{
					ROUTEPUMPTHREADINFO *pPTI = &pPump->Server->p_RoutePumpThreadInfo[iThread];
					WriteCon("        - #%d: (%d of %.0f)\n",
						pPTI->ThreadNumber, pPTI->ConnectionIndexes->size(),
						(float)pPump->Server->MaxConnections() / (float)pPump->Server->p_iMaxDataPumpThreads);
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned __stdcall p_ConsoleThreadProc(void *pData)
{
	WriteCon("Type [help] for a list of options, or [exit] to close this window.\n\n", gsTitleCaption, gsFileVersion);

	while(true)
	{
		WriteCon("\n> ");
		char sLine[1024];

		if(GetConsoleInput(sLine, sizeof(sLine)))
		{
			if(_strcmpi(sLine, "exit") == 0)
			{
				break;
			}

			int iArgs = 0;
			char **sArgs = Split(sLine, (int)strlen(sLine), &iArgs, " ", 1);
			if(iArgs)
			{
				ProcessConsoleCommand(sArgs[0], (const char **)sArgs + 1, iArgs - 1);
			}
			FreeArray(sArgs, iArgs);
		}
	}

	CloseConsole();

	_endthreadex(0);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI ConsoleHandlerRoutine(DWORD dwCtrlType)
{
    if(dwCtrlType == CTRL_CLOSE_EVENT)
    {
		return true;
    }

    return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool OpenConsole(void)
{
	if(Console_Thread_Handle)
	{
		return false;
	}

    if(AllocConsole())
    {
        SetConsoleCtrlHandler(ConsoleHandlerRoutine, TRUE);

        char sConTitle[255];
        sprintf_s(sConTitle, sizeof(sConTitle), "%s [Version: %s]", gsTitleCaption, gsFileVersion);
        SetConsoleTitle(sConTitle);

		for(int iRetry = 0; iRetry < 50; iRetry++)
		{
			HWND hWnd = FindWindow(NULL, sConTitle);
			if(hWnd)
			{
				SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_DRAWFRAME);
				EnableDialogCloseButton(hWnd, false);
				break;
			}
			Sleep(1);
		}

        ConIO.StdIn  = GetStdHandle(STD_INPUT_HANDLE);
        ConIO.StdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        ConIO.StdErr = GetStdHandle(STD_ERROR_HANDLE);

        Console_Thread_Handle = (HANDLE)_beginthreadex(NULL, 0, p_ConsoleThreadProc, (LPVOID) NULL, 0, NULL);
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CloseConsole(void)
{
    if(FreeConsole())
    {
        CloseHandle(Console_Thread_Handle);
        CloseHandle(ConIO.StdIn);
        CloseHandle(ConIO.StdOut);
        CloseHandle(ConIO.StdErr);
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool WriteConEx(const char *Buffer, int Length)
{
    DWORD NumberOfBytesWritten = 0;
    return WriteFile(ConIO.StdOut, Buffer, Length, &NumberOfBytesWritten, 0) > 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int WriteCon(const char *sFormat, ...)
{
	va_list ArgList;
	va_start(ArgList, sFormat);

	int iMemoryRequired = _vscprintf(sFormat, ArgList);

	#ifdef _USE_GLOBAL_MEMPOOL
		char *sBuf = (char *) pMem->Allocate(sizeof(char), iMemoryRequired + 1);
	#else
		char *sBuf = (char *) calloc(sizeof(char), iMemoryRequired + 1);
	#endif

	int iSz = _vsprintf_s_l(sBuf, iMemoryRequired + 1, sFormat, NULL, ArgList);
	va_end(ArgList);

	bool bResult = WriteConEx(sBuf, iSz);

	#ifdef _USE_GLOBAL_MEMPOOL
		pMem->Free(sBuf);
	#else
		free(sBuf);
	#endif

	return bResult;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD GetConsoleInput(char *sRetBuffer, int iMaxLength)
{
    DWORD iNumberOfBytesRead = 0;
    ReadFile(ConIO.StdIn, sRetBuffer, iMaxLength, &iNumberOfBytesRead, 0);
    iNumberOfBytesRead = (iNumberOfBytesRead - 2);
    if(iNumberOfBytesRead > 0)
    {
        sRetBuffer[iNumberOfBytesRead] = '\0';
        return iNumberOfBytesRead;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
