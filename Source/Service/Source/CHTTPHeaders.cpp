///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CHTTPHeaders_CPP
#define _CHTTPHeaders_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <WinSock2.H>
#include <WinSock.H>
#include <Windows.H>
#include <ShlObj.H>
#include <Stdio.H>
#include <ShlOBJ.H>
#include <Stdlib.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CHTTPHeaders.H"
#include "CRoutes.H"
#include "Entry.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::File;
using namespace NSWFL::ListView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CHTTPHeaders::~CHTTPHeaders(void)
{
	this->Destroy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CHTTPHeaders::CHTTPHeaders(void *lpRoutes)
{
	this->Initialized = false;
	this->pRoutes = lpRoutes;
	strcpy_s(sFileName, sizeof(sFileName), "");
	memset(&this->Collection, 0, sizeof(this->Collection));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CHTTPHeaders::Save(void)
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

bool CHTTPHeaders::ToXML(CXMLReader *lpXML)
{
	CXMLWriter xmlConfig("HTTPHeaders");

	for(int iItem = 0; iItem < this->Collection.Count; iItem++)
	{
		CXMLWriter Item("Header");
		Item.Add("Verbs", this->Collection.Items[iItem].Verbs);
		Item.Add("Method", this->MethodName(this->Collection.Items[iItem].Method));
		Item.Add("Name", this->Collection.Items[iItem].Name);
		Item.Add("Text", this->Collection.Items[iItem].Text);
		Item.Add("Description", this->Collection.Items[iItem].Description);
		Item.Add("Enable", this->Collection.Items[iItem].Enabled);
		xmlConfig.Add(&Item);
	}

	xmlConfig.ToReader(lpXML);

	xmlConfig.Destroy();

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char *CHTTPHeaders::MethodName(int iIndex)
{
	return HTTP_HANDLER_METHODS[iIndex];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CHTTPHeaders::MethodIndex(const char *sLBM)
{
	for(int i = 0; gsLBM[i]; i++)
	{
		if(_strcmpi(HTTP_HANDLER_METHODS[i], sLBM) == 0)
		{
			return i;
		}
	}
	return 0; //LBM_FAILOVERONLY
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CHTTPHeaders::Load(CXMLReader *xmlConfig)
{
	if(this->Initialized)
	{
		this->Destroy();
	}

	int iLength = 0;
	memset(&this->Collection, 0, sizeof(this->Collection));

	CRoutes *pRoutes = (CRoutes *)this->pRoutes;

	xmlConfig->ProgressiveScan(true);
	CXMLReader xmlItem;

	while(xmlConfig->ToReader("Header", &xmlItem))
	{
		char sName[HTTPHEADERS_MAX_NAME_LENGTH];
		char Text[HTTPHEADERS_MAX_REPLACE_LENGTH];
		char sVerbs[HTTPHEADERS_MAX_VERBS_LENGTH];
		char sMethod[HTTPHEADERS_MAX_METHOD_LENGTH];
		char sDescr[HTTPHEADERS_MAX_DESCR_LENGTH];

		this->Collection.Items = (HTTPHEADER *)
			pMem->ReAllocate(this->Collection.Items, sizeof(HTTPHEADER), this->Collection.Count + 1);

		HTTPHEADER *p = &this->Collection.Items[this->Collection.Count++];

		xmlItem.ToString("Verbs", sVerbs, sizeof(sVerbs), &iLength);
		p->Verbs = (char *)  pMem->StrDup(sVerbs);

		xmlItem.ToString("Method", sMethod, sizeof(sMethod));
		p->Method = this->MethodIndex(sMethod);

		xmlItem.ToString("Name", sName, sizeof(sName), &iLength);
		p->Name = (char *) pMem->StrDup(sName);

		xmlItem.ToString("Text", Text, sizeof(Text), &iLength);
		p->Text = (char *)  pMem->StrDup(Text);

		xmlItem.ToString("Description", sDescr, sizeof(sDescr), &iLength);
		p->Description = (char *)  pMem->StrDup(sDescr);

		p->Enabled = xmlItem.ToBoolean("Enable", true);

		xmlItem.Destroy();
	}

	return (this->Initialized = true); //Yes, i'm setting it.
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CHTTPHeaders::Destroy(void)
{
	if(this->Initialized)
	{
		this->Initialized = false;
		if(this->Collection.Count > 0)
		{
			for(int iPos = 0; iPos < this->Collection.Count; iPos++)
			{
				pMem->Free(this->Collection.Items[iPos].Verbs);
				pMem->Free(this->Collection.Items[iPos].Name);
				pMem->Free(this->Collection.Items[iPos].Text);
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
