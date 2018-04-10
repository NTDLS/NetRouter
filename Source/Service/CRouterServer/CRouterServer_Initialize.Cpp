///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CRouterServer_Initialize_Cpp
#define _CRouterServer_Initialize_Cpp
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

#include "../../../../@Libraries/NSWFL/NSWFL.h"

using namespace NSWFL::String;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to Initialize all of the class variables for a listening server.
*/
bool CRouterServer::Initialize(int iListenPort, int iMaxConnections, int iMaxDataPumpThreads, int iDataPumpSpawnThreshold, int iMaxResolverThreads, int iResolverSpawnThreshold)
{
	char sEventName[1024];

	this->p_bActive = false;
	this->p_bIsInitialized = true;

	memset(&this->p_ListenAddrs, 0, sizeof(this->p_ListenAddrs));
	memset(&this->p_ListenSockets, 0, sizeof(this->p_ListenSockets));

	if(!this->p_StartWindowsSocket())
	{
		return false;
	}

	if(iMaxConnections > this->p_iSocketLimit)
	{
		this->p_StopWindowsSocket();
		return this->p_ThrowError(32);
	}

	if(iMaxConnections > FD_SETSIZE)
	{
		this->p_StopWindowsSocket();
		return this->p_ThrowError(29);
	}

	this->p_iListenPort = iListenPort;
	this->p_iMaxConnections = iMaxConnections;
	this->p_iCurrentConnections = 0;
	this->p_iNextPeerID = 0;
	this->p_iMaxBufferSize = SSROUTE_DFLTBUFFERSIZE;
	this->p_iInitialBufferSize = SSROUTE_DFLTBUFFERSIZE;
	this->p_iMaxDataPumpThreads = iMaxDataPumpThreads;
	this->p_iDataPumpSpawnThreshold = iDataPumpSpawnThreshold;
	this->p_iMaxResolverThreads = iMaxResolverThreads;
	this->p_iResolverSpawnThreshold = iResolverSpawnThreshold;
	this->p_dMaxInactivityTime = 0;
	this->p_iLastAcceptingThreadNumber = -1;
	this->p_bAllowNewConnections = false;
	this->p_iResolverThreads = 0;
	this->p_iDataPumpThreads = 0;

	//Initialize the Critical Section objects.
	#ifndef _USE_ROUTE_GLOBAL_MEMPOOL
		InitializeCriticalSection(&this->p_MemoryLock);
	#endif

	InitializeCriticalSection(&this->p_PumpCS);
	InitializeCriticalSection(&this->p_ResolverCS);

	this->p_ResolverStack = new CStack(BEHAVIOR_FIFO);

	if((this->p_Clients = (CRouterClient *) this->Alloc(sizeof(CRouterClient), iMaxConnections)) == NULL)
	{
		this->p_StopWindowsSocket();
		return false;
	}

	NewGUID(sEventName, sizeof(sEventName));
	this->p_hResolverKickOffEvent = CreateEvent(NULL, FALSE, FALSE, sEventName);

	//Set all of our Client variables to their default values.
	for(int iSlot = 0; iSlot < iMaxConnections; iSlot++)
	{
		this->p_ResetClientSlot(&this->p_Clients[iSlot]);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to Destroy all of the class variables.
		Call this function when you are done with the class.
*/
bool CRouterServer::Destroy(void)
{
	if(this->p_bActive)
	{
		if(this->p_iCurrentConnections > 0) //Force disconnection.
		{
			for(int iSlot = 0; iSlot < this->p_iMaxConnections; iSlot++)
			{
				if(this->p_Clients[iSlot].IsConnected())
				{
					this->p_Clients[iSlot].Disconnect();
				}
			}
		}

		this->Stop(false);
	}

	this->p_bActive = false;
	this->p_bIsInitialized = false;

	if(this->p_Clients) this->Free(this->p_Clients);

	if(this->p_ListenSockets.Count > 0)
	{
		this->Free(this->p_ListenSockets.Socket);
	}

	this->ClearListenAddresses();
	delete this->p_ResolverStack;

	#ifndef _USE_ROUTE_GLOBAL_MEMPOOL
		DeleteCriticalSection(&this->p_MemoryLock);
	#endif

	CloseHandle(this->p_hResolverKickOffEvent);

	DeleteCriticalSection(&this->p_PumpCS);
	DeleteCriticalSection(&this->p_ResolverCS);

	return this->p_StopWindowsSocket();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
