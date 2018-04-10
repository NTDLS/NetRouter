#ifndef _CCore_H_
#define _CCore_H_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../../@Libraries/Debug/Debug.H"
#include "../../../../@Libraries/CMemPool/CMemPool.H"
#include "../../../../@Libraries/NSWFL/NSWFL.H"

#include "../CRouterServer/CRouterServer.H"
#include "../../../../@Libraries/CStringBuilder/CStringBuilder.H"
#include "../../../../@Libraries/CStack/CStack.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CRoutes.H"

#define MAX_TOKEN_SZ 2048

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_Pump {
	//int RouteSlot;
	//int LocalPort;
	//int RemotePort;
	//char *RemoteHost;
	char *RouteGUID;
	CRouterServer *Server;
	int Slot; //Index in the PUMPS connection;
	bool Used;
} PUMP, *LPPUMP;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_Pumps {
	PUMP *Items;
	int Count;
} PUMPS, *LPPUMPS;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CCore {

public:
	CCore(void);
	~CCore(void);

public:
	void ProcessHTTPHeaders(CRouterServer *pSock, ROUTE *pRoute, LPROUTECHUNK pChunk);
	friend void OnError_Client_Handler(CRouterServer *pSock, CRouterClient *pClient, int iErrorNumber, const char *sErrorMsg);
	friend bool OnAccept_Handler(CRouterServer *pSock, CRouterClient *pClient);
	friend bool OnRecv_Handler(CRouterServer *pSock, CRouterClient *pClient, LPROUTECHUNK pChunk);

	int GetLeastUsedIndex(int *iArray, int iArrayCount, int *iExclude, int iExcludeCount);
	bool IsVerb(const char *sVerbsList, const char *sVerb);
	bool ParseHeaderVariables(CStringBuilder *pOutput, const char *sFormula, const char *sHTTPVersion,
		const char *sHTTPVerb, CRouterServer *pSock, ROUTE *pRoute, char **sActualHeaders, int iActualHeadersCount);
	CRouterClient *ConnectPeer(CRouterServer *pSock, CRouterClient *pClient, ROUTE *pRoute, ROUTEPUMPTHREADINFO *pThreadInfo);

	bool Unload(void);
	bool Load(void);
	CRITICAL_SECTION _CS;

public:
	PUMP *PumpFromGUID(const char *sGUID);
	PUMP *AddPump(ROUTE *pRoute);
	bool RemovePump(int iSlot);
	bool RemovePump(PUMP *pPump);
	int RemoveAllPumps(void);
	PUMP *GetFreePumpSlot(void);
	bool Reload(void);
	bool CreateServer(PUMP *pPump, ROUTE *pRoute);
	bool DeleteServer(PUMP *pPump);
	bool SetPassword(const char *sPassword);
	bool CheckPassword(const char *sPasswordHash);
	bool AllowRemoteAdministration(void);
	bool AllowRemoteAdministration(bool bAllow);

	PUMPS Pumps;
	CRoutes *Routes;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

