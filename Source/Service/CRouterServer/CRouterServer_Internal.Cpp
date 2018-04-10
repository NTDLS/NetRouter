///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CRouterServer_Internal_Cpp
#define _CRouterServer_Internal_Cpp
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
#include "../Source/Entry.H"
#include "../Source/CManagement.H"
#include "../../../../@Libraries/CTrace/CTrace.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function sets up a socket to listen for incomming connections.
	This function should not be called directly!
*/
bool CRouterServer::p_OpenListenSocket(int iListenPort)
{
	for(int iIP = 0; iIP < this->p_ListenAddrs.Count || this->p_ListenAddrs.Count == 0; iIP++)
	{
		SOCKADDR_IN LocalListenSocketAddress; // Socket address of local server

		// Fill In The Address Structure For Local Server
		LocalListenSocketAddress.sin_family	= AF_INET; // Address Family.
		LocalListenSocketAddress.sin_port = htons(iListenPort); // Port Number.

		if(this->p_ListenAddrs.Count > 0)
		{
			LocalListenSocketAddress.sin_addr.s_addr = inet_addr(this->p_ListenAddrs.Address[iIP].IP);
		}
		else{
			LocalListenSocketAddress.sin_addr.s_addr = INADDR_ANY; // Let socket Library Assign Address.
		}

		int iMemory = (this->p_ListenSockets.Count + 1) * sizeof(SOCKET);
		this->p_ListenSockets.Socket = (SOCKET *) this->ReAlloc(this->p_ListenSockets.Socket, iMemory);
		SOCKET *pSocket = &this->p_ListenSockets.Socket[this->p_ListenSockets.Count++];

		// Create A TCP/IP Stream Socket To "Listen" with.
		if((*pSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
		{
			return this->p_ThrowError(16);
		}

		//Turn-on non-Blocking mode, otherwise accept() can block.
		DWORD dwNonBlockingMode = 1;
		ioctlsocket((SOCKET)*pSocket, FIONBIO, &dwNonBlockingMode);

		// Bind The Name To The Socket.
		if(bind(*pSocket, (SOCKADDR*)&LocalListenSocketAddress, sizeof(struct sockaddr)) == SOCKET_ERROR)
		{
			closesocket(*pSocket);
			return this->p_ThrowError(17);
		}

		// Set The Socket To Listen.
		if(listen(*pSocket, SOMAXCONN) == SOCKET_ERROR)
		{
			closesocket(*pSocket);
			return this->p_ThrowError(18);
		}

		if(this->p_ListenAddrs.Count == 0)
		{
			break; //We are listening on all addresses.
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function closes down the listening socket.
	This function should not be called directly!
*/
bool CRouterServer::p_CloseListenSocket(void)
{
	bool bResult = true;

	for(int iIndex = 0; iIndex < this->p_ListenSockets.Count; iIndex++)
	{
		if(closesocket(this->p_ListenSockets.Socket[iIndex]) == SOCKET_ERROR)
		{
			bResult = false;
		}
	}

	if(this->p_ListenSockets.Socket)
	{
		this->Free(this->p_ListenSockets.Socket);
		this->p_ListenSockets.Socket = NULL;
	}

	this->p_ListenSockets.Count = 0;
	
	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is called to disconnect a client.
	This functions return value has no meaning.
	This function should not be called directly!
*/
void CRouterServer::p_DisconnectClient(CRouterClient *pClient, ROUTEPUMPTHREADINFO *pThreadInfo)
{
	if(pClient->pPartner)
	{
		if(pClient->pPartner->Buffer.iLength == 0)
		{
			pClient->pPartner->bDisconnect = true;
		}

		pClient->pPartner->pPartner = NULL;
	}

	shutdown(pClient->Socket, SD_BOTH);
	closesocket(pClient->Socket);
	
	EnterCriticalSection(&this->p_PumpCS);

	this->p_iCurrentConnections--;
	pThreadInfo->ConnectionIndexes->remove(pClient->iSlot);
	this->Free(&pClient->Buffer);
	this->p_ResetClientSlot(pClient);

	LeaveCriticalSection(&this->p_PumpCS);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is called for each connecting client just
	before the client's bcConnected flag is set to TRUE.

	If this function returns FALSE the client will not be accepted
	or connected. This function should not be called directly!
*/
bool CRouterServer::p_InitializeClientVariables(CRouterClient *pClient)
{
	//EnterCriticalSection(&this->p_PumpCS);

	bool bResult = this->Alloc(&pClient->Buffer, this->p_iInitialBufferSize);

	//LeaveCriticalSection(&this->p_PumpCS);
   
	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns true if the passed in error number is OK, otherwise this function
		will call the OnError event then return false.
	This function should not be called directly!
*/
bool CRouterServer::p_ThrowError(int iErrorNumber, CRouterClient *pClient)
{
	if(iErrorNumber)
	{
		pTrace->Lock();

		bool bTracingEnabled = pTrace->IsTracingEnabled();
		if(!bTracingEnabled)
		{
			pTrace->Start();
		}
		else {
			if(pTrace->Type() == TRACE_TYPE_SPECIAL &&
				(_strnicmp(sRouterServerError[iErrorNumber], "Accept failed:", 14) == 0
				|| _strnicmp(sRouterServerError[iErrorNumber], "Connect failed:", 15) == 0
				|| _strcmpi(sRouterServerError[iErrorNumber], "Failed to resolve host name.") == 0))
			{
				pTrace->UnLock();
				return false;
			}
		}

		pTrace->Trace(sRouterServerError[iErrorNumber]);

		if(!bTracingEnabled)
		{
			pTrace->Stop();
		}

		pTrace->UnLock();

		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Returns true if the passed in error number is OK, otherwise this function
		will call the OnError event then return false.
	This function should not be called directly!
*/
bool CRouterServer::p_ThrowError(int iErrorNumber)
{
	return this->p_ThrowError(iErrorNumber, NULL);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function searched the bcConnected[] array to find a free client slot.

	If this function returns -1 the client wll not be accepted or connected.
	This function should not be called directly!
*/
CRouterClient *CRouterServer::p_GetFreeSocketSlot(int *pOutSlot, ROUTEPUMPTHREADINFO *pThreadInfo)
{
	EnterCriticalSection(&this->p_PumpCS);
	for(int iIndex = 0; iIndex < this->p_iMaxConnections; iIndex++)
	{
		if((iIndex % this->p_iMaxDataPumpThreads) == pThreadInfo->ThreadNumber)
		{
			if(!this->p_Clients[iIndex].bConnected
				&& !this->p_Clients[iIndex].bDisconnect
				&& !this->p_Clients[iIndex].bInitializing)
			{
				this->p_ResetClientSlot(&this->p_Clients[iIndex]);
				*pOutSlot = iIndex;

				//[bInitializing] must be set to [true] before releasing the lock
				//	because unline the [CSocketServer] class this function is
				//	accessed in parallel in the [CRouterServer] class and
				//	another thread may claim it before it is reserved externally.
				this->p_Clients[iIndex].bInitializing = true;

				LeaveCriticalSection(&this->p_PumpCS);
				return &this->p_Clients[iIndex];
			}
		}
	}

	LeaveCriticalSection(&this->p_PumpCS);
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	This function is used to clear the Clients position in the CRouterClient structure.
	The memset() is strictly for security purposes.

	This function should not be called directly!
*/
void CRouterServer::p_ResetClientSlot(CRouterClient *pClient)
{
	//EnterCriticalSection(&this->p_PumpCS);

	memset(pClient, 0, sizeof(CRouterClient)); //Set client to 0, in its entirety.
	pClient->pSockServer = this;

	time(&pClient->ttLastActivity);

	//LeaveCriticalSection(&this->p_PumpCS);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
void CRouterServer::p_SetThreadName(const char *sThreadName)
{
   ROUTER_THREADNAME_INFO info;
   info.dwType = 0x1000;
   info.szName = sThreadName;
   info.dwThreadID = GetCurrentThreadId();
   info.dwFlags = 0;

   __try
   {
	  RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
   }
   __except(EXCEPTION_EXECUTE_HANDLER)
   {
   }
}
#endif
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
