///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CRouterServer_Send_Cpp
#define _CRouterServer_Send_Cpp
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
#include "../Source/CCore.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Sends data to a remote peer.

	This function should not be called directly!
*/
bool CRouterServer::p_Send(CRouterClient *pClient)
{
	if(pClient->Buffer.iLength == 0 && !pClient->pPartner)
	{
		//There is nothing to send, and the partner has disconnected which means
		//	that we will not be receiving any further data to transmit to the end-point.
		pClient->Disconnect();
		return false;
	}

	if(pClient->Buffer.iLength > 0)
	{
		if(pClient->IsConnected())
		{
			//##(ROUTER_MAJOR_STEP:11)#############################################################################
			//## Send data to the connected end-point.
			//#####################################################################################################
			int iResult = send(pClient->Socket,	(char *)pClient->Buffer.pBuffer, pClient->Buffer.iLength, 0);
			if(iResult <= 0)
			{
				int iLastError = WSAGetLastError();
				if(iLastError == WSAEWOULDBLOCK || iLastError == WSAEINPROGRESS || iLastError == WSAEALREADY)
				{
					return false;
				}
				else if(iLastError == WSAENOBUFS)
				{
					this->p_ThrowError(11, pClient);
				}

				pClient->Disconnect();
			}
			else if(iResult != pClient->Buffer.iLength)
			{
				this->p_ThrowError(11, pClient);
				pClient->Disconnect();
			}
			else{
				//Data sent successfully!
				((SOCKETSERVERUSERDATA*)this->pUserData)->BytesTransferred += pClient->Buffer.iLength;
				pClient->Buffer.iLength = 0;
				time(&pClient->ttLastActivity);
				return true;
			}
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Receives data from a remote peer. This function should not be called directly!
*/
bool CRouterServer::p_Receive(CRouterClient *pClient)
{
	CRouterServer *pRouter = ((CRouterServer*)pClient->pSockServer);

	SOCKETSERVERUSERDATA *pSSUD = (SOCKETSERVERUSERDATA *)pRouter->pUserData;
	if(!pSSUD)
	{
		__debugbreak();
		return false;
	}

	CCore *pCore = (CCore *)pSSUD->pCore;
	if(!pCore)
	{
		__debugbreak();
		return false;
	}
	
	ROUTE *pRoute = ((ROUTE *)pClient->pRoute);
	if(!pRoute)
	{
		return false;
	}

	if(!pClient->pPartner)
	{
		pClient->Disconnect();
		return false;
	}

	if(pClient->pPartner->Buffer.iLength != 0)
	{
		return false;
	}

	//##(ROUTER_MAJOR_STEP:9)##############################################################################
	//## Receive data from connected end-point, directly into the associated end-point's buffer.
	//#####################################################################################################
	pClient->pPartner->Buffer.iLength = recv(pClient->Socket,
		(char *) pClient->pPartner->Buffer.pBuffer, pClient->pPartner->Buffer.iAlloc, 0);

	//This is where we perform auto buffer resizing. If we are receiving as
	//	much data as we can possibly hold in the buffer then we want to resize
	//	it in the hopes that we will increase throughput.
	if(pClient->pPartner->Buffer.iLength == pClient->pPartner->Buffer.iAlloc)
	{
		if(pClient->pPartner->Buffer.iAlloc < pRouter->p_iMaxBufferSize)
		{
			int iNewMemorySize = pClient->pPartner->Buffer.iAlloc + (int)(pClient->pPartner->Buffer.iAlloc * 0.10);
			if(iNewMemorySize > pRouter->p_iMaxBufferSize)
			{
				iNewMemorySize = pRouter->p_iMaxBufferSize;
			}
			this->ReAlloc(&pClient->pPartner->Buffer, iNewMemorySize);
		}
	}

	if(pClient->pPartner->Buffer.iLength <= 0)
	{
		pClient->bDisconnect = true;
		pClient->pPartner->Buffer.iLength = 0; //We dont want to leave this at some meaningless value.
	}
	else{
		//Data received successfully!
		if(pRoute->HTTPHeaders->Collection.Count > 0)
		{
			//##(ROUTER_MAJOR_STEP:10)#############################################################################
			//## Apply any applicable HTTP header rules / transformations.
			//#####################################################################################################
			pCore->ProcessHTTPHeaders(this, pRoute, &pClient->pPartner->Buffer);
		}
		((SOCKETSERVERUSERDATA*)this->pUserData)->BytesTransferred += pClient->pPartner->Buffer.iLength;
		time(&pClient->ttLastActivity);
		return true;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
