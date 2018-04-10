///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CRouterServer_Memory_Cpp
#define _CRouterServer_Memory_Cpp
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

//#define _USE_ROUTE_GLOBAL_MEMPOOL 1

#ifdef _USE_ROUTE_GLOBAL_MEMPOOL
#include "../../../../@Libraries/CMemPool/CMemPool.H"
extern CMemPool *pMem; //pMem must be defined and initalized elsewhere.
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Allocates a chunk of RAM of specified size.
*/
bool CRouterServer::Alloc(LPROUTECHUNK pChunk, int iTotalBytes)
{
	memset(pChunk, 0, sizeof(ROUTECHUNK));
	if(iTotalBytes > 0)
	{
		pChunk->iAlloc = iTotalBytes;
		if(!(pChunk->pBuffer = (char *) this->Alloc(pChunk->iAlloc, 1)))
		{
			return false;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Allocates a chunk of RAM of default size.
*/
bool CRouterServer::Alloc(LPROUTECHUNK pChunk)
{
	return this->Alloc(pChunk, SSROUTE_DFLTBUFFERSIZE);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Allocates additional RAM of specified size to a buffer.
*/
void *CRouterServer::ReAlloc(void *lpMemory, const int iSize)
{
	#ifdef _USE_ROUTE_GLOBAL_MEMPOOL
		char *sMemory = (char *) pMem->ReAllocate(lpMemory, sizeof(char), iSize);
	#else
		EnterCriticalSection(&this->p_MemoryLock);
		char *sMemory = (char *) realloc(lpMemory, iSize);
		LeaveCriticalSection(&this->p_MemoryLock);
	#endif

	return sMemory;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Allocates additional RAM of specified size to a chunk.
*/
bool CRouterServer::ReAlloc(LPROUTECHUNK pChunk, int iTotalBytes)
{
	if(iTotalBytes > 0)
	{
		pChunk->iAlloc = iTotalBytes;
		if(!(pChunk->pBuffer = (char *) this->ReAlloc(pChunk->pBuffer, pChunk->iAlloc)))
		{
			return false;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Allocates a buffer of RAM of specified size.
*/
void *CRouterServer::Alloc(const int iSize, const int iCount)
{
	#ifdef _USE_ROUTE_GLOBAL_MEMPOOL
		char *sMemory = (char *) pMem->Allocate(iCount, iSize);
	#else
		EnterCriticalSection(&this->p_MemoryLock);
		char *sMemory = (char *) calloc(iCount, iSize);
		LeaveCriticalSection(&this->p_MemoryLock);
	#endif
	return sMemory;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Frees a buffer of RAM.
*/
void CRouterServer::Free(void *lpMemory)
{
	#ifdef _USE_ROUTE_GLOBAL_MEMPOOL
		pMem->Free(lpMemory);
	#else
		EnterCriticalSection(&this->p_MemoryLock);
		free(lpMemory);
		LeaveCriticalSection(&this->p_MemoryLock);
	#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Frees a chunk of RAM.
*/
bool CRouterServer::Free(LPROUTECHUNK pChunk)
{
	if(pChunk->pBuffer)
	{
		this->Free(pChunk->pBuffer);
		memset(pChunk, 0, sizeof(ROUTECHUNK));
		return true;
	}
	memset(pChunk, 0, sizeof(ROUTECHUNK));
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Allocates a chunk of RAM of specified size.
*/
bool CRouterClient::Alloc(LPROUTECHUNK pChunk, int iTotalBytes)
{
	return ((CRouterServer *)this->pSockServer)->Alloc(pChunk, iTotalBytes);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Allocates a chunk of RAM of default size.
*/
bool CRouterClient::Alloc(LPROUTECHUNK pChunk)
{
	return ((CRouterServer *)this->pSockServer)->Alloc(pChunk);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Allocates additional RAM of specified size to a buffer.
*/
void *CRouterClient::ReAlloc(void *lpMemory, const int iSize)
{
	return ((CRouterServer *)this->pSockServer)->ReAlloc(lpMemory, iSize);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Allocates additional RAM of specified size to a chunk.
*/
bool CRouterClient::ReAlloc(LPROUTECHUNK pChunk, int iTotalBytes)
{
	return ((CRouterServer *)this->pSockServer)->ReAlloc(pChunk, iTotalBytes);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Allocates a buffer of RAM of specified size.
*/
void *CRouterClient::Alloc(const int iSize, const int iCount)
{
	return ((CRouterServer *)this->pSockServer)->Alloc(iSize, iCount);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Frees a buffer of RAM.
*/
void CRouterClient::Free(void *lpMemory)
{
	return ((CRouterServer *)this->pSockServer)->Free(lpMemory);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Frees a chunk of RAM.
*/
bool CRouterClient::Free(LPROUTECHUNK pChunk)
{
	return ((CRouterServer *)this->pSockServer)->Free(pChunk);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
