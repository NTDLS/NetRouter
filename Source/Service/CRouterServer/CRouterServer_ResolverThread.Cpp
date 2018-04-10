///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CRouterServer_ResolverThread_Cpp
#define _CRouterServer_ResolverThread_Cpp
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
#include <Process.H>

#include "CRouterServer.H"
#include "../../../../@Libraries/NSWFL/NSWFL.h"
#include "../Source/CCore.H"

using namespace NSWFL::String;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to Add and Start a host Resolver / connection acquisition thread.
*/
void CRouterServer::p_AddResolverThread(void)
{
	EnterCriticalSection(&this->p_ResolverCS);

	this->p_bShutdownResolverThread = false;

	//We allocate this at MAX because we do not want to risk reloacating it later.
	if(this->p_RouteResolverThreadInfo == NULL)
	{
		this->p_RouteResolverThreadInfo = (ROUTERESOLVERTHREADINFO *)
			this->Alloc(sizeof(ROUTERESOLVERTHREADINFO), this->p_iMaxResolverThreads);
	}

	char sEventName[255];
	NewGUID(sEventName, sizeof(sEventName));

	memset(&this->p_RouteResolverThreadInfo[this->p_iResolverThreads], 0, sizeof(ROUTERESOLVERTHREADINFO));

	this->p_RouteResolverThreadInfo[this->p_iResolverThreads].pSockRouter = this;
	this->p_RouteResolverThreadInfo[this->p_iResolverThreads].StartedEvent = CreateEvent(NULL, TRUE, FALSE, sEventName);
	this->p_RouteResolverThreadInfo[this->p_iResolverThreads].ThreadNumber = this->p_iResolverThreads;
	this->p_RouteResolverThreadInfo[this->p_iResolverThreads].ThreadHandle = (HANDLE) _beginthreadex(NULL, 0,
		p_RouterResolverThreadThreadProc, (LPVOID) &this->p_RouteResolverThreadInfo[this->p_iResolverThreads], 0, NULL);

	WaitForSingleObject(this->p_RouteResolverThreadInfo[this->p_iResolverThreads].StartedEvent, INFINITE);
	CloseHandle(this->p_RouteResolverThreadInfo[this->p_iResolverThreads].StartedEvent);
	this->p_RouteResolverThreadInfo[this->p_iResolverThreads].StartedEvent = NULL;

	this->p_iResolverThreads++;

	LeaveCriticalSection(&this->p_ResolverCS);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to stop all of the data Resolver threads for the route.
*/
void CRouterServer::p_StopResolverThreads(void)
{
	this->p_bShutdownResolverThread = true;

	if(this->p_iResolverThreads > 0)
	{
		int iEndedThreads = 0;
		while(iEndedThreads != this->p_iResolverThreads)
		{
			for(int iThread = 0; iThread < this->p_iResolverThreads; iThread++)
			{
				DWORD dwExitCode = 0;
				if(this->p_RouteResolverThreadInfo[iThread].ThreadHandle &&
					GetExitCodeThread(this->p_RouteResolverThreadInfo[iThread].ThreadHandle, &dwExitCode))
				{
					if(dwExitCode != STILL_ACTIVE)
					{
						CloseHandle(this->p_RouteResolverThreadInfo[iThread].ThreadHandle);
						this->p_RouteResolverThreadInfo[iThread].ThreadHandle = NULL;
						iEndedThreads++;
					}
					Sleep(1);
				}
			}
		}

		this->p_iResolverThreads = 0;
		this->Free(this->p_RouteResolverThreadInfo);
	}
	this->p_RouteResolverThreadInfo = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This is the actual "Connection Acquisition / Resolver" procedure,
		it is called by the data Resolver thread procedure: p_RouterResolverThreadThreadProc().

	FIXFIX: This could be EASILY centralized so that all routes could utilize one pool
		of acquisition / resolver threads. It would probably prove to be much more efficient.
*/
unsigned int CRouterServer::p_ResolverThread(ROUTERESOLVERTHREADINFO *pThreadInfo)
{
	SetEvent(pThreadInfo->StartedEvent);

	while(!this->p_bShutdownResolverThread)
	{
		WaitForSingleObject(this->p_hResolverKickOffEvent, 500);

		EnterCriticalSection(&this->p_ResolverCS);

		//##(ROUTER_MAJOR_STEP:3)##############################################################################
		//## Get a "Connection Acquisition / Resolver" work item (if any) from the stack.
		//#####################################################################################################
		ROUTERESOLVERSTACKITEM RSI;
		size_t ItemLength = 0;
		bool bGotItemFromStack = this->p_ResolverStack->Pop(&RSI, &ItemLength);

		//If we have more than 2 pending connection in the queue for each current
		//	resolver thread, then spin up a new resolver thread to handle the load.
		if(this->p_iResolverThreads < this->p_iMaxResolverThreads)
		{
			float fStackSize = (float)this->p_ResolverStack->StackSize();
			if(fStackSize > 0)
			{
				float fResolverThreadLoad = (fStackSize / (float)this->p_iResolverThreads) * 100.0f;
				if(fResolverThreadLoad > this->p_iResolverSpawnThreshold)
				{
					this->p_AddResolverThread();
				}
			}
		}

		LeaveCriticalSection(&this->p_ResolverCS);

		if(bGotItemFromStack)
		{
			SOCKETSERVERUSERDATA *pSSUD = (SOCKETSERVERUSERDATA *)this->pUserData;
			CCore *pCore = (CCore *)pSSUD->pCore;
			ROUTE *pRoute = pCore->Routes->FromGUID(pSSUD->RouteGUID);
			ROUTEPUMPTHREADINFO *pPTI = (ROUTEPUMPTHREADINFO *)RSI.PumpThreadInfo;
			CRouterClient *pClient = (CRouterClient *)RSI.pRouterClient;

			//##(ROUTER_MAJOR_STEP:4)##############################################################################
			//## Call [ConnectPeer()] to perform load balancing and to connect the companion end-point.
			//#####################################################################################################
			CRouterClient *pPeer = pCore->ConnectPeer(this, pClient, pRoute, pPTI);
			if(pPeer)
			{
				//We established a connection, join the two end-point conenctions togeather.
				pClient->pRoute = pRoute;
				pClient->pPartner = pPeer;
				pPeer->pRoute = pRoute;
				pPeer->pPartner = pClient;
			}
			else {
				//Failed to acquire a companion end-point connection. Just disconnect.
				pClient->Disconnect();
			}

			pClient->bWaitingOnResolver = false;
		}
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This is the actual data Resolver thread.
*/
unsigned __stdcall p_RouterResolverThreadThreadProc(void *pData)
{
	//---------------------(Initialize Thread [Begin])----------
	ROUTERESOLVERTHREADINFO *pThreadInfo = (ROUTERESOLVERTHREADINFO *) pData;

#ifdef _DEBUG
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
	char sThreadName[255];
	sprintf_s(sThreadName, sizeof(sThreadName), "ResolverThread[%d]", pThreadInfo->ThreadNumber);
	((CRouterServer *)(pThreadInfo->pSockRouter))->p_SetThreadName(sThreadName);
#endif
#endif

	SetEvent(pThreadInfo->StartedEvent);
	//---------------------(Initialize Thread [END])----------

	unsigned iResult = ((CRouterServer *)(pThreadInfo->pSockRouter))->p_ResolverThread(pThreadInfo);

	_endthreadex(iResult);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
