///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CRouterServer_StartStop_Cpp
#define _CRouterServer_StartStop_Cpp
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define _WIN32_WINNT 0x0500
#define WIN32_LEAN_AND_MEAN 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <WinSock2.H>
#include <WinSock.H>
#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <Time.H>

#include "CRouterServer.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to setup the socket server to listen on specific IP addresses.

	If this function is not called, the server will listen on all IP addresses.

*/
bool CRouterServer::AddListenAddress(const char *sIPv4)
{
	if(this->IsIPAddress(sIPv4))
	{
		int iMemory = (this->p_ListenAddrs.Count + 1) * sizeof(ROUTELISTENADDRESS);
		this->p_ListenAddrs.Address = (ROUTELISTENADDRESS *) this->ReAlloc(this->p_ListenAddrs.Address, iMemory);

		ROUTELISTENADDRESS *pAddress = &this->p_ListenAddrs.Address[this->p_ListenAddrs.Count++];

		strcpy_s(pAddress->IP, sizeof(pAddress->IP), sIPv4);

		return true;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to remove all specific IP listen addresses.
*/
void CRouterServer::ClearListenAddresses(void)
{
	if(this->p_ListenAddrs.Count > 0 && this->p_ListenAddrs.Address)
	{
		this->Free(this->p_ListenAddrs.Address);
	}

	memset(&this->p_ListenAddrs, 0, sizeof(this->p_ListenAddrs));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to Start the client or server.

	If the iListenPort is non-zero a listening port will be opened to
	accept incomming connections.
*/
bool CRouterServer::Start(void)
{
	//Are we going to be listening on any port?
	if(this->p_iListenPort > 0)
	{
		//Open our listening port.
		if(!this->p_OpenListenSocket(this->p_iListenPort))
		{
			this->p_CloseListenSocket();

			return false;
		}
	}

	this->p_AddDataPump();
	this->p_AddResolverThread();

	this->p_bActive = true;
	this->AllowNewConnections(true);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to Stop the server or client. If there is a listening socket
	it will be shutdown and closed.

	bForce tells the stop() routine to stop the service even if the OnStop() routine returns false.
*/
bool CRouterServer::Stop(bool bForce)
{
	this->AllowNewConnections(false);

	if(this->p_iCurrentConnections > 0) //Friendly disconnection.
	{
		for(int iSlot = 0; iSlot < this->p_iMaxConnections; iSlot++)
		{
			this->p_Clients[iSlot].bAbortConnection = true;
			if(this->p_Clients[iSlot].IsConnected())
			{
				this->p_Clients[iSlot].Disconnect();
			}
		}

		//The clients have to be dropped before the pump thread can be stopped
		//	because it is the pump thread which actually disconnects the peers.
		for(int iCount = SSROUTE_STOPFORCETIMEOUT; iCount > 0; iCount--)
		{
			if(this->p_iCurrentConnections == 0)
			{
				break;
			}
			Sleep(1);
		}
	}

	this->p_StopResolverThreads();
	this->p_StopDataPumps();
	this->p_CloseListenSocket();

	this->p_bActive = false;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
