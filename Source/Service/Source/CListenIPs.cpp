///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CListenIPs_CPP
#define _CListenIPs_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <ShlObj.H>
#include <Stdio.H>
#include <ShlOBJ.H>
#include <Stdlib.H>

extern HIMAGELIST hEnableDisableImageList; //Declared in MainDialog.cpp
extern HIMAGELIST hOnePixilImageList; //Declared in MainDialog.cpp

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CListenIPs.H"
#include "Entry.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::File;
using namespace NSWFL::ListView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CListenIPs::~CListenIPs(void)
{
	this->Destroy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CListenIPs::CListenIPs(void *lpRoutes)
{
	this->Initialized = false;
	this->pRoutes = lpRoutes;
	strcpy_s(sFileName, sizeof(sFileName), "");
	memset(&this->Collection, 0, sizeof(this->Collection));

	this->Collection.ListenOnAll = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CListenIPs::CListenIPs(void *lpRoutes, CXMLReader *xmlConfig)
{
	this->Initialized = false;
	this->pRoutes = lpRoutes;
	strcpy_s(sFileName, sizeof(sFileName), "");
	memset(&this->Collection, 0, sizeof(this->Collection));
	this->Load(xmlConfig);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CListenIPs::Save(void)
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

bool CListenIPs::ToXML(CXMLReader *lpXML)
{
	CXMLWriter xmlConfig("IPAddresses");

	xmlConfig.AddBool("ListenOnAll", this->Collection.ListenOnAll);

	for(int iItem = 0; iItem < this->Collection.Count; iItem++)
	{
		CXMLWriter Item("IPAddress");
		Item.Add("Address", this->Collection.Items[iItem].IP);
		Item.Add("Description", this->Collection.Items[iItem].Description);
		Item.Add("Enable", this->Collection.Items[iItem].Enabled);
		xmlConfig.Add(&Item);
	}

	xmlConfig.ToReader(lpXML);

	xmlConfig.Destroy();

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	Reloads the configuration from the file it was originally loaded from.
*/
bool CListenIPs::Reload(void)
{
	if(this->Initialized)
	{
		this->Destroy();
	}

	this->Load(this->sFileName);

	return this->Initialized;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CListenIPs::Load(const char *sXMLFileName)
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
		if(xmlConfig.ToReader("IPAddresses", &xmlEntity))
		{
			this->Initialized = this->Load(&xmlEntity);
		}
		xmlEntity.Destroy();
		xmlConfig.Destroy();
	}

	return this->Initialized;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CListenIPs::Load(CXMLReader *xmlConfig)
{
	if(this->Initialized)
	{
		this->Destroy();
	}

	int iLength = 0;
	memset(&this->Collection, 0, sizeof(this->Collection));

	this->Collection.ListenOnAll = xmlConfig->ToBoolean("ListenOnAll", true);

	xmlConfig->ProgressiveScan(true);
	CXMLReader xmlItem;

	while(xmlConfig->ToReader("IPAddress", &xmlItem))
	{
		char sIP[LISTENIPS_MAX_IP_LENGTH];
		char sDescr[LISTENIPS_MAX_DESCR_LENGTH];

		this->Collection.Items = (LISTENIP *)
			pMem->ReAllocate(this->Collection.Items, sizeof(LISTENIP), this->Collection.Count + 1);

		LISTENIP *p = &this->Collection.Items[this->Collection.Count++];

		xmlItem.ToString("Address", sIP, sizeof(sIP), &iLength);
		p->IP = (char *) pMem->Allocate(sizeof(char), iLength + 1);
		strcpy_s(p->IP, iLength + 1, sIP);

		xmlItem.ToString("Description", sDescr, sizeof(sDescr), &iLength);
		p->Description = (char *) pMem->Allocate(sizeof(char), iLength + 1);
		strcpy_s(p->Description, iLength + 1, sDescr);

		p->Enabled = xmlItem.ToBoolean("Enable", true);

		xmlItem.Destroy();
	}

	return (this->Initialized = true); //Yes, i'm setting it.
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CListenIPs::ListenOnAll(void)
{
	return this->Collection.ListenOnAll;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CListenIPs::Destroy(void)
{
	if(this->Initialized)
	{
		this->Initialized = false;
		if(this->Collection.Count > 0)
		{
			for(int iPos = 0; iPos < this->Collection.Count; iPos++)
			{
				pMem->Free(this->Collection.Items[iPos].IP);
				pMem->Free(this->Collection.Items[iPos].Description);
			}

			pMem->Free(this->Collection.Items);

			this->Collection.Count = 0;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
