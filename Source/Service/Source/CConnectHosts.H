///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CConnectHosts_H
#define _CConnectHosts_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../../@Libraries/CXML/CXMLReader.H"
#include "../../../../@Libraries/CStringBuilder/CStringBuilder.H"
#include "../../../../@Libraries/CStack/CStack.H"

#include "../../@Shared/ConnectHosts.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CConnectHosts{

private:
	bool Initialized;
	void *pRoutes;
	char sFileName[MAX_PATH];

public:
	CPersistenceMap *PersistenceMap;
	int LoadBalanceMethod;
	bool PersistBySourceIP; 
	LOADBALANCEINFO LBI;

public:
	CONNECTHOSTS Collection;

	~CConnectHosts(void);
	CConnectHosts(void *lpRoutes);
	bool Save(void);

	bool ToXML(CXMLReader *lpXML);

	bool Load(CXMLReader *xmlConfig, int iPersistenceTime);
	bool Destroy(void);

	int LBMIndex(const char *sLBM);
	const char *LBMName(int iIndex);

private:
	bool ResolveHost(const char *sHostName, char *sOutIP, int iMaxOutSz);
	bool IsIPAddress(const char *sString);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
