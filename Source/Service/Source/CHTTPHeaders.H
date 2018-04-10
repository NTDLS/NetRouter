///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CHTTPHeaders_H
#define _CHTTPHeaders_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../../@Libraries/CXML/CXMLReader.H"
#include "../../../../@Libraries/CStringBuilder/CStringBuilder.H"
#include "../../../../@Libraries/CStack/CStack.H"

#include "../../@Shared/HTTPHeaders.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CHTTPHeaders{

private:
	bool Initialized;
	void *pRoutes;
	char sFileName[MAX_PATH];

public:
	HTTPHEADERS Collection;

	~CHTTPHeaders(void);
	CHTTPHeaders(void *lpRoutes);
	bool Save(void);

	bool ToXML(CXMLReader *lpXML);

	bool Load(CXMLReader *xmlConfig);
	bool Destroy(void);

	int MethodIndex(const char *sMethod);
	const char *MethodName(int iIndex);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
