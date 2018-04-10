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
#include "Routines.H"
#include "CManagement.H"

#include "../Dialogs/MainDialog.H"

#include "../../../../@Libraries/CXML/CXML.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Conversion;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CManagement *gManage = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CManagement::ProcessUrgentCommand(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk)
{
	BASICHUNK CmdData;
	int iCmdLen = 0;

	if(pSock->CompareCommandAndBreakData(pChunk, "@Message->", &CmdData))
	{
		MessageBox(GetActiveWindow(), (char *)CmdData.pBuffer, gsTitleCaption, MB_ICONINFORMATION);
		pSock->Free(&CmdData);
		return true;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CManagement::SendCommand(const char *sText, ...)
{
	bool bResult = false;

	va_list ArgList;
	va_start(ArgList, sText);

	int iMemoryRequired = _vscprintf(sText, ArgList);
	char *sBuf = (char *) pMem->Allocate(sizeof(char), iMemoryRequired + 1);
	int iSz = _vsprintf_s_l(sBuf, iMemoryRequired + 1, sText, NULL, ArgList);
	va_end(ArgList);
	if(iSz > 0)
	{
		bResult = this->Client->SetNextSendData(sBuf, iSz);
	}
	pMem->Free(sBuf);

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CManagement::QueryBoolean(bool *bValue)
{
	EnterCriticalSection(&this->cs);

	BASICHUNK Data;
	memset(&Data, 0, sizeof(Data));
	if(this->Client->GetNextClientData(&Data))
	{
		*bValue = ParseBoolean((const char *)Data.pBuffer);
		LeaveCriticalSection(&this->cs);
		return true;
	}
	this->Socket->Free(&Data);
	LeaveCriticalSection(&this->cs);

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CManagement::QueryBoolean(bool *bValue, const char *sText, ...)
{
	EnterCriticalSection(&this->cs);

	bool bResult = false;

	va_list ArgList;
	va_start(ArgList, sText);

	int iMemoryRequired = _vscprintf(sText, ArgList);
	char *sBuf = (char *) pMem->Allocate(sizeof(char), iMemoryRequired + 1);
	int iSz = _vsprintf_s_l(sBuf, iMemoryRequired + 1, sText, NULL, ArgList);
	va_end(ArgList);
	if(iSz > 0)
	{
		if(this->Client->SetNextSendData(sBuf, iSz))
		{
			BASICHUNK Data;
			memset(&Data, 0, sizeof(Data));
			if((bResult = this->Client->GetNextClientData(&Data)))
			{
				*bValue = ParseBoolean((const char *)Data.pBuffer);
			}
			this->Socket->Free(&Data);
		}
	}
	pMem->Free(sBuf);
	LeaveCriticalSection(&this->cs);

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CManagement::QueryXMLReader(CXMLReader *pXML, const char *sText, ...)
{
	EnterCriticalSection(&this->cs);

	bool bResult = false;

	va_list ArgList;
	va_start(ArgList, sText);

	int iMemoryRequired = _vscprintf(sText, ArgList);
	char *sBuf = (char *) pMem->Allocate(sizeof(char), iMemoryRequired + 1);
	int iSz = _vsprintf_s_l(sBuf, iMemoryRequired + 1, sText, NULL, ArgList);
	va_end(ArgList);
	if(iSz > 0)
	{
		if(this->Client->SetNextSendData(sBuf, iSz))
		{
			BASICHUNK Data;
			memset(&Data, 0, sizeof(Data));
			if(this->Client->GetNextClientData(&Data))
			{
				if(pXML->Initialize((const char *)Data.pBuffer, Data.iLength))
				{
					bResult = true;
				}
			}
			this->Socket->Free(&Data);
		}
	}
	pMem->Free(sBuf);
	LeaveCriticalSection(&this->cs);

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CManagement::QueryChunk(LPBASICHUNK pReturnData)
{
	EnterCriticalSection(&this->cs);
	bool bValue = this->Client->GetNextClientData(pReturnData);
	LeaveCriticalSection(&this->cs);
	return bValue;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CManagement::QueryChunk(LPBASICHUNK pReturnData, const char *sText, ...)
{
	EnterCriticalSection(&this->cs);

	bool bResult = false;

	va_list ArgList;
	va_start(ArgList, sText);

	int iMemoryRequired = _vscprintf(sText, ArgList);
	char *sBuf = (char *) pMem->Allocate(sizeof(char), iMemoryRequired + 1);
	int iSz = _vsprintf_s_l(sBuf, iMemoryRequired + 1, sText, NULL, ArgList);
	va_end(ArgList);
	if(iSz > 0)
	{
		if(this->Client->SetNextSendData(sBuf, iSz))
		{
			bResult = this->Client->GetNextClientData(pReturnData);
		}
	}
	pMem->Free(sBuf);
	LeaveCriticalSection(&this->cs);

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Mgmt_Sock_OnSockError(CSocketServer *pSock, CSocketClient *pClient, int iErrorNumber, const char *sErrorMsg)
{
	MessageBox(NULL, sErrorMsg, gsTitleCaption, MB_ICONERROR|MB_TASKMODAL);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MgMt_OnAfterDePacketize(CSocketServer *pSock, CSocketClient *pClient, LPBASICHUNK pChunk)
{
	CManagement *pMgmt  = ((CManagement*)pSock->pUserData);
	pMgmt->Cytpto.Cipher(pChunk->pBuffer, pChunk->iLength);

	//printf("Recv: %s\n", pChunk->pBuffer);

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
	//printf("Send: %s\n", pChunk->pBuffer);
	pMgmt->Cytpto.Cipher(pChunk->pBuffer, pChunk->iLength);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MgMt_OnAcceptConnect(CSocketServer *pSock, CSocketClient *pClient)
{
	CManagement *pMgmt  = ((CManagement*)pSock->pUserData);

	/*
	#ifdef _DEBUG
	AllocConsole();

	//Redirect stdout to the newly allocated console.
	int hCrt, i;
	FILE *hf;
	hCrt = _open_osfhandle((long) GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT);
	hf = _fdopen( hCrt, "w" );
	*stdout = *hf;
	i = setvbuf( stdout, NULL, _IONBF, 0);

	printf("%s Service %s\n\n", gsTitleCaption, gsFileVersion);
	#endif
	*/

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MgMt_OnDisconnect(CSocketServer *pSock, CSocketClient *pClient)
{
	SendNotifyMessage(gMainDialog, UM_MGMT_DISCONNECTED, 0, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CManagement::ManagementInitialize()
{
	InitializeCriticalSection(&this->cs);

	this->Socket = new CSocketServer();
	this->Cytpto.InitializeCryptography(MGMT_INITIAL_KEY, MGMT_INITIAL_KEY_SZ, true);
	this->Socket->pUserData = this;

	memset(&this->CurrentConnectioninfo, 0, sizeof(this->CurrentConnectioninfo));

	if(this->Socket->Initialize(Mgmt_Sock_OnSockError))
	{
		this->Socket->pUserData = this;

		this->Socket->SetOnAcceptConnect(MgMt_OnAcceptConnect);
		this->Socket->SetOnDisconnect(MgMt_OnDisconnect);
		this->Socket->SetOnAfterDePacketize(MgMt_OnAfterDePacketize);
		this->Socket->SetOnBeforePacketize(MgMt_OnBeforePacketize);

		if(this->Socket->Start())
		{
			return true;
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CManagement::Disconnect()
{
	memset(&this->CurrentConnectioninfo, 0, sizeof(this->CurrentConnectioninfo));
	return this->Client->Disconnect();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CManagement::Connect(const char *sHostRrIP)
{
	memset(&this->CurrentConnectioninfo, 0, sizeof(this->CurrentConnectioninfo));

	if(this->Socket->Connect(sHostRrIP, dwManagementPort, &this->Client))
	{
		this->CurrentConnectioninfo.Port = dwManagementPort;
		strcpy_s(this->CurrentConnectioninfo.Host, sizeof(this->CurrentConnectioninfo.Host), sHostRrIP);
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CManagement::ManagementDestroy()
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

	memset(&this->CurrentConnectioninfo, 0, sizeof(this->CurrentConnectioninfo));

	this->Cytpto.DestroyCryptography();

	DeleteCriticalSection(&this->cs);

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
