///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CRouterServer_ClassProperties_Cpp
#define _CRouterServer_ClassProperties_Cpp
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
	This function returns a pointer to one of the client slots by its Peer ID.
*/
CRouterClient *CRouterServer::Peer(int iPeerID)
{
	for(int iSlot = 0; iSlot < this->p_iMaxConnections; iSlot++)
	{
		CRouterClient *pClient = this->Client(iSlot);

		if(pClient && pClient->iPeerID == iPeerID)
		{
			return &this->p_Clients[iSlot];
		}
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function returns a pointer to one of the client slots by is Client ID
*/
CRouterClient *CRouterServer::Client(int iSlot)
{
	if(iSlot < 0 || iSlot >= this->p_iMaxConnections)
	{
		return NULL;
	}
	return &this->p_Clients[iSlot];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function can be used to poll for connected clients (blocking).

	Call and it will pass out a pointer to a qualified
		(connected and not pending disconnection) connected client.
*/
CRouterClient *CRouterServer::GetNextConnectedClient(void)
{
	static int iLastClient = 0;
	int iStartingPoint = iLastClient;

	if(iLastClient < 0 || iLastClient >= this->p_iMaxConnections)
	{
		iLastClient = 0;
	}

	while(this->IsActive())
	{
		//Loop from where we left off last time, to the MAX count of clients.
		for(;iLastClient < this->p_iMaxConnections; iLastClient++)
		{
			if(this->p_Clients[iLastClient].IsConnected())
			{
				return &this->p_Clients[iLastClient++];
			}
		}
		iLastClient = 0;

		//Loop from slot 0 to where the top loop started, this makes
		//	sure we will never Sleep() while clients are connected.
		for(;iLastClient < iStartingPoint; iLastClient++)
		{
			if(this->p_Clients[iLastClient].IsConnected())
			{
				return &this->p_Clients[iLastClient++];
			}
		}
		iStartingPoint = 0;

		Sleep(1);
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	This function can be used to poll for connected clients (blocking).

	Pass in a pointer to an integer, and it will pass out the client
		slot number of a qualified (connected and not pending disconnection) connected client.
*/
bool CRouterServer::GetNextConnectedClient(int *iSlot)
{
	static int iLastClient = 0;
	int iStartingPoint = iLastClient;

	if(iLastClient < 0 || iLastClient >= this->p_iMaxConnections)
	{
		iLastClient = 0;
	}

	while(this->IsActive())
	{
		//Loop from where we left off last time, to the MAX count of clients.
		for(;iLastClient < this->p_iMaxConnections; iLastClient++)
		{
			if(this->p_Clients[iLastClient].IsConnected())
			{
				*iSlot = iLastClient++;
				return true;
			}
		}
		iLastClient = 0;

		//Loop from slot 0 to where the top loop started, this makes
		//	sure we will never Sleep() while clients are connected.
		for(;iLastClient < iStartingPoint; iLastClient++)
		{
			if(this->p_Clients[iLastClient].IsConnected())
			{
				*iSlot = iLastClient++;
				return true;
			}
		}
		iStartingPoint = 0;

		Sleep(1);
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function can be used to poll for connected clients (non-blocking).

	Call and it will pass out a pointer to a qualified
		(connected and not pending disconnection) connected client.
*/
CRouterClient *CRouterServer::GetConnectedClient(void)
{
	static int iLastClient = 0;
	int iStartingPoint = iLastClient;

	if(iLastClient < 0 || iLastClient >= this->p_iMaxConnections)
	{
		iLastClient = 0;
	}

	//Loop from where we left off last time, to the MAX count of clients.
	for(;iLastClient < this->p_iMaxConnections; iLastClient++)
	{
		if(this->p_Clients[iLastClient].IsConnected())
		{
			return &this->p_Clients[iLastClient++];
		}
	}
	iLastClient = 0;

	//Loop from slot 0 to where the top loop started, this makes
	//	sure we will never Sleep() while clients are connected.
	for(;iLastClient < iStartingPoint; iLastClient++)
	{
		if(this->p_Clients[iLastClient].IsConnected())
		{
			return &this->p_Clients[iLastClient++];
		}
	}
	iStartingPoint = 0;

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	This function can be used to poll for connected clients (non-blocking).

	Pass in a pointer to an integer, and it will pass out the client
		slot number of a qualified (connected and not pending disconnection) connected client.
*/
bool CRouterServer::GetConnectedClient(int *iSlot)
{
	static int iLastClient = 0;
	int iStartingPoint = iLastClient;

	if(iLastClient < 0 || iLastClient >= this->p_iMaxConnections)
	{
		iLastClient = 0;
	}

	//Loop from where we left off last time, to the MAX count of clients.
	for(;iLastClient < this->p_iMaxConnections; iLastClient++)
	{
		if(this->p_Clients[iLastClient].IsConnected())
		{
			*iSlot = iLastClient++;
			return true;
		}
	}
	iLastClient = 0;

	//Loop from slot 0 to where the top loop started, this makes
	//	sure we will never Sleep() while clients are connected.
	for(;iLastClient < iStartingPoint; iLastClient++)
	{
		if(this->p_Clients[iLastClient].IsConnected())
		{
			*iSlot = iLastClient++;
			return true;
		}
	}
	iStartingPoint = 0;

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function sets the buffer size.
*/
void CRouterServer::MaxBufferSize(int iSize)
{
	if(this->p_iMaxBufferSize > 0)
	{
		this->p_iMaxBufferSize = iSize;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function returns the buffer size.
*/
int CRouterServer::MaxBufferSize(void)
{
	return this->p_iMaxBufferSize;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function sets the buffer size.
*/
void CRouterServer::InitialBufferSize(int iSize)
{
	if(this->p_iInitialBufferSize > 0)
	{
		this->p_iInitialBufferSize = iSize;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function returns the buffer size.
*/
int CRouterServer::InitialBufferSize(void)
{
	return this->p_iInitialBufferSize;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function returns listen port for the server.
*/
int CRouterServer::ListenPort(void)
{
	return this->p_iListenPort;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function sets the listen port then returns the previous port for the server.
*/
int CRouterServer::ListenPort(int iNewPort)
{
	int iOldPort = this->p_iListenPort;
	this->p_iListenPort = iNewPort;
	return iOldPort;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function returns true if the client / server is running.
*/
bool CRouterServer::IsActive(void)
{
	return this->p_bActive;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function returns true if the class has been initilized.
*/
bool CRouterServer::IsInitialized(void)
{
	return this->p_bIsInitialized;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to set enable the Accept / Connect functionality.
*/
void CRouterServer::AllowNewConnections(bool bAllow)
{
	this->p_bAllowNewConnections = bAllow;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns the number of defined pump threads.
*/
int CRouterServer::DataPumpThreads(void)
{
	return this->p_iDataPumpThreads;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns the pointer to used defined per class instance data.
*/
VOID *CRouterServer::UserData(void)
{
	return this->pUserData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns the pointer to used defined per class instance data.
*/
void CRouterServer::UserData(VOID *pData)
{
	this->pUserData = pData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to set p_iNextPeerID to a new value. 
*/
void CRouterServer::NextPeerID(int iPeerID)
{
	this->p_iNextPeerID = iPeerID;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to get the value of p_iNextPeerID. 
*/
int CRouterServer::NextPeerID(void)
{
	return this->p_iNextPeerID;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to get the value of p_iMaxConnections. 
*/
int CRouterServer::MaxConnections(void)
{
	return this->p_iMaxConnections;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to get the count of currently connected clients.
*/
int CRouterServer::CurrentConnections(void)
{
	return this->p_iCurrentConnections;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns the current Max Inactivity Time brfore a peer is disconnected.
*/
double CRouterServer::MaxInactivityTime(void)
{
	return this->p_dMaxInactivityTime;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Sets the Max Inactivity Time brfore a peer is disconnected.
*/
void CRouterServer::MaxInactivityTime(double dSeconds)
{
	this->p_dMaxInactivityTime = dSeconds;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
