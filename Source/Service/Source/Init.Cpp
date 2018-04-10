#ifndef _INIT_CPP
#define _INIT_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <WindowsX.H>
#include <ShellAPI.H>
#include <Stdio.H>
#include <Stdlib.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Init.H"
#include "Entry.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Registry;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool InitializeApp(void)
{
	pMem = new CMemPool();

	LoadString(ghAppInstance, IDS_APPCOPYRIGHT, gsAppCopyRight, sizeof(gsAppCopyRight));
    LoadString(ghAppInstance, IDS_FILEVERSION, gsFileVersion, sizeof(gsFileVersion));
    LoadString(ghAppInstance, IDS_TITLECAPTION, gsTitleCaption, sizeof(gsTitleCaption));
    LoadString(ghAppInstance, IDS_REGISTRYKEY, gsRegistryKey, sizeof(gsRegistryKey));

	WORD WSARequiredVersion = 0x0101; // Version 1.1
	WSADATA WSAData;
	WSADATA *WSAPointerData = &WSAData;

	pTrace = new CTrace(gsTitleCaption, gsFileVersion);

	pTrace->Start();

	if(!LoadRegistry())
	{
		pTrace->Trace("Failed to load registry values.");
	}

	if(WSAStartup(WSARequiredVersion, WSAPointerData) != 0)
	{
		pTrace->Trace("Failed to initialize windows socket.");
	}

    return(pTrace->Stop() == 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UninitializeApp(void)
{
	pTrace->Start();

	if(!SaveRegistry())
	{
		pTrace->Trace("Failed to save registry values.");
	}

	bool bResult = (pTrace->Stop() == 0);

	delete pTrace;
	delete pMem;

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LoadRegistry(void)
{
	DWORD dwSize = 0;
	DWORD dwBuffer = 0;

	dwSize = sizeof(gsPath);
	memset(gsPath, 0, dwSize);
	if(!Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "Path", gsPath, dwSize))
	{
		return false;
	}

	dwSize = sizeof(gsDataPath);
	memset(gsDataPath, 0, dwSize);
	if(!Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "DataPath", gsDataPath, dwSize))
	{
		return false;
	}

	if(!Get_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "ManagementPort", dwManagementPort))
	{
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SaveRegistry(void)
{
	bool bResult = true;

	//None of this needs to be saved:
	//bResult = bResult && Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "Path", gsPath);
	//bResult = bResult && Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "DataPath", gsDataPath);
	//bResult = bResult && Set_DWORDRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "ManagementPort", dwManagementPort);

	return bResult;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

