///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CConnectHosts_CPP
#define _CConnectHosts_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define _WIN32_WINNT 0x0500
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <WinSock2.H>
#include <WinSock.H>
#include <Windows.H>
#include <ShlObj.H>
#include <Stdio.H>
#include <ShlOBJ.H>
#include <Stdlib.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CConnectHosts.H"
#include "CRoutes.H"
#include "Entry.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::File;
using namespace NSWFL::ListView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CConnectHosts::~CConnectHosts(void)
{
	this->Destroy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CConnectHosts::CConnectHosts(void *lpRoutes)
{
	this->Initialized = false;
	this->pRoutes = lpRoutes;
	strcpy_s(sFileName, sizeof(sFileName), "");
	memset(&this->Collection, 0, sizeof(this->Collection));
	memset(&this->LBI, 0, sizeof(LOADBALANCEINFO));

	this->LoadBalanceMethod = LBM_ROUNDROBIN;
	this->PersistBySourceIP = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CConnectHosts::Save(void)
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

bool CConnectHosts::ToXML(CXMLReader *lpXML)
{
	CXMLWriter xmlConfig("Hosts");

	xmlConfig.Add("LoadBalanceMethod", this->LBMName(this->LoadBalanceMethod));
	xmlConfig.AddBool("PersistBySourceIP", this->PersistBySourceIP);

	for(int iItem = 0; iItem < this->Collection.Count; iItem++)
	{
		CXMLWriter Item("Host");
		Item.Add("Address", this->Collection.Items[iItem].Address);
		Item.Add("Port", this->Collection.Items[iItem].Port);
		Item.Add("Description", this->Collection.Items[iItem].Description);
		Item.Add("Enable", this->Collection.Items[iItem].Enabled);
		xmlConfig.Add(&Item);
	}

	xmlConfig.ToReader(lpXML);

	xmlConfig.Destroy();

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char *CConnectHosts::LBMName(int iIndex)
{
	return gsLBM[iIndex];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CConnectHosts::LBMIndex(const char *sLBM)
{
	for(int i = 0; gsLBM[i]; i++)
	{
		if(_strcmpi(gsLBM[i], sLBM) == 0)
		{
			return i;
		}
	}
	return 0; //LBM_FAILOVERONLY
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CConnectHosts::ResolveHost(const char *sHostName, char *sOutIP, int iMaxOutSz)
{
	struct in_addr Address;
	struct hostent *Host;

	memset(&Host, 0, sizeof(Host));
	memset(&Address, 0, sizeof(Address));

	if((Host = gethostbyname(sHostName)) == NULL)
	{
		return "";
	}

	memcpy(&Address.s_addr, (*Host).h_addr_list[0], (*Host).h_length);
	strcpy_s(sOutIP, iMaxOutSz, inet_ntoa(Address));

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CConnectHosts::IsIPAddress(const char *sString)
{
	int iStrSz = (int)strlen(sString);

	int iDotCount = 0;
	char sSeg[5];

	int iWPos = 0;

	if(iStrSz < 7 || iStrSz > 15 || inet_addr(sString) == INADDR_NONE)
	{
		return false;
	}
	else{
		for(int iRPos = 0; iRPos < iStrSz; iRPos++)
		{
			sSeg[iWPos++] = sString[iRPos];

			if(sString[iRPos] == '.')
			{
				int iVal = atol(sSeg);
				sSeg[iWPos] = '\0';

				if(iVal < 0 || iVal > 255)
				{
					return false;
				}

				iWPos = 0;
				iDotCount++;
			}

			if(iWPos > 3)
			{
				return false;
			}
		}
	}

	return iDotCount == 3;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CConnectHosts::Load(CXMLReader *xmlConfig, int iPersistenceTime)
{
	if(this->Initialized)
	{
		this->Destroy();
	}

	int iLength = 0;
	memset(&this->Collection, 0, sizeof(this->Collection));
	memset(&this->LBI, 0, sizeof(LOADBALANCEINFO));

	CRoutes *pRoutes = (CRoutes *)this->pRoutes;

	this->PersistenceMap = new CPersistenceMap(iPersistenceTime);

	char sLoadBalanceMethod[CONNECTHOSTS_MAX_LBM_LENGTH];
	xmlConfig->ToString("LoadBalanceMethod", sLoadBalanceMethod, sizeof(sLoadBalanceMethod));
	this->LoadBalanceMethod = this->LBMIndex(sLoadBalanceMethod);
	this->PersistBySourceIP = xmlConfig->ToBoolean("PersistBySourceIP", false);

	xmlConfig->ProgressiveScan(true);
	CXMLReader xmlItem;

	while(xmlConfig->ToReader("Host", &xmlItem))
	{
		char sHost[CONNECTHOSTS_MAX_HOST_LENGTH];
		char sDescr[CONNECTHOSTS_MAX_DESCR_LENGTH];

		this->Collection.Items = (CONNECTHOST *)
			pMem->ReAllocate(this->Collection.Items, sizeof(CONNECTHOST), this->Collection.Count + 1);

		CONNECTHOST *p = &this->Collection.Items[this->Collection.Count++];

		xmlItem.ToString("Address", sHost, sizeof(sHost), &iLength);
		p->Address = (char *) pMem->StrDup(sHost);

		p->Port = xmlItem.ToInteger("Port");

		if(!this->IsIPAddress(sHost))
		{
			this->ResolveHost(p->Address, sHost, sizeof(sHost));
		}

		p->ResolvedIP = (char *) pMem->StrDup(sHost);

		xmlItem.ToString("Description", sDescr, sizeof(sDescr), &iLength);
		p->Description = (char *)  pMem->StrDup(sDescr);

		p->Enabled = xmlItem.ToBoolean("Enable", true);

		xmlItem.Destroy();
	}

	return (this->Initialized = true); //Yes, i'm setting it.
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CConnectHosts::Destroy(void)
{
	if(this->Initialized)
	{
		this->Initialized = false;
		if(this->Collection.Count > 0)
		{
			for(int iPos = 0; iPos < this->Collection.Count; iPos++)
			{
				pMem->Free(this->Collection.Items[iPos].Address);
				pMem->Free(this->Collection.Items[iPos].Description);
				pMem->Free(this->Collection.Items[iPos].ResolvedIP);
			}

			pMem->Free(this->Collection.Items);

			this->Collection.Count = 0;
		}

		delete this->PersistenceMap;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
