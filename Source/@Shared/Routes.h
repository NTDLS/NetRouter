///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _Routes_Types_H
#define _Routes_Types_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef ROUTER_SERVICE
#include "CListenIPs.H"
#include "CConnectHosts.H"
#include "CHTTPHeaders.H"
#endif

#define ROUTE_LIST_POS_NAME             0
#define ROUTE_LIST_POS_LOCALPORT        1
#define ROUTE_LIST_POS_CURRENTCONNECTS  2
//#define ROUTE_LIST_POS_TOTALCONNECTS  x
#define ROUTE_LIST_POS_BYTESTRANSFERRED 3
#define ROUTE_LIST_POS_DESCRIPTION      4
#define ROUTE_LIST_POS_GUID             5

#define ROUTE_MAX_MAX              1024
#define ROUTE_MAX_GUID             33
#define ROUTE_MAX_NAME             255
#define ROUTE_MAX_DESCRIPTION      1024
#define ROUTE_MAX_REMOTEHOST       1024

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_Route {
	char *GUID;
	char *Name;
	char *Description;
	int LocalPort;
	int MaxConnections;
	int DataPumpThreads;
	int ResolverThreads;
	int MaxBufferSize;
	int InitialBufferSize;
	int IdleDisconnectTimeout;
	int DataPumpSpawnThreshold;
	int ResolverSpawnThreshold;
	int PersistenceTime;
	bool AutoStart;

#ifdef ROUTER_SERVICE
	CListenIPs *ListenIPs;
	CConnectHosts *ConnectHosts;
	CHTTPHeaders *HTTPHeaders;
	int Slot;
#else
	bool IsRunning;
	__int64 BytesTransferred;
	int CurrentConnections;
	int TotalConnections;
#endif
} ROUTE, *LPROUTE;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_Routes {
	ROUTE *Items;
	int Count;
} ROUTES, *LPROUTES;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
