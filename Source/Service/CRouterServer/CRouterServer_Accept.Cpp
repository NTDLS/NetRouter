///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CRouterServer_Accept_Cpp
#define _CRouterServer_Accept_Cpp
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

#include "../Source/CCore.H"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function accepts a new connection from a remote peer.
	This function should not be called directly!
*/
bool CRouterServer::p_Accept(SOCKET listenSocket, ROUTEPUMPTHREADINFO *pThreadInfo)
{
	if(!this->p_bAllowNewConnections)
	{
		return false;
	}

	int iLength = 0;
	SOCKET sckLocalSocket = 0;

	SOCKADDR_IN RemoteClientSocketAddress;

	iLength = sizeof(SOCKADDR);
	//##(ROUTER_MAJOR_STEP:1)##############################################################################
	//## Accept a connection. Socket is non, blocking so if no connection is present we will not lock.
	//#####################################################################################################
	if((sckLocalSocket = accept(listenSocket, (SOCKADDR*)&RemoteClientSocketAddress, &iLength)) == SOCKET_ERROR)
	{
		int iLastError = WSAGetLastError();
		if(iLastError == WSAEWOULDBLOCK || iLastError == WSAEINPROGRESS || iLastError == WSAEALREADY)
		{
			return false;
		}
		return this->p_ThrowError(3);
	}
	else if(sckLocalSocket == 0)
	{
		return this->p_ThrowError(4);
	}

	if(this->p_iCurrentConnections >= this->p_iMaxConnections)
	{
		shutdown(sckLocalSocket, SD_BOTH);
		closesocket(sckLocalSocket);
		return this->p_ThrowError(5);
	}

	int iSlot = 0;
	CRouterClient *pClient = this->p_GetFreeSocketSlot(&iSlot, pThreadInfo);
	if(!pClient)
	{
		shutdown(sckLocalSocket, SD_BOTH);
		closesocket(sckLocalSocket);
		return this->p_ThrowError(6);
	}

	if(!this->p_InitializeClientVariables(pClient))
	{
		shutdown(sckLocalSocket, SD_BOTH);
		closesocket(sckLocalSocket);

		this->p_ResetClientSlot(pClient);

		return this->p_ThrowError(7, pClient);
	}

	pClient->iSlot = iSlot;
	pClient->bWasConnect = false;
	pClient->bConnected = true;
	pClient->bDisconnect = false;
	pClient->Socket = sckLocalSocket;
	pClient->iPeerID = this->p_iNextPeerID++;
	pClient->iPeerPort = ntohs(RemoteClientSocketAddress.sin_port);
 	pClient->bWaitingOnResolver = true;
	strcpy_s(pClient->sPeerAddress,
		sizeof(pClient->sPeerAddress),
		inet_ntoa(RemoteClientSocketAddress.sin_addr));

	EnterCriticalSection(&this->p_ResolverCS);
	ROUTERESOLVERSTACKITEM RSI;
	RSI.pRouterClient = pClient;
	RSI.PumpThreadInfo = pThreadInfo;

	//##(ROUTER_MAJOR_STEP:2)##############################################################################
	//## Add the new connection to the "Connection Acquisition / Resolver" queue.
	//#####################################################################################################
	this->p_ResolverStack->Push(&RSI, sizeof(RSI));

	SetEvent(this->p_hResolverKickOffEvent);
	LeaveCriticalSection(&this->p_ResolverCS);

	EnterCriticalSection(&this->p_PumpCS);
	pThreadInfo->ConnectionIndexes->push_back(iSlot);
	this->p_iCurrentConnections++;
	LeaveCriticalSection(&this->p_PumpCS);

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
