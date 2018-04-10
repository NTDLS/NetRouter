///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CPersistenceMap_CPP
#define _CPersistenceMap_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <WinSock2.H>
#include <WinSock.H>
#include <Windows.H>
#include <ShlObj.H>
#include <Stdio.H>
#include <ShlOBJ.H>
#include <Stdlib.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CPersistenceMap.H"
#include "Entry.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::File;
using namespace NSWFL::ListView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CPersistenceMap::~CPersistenceMap(void)
{
	for(int i = 0; i < this->Collection.Count; i++)
	{
		pMem->Free(this->Collection.Item[i].Address);
	}
	memset(&this->Collection, 0, sizeof(this->Collection));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CPersistenceMap::CPersistenceMap(int iPersistenceTime)
{
	memset(&this->Collection, 0, sizeof(this->Collection));
	this->Collection.PersistenceTime = iPersistenceTime;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CPersistenceMap::Free(int iMapIndex)
{
	pMem->Free(this->Collection.Item[iMapIndex].Address);
	memset(&this->Collection.Item[iMapIndex], 0, sizeof(PERSISTENCEMAPENTRY));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CPersistenceMap::Add(const char *sAddress, int iHostIndex)
{
	PERSISTENCEMAPENTRY *pEntry = NULL;

	for(int i = 0; i < this->Collection.Count; i++)
	{
		if(!this->Collection.Item[i].Used)
		{
			pEntry = &this->Collection.Item[i];
			break;

		}
	}

	if(!pEntry)
	{
		this->Collection.Item = (PERSISTENCEMAPENTRY *) pMem->ReAllocate(this->Collection.Item, sizeof(PERSISTENCEMAPENTRY), this->Collection.Count + 1);
		pEntry = &this->Collection.Item[this->Collection.Count++];
	}

	pEntry->Address = pMem->StrDup(sAddress);
	pEntry->HostIndex = iHostIndex;
	pEntry->Used = true;
	time(&pEntry->LastActivityTime);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CPersistenceMap::Get(const char *sAddress)
{
	for(int i = 0; i < this->Collection.Count; i++)
	{
		if(this->Collection.Item[i].Used && _strcmpi(this->Collection.Item[i].Address, sAddress) == 0)
		{
			if(this->Collection.PersistenceTime > 0)
			{
				time_t ttNow;
				//Has the persistence been inactive for too long?
				if(difftime(time(&ttNow), this->Collection.Item[i].LastActivityTime) > this->Collection.PersistenceTime)
				{
					this->Free(i);
					return -1;
				}
			}

			return this->Collection.Item[i].HostIndex;
		}
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
