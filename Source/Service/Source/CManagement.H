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

#include "../../../../@Libraries/CStringBuilder/CStringBuilder.H"
#include "../../../../@Libraries/CSocketServer/CSocketServer.H"
#include "../../../../@Libraries/CNASCCL (Stream)/CNASCCL.H"
#include "../../../../@Libraries/CXML/CXML.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MAX_TEXT_SIZE		2048
#define MGMT_INITIAL_KEY	"109238uuaskjg09234j21487t7dfklnsdg09u124pok12rlksdafv087y215"
#define MGMT_REGISTRY_KEY	"4^%!%&94^HY27$*&!54`45b^%$&$*3568,3w567b2q3tv1CQ543q%B$#Q^%6"
#define MGMT_INITIAL_KEY_SZ	20
#define MGMT_MAX_PASS_SZ	1024

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CManagement {
	
public:
	~CManagement(void);

	bool ManagementInitialize(void);
	bool SendCommand(CSocketClient *pClient, const char *sText, ...);
	bool SendCommand(CSocketClient *pClient, LPBASICHUNK pReturnData, const char *sText, ...);
	bool SendCommand(CSocketClient *pClient, CXMLWriter *pXML);
	bool SendCommand(CSocketClient *pClient, CXMLReader *pXML);

protected:
	CSocketServer *Socket;
	CNASCCL Cytpto;
	bool bIsAuthorized;

	void TempStopAll(CStringBuilder *pGUIDS);
	void TempRestartAll(CStringBuilder *pGUIDS);

	bool ProcessCommand(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk);
	bool ProcessUrgentCommand(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk);

	friend void Mgmt_Sock_OnSockError(CSocketServer *pSock, CSocketClient *pClient, int iErrorNumber, const char *sErrorMsg);
	friend void Mgmt_Sock_ClientThread(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk);
	friend void MgMt_OnBeginClientThread(CSocketServer *pSock, CSocketClient *pClient, HANDLE hThread);
	friend void MgMt_OnEndClientThread(CSocketServer *pSock, CSocketClient *pClient);
	friend bool MgMt_OnAfterDePacketize(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk);
	friend bool MgMt_OnBeforePacketize(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk);
	friend bool MgMt_OnAccept(CSocketServer *pSock, CSocketClient *pClient);
};

extern CManagement *gManage;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

