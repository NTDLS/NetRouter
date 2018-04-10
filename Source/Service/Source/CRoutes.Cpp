///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CRoutes_Cpp
#define _CRoutes_Cpp
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <ShlObj.H>
#include <Stdio.H>
#include <ShlOBJ.H>
#include <Stdlib.H>
#include <ShellAPI.H>
#include <Math.H>

extern HIMAGELIST hEnableDisableImageList; //Declared in MainDialog.cpp
extern HIMAGELIST hOnePixilImageList; //Declared in MainDialog.cpp

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CRoutes.H"
#include "Entry.H"
#include "CCore.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::File;
using namespace NSWFL::ListView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CRoutes::CRoutes(void *lpCore)
{
	InitializeCriticalSection(&this->CS);
	this->Initialized = false;
	this->pCore = lpCore;
	strcpy_s(sFileName, sizeof(sFileName), "");
	memset(&this->Collection, 0, sizeof(this->Collection));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CRoutes::CRoutes(void *lpCore, CXMLReader *xmlConfig)
{
	InitializeCriticalSection(&this->CS);
	this->Initialized = false;
	this->pCore = lpCore;
	strcpy_s(sFileName, sizeof(sFileName), "");
	memset(&this->Collection, 0, sizeof(this->Collection));
	this->Load(xmlConfig);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CRoutes::~CRoutes(void)
{
	this->Destroy();
	DeleteCriticalSection(&this->CS);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CRoutes::Save(void)
{
	CXMLReader xmlConfig;
	if(this->ToXML(&xmlConfig))
	{
		bool bResult = xmlConfig.ToFile(this->sFileName);
		if(!bResult)
		{
			char sErrorMsg[2048];
			GetLastError(sErrorMsg, sizeof(sErrorMsg),
				"Failed to save websites configuration file (", ")");
			strcat_s(sErrorMsg, sizeof(sErrorMsg), "\r\n\r\n\"");
			strcat_s(sErrorMsg, sizeof(sErrorMsg), this->sFileName);
			strcat_s(sErrorMsg, sizeof(sErrorMsg), "\"");
			pTrace->Trace(sErrorMsg);
		}
		return bResult;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CRoutes::ToXML(CXMLReader *lpXML)
{
	return this->ToXML(lpXML, false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CRoutes::ToXML(ROUTE *pRoute, CXMLReader *lpXML, bool bIncludeStatus)
{
	CXMLWriter Item("Route");
	Item.Add("GUID", pRoute->GUID);
	Item.Add("Name", pRoute->Name);
	Item.Add("Description", pRoute->Description);
	Item.Add("LocalPort", pRoute->LocalPort);
	Item.Add("MaxConnections", pRoute->MaxConnections);
	Item.Add("DataPumpThreads", pRoute->DataPumpThreads);
	Item.Add("ResolverThreads", pRoute->ResolverThreads);
	Item.Add("InitialBufferSize", pRoute->InitialBufferSize);
	Item.Add("MaxBufferSize", pRoute->MaxBufferSize);
	Item.Add("IdleDisconnectTimeout", pRoute->IdleDisconnectTimeout);
	Item.Add("DataPumpSpawnThreshold", pRoute->DataPumpSpawnThreshold);
	Item.Add("ResolverSpawnThreshold", pRoute->ResolverSpawnThreshold);
	Item.Add("PersistenceTime", pRoute->PersistenceTime);
	Item.AddBool("AutoStart", pRoute->AutoStart);

	CXMLReader xmlListenIPs("IPAddresses");
	if(pRoute->ListenIPs->ToXML(&xmlListenIPs))
	{
		Item.Add(&xmlListenIPs);
	}

	CXMLReader xmlHosts("Hosts");
	if(pRoute->ConnectHosts->ToXML(&xmlHosts))
	{
		Item.Add(&xmlHosts);
	}

	CXMLReader xmlHTTPHeaders("HTTPHeaders");
	if(pRoute->HTTPHeaders->ToXML(&xmlHTTPHeaders))
	{
		Item.Add(&xmlHTTPHeaders);
	}
	
	if(bIncludeStatus)
	{
		CXMLWriter xmlStatus("Status");
		PUMP *pPump = ((CCore *) this->pCore)->PumpFromGUID(pRoute->GUID);
		if(pPump)
		{
			xmlStatus.AddBool("IsRunning", pPump->Server->IsActive());
			xmlStatus.Add("CurrentConnections", pPump->Server->CurrentConnections());
		}
		Item.Add(&xmlStatus);
	}

	CXMLWriter xmlStatus("Statistics");
	PUMP *pPump = ((CCore *) this->pCore)->PumpFromGUID(pRoute->GUID);
	if(pPump)
	{
		xmlStatus.Add("BytesTransferred", ((SOCKETSERVERUSERDATA*)pPump->Server->pUserData)->BytesTransferred);
		xmlStatus.Add("TotalConnections", pPump->Server->NextPeerID());
	}
	Item.Add(&xmlStatus);

	Item.ToReader(lpXML);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CRoutes::ToXML(CXMLReader *lpXML, bool bIncludeStatus)
{
	CXMLWriter xmlConfig("Routes");

	for(int i = 0; i < this->Collection.Count; i++)
	{
		if(this->Collection.Items[i].GUID)
		{
			CXMLReader Item;
			if(this->ToXML(&this->Collection.Items[i], &Item, bIncludeStatus))
			{
				xmlConfig.Add(&Item);
			}
		}
	}

	xmlConfig.ToReader(lpXML);

	xmlConfig.Destroy();

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	Reloads the configuration from the file it was originally loaded from.
*/
bool CRoutes::Reload(void)
{
	if(this->Initialized)
	{
		this->Destroy();
	}

	this->Load(this->sFileName);

	return this->Initialized;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CRoutes::Load(const char *sXMLFileName)
{
	if(this->Initialized)
	{
		this->Destroy();
	}

	strcpy_s(this->sFileName, sizeof(this->sFileName), sXMLFileName);

	CXMLReader xmlConfig;
	if(xmlConfig.FromFile(sXMLFileName))
	{
		CXMLReader xmlEntity;
		if(xmlConfig.ToReader("Routes", &xmlEntity))
		{
			this->Initialized = this->Load(&xmlEntity);
		}
		xmlEntity.Destroy();
		xmlConfig.Destroy();
	}

	return this->Initialized;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ROUTE *CRoutes::Set(ROUTE *pRoute, CXMLReader *xmlItem)
{
	int iLength = 0;

	char sNewGUID[ROUTE_MAX_GUID];
	char sGUID[ROUTE_MAX_GUID];
	char sName[ROUTE_MAX_NAME];
	char sDescription[ROUTE_MAX_DESCRIPTION];

	if(NewGUID(sNewGUID, sizeof(sNewGUID)))
	{
		xmlItem->ToString("GUID", sGUID, sizeof(sGUID), sNewGUID, &iLength);
		pRoute->GUID = (char *) pMem->Allocate(sizeof(char), iLength + 1);
		strcpy_s(pRoute->GUID, iLength + 1, sGUID);

		xmlItem->ToString("Name", sName, sizeof(sName), &iLength);
		pRoute->Name = (char *) pMem->Allocate(sizeof(char), iLength + 1);
		strcpy_s(pRoute->Name, iLength + 1, sName);

		xmlItem->ToString("Description", sDescription, sizeof(sDescription), &iLength);
		pRoute->Description = (char *) pMem->Allocate(sizeof(char), iLength + 1);
		strcpy_s(pRoute->Description, iLength + 1, sDescription);

		pRoute->LocalPort = xmlItem->ToInteger("LocalPort", 0);
		pRoute->MaxConnections = xmlItem->ToInteger("MaxConnections", 1024);
		pRoute->DataPumpThreads = xmlItem->ToInteger("DataPumpThreads", 16);
		pRoute->ResolverThreads = xmlItem->ToInteger("ResolverThreads", (pRoute->DataPumpThreads / 4) > 1 ? pRoute->DataPumpThreads / 4 : 1);
		pRoute->InitialBufferSize = xmlItem->ToInteger("InitialBufferSize", 16 * 1024);
		pRoute->MaxBufferSize = xmlItem->ToInteger("MaxBufferSize", 256 * 1024);
		pRoute->IdleDisconnectTimeout = xmlItem->ToInteger("IdleDisconnectTimeout", 3600);
		pRoute->DataPumpSpawnThreshold = xmlItem->ToInteger("DataPumpSpawnThreshold", 75);
		pRoute->ResolverSpawnThreshold = xmlItem->ToInteger("ResolverSpawnThreshold", 200);
		pRoute->PersistenceTime = xmlItem->ToInteger("PersistenceTime", 3600);
		pRoute->AutoStart = xmlItem->ToBoolean("AutoStart", true);

		bool bConflictsExist = false;
		int iConflictCorrectionAttempts = 0;

		//---------------------------(Sanity Checks Begin)---

		if(pRoute->MaxConnections > FD_SETSIZE)
		{
			pRoute->MaxConnections = FD_SETSIZE / 2;
		}
		if(pRoute->DataPumpThreads > 1000)
		{
			pRoute->DataPumpThreads = 2;
		}

		int iOriginalMaxConnections = pRoute->MaxConnections;

		do{
			bConflictsExist = false;

			if(pRoute->DataPumpThreads < 1)
			{
				//Pump threads threads must be be at least 1.
				pRoute->DataPumpThreads = 2;
				bConflictsExist = true;
			}

			if(pRoute->MaxConnections <= 2)
			{
				//Max connections must be at least 2.
				pRoute->MaxConnections = 2;
				bConflictsExist = true;
			}

			if((pRoute->MaxConnections % 2) != 0)
			{
				//Max connections must be a multiple of 2.
				pRoute->MaxConnections--;
				bConflictsExist = true;
			}

			//Pump threads must not exceed 50% of the max connections count.
			if(pRoute->DataPumpThreads > (pRoute->MaxConnections / 2))
			{
				pRoute->DataPumpThreads = pRoute->MaxConnections / 2;
				bConflictsExist = true;
			}

			//Ensure that the connections per thread equate to a whole number.
			double dCPT_Fractional  = 0;
			do {
				double dConnectionPerThread = ((double)pRoute->MaxConnections) / ((double)pRoute->DataPumpThreads);
				double dCPT_Integer;

				dCPT_Fractional = modf(dConnectionPerThread, &dCPT_Integer);

				if(dCPT_Fractional != 0)
				{
					pRoute->MaxConnections++;
					bConflictsExist = true;
				}
			} while(dCPT_Fractional != 0);

			//The connections per thread must be evenly divisible by two.
			while(((pRoute->MaxConnections / pRoute->DataPumpThreads) % 2) != 0)
			{
				pRoute->MaxConnections++;
				bConflictsExist = true;
			}

			//Max connections must be a multiple of the pump thread count.
			while((pRoute->MaxConnections % pRoute->DataPumpThreads) != 0)
			{
				//pRoute->DataPumpThreads--;
				bConflictsExist = true;
			}

			if(pRoute->MaxConnections > FD_SETSIZE)
			{
				bConflictsExist = true;
			}

			if(bConflictsExist)
			{
				iConflictCorrectionAttempts++;

				//Too many attempts have been made to iron out the thread/connection count. Abort.
				if(iConflictCorrectionAttempts == 100)
				{
					//FIXFIX: Record this error.
					pRoute->MaxConnections = 100;
					pRoute->DataPumpThreads = 2;
					pRoute->AutoStart = false;
					break;
				}

				//We've made too many attempts at solving the problem by increasing the connection count, so now
				//	it's time to (1) reset the connection count, (2) decrement the thread count and (3) retry.
				if(pRoute->MaxConnections > FD_SETSIZE || (iConflictCorrectionAttempts % 10) == 0)
				{
					pRoute->MaxConnections = iOriginalMaxConnections;
					pRoute->DataPumpThreads--;
				}
			}

		} while(bConflictsExist);

		//---------------------------(Sanity Checks End)---

		pRoute->ListenIPs = new CListenIPs(this);
		pRoute->ConnectHosts = new CConnectHosts(this);
		pRoute->HTTPHeaders = new CHTTPHeaders(this);

		CXMLReader xmlConnectHosts;
		if(xmlItem->ToReader("Hosts", &xmlConnectHosts))
		{
			pRoute->ConnectHosts->Load(&xmlConnectHosts, pRoute->PersistenceTime);
			xmlConnectHosts.Destroy();
		}

		CXMLReader xmlHTTPHeaders;
		if(xmlItem->ToReader("HTTPHeaders", &xmlHTTPHeaders))
		{
			pRoute->HTTPHeaders->Load(&xmlHTTPHeaders);
			xmlHTTPHeaders.Destroy();
		}

		CXMLReader xmlIPAddresses;
		if(xmlItem->ToReader("IPAddresses", &xmlIPAddresses))
		{
			pRoute->ListenIPs->Load(&xmlIPAddresses);
			xmlIPAddresses.Destroy();
		}
	}

	return pRoute;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CRoutes::IsResizeRequired()
{
	for(int i = 0; i < this->Collection.Count; i++)
	{
		if(!this->Collection.Items[i].GUID)
		{
			return false;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ROUTE *CRoutes::Add(CXMLReader *xmlItem)
{
	for(int i = 0; i < this->Collection.Count; i++)
	{
		if(!this->Collection.Items[i].GUID)
		{
			ROUTE *p = &this->Collection.Items[i];
			memset(p, 0, sizeof(ROUTE));
			p->Slot = i;
			return this->Set(p, xmlItem);
		}
	}

	//Allocate a few additional slots.
	for(int i = 0; i < 20; i++)
	{
		this->Collection.Items = (ROUTE *)
			pMem->ReAllocate(this->Collection.Items, sizeof(ROUTE), this->Collection.Count + 1);

		ROUTE *p = &this->Collection.Items[this->Collection.Count++];

		memset(p, 0, sizeof(ROUTE));
	}

	return this->Add(xmlItem);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ROUTE *CRoutes::Update(CXMLReader *xmlItem)
{
	int iLength = 0;
	char sGUID[ROUTE_MAX_GUID];
	xmlItem->ToString("GUID", sGUID, sizeof(sGUID), &iLength);

	ROUTE *pRoupe = this->FromGUID(sGUID);
	if(pRoupe)
	{
		PUMP *pPump = ((CCore *)this->pCore)->PumpFromGUID(sGUID);
		if(pPump)
		{
			bool bRunning = pPump->Server->IsActive();

			if(bRunning)
			{
				pPump->Server->Stop(true);
			}

			if(this->Destroy(pRoupe))
			{
				__int64 i64BytesTransferred = ((SOCKETSERVERUSERDATA*)pPump->Server->pUserData)->BytesTransferred;
				int iTotalConnections = pPump->Server->NextPeerID();

				if(this->Set(pRoupe, xmlItem))
				{
					if(((CCore *)this->pCore)->DeleteServer(pPump))
					{
						if(((CCore *)this->pCore)->CreateServer(pPump, pRoupe))
						{
							pPump->Server->NextPeerID(iTotalConnections);
							((SOCKETSERVERUSERDATA*)pPump->Server->pUserData)->BytesTransferred = i64BytesTransferred;

							if(bRunning)
							{
								pPump->Server->Start();
							}
						}
					}
					return pRoupe;
				}
			}
		}
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CRoutes::Load(CXMLReader *xmlConfig)
{
	if(this->Initialized)
	{
		this->Destroy();
	}

	memset(&this->Collection, 0, sizeof(this->Collection));

	xmlConfig->ProgressiveScan(true);

	for(CXMLReader xmlItem; xmlConfig->ToReader("Route", &xmlItem);)
	{
		this->Add(&xmlItem);
		xmlItem.Destroy();
	}

	return this->Initialized = true; //Yes, it is being set.
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CRoutes::Destroy(const char *sGUID)
{
	ROUTE *pRoute = this->FromGUID(sGUID);
	if(pRoute)
	{
		return this->Destroy(pRoute);
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CRoutes::Destroy(ROUTE *pRoute)
{
	pMem->Free(pRoute->GUID);
	pMem->Free(pRoute->Name);
	pMem->Free(pRoute->Description);

	delete pRoute->ListenIPs;
	delete pRoute->ConnectHosts;
	delete pRoute->HTTPHeaders;

	memset(pRoute, 0, sizeof(pRoute));

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CRoutes::Destroy(void)
{
	if(this->Initialized)
	{
		this->Initialized = false;
		if(this->Collection.Count > 0)
		{
			for(int i = 0; i < this->Collection.Count; i++)
			{
				if(this->Collection.Items[i].GUID)
				{
					this->Destroy(&this->Collection.Items[i]);
				}
			}

			pMem->Free(this->Collection.Items);

			this->Collection.Count = 0;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CRoutes::Lock(void)
{
	EnterCriticalSection(&this->CS);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CRoutes::UnLock(void)
{
	LeaveCriticalSection(&this->CS);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ROUTE *CRoutes::FromGUID(const char *sGUID)
{
	for(int i = 0; i < this->Collection.Count; i++)
	{
		if(this->Collection.Items[i].GUID)
		{
			if(strcmp(this->Collection.Items[i].GUID, sGUID) == 0)
			{
				return &this->Collection.Items[i];
			}
		}
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
