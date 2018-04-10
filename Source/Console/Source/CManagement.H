///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _Management_H
#define _Management_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../../@Libraries/CSocketServer/CSocketServer.H"
#include "../../../../@Libraries/CXML/CXML.H"
#include "../../../../@Libraries/CNASCCL (Stream)/CNASCCL.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MAX_TEXT_SIZE		2048
#define MGMT_INITIAL_KEY	"109238uuaskjg09234j21487t7dfklnsdg09u124pok12rlksdafv087y215"
#define MGMT_INITIAL_KEY_SZ	20

#define MGMT_MAX_HOST_SZ	1024
#define MGMT_MAX_PASS_SZ	1024

typedef struct _tag_Current_Connection_info {
	char Host[MGMT_MAX_HOST_SZ];
	DWORD Port;
	char Username[MGMT_MAX_HOST_SZ];
	char Password[MGMT_MAX_PASS_SZ];
} MGMTCURRENTCONNECTIONINFO, *LPMGMTCURRENTCONNECTIONINFO;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CManagement {
	
public:
	bool ManagementDestroy(void);
	bool ManagementInitialize(void);
	bool SendCommand(const char *sText, ...);
	bool Connect(const char *sHostRrIP);
	bool Disconnect();

	bool QueryChunk(LPBASICHUNK pReturnData, const char *sText, ...);
	bool QueryXMLReader(CXMLReader *pXML, const char *sText, ...);
	bool QueryBoolean(bool *bValue, const char *sText, ...);

	bool QueryChunk(LPBASICHUNK pReturnData);
	bool QueryBoolean(bool *bValue);

	CSocketServer *Socket;
	CSocketClient *Client;
	CNASCCL Cytpto;
	CRITICAL_SECTION cs;
	
	MGMTCURRENTCONNECTIONINFO CurrentConnectioninfo;

protected:
	bool ProcessUrgentCommand(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk);

	friend void Mgmt_Sock_OnSockError(CSocketServer *pSock, CSocketClient *pClient, int iErrorNumber, const char *sErrorMsg);
	friend void MgMt_OnBeginClientThread(CSocketServer *pSock, CSocketClient *pClient, HANDLE hThread);
	friend void MgMt_OnEndClientThread(CSocketServer *pSock, CSocketClient *pClient);
	friend bool MgMt_OnAfterDePacketize(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk);
	friend bool MgMt_OnBeforePacketize(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk);
};

extern CManagement *gManage;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

