///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CRouterServer_Utilities_Cpp
#define _CRouterServer_Utilities_Cpp
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define _WIN32_WINNT 0x0500
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <WinSock2.H>
#include <WinSock.H>
#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <Time.H>
#include <Ws2tcpip.h>

#include "CRouterServer.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function resolves a Host Name to an IPv4 IP Address.
*/
bool CRouterServer::ResolveHost(const char *sHostName, char *sOutIP, int iMaxOutSz)
{
	struct in_addr Address;
	struct hostent *Host;

	memset(&Host, 0, sizeof(Host));
	memset(&Address, 0, sizeof(Address));

	if((Host = gethostbyname(sHostName)) == NULL)
	{
		return this->p_ThrowError(30);
	}

	memcpy(&Address.s_addr, (*Host).h_addr_list[0], (*Host).h_length);

	strcpy_s(sOutIP, iMaxOutSz, inet_ntoa(Address));

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns true if the passed in address is a valid IPv4 IP address.
*/
bool CRouterServer::IsIPAddress(const char *sString)
{

	int iStrSz = (int)strlen(sString);

	int iDotCount = 0;
	char sSeg[5];

	int iWPos = 0;

	if(iStrSz < 7 || iStrSz > 15 || inet_addr(sString) == INADDR_NONE)
	{
		return false;
	}
	else{
		for(int iRPos = 0; iRPos < iStrSz; iRPos++)
		{
			sSeg[iWPos++] = sString[iRPos];

			if(sString[iRPos] == '.')
			{
				int iVal = atol(sSeg);
				sSeg[iWPos] = '\0';

				if(iVal < 0 || iVal > 255)
				{
					return false;
				}

				iWPos = 0;
				iDotCount++;
			}

			if(iWPos > 3)
			{
				return false;
			}
		}
	}

	return iDotCount == 3;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
