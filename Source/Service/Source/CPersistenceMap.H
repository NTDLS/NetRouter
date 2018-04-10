///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CPersistenceMap_H
#define _CPersistenceMap_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Time.H>

#include "../../../../@Libraries/CXML/CXMLReader.H"
#include "../../../../@Libraries/CStringBuilder/CStringBuilder.H"
#include "../../../../@Libraries/CStack/CStack.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_PersistenceMapEntry {
	bool Used;
	char *Address;
	int HostIndex;
	time_t LastActivityTime;
} PERSISTENCEMAPENTRY, *LPPERSISTENCEMAPENTRY;

typedef struct _tag_PersistenceMap {
	PERSISTENCEMAPENTRY *Item;
	int Count;
	int PersistenceTime;
} PERSISTENCEMAP, *LPPERSISTENCEMAP;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CPersistenceMap {

private:
	PERSISTENCEMAP Collection;

public:
	void Add(const char *sAddress, int iHostIndex);
	int Get(const char *sAddress);
	void Free(int iMapIndex);

    CPersistenceMap(int iPersistenceTime);
	~CPersistenceMap(void);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
