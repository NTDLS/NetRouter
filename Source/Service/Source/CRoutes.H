///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CRoutes_H
#define _CRoutes_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../../@Libraries/CXML/CXMLReader.H"
#include "../../../../@Libraries/CXML/CXMLWriter.H"
#include "../../../../@Libraries/CStringBuilder/CStringBuilder.H"
#include "../../../../@Libraries/CStack/CStack.H"

#include "../../@Shared/Routes.h"
#include "CListenIPs.H"
#include "CConnectHosts.H"
#include "CHTTPHeaders.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CRoutes{

public:
	ROUTES Collection;

private:
	bool Initialized;
	void *pCore;
	char sFileName[MAX_PATH];
	CRITICAL_SECTION CS;
	
public:
	~CRoutes(void);
	CRoutes(void *lpCore);
	CRoutes(void *lpCore, CXMLReader *xmlConfig);
	bool Save(void);
	bool ToXML(CXMLReader *lpXML);
	bool ToXML(CXMLReader *lpXML, bool bIncludeStatus);
	bool Load(const char *sXMLFileName);
	bool Load(CXMLReader *xmlConfig);
	bool Reload(void);
	bool Destroy();
	bool Destroy(ROUTE *pRoute);
	bool Destroy(const char *sGUID);
	bool ToXML(ROUTE *pRoute, CXMLReader *lpXML, bool bIncludeStatus);
	ROUTE *Update(CXMLReader *xmlItem);
	ROUTE *Add(CXMLReader *xmlItem);
	ROUTE *Set(ROUTE *pRoute, CXMLReader *xmlItem);
	bool IsResizeRequired();

public:
	ROUTE *FromGUID(const char *sGUID);

	void Lock(void);
	void UnLock(void);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
