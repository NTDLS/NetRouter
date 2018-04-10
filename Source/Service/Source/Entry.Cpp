#ifndef _ENTRY_CPP
#define _ENTRY_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#ifdef _DEBUG
#include <fcntl.h>
#include <io.h>
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Entry.H"
#include "Init.H"
#include "CCore.H"
#include "CManagement.H"
#include "ServiceConsole.H"

#include "../../../../@Libraries/CService/CService.H"
#include "../../../../@Libraries/CSocketServer/CSocketServer.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;

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
CCore *pCore;
CTrace *pTrace;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IsApplicationRunning(void)
{
	bool bResult = false;
	HANDLE hMutex = NULL;
	SECURITY_DESCRIPTOR SD;
	SECURITY_ATTRIBUTES SA;

	if(!InitializeSecurityDescriptor(&SD, SECURITY_DESCRIPTOR_REVISION))
	{
		return false;
	}

	if(!SetSecurityDescriptorDacl(&SD, TRUE, 0, FALSE))
	{
		return false;
	}

	SA.nLength				= sizeof(SA);
	SA.lpSecurityDescriptor = &SD;
	SA.bInheritHandle		= FALSE;

	char sMutex[255];
	sprintf_s(sMutex, "Global\\%s_NetworkDLS", gsTitleCaption);
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

bool ServiceProcedure(LPVOID lpVoid)
{
	HANDLE MutexHandle = NULL;

	//-------------------------------------------------------------------------------
	SECURITY_DESCRIPTOR SD;
	SECURITY_ATTRIBUTES SA;

	if(InitializeSecurityDescriptor(&SD, SECURITY_DESCRIPTOR_REVISION) == FALSE)
		return false;

	if(SetSecurityDescriptorDacl(&SD, TRUE, 0, FALSE) == FALSE)
		return false;

	SA.nLength				= sizeof(SA);
	SA.lpSecurityDescriptor = &SD;
	SA.bInheritHandle		= FALSE;

	pTrace->Start();

	char sMutex[255];
	sprintf_s(sMutex, "Global\\%s_NetworkDLS", gsTitleCaption);
	if((MutexHandle = CreateMutex(&SA, FALSE, sMutex)) != NULL)
	{
		if(GetLastError() == ERROR_ALREADY_EXISTS)
		{
			pTrace->Trace("The application already running.");
			pTrace->Stop();
			return false;
		}
	}

	if((pCore = new CCore))
	{
		gManage = new CManagement();
		if(gManage->ManagementInitialize())
		{
			pTrace->Stop();

			//OpenConsole();

			gService.KeepServiceRunning();
			delete gManage;
		}
		else {
			pTrace->Trace("Failed to initialize the management interface, is the service already running?");
		}

		pTrace->Stop();

		delete pCore;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int ProcessCommandLine(const char *sCmdLine)
{
	if(_strcmpi(sCmdLine, "/RunService") == 0 || _strcmpi(sCmdLine, "/Standalone") == 0 )
	{
		int iResult = 0;

		if(!IsApplicationRunning() && gService.IsActive() == _SVC_NO_)
		{
			if(_strcmpi(sCmdLine, "/RunService") == 0 )
			{
				if(gService.RunAsService() == _SVC_SUCCESS_)
				{
					iResult = 0;
				}
				else{
					iResult = 1;
				}
			}
			else{
				gService.RunAsStandAlone();
			}
		}

		return iResult;
	}
	//------------------------------------------------------------------------------------------
	else if(_strcmpi(sCmdLine, "/Install") == 0 )
	{
		if(gService.IsInstalled() == _SVC_YES_)
		{
    		//MessageBox(NULL, "The service is already installed.", gsTitleCaption, MB_ICONINFORMATION);
			return 0;
		}

		char sApplication[257];
		sprintf_s(sApplication, sizeof(sApplication), "NetworkDLS %s", gsTitleCaption);
		if(gService.Install(sApplication) == _SVC_SUCCESS_ )
		{
			gService.SetDescription("Provides TCP/IP routing services on NT based Operating Systems.");
			return 0;
		}
		else{
			MessageBox(NULL, "Failed to install the service.", gsTitleCaption, MB_ICONWARNING);
			return 0;
		}

		return 0;
	}
	//------------------------------------------------------------------------------------------
	else if(_strcmpi(sCmdLine, "/Delete") == 0 )
	{
		if(gService.IsInstalled() == _SVC_NO_)
		{
			return 0;
		}
		else if(gService.IsActive() == _SVC_YES_)
		{
			gService.Stop(); // Failure would not be an error.
			return 0;
		}
    	else if(gService.Uninstall() != _SVC_SUCCESS_)
    	{
    		MessageBox(NULL, "Failed to mark the service for deletion.", gsTitleCaption, MB_ICONWARNING);
			return 0;
		}

		return 0;
	}
	//------------------------------------------------------------------------------------------
	else if(_strcmpi(sCmdLine, "/Start") == 0 )
	{
		if(IsApplicationRunning())
		{
			return 0;
		}
		else if(gService.IsInstalled() == _SVC_NO_)
		{
    		MessageBox(NULL, "The service is not installed.", gsTitleCaption, MB_ICONWARNING);
			return 0;
		}
		else if(gService.IsActive() == _SVC_YES_)
		{
			return 0;
		}
		else if(gService.Start() != _SVC_SUCCESS_)
		{
    		MessageBox(NULL, "Failed to start the service.", gsTitleCaption, MB_ICONWARNING);
   			return 0;
		}

		return 0;
	}
	//------------------------------------------------------------------------------------------
	else if(_strcmpi(sCmdLine, "/Stop") == 0 )
	{
		if(gService.IsInstalled() == _SVC_NO_)
		{
    		MessageBox(NULL, "The service is not installed.", gsTitleCaption, MB_ICONWARNING);
			return 0;
		}
		else if(gService.IsActive() == _SVC_NO_)
		{
			return 0; //Already stopped.
		}
    	else if(gService.Stop() != _SVC_SUCCESS_)
    	{
    		MessageBox(NULL, "Failed to stop the service.", gsTitleCaption, MB_ICONWARNING);
			return 0;
		}
		return 0;
	}
	//------------------------------------------------------------------------------------------
	else if(_strcmpi(sCmdLine, "/SilentStart") == 0 )
	{
		if(IsApplicationRunning())
		{
			return 0;
		}
		else if(gService.IsInstalled() == _SVC_NO_)
		{
			return 0;
		}
		else if(gService.IsActive() == _SVC_YES_)
		{
			return 0;
		}
		else if(gService.Start() != _SVC_SUCCESS_)
		{
   			return 0;
		}

		return 0;
	}
	//------------------------------------------------------------------------------------------
	else if(_strcmpi(sCmdLine, "/SilentStop") == 0 )
	{
		if(gService.IsInstalled() == _SVC_NO_)
		{
			return 0;
		}
		else if(gService.IsActive() == _SVC_NO_)
		{
			return 0; //Already stopped.
		}
    	else if(gService.Stop() != _SVC_SUCCESS_)
    	{
			return 0;
		}
		return 0;
	}
	//------------------------------------------------------------------------------------------
	else if(_strcmpi(sCmdLine, "/VerboseStart") == 0 )
	{
		if(IsApplicationRunning())
		{
			return 0;
		}
		else if(gService.IsInstalled() == _SVC_NO_)
		{
    		MessageBox(NULL, "The service is not installed.", gsTitleCaption, MB_ICONWARNING);
			return 0;
		}
		else if(gService.IsActive() == _SVC_YES_)
		{
    		MessageBox(NULL, "The service is already running.", gsTitleCaption, MB_ICONINFORMATION);
			return 0;
		}
		else if(gService.Start() != _SVC_SUCCESS_)
		{
    		MessageBox(NULL, "Failed to start the service.", gsTitleCaption, MB_ICONWARNING);
   			return 0;
		}
		else{
    		MessageBox(NULL, "The service was successfully started.", gsTitleCaption, MB_ICONINFORMATION);
		}

		return 0;
	}
	//------------------------------------------------------------------------------------------
	else if(_strcmpi(sCmdLine, "/VerboseStop") == 0 )
	{
		if(gService.IsInstalled() == _SVC_NO_)
		{
    		MessageBox(NULL, "The service is not installed.", gsTitleCaption, MB_ICONWARNING);
			return 0;
		}
		else if(gService.IsActive() == _SVC_NO_)
		{
    		MessageBox(NULL, "The service is not running.", gsTitleCaption, MB_ICONINFORMATION);
			return 0;
		}
    	else if(gService.Stop() != _SVC_SUCCESS_)
    	{
    		MessageBox(NULL, "Failed to stop the service.", gsTitleCaption, MB_ICONWARNING);
			return 0;
		}
		else{
    		MessageBox(NULL, "The service was successfully stopped.", gsTitleCaption, MB_ICONINFORMATION);
		}
		return 0;
	}
	//------------------------------------------------------------------------------------------

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevious, LPSTR sCmdLine, int iCmdShow)
{
	ghAppInstance = GetModuleHandle(NULL);

#ifdef _DEBUG
	AllocConsole();
	RedirectIO(stdout, GetStdHandle(STD_OUTPUT_HANDLE));
	RedirectIO(stderr, GetStdHandle(STD_OUTPUT_HANDLE));
#endif

    int iReturnVal = 0;

	if(!InitializeApp())
    {
        return 1;
    }

	gService.Initialize("NetworkDLS NetRouter", &ServiceProcedure, NULL);

	if(strlen(sCmdLine) > 0)
	{
		iReturnVal = ProcessCommandLine(sCmdLine);
	}
	else {

#if _DEBUG
	if(IsDebuggerPresent())
	{
		iReturnVal = ProcessCommandLine("/Standalone");
	}
	else {
		MessageBox(NULL, "This service was not intended to run standalone. To force this mode, pass the /Standalone parameter.", gsTitleCaption, MB_ICONINFORMATION);
	}
#else
		MessageBox(NULL, "This service was not intended to run standalone. To force this mode, pass the /Standalone parameter.", gsTitleCaption, MB_ICONINFORMATION);
#endif
	}

	UninitializeApp();

	return iReturnVal;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GetLastError(char *sOutErrorMessage, int iMaxOutErrorMessage, const char *sPrefix, const char *sSuffix)
{
	if(GetLastError(sOutErrorMessage, iMaxOutErrorMessage))
	{
		int iAllocate = (int)strlen(sOutErrorMessage) + 1;
		char *sTemp = (char *)calloc(sizeof(char), iAllocate);
		strcpy_s(sTemp, iAllocate, sOutErrorMessage);
		Trim(sTemp, iAllocate - 1);

		strcpy_s(sOutErrorMessage, iMaxOutErrorMessage, sPrefix);
		strcat_s(sOutErrorMessage, iMaxOutErrorMessage, sTemp);
		strcat_s(sOutErrorMessage, iMaxOutErrorMessage, sSuffix);
		free(sTemp);
		return true;
	}
	else{
		strcpy_s(sOutErrorMessage, iMaxOutErrorMessage, sPrefix);
		strcat_s(sOutErrorMessage, iMaxOutErrorMessage, sSuffix);
		return false;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GetLastError(char *sOutErrorMessage, int iMaxOutErrorMessage, const char *sPrefix)
{
	if(GetLastError(sOutErrorMessage, iMaxOutErrorMessage))
	{
		int iAllocate = (int)strlen(sOutErrorMessage) + 1;
		char *sTemp = (char *)calloc(sizeof(char), iAllocate);
		strcpy_s(sTemp, iAllocate, sOutErrorMessage);
		Trim(sTemp, iAllocate - 1);

		strcpy_s(sOutErrorMessage, iMaxOutErrorMessage, sPrefix);
		strcat_s(sOutErrorMessage, iMaxOutErrorMessage, sTemp);
		free(sTemp);
		return true;
	}
	else{
		strcpy_s(sOutErrorMessage, iMaxOutErrorMessage, sPrefix);
		return false;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Retreives a textual error message from a threads GetLastError() call.
	Returns TRUE if error information was received.
*/
bool GetLastError(char *sOutErrorMessage, int iMaxOutErrorMessage)
{
	DWORD dwLastError = GetLastError();
	if(dwLastError)
	{
		char *sErrorMessage = NULL;

		if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, dwLastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&sErrorMessage, 0, NULL))
		{
			strcpy_s(sOutErrorMessage, iMaxOutErrorMessage, sErrorMessage);
			LocalFree(sErrorMessage);
			return true;
		}

		strcpy_s(sOutErrorMessage, iMaxOutErrorMessage, "Unknown logon error.");
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif


