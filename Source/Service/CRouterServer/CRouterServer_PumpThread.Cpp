///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CRouterServer_PumpThread_Cpp
#define _CRouterServer_PumpThread_Cpp
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define _WIN32_WINNT 0x0500
#define WIN32_LEAN_AND_MEAN 

#define EXCHANGE_COUNT_MIN		1
#define EXCHANGE_COUNT_MAX		1000
#define EXCHANGE_SENSITIVITY	10

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

using namespace NSWFL::String;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to Add and Start a new data pump thread.
*/
void CRouterServer::p_AddDataPump(void)
{
	EnterCriticalSection(&this->p_PumpCS);

	this->p_bShutdownDataPump = false;

	//We allocate this at MAX because we do not want to risk reloacating it later.
	if(this->p_RoutePumpThreadInfo == NULL)
	{
		this->p_RoutePumpThreadInfo = (ROUTEPUMPTHREADINFO *)
			this->Alloc(sizeof(ROUTEPUMPTHREADINFO), this->p_iMaxDataPumpThreads);
	}

	char sEventName[255];
	NewGUID(sEventName, sizeof(sEventName));

	memset(&this->p_RoutePumpThreadInfo[this->p_iDataPumpThreads], 0, sizeof(ROUTEPUMPTHREADINFO));

	this->p_RoutePumpThreadInfo[this->p_iDataPumpThreads].pSockRouter = this;
	this->p_RoutePumpThreadInfo[this->p_iDataPumpThreads].StartedEvent = CreateEvent(NULL, TRUE, FALSE, sEventName);
	this->p_RoutePumpThreadInfo[this->p_iDataPumpThreads].ThreadNumber = this->p_iDataPumpThreads;
	this->p_RoutePumpThreadInfo[this->p_iDataPumpThreads].ConnectionIndexes = new list<int>();
	this->p_RoutePumpThreadInfo[this->p_iDataPumpThreads].ThreadHandle = (HANDLE) _beginthreadex(NULL, 0,
		p_RouterDataPumpThreadProc, (LPVOID) &this->p_RoutePumpThreadInfo[this->p_iDataPumpThreads], 0, NULL);

	WaitForSingleObject(this->p_RoutePumpThreadInfo[this->p_iDataPumpThreads].StartedEvent, INFINITE);
	CloseHandle(this->p_RoutePumpThreadInfo[this->p_iDataPumpThreads].StartedEvent);
	this->p_RoutePumpThreadInfo[this->p_iDataPumpThreads].StartedEvent = NULL;

	this->p_iDataPumpThreads++;

	LeaveCriticalSection(&this->p_PumpCS);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to stop all of the data pump threads for the route.
*/
void CRouterServer::p_StopDataPumps(void)
{
	this->p_bShutdownDataPump = true;

	if(this->p_iDataPumpThreads > 0)
	{
		int iEndedThreads = 0;
		while(iEndedThreads != this->p_iDataPumpThreads)
		{
			for(int iThread = 0; iThread < this->p_iDataPumpThreads; iThread++)
			{
				DWORD dwExitCode = 0;
				if(this->p_RoutePumpThreadInfo[iThread].ThreadHandle &&
					GetExitCodeThread(this->p_RoutePumpThreadInfo[iThread].ThreadHandle, &dwExitCode))
				{
					if(dwExitCode != STILL_ACTIVE)
					{
						CloseHandle(this->p_RoutePumpThreadInfo[iThread].ThreadHandle);
						this->p_RoutePumpThreadInfo[iThread].ThreadHandle = NULL;
						delete this->p_RoutePumpThreadInfo[iThread].ConnectionIndexes;
						iEndedThreads++;
					}
					Sleep(1);
				}
			}
		}

		this->p_iDataPumpThreads = 0;
		this->Free(this->p_RoutePumpThreadInfo);
	}
	this->p_RoutePumpThreadInfo = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This thread determines if a given thread should accept connections,
		it is called by the data pump thread procedure: p_RouterDataPumpThreadProc().
*/
bool CRouterServer::ShouldThreadAcceptConnections(ROUTEPUMPTHREADINFO *pThreadInfo)
{
	EnterCriticalSection(&this->p_PumpCS);

	//The logic below is designed to force the last "new connection accepting thread"
	//	to "give up" it's position as the "new connection accepting thread" before
	//	another thread can rise to the position. This was intended to *ENSURE*
	//	than no two threads will ever be listening at the same time because it was
	//	causing [accept()] to block.
	if(this->p_iLastAcceptingThreadNumber > 0 &&
		pThreadInfo->ThreadNumber != this->p_iLastAcceptingThreadNumber)
	{
		//Another thread is already servicing incomming connections.
		LeaveCriticalSection(&this->p_PumpCS);
		return false;
	}

	int iLeastUsedCount = this->p_iMaxConnections;
	int iLeastUsedIndex = 0;

	for(int iIndex = 0; iIndex < this->p_iDataPumpThreads; iIndex++)
	{
		if(this->p_RoutePumpThreadInfo[iIndex].ConnectionIndexes->size() < (unsigned int)iLeastUsedCount)
		{
			iLeastUsedCount = (int)this->p_RoutePumpThreadInfo[iIndex].ConnectionIndexes->size();
			iLeastUsedIndex = iIndex;
			if(iLeastUsedCount == 0)
			{
				break; //No need to look further, this thread is unused.
			}
		}
	}

	bool bResult = (pThreadInfo->ThreadNumber == iLeastUsedIndex);
	if(!bResult)
	{
		//This thread nolonger is nolonger accepting connections so another thread will take it's place.
		if(pThreadInfo->ThreadNumber != this->p_iLastAcceptingThreadNumber)
		{
			this->p_iLastAcceptingThreadNumber = -1;
		}
	}

	LeaveCriticalSection(&this->p_PumpCS);

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This is the actual data pump procedure,	it is called by the
		data pump thread procedure: p_RouterDataPumpThreadProc().
*/
unsigned int CRouterServer::p_DataPump(ROUTEPUMPTHREADINFO *pThreadInfo)
{
	int iSelectResult = 0;

	struct timeval tTimeOut; // The TimeOut structure.
	tTimeOut.tv_sec = 1;
	tTimeOut.tv_usec = 0;

	fd_set fdExcept;
	fd_set fdRead;
	fd_set fdWrite;

	time_t ttNow;

	int iExchangesToPerform = EXCHANGE_COUNT_MIN;
	int iExchangeTuningThreshold = 0;

	list<int>::iterator it;

	while(!this->p_bShutdownDataPump)
	{
		int iProductiveExchanges = 0;

		EnterCriticalSection(&this->p_PumpCS);

		//If we have not reached the maximum thread count for this route and the overall
		//	Cconnection load is more than xx.x% , then spin up an additional data-pump thread.
		if(this->p_iDataPumpThreads < this->p_iMaxDataPumpThreads)
		{
			float fConnectionsPerThread = ((float)this->p_iMaxConnections / ((float)this->p_iMaxDataPumpThreads));
			float fOverallConnectionLoad = (((float)this->p_iCurrentConnections) / (((float)this->p_iDataPumpThreads) * fConnectionsPerThread)) * 100.0f;

			if(fOverallConnectionLoad > this->p_iDataPumpSpawnThreshold)
			{
				this->p_AddDataPump();
			}
		}

		bool bShouldThreadAcceptConnections = this->ShouldThreadAcceptConnections(pThreadInfo);

		LeaveCriticalSection(&this->p_PumpCS);

		//This is a data-pump, so our focus is on pumping data... So, pump
		//	data a few times in-between load balancing operations. Reduces
		//	locks and dramaticly boosts performance without larger buffer sizes.
		for(int iExchangeCount = 0; iExchangeCount < iExchangesToPerform; iExchangeCount++)
		{
			bool bDataTransferred = false;
			int iSocketCount = 0;

			// Zero the Read, Write and Except
			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExcept);

			// Set all connected clients to Read, Write and Except
			for(it = pThreadInfo->ConnectionIndexes->begin(); it != pThreadInfo->ConnectionIndexes->end(); it++)
			{
				if(this->p_Clients[*it].bConnected && !this->p_Clients[*it].bWaitingOnResolver)
				{
					FD_SET(this->p_Clients[*it].Socket, &fdRead);
					FD_SET(this->p_Clients[*it].Socket, &fdWrite);
					FD_SET(this->p_Clients[*it].Socket, &fdExcept);
					iSocketCount++;
				}
			}

			//The concept here is that we always want to ensure that
			//	the "least busy" thread is listening. This reduces
			//	the load on the threads with a larger connection base.
			//In addition, only one thread "should" set the FD on the
			//	listen socket because all listening threads would be 
			//	notified and all but one would block untill another
			//	incoming connection was encountered.
			if(bShouldThreadAcceptConnections)
			{
				// Set LocalListenSocket socket to Read, Except
				for(int iIndex = 0; iIndex < this->p_ListenSockets.Count; iIndex++)
				{
					FD_SET(this->p_ListenSockets.Socket[iIndex], &fdRead);
					FD_SET(this->p_ListenSockets.Socket[iIndex], &fdExcept);
					iSocketCount++;
				}
			}

			if(iSocketCount > 0)
			{
				// Socket Select...
				if(iSelectResult = select(-1, &fdRead, &fdWrite, &fdExcept, &tTimeOut) == SOCKET_ERROR)
				{
					DWORD dwError = WSAGetLastError();
					return this->p_ThrowError(25);
				}

				if(this->p_bAllowNewConnections && this->p_iCurrentConnections < this->p_iMaxConnections && bShouldThreadAcceptConnections)
				{
					// Accept any incoming connections.
					for(int iIndex = 0; iIndex < this->p_ListenSockets.Count; iIndex++)
					{
						if(FD_ISSET(this->p_ListenSockets.Socket[iIndex], &fdRead))
						{
							this->p_Accept(this->p_ListenSockets.Socket[iIndex], pThreadInfo);
						}
					}
				}

				//##(ROUTER_MAJOR_STEP:8)##############################################################################
				//## Exchange data between associated remote end-points.
				//#####################################################################################################
				for(it = pThreadInfo->ConnectionIndexes->begin(); it != pThreadInfo->ConnectionIndexes->end(); it++)
				{
					if(this->p_Clients[*it].bConnected && !this->p_Clients[*it].bWaitingOnResolver)
					{
						if(FD_ISSET(this->p_Clients[*it].Socket, &fdRead))
						{
   							if(this->p_Receive(&this->p_Clients[*it]))
							{
								bDataTransferred = true;
							}
						}

						if(FD_ISSET(this->p_Clients[*it].Socket, &fdWrite))
						{
   							if(this->p_Send(&this->p_Clients[*it]))
							{
								bDataTransferred = true;
							}
						}

						if(FD_ISSET(this->p_Clients[*it].Socket, &fdExcept))
						{
							if(this->p_Clients[*it].Disconnect())
							{
								bDataTransferred = true;
							}
						}

						if(this->p_dMaxInactivityTime > 0) //Is max inactivity time enabled?
						{
							//Has the client been inactive for too long (no send/receive for x seconds)?
							if(difftime(time(&ttNow), this->p_Clients[*it].ttLastActivity) > this->p_dMaxInactivityTime)
							{
								this->p_Clients[*it].bDisconnect = true;
								this->p_ThrowError(2, &this->p_Clients[*it]);
							}
						}

						// Disconnect any pending disconnects.
						if(this->p_Clients[*it].bDisconnect)
						{
							this->p_DisconnectClient(&this->p_Clients[*it], pThreadInfo);
							bDataTransferred = true;
							if(!pThreadInfo->ConnectionIndexes->empty())
							{
								it = pThreadInfo->ConnectionIndexes->begin();
							}
							else {
								break;
							}
						}
					}
				}
			}

			if(pThreadInfo->ConnectionIndexes->empty())
			{
				//No connections to the thread? No need to try to stay in the data-pump optimizer.
				break;
			}

			if(!bDataTransferred)
			{
				break;
			}

			if(bDataTransferred)
			{
				iProductiveExchanges++;
			}
		}

		if(pThreadInfo->ConnectionIndexes->empty())
		{
			//No connected peers, give the CPU alot of rest to reduce idle-time cycles.
			//Keep in mind that this will decrease the initial responsiveness of the first connection.
			Sleep(10);
		}
		else if(iProductiveExchanges == 0)
		{
			//If we have not transferred data in any of the above attempts, then we
			//	must be fairly caught-up. Give the CPU some time to rest, ~1ms should do it.
			Sleep(1);
		}
		else
		{
			//Here, we determine how efficiently we are utilizing our CPU time
			//	when polling for data. If we’re looping a lot and not sending much
			//	data, then reduce the number of loops. If we’re sending data most
			//	of the time when looping then increase the number of loops.
	
			float fDataExchangeProductivity = ((float)iProductiveExchanges) / ((float)iExchangesToPerform);

			//If we are exchanging data more than 20% of the time that we are looping then increment the tuning threshold.
			if(fDataExchangeProductivity > 0.20 && iExchangesToPerform < EXCHANGE_COUNT_MAX)
			{
				iExchangeTuningThreshold++;
			}
			//If we are exchanging data less than 1% of the time that we are looping then decrement the tuning threshold.
			else if(fDataExchangeProductivity < 0.01 && iExchangesToPerform > EXCHANGE_COUNT_MIN)
			{
				iExchangeTuningThreshold--;
			}

			//Once the tuning treshold has hit the exchange sensitivity, increase the number of sleep-free loops.
			if(iExchangeTuningThreshold >= EXCHANGE_SENSITIVITY)
			{
				iExchangesToPerform += 100; //We are heavier on the bandwidth ramp-up than we are with the ramp-down.
				iExchangeTuningThreshold = 0;
			}
			else if(iExchangeTuningThreshold <= -EXCHANGE_SENSITIVITY)
			{
				iExchangesToPerform--;
				iExchangeTuningThreshold = 0;
			}
		}
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This is the actual data pump thread.
*/
unsigned __stdcall p_RouterDataPumpThreadProc(void *pData)
{
	//---------------------(Initialize Thread [Begin])----------
	ROUTEPUMPTHREADINFO *pThreadInfo = (ROUTEPUMPTHREADINFO *) pData;

#ifdef _DEBUG
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
	char sThreadName[255];
	sprintf_s(sThreadName, sizeof(sThreadName), "DataPump[%d]", pThreadInfo->ThreadNumber);
	((CRouterServer *)(pThreadInfo->pSockRouter))->p_SetThreadName(sThreadName);
#endif
#endif

	SetEvent(pThreadInfo->StartedEvent);
	//---------------------(Initialize Thread [END])----------

	unsigned iResult = ((CRouterServer *)(pThreadInfo->pSockRouter))->p_DataPump(pThreadInfo);

	_endthreadex(iResult);

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
