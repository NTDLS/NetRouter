///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CListenIPs_H
#define _CListenIPs_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../../@Libraries/CXML/CXMLReader.H"
#include "../../../../@Libraries/CStringBuilder/CStringBuilder.H"
#include "../../../../@Libraries/CStack/CStack.H"

#include "../../@Shared/ListenIPs.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CListenIPs{

private:
	bool Initialized;
	void *pRoutes;
	char sFileName[MAX_PATH];

public:
	LISTENIPS Collection;

	~CListenIPs(void);
	CListenIPs(void *lpRoutes);
	CListenIPs(void *lpRoutes, CXMLReader *xmlConfig);
	bool Save(void);

	bool ToXML(CXMLReader *lpXML);

	bool Load(const char *sXMLFileName);
	bool Load(CXMLReader *xmlConfig);
	bool Reload(void);
	bool Destroy(void);
	bool ListenOnAll(void);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
