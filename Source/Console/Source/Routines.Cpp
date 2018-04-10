///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _ROUTINES_CPP
#define _ROUTINES_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <WindowsX.H>
#include <ShellAPI.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <ShlObj.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Entry.H"
#include "Routines.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::String;
using namespace NSWFL::Conversion;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GetLastError(char *sOutErrorMessage, int iMaxOutErrorMessage, const char *sPrefix, const char *sSuffix)
{
	if(GetLastError(sOutErrorMessage, iMaxOutErrorMessage))
	{
		int iAllocate = (int)strlen(sOutErrorMessage) + 1;
		char *sTemp = (char *)pMem->Allocate(sizeof(char), iAllocate);
		strcpy_s(sTemp, iAllocate, sOutErrorMessage);
		Trim(sTemp, iAllocate - 1);

		strcpy_s(sOutErrorMessage, iMaxOutErrorMessage, sPrefix);
		strcat_s(sOutErrorMessage, iMaxOutErrorMessage, sTemp);
		strcat_s(sOutErrorMessage, iMaxOutErrorMessage, sSuffix);
		pMem->Free(sTemp);
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
		char *sTemp = (char *)pMem->Allocate(sizeof(char), iAllocate);
		strcpy_s(sTemp, iAllocate, sOutErrorMessage);
		Trim(sTemp, iAllocate - 1);

		strcpy_s(sOutErrorMessage, iMaxOutErrorMessage, sPrefix);
		strcat_s(sOutErrorMessage, iMaxOutErrorMessage, sTemp);
		pMem->Free(sTemp);
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

HANDLE NTLogin(const char *sUsername, const char *sDomain, const char *sPassword, char *sOutErrorMessage, int iMaxOutErrorMessage)
{
	HANDLE hToken = NULL;

	/*
	#define LOGON32_LOGON_INTERACTIVE       2
	#define LOGON32_LOGON_NETWORK           3
	#define LOGON32_LOGON_BATCH             4
	#define LOGON32_LOGON_SERVICE           5
	#define LOGON32_LOGON_UNLOCK            7
	#define LOGON32_LOGON_NETWORK_CLEARTEXT 8
	#define LOGON32_LOGON_NEW_CREDENTIALS   9
	*/

	if(!LogonUser(sUsername, sDomain, sPassword,
		LOGON32_LOGON_NEW_CREDENTIALS, LOGON32_PROVIDER_WINNT50, &hToken))
	{
		if(!GetLastError(sOutErrorMessage, iMaxOutErrorMessage))
		{
			strcpy_s(sOutErrorMessage, iMaxOutErrorMessage, "Unknown logon error.");
		}

		return NULL;
	}
	return hToken;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GetNextToken(const char *sBuf, int iBufSz, char *&sOutToken, int *iInOutRPos)
{
	int iRPos = *iInOutRPos;
	int iWPos = 0;
	int iAlloc = 0;

	SkipWhiteSpaces(sBuf, iBufSz, &iRPos);

	while(!IsWhiteSpace(sBuf[iRPos]) && iRPos < iBufSz)
	{
		iRPos++;
		iAlloc++;
	}

	sOutToken = (char *) pMem->Allocate(sizeof(char), iAlloc + 1);

	iRPos -= iAlloc;

	if(sOutToken)
	{
		for(iWPos = 0; iWPos < iAlloc; iWPos++)
		{
			sOutToken[iWPos] = sBuf[iRPos++];
		}

		sOutToken[iWPos] = '\0';
	}

	SkipWhiteSpaces(sBuf, iBufSz, &iRPos);

	*iInOutRPos = iRPos;

	return iWPos; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IsValidIPAddressV4(const char *sIP)
{
	char **sIPs = Split(sIP, ".");

	if(UBound(sIPs) != 4)
	{
		FreeArray(sIPs);
		return false;
	}

	int iOctLen = 0;

	for(unsigned short iOct = 0; iOct < 4; iOct++)
	{
		if((iOctLen = (int)strlen(sIPs[iOct])) > 3)
		{
			FreeArray(sIPs);
			return false;
		}

		for(int iPos = 0; iPos < iOctLen; iPos++)
		{
			if(!IsNumeric(sIPs[iOct][iPos]))
			{
				FreeArray(sIPs);
				return false;
			}
		}

		if(atol(sIPs[iOct]) > 255)
		{
			FreeArray(sIPs);
			return false;
		}
	}

	FreeArray(sIPs);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
