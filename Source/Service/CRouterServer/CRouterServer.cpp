///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CRouterServer_CPP
#define _CRouterServer_CPP
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

#ifdef _USE_GLOBAL_MEMPOOL
#include "../CMemPool/CMemPool.H"
extern CMemPool *pMem; //pMem must be defined and initalized elsewhere.
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char *sRouterServerError[] = {
	"OK",																		//0
	"Received malformed packet.",												//1
	"Connection expired due to inactivity.",									//2
	"Accept failed: Socket error.",												//3
	"Accept failed: Pre-accept disconnection.",									//4
	"Accept failed: Too many clients.",											//5
	"Accept failed: Not enough resources.",										//6
	"Accept failed: Could not initialize client.",								//7
	"Accept failed: Rejected by filter.",										//8
	"WSAStartup failed.",														//9
	"WSACleanup failed.",														//10
	"Winsock buffer is full.",													//11
	"CreateThread failed.",														//12
	"Memory Allocation Error.",													//13
	"GetExitCodeThread failed.",												//14
	"Possible thread dead-lock. Thread will be terminated.",					//15
	"Socket failed.",															//16
	"Bind failed. Port already in use?",										//17
	"Listen failed.",															//18
	"OpenListenSocket failed.",													//19
	"StartDataPump failed.",													//20
	"StopDataPump failed.",														//21
	"CloseListenSocket failed.",												//22
	"CreateThread failed.",														//23
	"GetExitCodeThread failed.",												//24
	"Socket select error.",														//25
	"Connect failed: Too many clients.",										//26
	"Connect failed: Not enough resources.",									//27
	"Connect failed: Could not initialize client.",								//28
	"Maximum connections is set too high. Reduce value or increse FD_SETSIZE.",	//29
	"Failed to resolve host name.",												//30
	"Connect failed: Rejected by filter.",										//31
	"Maximum connections is set too high. Check value of SocketLimit().",		//32
	"Connect failed: General connection failure.",								//33
	NULL
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
