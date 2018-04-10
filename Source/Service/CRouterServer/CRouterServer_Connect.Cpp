///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CRouterServer_Connect_Cpp
#define _CRouterServer_Connect_Cpp
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

#include "CRouterServer.H"
#include "../../../../@Libraries/NSWFL/NSWFL.H"
using namespace NSWFL::String;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	Establishes a new connection to a remote peer.

	This Function is not multi-thread safe, do not use this function directly.
*/
bool CRouterServer::p_Connect(const char *sIPAddress, int iRemotePort, CRouterClient **pOutClient, ROUTEPUMPTHREADINFO *pThreadInfo)
{
	if(!this->p_bAllowNewConnections)
	{
		return false;
	}

	SOCKADDR_IN RemoteServerSocketAddress;

	SOCKET sckLocalSocket = 0;

	if(this->p_iCurrentConnections == this->p_iMaxConnections)
	{
		return this->p_ThrowError(26);
	}

	RemoteServerSocketAddress.sin_family	  = AF_INET;
	RemoteServerSocketAddress.sin_port		  = htons(iRemotePort);
	RemoteServerSocketAddress.sin_addr.s_addr = inet_addr(sIPAddress);

	sckLocalSocket = socket(AF_INET, SOCK_STREAM, 0);

	if(connect(sckLocalSocket,
		(struct sockaddr *) &RemoteServerSocketAddress,
		sizeof(RemoteServerSocketAddress)) == SOCKET_ERROR)
	{
		shutdown(sckLocalSocket, SD_BOTH);
		closesocket(sckLocalSocket);
		return this->p_ThrowError(33);
	}

	int iSlot = 0;
	CRouterClient *pClient = this->p_GetFreeSocketSlot(&iSlot, pThreadInfo);
	*pOutClient = pClient;

	if(!pClient)
	{
		shutdown(sckLocalSocket, SD_BOTH);
		closesocket(sckLocalSocket);
		return this->p_ThrowError(27);
	}

	if(!this->p_InitializeClientVariables(pClient))
	{
		shutdown(sckLocalSocket, SD_BOTH);
		closesocket(sckLocalSocket);

		this->p_ResetClientSlot(pClient);
		return this->p_ThrowError(28);
	}

	pClient->iSlot = iSlot;
	pClient->bWasConnect = true;
	pClient->bConnected = true;
	pClient->bDisconnect = false;
	pClient->Socket = sckLocalSocket;
	pClient->iPeerID = this->p_iNextPeerID++;
	pClient->iPeerPort = iRemotePort;
	strcpy_s(pClient->sPeerAddress, sizeof(pClient->sPeerAddress), sIPAddress);

	EnterCriticalSection(&this->p_PumpCS);
	pThreadInfo->ConnectionIndexes->push_back(iSlot);
	this->p_iCurrentConnections++;
	LeaveCriticalSection(&this->p_PumpCS);

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Establishes a new connection to a remote peer by IP or Hostname then returns.
*/
bool CRouterServer::Connect(const char *sHostOrIP, int iRemotePort, CRouterClient **pOutClient, ROUTEPUMPTHREADINFO *pThreadInfo)
{
	if(this->IsIPAddress(sHostOrIP))
	{
		return this->ConnectByIP(sHostOrIP, iRemotePort, pOutClient, pThreadInfo);
	}
	else{
		return this->ConnectByHost(sHostOrIP, iRemotePort, pOutClient, pThreadInfo);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Establishes a new connection to a remote peer by Hostname then returns.
*/
bool CRouterServer::ConnectByHost(const char *sHostName, int iRemotePort, CRouterClient **pOutClient, ROUTEPUMPTHREADINFO *pThreadInfo)
{
	char sIPAddress[16];
	if(this->ResolveHost(sHostName, sIPAddress, sizeof(sIPAddress)))
	{
		return this->ConnectByIP(sIPAddress, iRemotePort, pOutClient, pThreadInfo);
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Establishes a new connection to a remote peer by IP then returns.
*/
bool CRouterServer::ConnectByIP(const char *sIPAddress, int iRemotePort, CRouterClient **pOutClient, ROUTEPUMPTHREADINFO *pThreadInfo)
{
	return this->p_Connect(sIPAddress, iRemotePort, pOutClient, pThreadInfo);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
