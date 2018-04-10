#ifndef _ENTRY_H
#define _ENTRY_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../../@Libraries/Debug/Debug.H"
#include "../../../../@Libraries/CMemPool/CMemPool.H"
#include "../../../../@Libraries/NSWFL/NSWFL.H"

#include "../Resources/Resource.H"
#include "CCore.H"
#include "../../../../@Libraries/CTrace/CTrace.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern HINSTANCE ghAppInstance;

extern char gsAppCopyRight[64];
extern char gsFileVersion[10];
extern char gsTitleCaption[64];
extern char gsRegistryKey[255];

extern CMemPool *pMem;
extern CCore *pCore;
extern CTrace *pTrace;

extern char gsPath[MAX_PATH];
extern char gsDataPath[MAX_PATH];
extern DWORD dwManagementPort;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GetLastError(char *sOutErrorMessage, int iMaxOutErrorMessage, const char *sPrefix, const char *sSuffix);
bool GetLastError(char *sOutErrorMessage, int iMaxOutErrorMessage, const char *sPrefix);
bool GetLastError(char *sOutErrorMessage, int iMaxOutErrorMessage);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

