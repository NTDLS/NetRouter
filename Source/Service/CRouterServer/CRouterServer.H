///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CRouterServer_H
#define _CRouterServer_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <list>
using namespace std;

#include "../../../../@Libraries/CStack/CStack.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SSROUTE_DFLTBUFFERSIZE		1024	// The initial size in bytes of the client send and receive buffers.
#define SSROUTE_STOPFORCETIMEOUT	5000	// The number of milliseconds to wait for clients to disconnect on destroy.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern const char *sRouterServerError[];

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_SocketServer_UserData {
	char *RouteGUID;
	void *pCore;
	__int64 BytesTransferred;
} SOCKETSERVERUSERDATA, *LPSOCKETSERVERUSERDATA;

#ifdef _DEBUG
#define MS_VC_EXCEPTION 0x406D1388
typedef struct tag_ROUTER_THREADNAME_INFO {
   DWORD dwType;		// Must be 0x1000.
   LPCSTR szName;		// Pointer to name (in user address space).
   DWORD dwThreadID;	// Thread ID (-1 = caller thread).
   DWORD dwFlags;		// Reserved for future use, must be zero.
} ROUTER_THREADNAME_INFO;
#endif

typedef struct _tag_Route_Resolver_Thread_Info {
	int ThreadNumber;
	void *pSockRouter; //(CRouterServer*)
	HANDLE StartedEvent;
	HANDLE ThreadHandle;
	int Connections;
} ROUTERESOLVERTHREADINFO, *LPROUTERESOLVERTHREADINFO;

typedef struct _tag_Route_Pump_Thread_Info {
	int ThreadNumber;
	void *pSockRouter; //(CRouterServer*)
	HANDLE StartedEvent;
	HANDLE ThreadHandle;
	list<int> *ConnectionIndexes;
} ROUTEPUMPTHREADINFO, *LPROUTEPUMPTHREADINFO;

typedef struct _tag_Route_Resolver_Stack_Item {
	ROUTEPUMPTHREADINFO *PumpThreadInfo;
	void *pRouterClient; //(CRouterClient *)
} ROUTERESOLVERSTACKITEM, *LPROUTERESOLVERSTACKITEM;

typedef struct _tag_Route_Basic_Chunk{
	void *pBuffer;
	int iLength;
	int iAlloc;
} ROUTECHUNK, *LPROUTECHUNK;

typedef struct _tag_Route__Listen_Address {
	char IP[16]; //IPv4
} ROUTELISTENADDRESS, *LPROUTELISTENADDRESS;

typedef struct _tag_Route__Listen_Addresses {
	int Count;
	ROUTELISTENADDRESS *Address;
} ROUTELISTENADDRESSES, *LPROUTELISTENADDRESSES;

typedef struct _tag_Route__Listen_Sockets {
	int Count;
	SOCKET *Socket;
} ROUTELISTENSOCKETS, *LPROUTELISTENSOCKETS;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CRouterClient {

public:
	bool bInitializing;			// The client is in the process of connecting.
	bool bConnected;			// Remote client's connection flag.
	bool bDisconnect;			// Remote client's disconnect flag.
	bool bWasConnect;			// True is the client was connected, false if the client was accepted.
	bool bAbortConnection;		// Used internally by the disconnection routine to exit all waiting "sends" and "receives".
	bool bWaitingOnResolver;	// Used to notate wheather a client is waiting on a companion connection to be established.

	char sPeerAddress[64];		// The IP address of the connected Peer.

	int iPeerPort;				// The IP port of the connected Peer.
	int iPeerID;				// Remote client's ID (For client identification).
	int iSlot;					// The slot that the this (client class) occupies in the parent class.

	ROUTECHUNK Buffer;			// Clients only solitary buffer.

	SOCKET Socket;				// Remote client's socket.
	HANDLE hThreadEvent;		// Used for various waiting operations.
	time_t ttLastActivity;		// Used to keep track of client activity.

	VOID *pSockServer;			// Pointer to the "parent" socket server class.

public:
	CRouterClient *pPartner;
	void *pRoute; //(ROUTE *)

public: //General public functions.
	bool WasConnect(void);

	HANDLE ThreadHandle(void);
	HANDLE EventHandle(void);

	int PeerID(void);
	int Slot(void);

	char *PeerAddress(void);
	int PeerAddress(char *sOutAddress, int iMaxSz);
	int PeerPort(void);

	VOID *UserData(void);
	void UserData(VOID *pData);

	bool IsDisconnectSet(void);
	bool IsConnected(void);
	bool Disconnect(void);

public: //Internal memory handing (**Extremely** thread safe):
	bool Alloc(LPROUTECHUNK pChunk);
	bool Alloc(LPROUTECHUNK pChunk, int iTotalBytes);
	void *Alloc(const int iSize, const int iCount);

	bool ReAlloc(LPROUTECHUNK pChunk, int iTotalBytes);
	void *ReAlloc(void *lpMemory, const int iSize);

	bool Free(LPROUTECHUNK pChunk);
	void Free(void *lpMemory);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CRouterServer {

friend CRouterClient;
friend void ProcessConsoleCommand(const char *sCommand, const char **sParams, int iParams);

public: //Internal memory handing (**Extremely** thread safe):
	bool Alloc(LPROUTECHUNK pChunk);
	bool Alloc(LPROUTECHUNK pChunk, int iTotalBytes);
	void *Alloc(const int iSize, const int iCount);

	bool ReAlloc(LPROUTECHUNK pChunk, int iTotalBytes);
	void *ReAlloc(void *lpMemory, const int iSize);

	bool Free(LPROUTECHUNK pChunk);
	void Free(void *lpMemory);

public: //General public functions.

	CRouterClient *Client(int iSlot);
	CRouterClient *Peer(int iPeerID);

	bool GetNextConnectedClient(int *iSlot);
	CRouterClient *GetNextConnectedClient(void);

	bool GetConnectedClient(int *iSlot);
	CRouterClient *GetConnectedClient(void);

	bool IsIPAddress(const char *sString);
	bool ResolveHost(const char *sHostName, char *sOutIP, int iMaxOutSz);

	bool Connect(const char *sHostOrIP, int iRemotePort, CRouterClient **pOutClient, ROUTEPUMPTHREADINFO *pThreadInfo);
	bool ConnectByHost(const char *sHostName, int iRemotePort, CRouterClient **pOutClient, ROUTEPUMPTHREADINFO *pThreadInfo);
	bool ConnectByIP(const char *sIPAddress, int iRemotePort, CRouterClient **pOutClient, ROUTEPUMPTHREADINFO *pThreadInfo);

	double MaxInactivityTime(void);
	void MaxInactivityTime(double dSeconds);

	void AllowNewConnections(bool bAllow);

	bool PausePumpThread(void);
	bool ResumePumpThread(void);

	void NextPeerID(int iPeerID);
	int NextPeerID(void);

	int ListenPort(void);
	int ListenPort(int iNewPort);
	void MaxBufferSize(int iSize);
	int MaxBufferSize(void);
	void InitialBufferSize(int iSize);
	int InitialBufferSize(void);

	VOID *UserData(void);
	void UserData(VOID *pData);

	int CurrentConnections(void);
	int MaxConnections(void);

	int DataPumpThreads(void);

	bool IsActive(void);
	bool IsInitialized(void);

	bool Start(void);
	bool Stop(bool bForce);

	bool AddListenAddress(const char *sIPv4);
	void ClearListenAddresses(void);

	bool Destroy(void);

	bool Initialize(int iListenPort, int iMaxConnections, int iMaxDataPumpThreads, int iDataPumpSpawnThreshold, int iMaxResolverThreads, int iResolverSpawnThreshold);

private: //Various internal functions.
	bool p_CloseListenSocket(void);
	void p_DisconnectClient(CRouterClient *pClient, ROUTEPUMPTHREADINFO *pThreadInfo);
	bool p_Accept(SOCKET listenSocket, ROUTEPUMPTHREADINFO *pThreadInfo);
	bool p_Connect(const char *sIPAddress, int iRemotePort, CRouterClient **pOutClient, ROUTEPUMPTHREADINFO *pThreadInfo);
	bool p_Receive(CRouterClient *pClient);
	bool p_Send(CRouterClient *pClient);
	bool p_InitializeClientVariables(CRouterClient *pClient);
	bool p_OpenListenSocket(int iListenPort);
	bool p_StartWindowsSocket(void);
	bool p_StopWindowsSocket(void);
	bool p_ThrowError(int iErrorNumber);
	bool p_ThrowError(int iErrorNumber, CRouterClient *pClient);
	CRouterClient *p_GetFreeSocketSlot(int *pOutSlot, ROUTEPUMPTHREADINFO *pThreadInfo);
	void p_ResetClientSlot(CRouterClient *pClient);

	#ifdef _DEBUG
	#if defined(_MSC_VER) && (_MSC_VER >= 1400)
	void p_SetThreadName(const char *sThreadName);
	#endif
	#endif

public:
	VOID *pUserData;	// Generic pointer for storing data (feel free to use).
	
private: //Various internal variables.
	bool p_bActive;					// Is the server started?
	bool p_bIsInitialized;			// is the class initialized?
	bool p_bAllowNewConnections;	// Does the class allow new connection (Accept / Connect)?
	int p_iListenPort;				// The port the server was set to listen on (if any).

	ROUTELISTENADDRESSES p_ListenAddrs;  // The IP addresses to listen on (of all if not set).

	CRouterClient *p_Clients;		// The Conneced Client Information structure.

	int p_iInitialBufferSize;		// The initial size of the per-client buffer.
	int p_iMaxBufferSize;			// The maximum size of the per-client buffer.
	int p_iCurrentConnections;		// The number of clients connected to the server.
	int p_iMaxConnections;			// Max clients allowed to connect.
	int p_iNextPeerID;				// The next assigned Client ID.
	int p_iSocketLimit;				// The maximum number of sockets allowed by winsock.
	double p_dMaxInactivityTime;	// The maximum amount of time (in seconds) that a peer is alowed to be idle.

	ROUTELISTENSOCKETS p_ListenSockets;  // Listen socket(s) of local server.

	//------------------------------------------------------------------------
	//The DataPump routine threading items (Begin)
	//------------------------------------------------------------------------
	friend unsigned __stdcall p_RouterDataPumpThreadProc(void *pData);

	bool ShouldThreadAcceptConnections(ROUTEPUMPTHREADINFO *pThreadInfo);
	unsigned int p_DataPump(ROUTEPUMPTHREADINFO *pThreadInfo);
	void p_AddDataPump(void);
	void p_StopDataPumps(void);

	int p_iMaxDataPumpThreads;			// The max number of pump threads which can be used by the route.
	int p_iDataPumpThreads;				// The number of pump threads used by the route.
	int p_iDataPumpSpawnThreshold;		// The level of load that the route must encounter before spawining an additional pump thread.
	int p_iResolverSpawnThreshold;		// The level of load that the route must encounter before spawining an additional resolver thread.
	int p_iLastAcceptingThreadNumber;
	bool p_bShutdownDataPump;
	ROUTEPUMPTHREADINFO *p_RoutePumpThreadInfo; //Array of pump thread info structures.
	CRITICAL_SECTION p_PumpCS;
	//------------------------------------------------------------------------
	//The DataPump routine threading items (End)
	//------------------------------------------------------------------------

	//------------------------------------------------------------------------
	//The ResolverThread items (Begin)
	//------------------------------------------------------------------------
	friend unsigned __stdcall p_RouterResolverThreadThreadProc(void *pData);

	void p_AddResolverThread(void);
	void p_StopResolverThreads(void);
	unsigned int p_ResolverThread(ROUTERESOLVERTHREADINFO *pThreadInfo);

	int p_iMaxResolverThreads;			// The max number of resolver threads which can be used by the route.
	int p_iResolverThreads;				// The number of resolver threads used by the route.
	bool p_bShutdownResolverThread;
	ROUTERESOLVERTHREADINFO *p_RouteResolverThreadInfo; //Array of Resolver thread info structures.
	CRITICAL_SECTION p_ResolverCS;

	HANDLE p_hResolverKickOffEvent;

	CStack *p_ResolverStack;

	//------------------------------------------------------------------------
	//The ResolverThread items (End)
	//------------------------------------------------------------------------

	#ifndef _USE_ROUTE_GLOBAL_MEMPOOL
	CRITICAL_SECTION p_MemoryLock;
	#endif
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
