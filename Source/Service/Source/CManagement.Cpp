///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _Management_Cpp
#define _Management_Cpp
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <IO.H>
#include <fcntl.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Entry.H"
#include "CManagement.H"
#include "CCore.H"
#include "ServiceConsole.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::KeyGeneration;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CManagement::~CManagement(void)
{
	if(this->Socket)
	{
		if(this->Socket->IsActive())
		{
			this->Socket->Stop(true);
		}
		this->Socket->Destroy();
		delete this->Socket;
	}

	this->Cytpto.DestroyCryptography();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CManagement::ProcessUrgentCommand(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk)
{
	BASICHUNK CmdData;

	if(!this->bIsAuthorized)
	{
		if(pSock->CompareCommandAndBreakData(pChunk, "::Login|", &CmdData))
		{
			CXMLReader xml((char *)CmdData.pBuffer, CmdData.iLength);
			pSock->Free(&CmdData);

			bool bResult = false;
			char sPassword[MGMT_MAX_PASS_SZ];

			xml.ToString("Password", sPassword, sizeof(sPassword));

			//FIXFIX: Check login.
			SendCommand(pClient, pCore->CheckPassword(sPassword) ? "Yes" : "No");
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CManagement::TempStopAll(CStringBuilder *pGUIDS)
{
	bool *bWasRunning = (bool *) pMem->Allocate(sizeof(bool), pCore->Pumps.Count);

	for(int i = 0; i < pCore->Pumps.Count; i++)
	{
		if((bWasRunning[i] =  pCore->Pumps.Items[i].Server->IsActive()))
		{
			pGUIDS->AppendF("%s,", pCore->Pumps.Items[i].RouteGUID);
			pCore->Pumps.Items[i].Server->Stop(true);
		}
	}

	if(pGUIDS->Length > 0)
	{
		pGUIDS->Terminate(pGUIDS->Length - 1); //Remove trailing comma.
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CManagement::TempRestartAll(CStringBuilder *pGUIDS)
{
	int iItems = 0;
	char **sItems = Split(pGUIDS->Buffer, pGUIDS->Length, &iItems, ",");
	for(int i = 0; i < iItems; i++)
	{
		PUMP *pPump = pCore->PumpFromGUID(sItems[i]);
		if(pPump)
		{
			pPump->Server->Start();
		}
	}

	FreeArray(sItems, iItems);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CManagement::ProcessCommand(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk)
{
	BASICHUNK CmdData;
	int iCmdLen = 0;

	memset(&CmdData, 0, sizeof(CmdData));

	pTrace->Start(TRACE_TYPE_SPECIAL);

	//---------------------------------------------------------------------------------------------------------------------
	if(pSock->CompareCommand(pChunk, "::Server:OpenConsole"))
	{
		OpenConsole();
	}
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Information:Server:Performance", &CmdData))
	{
		CXMLWriter xml("Performance");
		SendCommand(pClient, &xml);
	} //::Information:Server:Performance|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Security:AlterLogin", &CmdData))
	{
		CXMLReader xmlLogin((char *)(CmdData).pBuffer);

		char sPassword[MGMT_MAX_PASS_SZ];
		xmlLogin.ToString("Password", sPassword, sizeof(sPassword));

		CStringBuilder errorText;
		bool bResult = true;
		bResult = pCore->Reload();

		bResult = pCore->AllowRemoteAdministration(xmlLogin.ToBoolean("AllowRemoteAdministration")) && bResult;
		bResult = pCore->SetPassword(sPassword) && bResult;

		pTrace->Start(TRACE_TYPE_SPECIAL);

		CXMLWriter xml("Result");
		xml.AddBool("Value", bResult);
		xml.Add("Text", errorText.Buffer);
		SendCommand(pClient, &xml);
	} //::Security:AlterLogin
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Server:Restart", &CmdData))
	{
		CStringBuilder errorText;
		bool bResult = true;
		bResult = pCore->Reload();
		bResult = pTrace->Stop(&errorText) == 0 && bResult;
		pTrace->Start(TRACE_TYPE_SPECIAL);

		CXMLWriter xml("Result");
		xml.AddBool("Value", bResult);
		xml.Add("Text", errorText.Buffer);
		SendCommand(pClient, &xml);
	} //::Server:Restart
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Config:Get:AllowRemoteAdministration", &CmdData))
	{
		CXMLWriter xmlConfig("AllowRemoteAdministration");
		xmlConfig.AddBool("Value", pCore->AllowRemoteAdministration());
		SendCommand(pClient, &xmlConfig);
	} //::Config:Get:AllowRemoteAdministration
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Config:Get:Routes", &CmdData))
	{
		CXMLReader xml;
		if(pCore->Routes->ToXML(&xml, true))
		{
			SendCommand(pClient, &xml);
		}
		else {
			pTrace->Trace("Routes->ToXML failed.");
		}
	} //::Config:Get:Routes
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Config:Get:Route|", &CmdData))
	{
		const char *sGUID = (char *)(CmdData).pBuffer;

		ROUTE *pRoute = pCore->Routes->FromGUID(sGUID);
		if(pRoute)
		{
			CXMLReader xml;
			if(pCore->Routes->ToXML(pRoute, &xml, true))
			{
				SendCommand(pClient, &xml);
			}
			else {
				pTrace->Trace("Route->ToXML failed.");
			}
		}
		else {
			pTrace->Trace("No such route exists.");
		}
	} //::Config:Get:Route|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Start:Route|", &CmdData))
	{
		const char *sGUID = (char *)(CmdData).pBuffer;

		PUMP *pPump = pCore->PumpFromGUID(sGUID);
		if(pPump)
		{
			CStringBuilder errorText;
			bool bResult = true;
			bResult = pPump->Server->Start();
			bResult = pTrace->Stop(&errorText) == 0 && bResult;
			pTrace->Start(TRACE_TYPE_SPECIAL);

			CXMLWriter xml("Result");
			xml.AddBool("Value", bResult);
			xml.Add("Text", errorText.Buffer);
			SendCommand(pClient, &xml);
		}
	} //::Start:Route|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Stop:Route|", &CmdData))
	{
		const char *sGUID = (char *)(CmdData).pBuffer;

		PUMP *pPump = pCore->PumpFromGUID(sGUID);
		if(pPump)
		{
			CStringBuilder errorText;
			bool bResult = true;
			bResult = pPump->Server->Stop(true);
			bResult = pTrace->Stop(&errorText) == 0 && bResult;
			pTrace->Start(TRACE_TYPE_SPECIAL);

			CXMLWriter xml("Result");
			xml.AddBool("Value", bResult);
			xml.Add("Text", errorText.Buffer);
			SendCommand(pClient, &xml);
		}
	} //::Stop:Route|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Config:Add:Route|", &CmdData))
	{

		CStringBuilder runningGuids;

		CXMLReader xmlRoute((char *)(CmdData).pBuffer);

		bool bResult = false;
		bool bIsResizeRequired = false;

		if((bIsResizeRequired = pCore->Routes->IsResizeRequired()))
		{
			this->TempStopAll(&runningGuids); //All routes have to be stopped because the collection may be relocated.
		}

		ROUTE *pRoute = pCore->Routes->Add(&xmlRoute);
		if(pRoute)
		{
			PUMP *pPump = pCore->AddPump(pRoute);
			if(pPump)
			{
				if(pRoute->AutoStart)
				{
					bResult = pPump->Server->Start();
				}
				else {
					bResult = true;
				}

				if(bResult)
				{
					bResult = pCore->Routes->Save();
				}
			}
		}

		if(bIsResizeRequired)
		{
			this->TempRestartAll(&runningGuids); //All routes have to be restarted because the collection may be relocated.
		}

		CStringBuilder errorText;
		bResult = pTrace->Stop(&errorText) == 0 && bResult;
		pTrace->Start(TRACE_TYPE_SPECIAL);

		CXMLWriter xml("Result");
		xml.AddBool("Value", bResult);
		xml.Add("Text", errorText.Buffer);
		SendCommand(pClient, &xml);
	} //::Config:Add:Route|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Config:Update:Route|", &CmdData))
	{
		CXMLReader xmlRoute((char *)(CmdData).pBuffer);

		bool bResult = (pCore->Routes->Update(&xmlRoute) != NULL);

		bResult = pCore->Routes->Save() && bResult;

		CStringBuilder errorText;
		bResult = pTrace->Stop(&errorText) == 0 && bResult;
		pTrace->Start(TRACE_TYPE_SPECIAL);

		CXMLWriter xml("Result");
		xml.AddBool("Value", bResult);
		xml.Add("Text", errorText.Buffer);
		SendCommand(pClient, &xml);
	} //::Config:Update:Route|
	//---------------------------------------------------------------------------------------------------------------------
	else if(pSock->CompareCommandAndBreakData(pChunk, "::Config:Delete:Route|", &CmdData))
	{
		const char *sGUID = (char *)(CmdData).pBuffer;

		PUMP *pPump = pCore->PumpFromGUID(sGUID);
		if(pPump)
		{
			CStringBuilder errorText;
			bool bResult = true;
			if(pPump->Server->IsActive())
			{
				bResult = pPump->Server->Stop(true);
			}

			bResult = pCore->RemovePump(pPump) && bResult;
			bResult = pCore->Routes->Destroy(sGUID) && bResult;
			if(bResult)
			{
				bResult = pCore->Routes->Save() && bResult;
			}
			bResult = pTrace->Stop(&errorText) == 0 && bResult;
			pTrace->Start(TRACE_TYPE_SPECIAL);

			CXMLWriter xml("Result");
			xml.AddBool("Value", bResult);
			xml.Add("Text", errorText.Buffer);
			SendCommand(pClient, &xml);
		}
	} //::Config:Delete:Route|
	//---------------------------------------------------------------------------------------------------------------------
	else{
		//printf("Unknown command: \"%s\"\n", pChunk->pBuffer);
	}
	//---------------------------------------------------------------------------------------------------------------------

	pSock->Free(&CmdData);

	CStringBuilder errorText;
	if(pTrace->Stop(&errorText) > 0)
	{
		CXMLWriter xml("Result");
		xml.AddBool("Value", false);
		xml.Add("Text", errorText.Buffer);
		SendCommand(pClient, &xml);
	}

	pTrace->Stop();

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CManagement::SendCommand(CSocketClient *pClient, const char *sText, ...)
{
	bool bResult = false;

	va_list ArgList;
	va_start(ArgList, sText);

	int iMemoryRequired = _vscprintf(sText, ArgList);
	char *sBuf = (char *) calloc(sizeof(char), iMemoryRequired + 1);
	int iSz = _vsprintf_s_l(sBuf, iMemoryRequired + 1, sText, NULL, ArgList);
	va_end(ArgList);
	if(iSz > 0)
	{
		bResult = pClient->SetNextSendData(sBuf, iSz);
	}
	free(sBuf);

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CManagement::SendCommand(CSocketClient *pClient, CXMLReader *pXML)
{
	const char *sText = pXML->Text();
	if(sText)
	{
		SendCommand(pClient, "%s", pXML->Text());
	}
	else{
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CManagement::SendCommand(CSocketClient *pClient, CXMLWriter *pXML)
{
	char *sText = pXML->Text();
	if(sText)
	{
		SendCommand(pClient, "%s", sText);
		free(sText);
	}
	else{
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CManagement::SendCommand(CSocketClient *pClient, LPBASICHUNK pReturnData, const char *sText, ...)
{
	bool bResult = false;

	va_list ArgList;
	va_start(ArgList, sText);

	int iMemoryRequired = _vscprintf(sText, ArgList);
	char *sBuf = (char *) calloc(sizeof(char), iMemoryRequired + 1);
	int iSz = _vsprintf_s_l(sBuf, iMemoryRequired + 1, sText, NULL, ArgList);
	va_end(ArgList);
	if(iSz > 0)
	{
		if(pClient->SetNextSendData(sBuf, iSz))
		{
			bResult = pClient->GetNextClientData(pReturnData);
		}
	}
	free(sBuf);

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Mgmt_Sock_OnSockError(CSocketServer *pSock, CSocketClient *pClient, int iErrorNumber, const char *sErrorMsg)
{
	pTrace->Lock();

	bool bTracingEnabled = pTrace->IsTracingEnabled();
	if(!bTracingEnabled)
	{
		pTrace->Start(TRACE_TYPE_SPECIAL);
	}

	pTrace->Trace(sErrorMsg);

	if(!bTracingEnabled)
	{
		pTrace->Stop();
	}

	pTrace->UnLock();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Mgmt_Sock_ClientThread(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk)
{
	while(pClient->IsConnected())
	{
		if(pClient->GetClientData(pChunk))
		{
			if(!((CManagement*)pSock->pUserData)->ProcessCommand(pSock, pClient, pChunk))
			{
				return;
			}
		}
		Sleep(1);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MgMt_OnAfterDePacketize(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk)
{
	CManagement *pMgmt  = ((CManagement*)pSock->pUserData);
	pMgmt->Cytpto.Cipher(pChunk->pBuffer, pChunk->iLength);

	if(((CManagement*)pSock->pUserData)->ProcessUrgentCommand(pSock, pClient, pChunk))
	{
		return false; //This packet was handled and should be removed from the queue.
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MgMt_OnBeforePacketize(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk)
{
	CManagement *pMgmt  = ((CManagement*)pSock->pUserData);
	pMgmt->Cytpto.Cipher(pChunk->pBuffer, pChunk->iLength);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MgMt_OnBeginClientThread(CSocketServer *pSock, CSocketClient *pClient, HANDLE hThread)
{
	CManagement *pMgmt  = ((CManagement*)pSock->pUserData);

	if(pSock->CurrentConnections() > 1)
	{
		pMgmt->SendCommand(pClient, "This server is already being administered.");
		pClient->WaitOnDataToBeSent();
		pClient->Disconnect();
	}
	else if(pClient->iSlot == 0) //Only slot zero it ever valid.
	{
		((CManagement*)pSock->pUserData)->bIsAuthorized = false;
		pMgmt->SendCommand(pClient, "Ok");
	}
	else {
		pMgmt->SendCommand(pClient, "The server rejected your connection.");
		pClient->WaitOnDataToBeSent();
		pClient->Disconnect();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MgMt_OnEndClientThread(CSocketServer *pSock, CSocketClient *pClient)
{
	if(pClient->iSlot == 0) //Only slot zero is ever valid.
	{
		((CManagement*)pSock->pUserData)->bIsAuthorized = false;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MgMt_OnAccept(CSocketServer *pSock, CSocketClient *pClient)
{
	if(pCore)
	{
		if(pCore->AllowRemoteAdministration())
		{
			return true;
		}
		else {
			return(strcmp(pClient->PeerAddress(), "127.0.0.1") == 0);
		}
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CManagement::ManagementInitialize()
{
	this->bIsAuthorized = false;

	this->Socket = new CSocketServer();
	this->Cytpto.InitializeCryptography(MGMT_INITIAL_KEY, MGMT_INITIAL_KEY_SZ, true);

	if(this->Socket->Initialize(dwManagementPort, 2, Mgmt_Sock_ClientThread, Mgmt_Sock_OnSockError))
	{
		this->Socket->pUserData = this;
		this->Socket->SetOnBeginClientThread(MgMt_OnBeginClientThread);
		this->Socket->SetOnEndClientThread(MgMt_OnEndClientThread);
		this->Socket->SetOnAfterDePacketize(MgMt_OnAfterDePacketize);
		this->Socket->SetOnBeforePacketize(MgMt_OnBeforePacketize);
		this->Socket->SetOnAccept(MgMt_OnAccept);

		return this->Socket->Start();
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
