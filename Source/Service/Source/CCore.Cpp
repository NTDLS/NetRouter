#ifndef _CCore_Cpp_
#define _CCore_Cpp_
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Entry.H"
#include "CCore.H"
#include "CManagement.H"

#include "../../../../@Libraries/CNASCCL (Stream)/CNASCCL.H"
#include "../../../../@Libraries/CSHA1/CSHA1.H"
#include "../../../../@Libraries/CStringBuilder/CStringBuilder.H"

using namespace NSWFL::DateTime;
using namespace NSWFL::File;
using namespace NSWFL::System;
using namespace NSWFL::String;
using namespace NSWFL::Registry;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCore::IsVerb(const char *sVerbsList, const char *sVerb)
{
	char sToken[128];
	int iRPos = 0;
	int iVerbListLength = (int)strlen(sVerbsList);

	while(GetNextToken(sVerbsList, iVerbListLength, sToken, sizeof(sToken), &iRPos, ",", 1) > 0)
	{
		if(_strcmpi(sToken, sVerb) == 0)
		{
			return true;
		}

		if(sVerbsList[iRPos] == ',')
		{
			iRPos++; //Skip the comma;
		}
		SkipWhiteSpaces(sVerbsList, iVerbListLength, &iRPos);
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCore::ParseHeaderVariables(CStringBuilder *pOutput, const char *sFormula, const char *sHTTPVersion,
	const char *sHTTPVerb, CRouterServer *pSock, ROUTE *pRoute, char **sActualHeaders, int iActualHeadersCount)
{
	char sToken[MAX_TOKEN_SZ];
	int iFormulaLength = (int)strlen(sFormula);
	int iOffset = 0;

	int iStartPos = 0;
	int iEndPos = 0;
	int iLastStartPos = 0;

	while(true)
	{
		iStartPos = CharIndex(sFormula + iOffset, iFormulaLength - iOffset, '[');
		iEndPos = CharIndex(sFormula + iOffset, iFormulaLength - iOffset, ']');

		if(iStartPos < 0 || iStartPos > iEndPos)
		{
			iStartPos = iFormulaLength - iLastStartPos;
			pOutput->Append(sFormula + iLastStartPos, iStartPos);
			return true;
		}

		pOutput->Append(sFormula + iLastStartPos, iStartPos);

		if(iEndPos > iStartPos)
		{
			int iTokenLength = (iEndPos - iStartPos) - 1;

			strncpy_s(sToken, sizeof(sToken), sFormula + iOffset + iStartPos + 1, iTokenLength);

			int iOpenParenPos = CharIndex(sToken, iTokenLength, '(');
			int iCloseParenPos = 0;
			int iVarLength = iOpenParenPos;
			char *sParam = "";

			if(iVarLength < 0)
			{
				iVarLength = iTokenLength;
			}

			//Remove whitespaces from the "variable name".
			while(IsWhiteSpace(sToken[iVarLength - 1]))
			{
				iVarLength--;
			}

			//Figure out wheather we have a parameter.
			if(iOpenParenPos > 0)
			{
				if((iCloseParenPos = CharIndex(sToken + iOpenParenPos, iTokenLength - iOpenParenPos, ')')))
				{
					sToken[iOpenParenPos + iCloseParenPos] = '\0';
					sParam = sToken + iOpenParenPos + 1;
				}
			}

			//--------------------------------------------------------------------------------------------------------------
			if(_strnicmp(sToken, "Date", iVarLength) == 0)
			{
				char sDate[MAX_TOKEN_SZ];
				SYSTEMTIME ST;
				GetLocalTime(&ST);
			    if(!GetDateFormat((LCID)NULL, (DWORD)NULL, &ST, sParam, sDate, sizeof(sDate)))
				{
					return false;
				}
				pOutput->Append(sDate);
			}
			//--------------------------------------------------------------------------------------------------------------
			else if(_strnicmp(sToken, "Time", iVarLength) == 0)
			{
				char sTime[MAX_TOKEN_SZ];
				SYSTEMTIME ST;
				GetLocalTime(&ST);
			    if(!GetTimeFormat((LCID)NULL, (DWORD)NULL, &ST, sParam, sTime, sizeof(sTime)))
				{
					return false;
				}
				pOutput->Append(sTime);
			}
			//--------------------------------------------------------------------------------------------------------------
			else if(_strnicmp(sToken, "Server.Memory.Total", iVarLength) == 0)
			{
				MEMORYSTATUSEX MS;
				memset(&MS, 0, sizeof(MS));
				MS.dwLength = sizeof(MS);
				if(!GlobalMemoryStatusEx(&MS))
				{
					return false;
				}
				pOutput->Append(MS.ullTotalPhys);
			}
			//--------------------------------------------------------------------------------------------------------------
			else if(_strnicmp(sToken, "Server.Memory.Free", iVarLength) == 0)
			{
				MEMORYSTATUSEX MS;
				memset(&MS, 0, sizeof(MS));
				MS.dwLength = sizeof(MS);
				if(!GlobalMemoryStatusEx(&MS))
				{
					return false;
				}
				pOutput->Append(MS.ullTotalPhys);
			}
			//--------------------------------------------------------------------------------------------------------------
			else if(_strnicmp(sToken, "Server.Memory.Used", iVarLength) == 0)
			{
				MEMORYSTATUSEX MS;
				memset(&MS, 0, sizeof(MS));
				MS.dwLength = sizeof(MS);
				if(!GlobalMemoryStatusEx(&MS))
				{
					return false;
				}
				pOutput->Append(MS.ullTotalPhys - MS.ullAvailPhys);
			}
			//--------------------------------------------------------------------------------------------------------------
			else if(_strnicmp(sToken, "Server.CPU.Count", iVarLength) == 0)
			{
				SYSTEM_INFO SI;
				GetSystemInfo(&SI);
				pOutput->Append(SI.dwNumberOfProcessors);
			}
			//--------------------------------------------------------------------------------------------------------------
			else if(_strnicmp(sToken, "Server.CPU.Name", iVarLength) == 0)
			{
				DWORD dwTokenSz = sizeof(sToken);
				memset(sToken, 0, dwTokenSz);
				if(!Get_StringRegistryValue(HKEY_LOCAL_MACHINE,
					"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", "ProcessorNameString", sToken, dwTokenSz))
				{
					return false;
				}

				Trim(sToken, dwTokenSz);
				pOutput->Append(sToken);
			}
			//--------------------------------------------------------------------------------------------------------------
			else if(_strnicmp(sToken, "Server.CPU.Speed", iVarLength) == 0)
			{
				DWORD dwCPUSpeed = 0;
				if(!Get_DWORDRegistryValue(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", "~MHz", dwCPUSpeed))
				{
					return false;
				}
				pOutput->Append(dwCPUSpeed);
			}
			//--------------------------------------------------------------------------------------------------------------
			else if(_strnicmp(sToken, "Server.OS.Name", iVarLength) == 0)
			{
		        if(!Get_OsVersion(sToken, sizeof(sToken)))
				{
					return false;
				}
				pOutput->Append(sToken);
			}
			//--------------------------------------------------------------------------------------------------------------
			else if(_strnicmp(sToken, "Server.Software.Name", iVarLength) == 0)
			{
				pOutput->Append(gsTitleCaption);
			}
			//--------------------------------------------------------------------------------------------------------------
			else if(_strnicmp(sToken, "Server.Software.Version", iVarLength) == 0)
			{
				pOutput->Append(gsFileVersion);
			}
			//--------------------------------------------------------------------------------------------------------------
			else if(_strnicmp(sToken, "HTTP.Version", iVarLength) == 0)
			{
				pOutput->Append(sHTTPVersion);
			}
			//--------------------------------------------------------------------------------------------------------------
			else if(_strnicmp(sToken, "HTTP.Verb", iVarLength) == 0)
			{
				pOutput->Append(sHTTPVerb);
			}
			//--------------------------------------------------------------------------------------------------------------
			else if(_strnicmp(sToken, "HTTP.Header", iVarLength) == 0)
			{
				char sActualHeaderField[MAX_TOKEN_SZ];
		
				for(int iActualHeaderIndex = 0; iActualHeaderIndex < iActualHeadersCount; iActualHeaderIndex++)
				{
					int iColonPos = 0;
					int iActualHeaderFieldLength = CleanWhiteSpaces(sActualHeaders[iActualHeaderIndex]);
					if(iActualHeaderFieldLength <= 0)
					{
						return false; //All white space.
					}

					if(GetNextToken(sActualHeaders[iActualHeaderIndex], iActualHeaderFieldLength, sActualHeaderField, sizeof(sActualHeaderField), &iColonPos, ":", 1) > 0)
					{
						if(_strcmpi(sActualHeaderField, sParam) == 0)
						{
							iColonPos++; //Skip the colon.
							while(IsWhiteSpace(sActualHeaders[iActualHeaderIndex][iColonPos]))
							{
								iColonPos++;
							}
							pOutput->Append(sActualHeaders[iActualHeaderIndex] + iColonPos);
							break;
						}
					}
				}
			}
			//--------------------------------------------------------------------------------------------------------------
			else if(_strnicmp(sToken, "Server.Host.Name", iVarLength) == 0)
			{
				if(!Get_MachineName(sToken, sizeof(sToken)))
				{
					return false;
				}
				pOutput->Append(sToken);
			}
			//--------------------------------------------------------------------------------------------------------------
			else if(_strnicmp(sToken, "Server.User.Name", iVarLength) == 0)
			{
				if(!Get_UserName(sToken, sizeof(sToken)))
				{
					return false;
				}
				pOutput->Append(sToken);
			}
			//--------------------------------------------------------------------------------------------------------------
		}

		iOffset += iEndPos + 1;
		iLastStartPos = iOffset;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCore::ProcessHTTPHeaders(CRouterServer *pSock, ROUTE *pRoute, LPROUTECHUNK pChunk)
{
	char sToken[MAX_TOKEN_SZ];
	int iHTTPScanLength = pChunk->iLength < 2048 ? pChunk->iLength : 2048;
	const char *pBuffer = (const char *)pChunk->pBuffer;

	int iLineBreakPos = CharIndex(pBuffer, iHTTPScanLength, '\n');
	if(iLineBreakPos < 0)
	{
		return; //Couldnt find the line break in a reasonable position.
	}

	char sHTTPVersion[128];
	char sHTTPVerb[128];
	bool bIsRequest = false;
	bool bIsResponse = false;
	int iRPos = 0;

	if(GetNextToken(pBuffer, iLineBreakPos, sToken, sizeof(sToken), &iRPos) > 0)
	{
		if(_strnicmp(sToken, "HTTP/", 5) == 0) //Parse RESPONSE header.
		{
			strcpy_s(sHTTPVersion, sizeof(sHTTPVersion), sToken);
			strcpy_s(sHTTPVerb, sizeof(sHTTPVerb), "");
			bIsResponse = true;
		}
		else if (IsHTTPVerb(sToken)) //Parse REQUEST header.
		{
			strcpy_s(sHTTPVerb, sizeof(sHTTPVerb), sToken);

			if(GetNextToken(pBuffer, iLineBreakPos, sToken, sizeof(sToken), &iRPos) > 0) //Skip the request.
			{
				if(GetNextToken(pBuffer, iLineBreakPos, sToken, sizeof(sToken), &iRPos) > 0)
				{
					if(_strnicmp(sToken, "HTTP/", 5) == 0)
					{
						strcpy_s(sHTTPVersion, sizeof(sHTTPVersion), sToken);
						bIsRequest = true;
					}
				}
			}
		}
	}

	if(!bIsRequest && ! bIsResponse)
	{
		return;
	}

	const char *sDelimiter = NULL;

	int iWhiteSpace = 0;
	int iActualHeaderLength = 0;
	while((iWhiteSpace = CharIndex(pBuffer + iActualHeaderLength, pChunk->iLength - iActualHeaderLength, '\n')) > 0)
	{
		iActualHeaderLength += iWhiteSpace;

		if((iActualHeaderLength != pChunk->iLength - 1))
		{
			//Look for one additional trailing line-feed.
			if(pBuffer[iActualHeaderLength + 1] == '\n')
			{
				sDelimiter = "\n";
				iActualHeaderLength += 2;
				break;
			}

			if((iActualHeaderLength < pChunk->iLength - 2))
			{
				//Look for one additional trailing carriage-return and line-feed.
				if(pBuffer[iActualHeaderLength - 1] == '\r' && pBuffer[iActualHeaderLength + 1] == '\r' && pBuffer[iActualHeaderLength + 2] == '\n')
				{
					sDelimiter = "\r\n";
					iActualHeaderLength += 3;
					break;
				}
			}
		}
		iActualHeaderLength++;
	}
	if(iActualHeaderLength < 0)
	{
		return; //Couldnt find the line break in a reasonable position.
	}

	CStringBuilder newBuffer;

	int iActualHeadersCount = 0;
	char **sActualHeaders = Split(pBuffer, iActualHeaderLength, &iActualHeadersCount, "\n", 1);

	//Rebuild the header, performing any necessary manipulation.
	for(int iActualHeaderIndex = 0; iActualHeaderIndex < iActualHeadersCount; iActualHeaderIndex++)
	{
		bool bhandled = false;
		int iActualHeaderFieldLength = CleanWhiteSpaces(sActualHeaders[iActualHeaderIndex]);
		if(iActualHeaderFieldLength <= 0)
		{
			continue; //All white space.
		}

		//Performing necessary manipulation.
		if(GetToken(sActualHeaders[iActualHeaderIndex], iActualHeaderFieldLength, sToken, sizeof(sToken), ":", 1) > 0)
		{
			for(int iRuleIndex = 0; iRuleIndex < pRoute->HTTPHeaders->Collection.Count; iRuleIndex++)
			{
				if( pRoute->HTTPHeaders->Collection.Items[iRuleIndex].Enabled
					&& (this->IsVerb(pRoute->HTTPHeaders->Collection.Items[iRuleIndex].Verbs, sHTTPVerb)
					|| (this->IsVerb(pRoute->HTTPHeaders->Collection.Items[iRuleIndex].Verbs, "<Any Request>") && bIsRequest)
					|| (this->IsVerb(pRoute->HTTPHeaders->Collection.Items[iRuleIndex].Verbs, "<Any Response>") && bIsResponse)
					|| this->IsVerb(pRoute->HTTPHeaders->Collection.Items[iRuleIndex].Verbs, "<Any>")))
				{
					if(_strcmpi(sToken, pRoute->HTTPHeaders->Collection.Items[iRuleIndex].Name) == 0)
					{
						if(pRoute->HTTPHeaders->Collection.Items[iRuleIndex].Method == HEADER_METHOD_DELETE
							|| pRoute->HTTPHeaders->Collection.Items[iRuleIndex].Method == HEADER_METHOD_ADDREPLACE)
						{
							//We remove headers that are marked to be deleted and that are marked to be
							//	added because we do not want to add duplicate headers to the collection.
						}
						else if(pRoute->HTTPHeaders->Collection.Items[iRuleIndex].Method == HEADER_METHOD_REPLACE)
						{
							CStringBuilder Text(64);
							if(CharIndex(pRoute->HTTPHeaders->Collection.Items[iRuleIndex].Text, '[') >= 0
								&& CharIndex(pRoute->HTTPHeaders->Collection.Items[iRuleIndex].Text, ']') >= 0)
							{
								if(!this->ParseHeaderVariables(&Text, pRoute->HTTPHeaders->Collection.Items[iRuleIndex].Text,
									sHTTPVersion, sHTTPVerb, pSock, pRoute, sActualHeaders, iActualHeadersCount))
								{
									Text.Clear();
								}
							}
							else {
								Text.Append(pRoute->HTTPHeaders->Collection.Items[iRuleIndex].Text);
							}

							if(Text.Length > 0)
							{
								newBuffer.AppendF("%s: %s%s",
									pRoute->HTTPHeaders->Collection.Items[iRuleIndex].Name,
									Text.Buffer, sDelimiter);
							}
						}

						bhandled = true;
						break; //No need to replace multiple times (right?).
					}
				}
			}
		}

		if(!bhandled)
		{
			newBuffer.Append(sActualHeaders[iActualHeaderIndex], iActualHeaderFieldLength);
			newBuffer.Append(sDelimiter);
		}
	}

	//Add additional headers.
	for(int iRuleIndex = 0; iRuleIndex < pRoute->HTTPHeaders->Collection.Count; iRuleIndex++)
	{
		if( pRoute->HTTPHeaders->Collection.Items[iRuleIndex].Enabled
			&& (this->IsVerb(pRoute->HTTPHeaders->Collection.Items[iRuleIndex].Verbs, sHTTPVerb)
			|| (this->IsVerb(pRoute->HTTPHeaders->Collection.Items[iRuleIndex].Verbs, "<Any Request>") && bIsRequest)
			|| (this->IsVerb(pRoute->HTTPHeaders->Collection.Items[iRuleIndex].Verbs, "<Any Response>") && bIsResponse)
			|| this->IsVerb(pRoute->HTTPHeaders->Collection.Items[iRuleIndex].Verbs, "<Any>")))
		{
			if(pRoute->HTTPHeaders->Collection.Items[iRuleIndex].Method == HEADER_METHOD_ADDREPLACE)
			{
				CStringBuilder Text(64);
				if(CharIndex(pRoute->HTTPHeaders->Collection.Items[iRuleIndex].Text, '[') >= 0
					&& CharIndex(pRoute->HTTPHeaders->Collection.Items[iRuleIndex].Text, ']') >= 0)
				{
					if(!this->ParseHeaderVariables(&Text, pRoute->HTTPHeaders->Collection.Items[iRuleIndex].Text,
						sHTTPVersion, sHTTPVerb, pSock, pRoute, sActualHeaders, iActualHeadersCount))
					{
						Text.Clear();
					}
				}
				else {
					Text.Append(pRoute->HTTPHeaders->Collection.Items[iRuleIndex].Text);
				}

				if(Text.Length > 0)
				{
					newBuffer.AppendF("%s: %s%s",
						pRoute->HTTPHeaders->Collection.Items[iRuleIndex].Name,
						Text.Buffer, sDelimiter);
				}
			}
		}
	}

	FreeArray(sActualHeaders, iActualHeadersCount);

	newBuffer.Append(sDelimiter); //Terminate the header.

	//Copy the chunk data to the "header" buffer.
	newBuffer.Append(pBuffer + iActualHeaderLength, pChunk->iLength - iActualHeaderLength);

	//Reallocate extra ram is necessary.
	if(pChunk->iAlloc < newBuffer.Length + 1)
	{
		pSock->ReAlloc(pChunk, newBuffer.Length + 1);
	}

	//Copy the result to the passed in buffer.
	pChunk->iLength = newBuffer.Length;
	memcpy_s(pChunk->pBuffer, pChunk->iAlloc, newBuffer.Buffer, newBuffer.Length);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CCore::GetLeastUsedIndex(int *iArray, int iArrayCount, int *iExclude, int iExcludeCount)
{
	int iLeastValue = -1;
	int iLeastIndex = -1;

	for(int i = 0; i < iArrayCount; i++)
	{
		if((iArray[i] < iLeastValue || iLeastValue == -1) && iArray[i] != -1)
		{
			for(int y = 0; y < iExcludeCount; y++)
			{
				if(i == iExclude[y])
				{
					continue;
				}
			}

			iLeastValue = iArray[i];
			iLeastIndex = i;
		}
	}

	return iLeastIndex;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Selects an appropriate remote host by performing all load-balance and fail-over logic and returns the connection.
*/
CRouterClient *CCore::ConnectPeer(CRouterServer *pSock, CRouterClient *pClient, ROUTE *pRoute, ROUTEPUMPTHREADINFO *pThreadInfo)
{
	CRouterClient *pPeer = NULL;

	//##(ROUTER_MAJOR_STEP:5)##############################################################################
	//## If necessary (due to setting) persist the companion end-point by Source IP. (AKA:Sticky Sessions).
	//#####################################################################################################
	if(pRoute->ConnectHosts->PersistBySourceIP)
	{
		int iHostIndex = pRoute->ConnectHosts->PersistenceMap->Get(pClient->PeerAddress());
		if(iHostIndex >= 0)
		{
			CONNECTHOST *pHost = &pRoute->ConnectHosts->Collection.Items[iHostIndex];

			if(pSock->Connect(pHost->Address, pHost->Port, &pPeer, pThreadInfo))
			{
				return pPeer; //Connected!
			}
		}
	}

	//##(ROUTER_MAJOR_STEP:6)##############################################################################
	//## Determine the load balance method and connect to the appropriate companion end-point.
	//#####################################################################################################
	if(pRoute->ConnectHosts->Collection.Count > 1)
	{
		//----------------------------------------------------------------------------------------------------------------------------------------------------------
		//Load Balance Method: Fail-Over Only and Round-Robin
		//----------------------------------------------------------------------------------------------------------------------------------------------------------
		if(pRoute->ConnectHosts->LoadBalanceMethod == LBM_FAILOVERONLY || pRoute->ConnectHosts->LoadBalanceMethod == LBM_ROUNDROBIN)
		{
			int iHostIndex = pRoute->ConnectHosts->LBI.LastConnectIndex;
			int iStartingIndex = iHostIndex;

			while(true)
			{
				CONNECTHOST *pHost = &pRoute->ConnectHosts->Collection.Items[iHostIndex];

				if(pHost->Enabled)
				{
					if(pSock->Connect(pHost->Address, pHost->Port, &pPeer, pThreadInfo))
					{
						if(pRoute->ConnectHosts->PersistBySourceIP)
						{
							pRoute->ConnectHosts->PersistenceMap->Add(pClient->PeerAddress(), iHostIndex);
						}
						break; //Connected!
					}
				}

				iHostIndex++;

				if(iHostIndex == pRoute->ConnectHosts->Collection.Count)
				{
					iHostIndex = 0;
				}

				if(iHostIndex == iStartingIndex)
				{
					break;
				}
			}

			if(pRoute->ConnectHosts->LoadBalanceMethod == LBM_ROUNDROBIN)
			{
				iHostIndex++;

				//This is a very simple way to do round-robin (with fail-over).
				if(iHostIndex >= pRoute->ConnectHosts->Collection.Count)
				{
					iHostIndex = 0;
				}
			}

			pRoute->ConnectHosts->LBI.LastConnectIndex = iHostIndex;
		}
		//----------------------------------------------------------------------------------------------------------------------------------------------------------
		//Load Balance Method: Least Connections
		//----------------------------------------------------------------------------------------------------------------------------------------------------------
		else if(pRoute->ConnectHosts->LoadBalanceMethod == LBM_LEASTCONNECTIONS)
		{
			int *iHostConnections = (int *) pMem->Allocate(sizeof(int),  pRoute->ConnectHosts->Collection.Count);
			int *iTriedConnections = (int *) pMem->Allocate(sizeof(int),  pRoute->ConnectHosts->Collection.Count);

			//Create a list of the connection counts associated with each host.
			for(int iConn = 0; iConn < pSock->CurrentConnections(); iConn++)
			{
				for(int iHost = 0; iHost < pRoute->ConnectHosts->Collection.Count; iHost++)
				{
					if(pRoute->ConnectHosts->Collection.Items[iHost].Enabled)
					{
						if(_strcmpi(pSock->Peer(iConn)->sPeerAddress, pRoute->ConnectHosts->Collection.Items[iHost].ResolvedIP) == 0)
						{
							iHostConnections[iHost]++;
						}
					}
					else {
						iHostConnections[iHost] = -1;
					}
				}
			}

			for(int iTryCount = 0; iTryCount < pRoute->ConnectHosts->Collection.Count; iTryCount++)
			{
				int iHostIndex = this->GetLeastUsedIndex(iHostConnections, pRoute->ConnectHosts->Collection.Count, iTriedConnections, iTryCount);
				if(iHostIndex >= 0)
				{
					CONNECTHOST *pHost = &pRoute->ConnectHosts->Collection.Items[iHostIndex];

					if(pSock->Connect(pHost->Address, pHost->Port, &pPeer, pThreadInfo))
					{
						if(pRoute->ConnectHosts->PersistBySourceIP)
						{
							pRoute->ConnectHosts->PersistenceMap->Add(pClient->PeerAddress(), iHostIndex);
						}

						break; //Connected!
					}
					else
					{
						iTriedConnections[iTryCount] = iHostIndex;
					}
				}
			}

			pMem->Free(iHostConnections);
			pMem->Free(iTriedConnections);
		}
		//----------------------------------------------------------------------------------------------------------------------------------------------------------
		//Load Balance Method: *Not Defined or Not Implemented*
		//----------------------------------------------------------------------------------------------------------------------------------------------------------
		else {
			//Connect to the first non-disabled host.
			for(int iHostIndex = 0; iHostIndex < pRoute->ConnectHosts->Collection.Count; iHostIndex++)
			{
				CONNECTHOST *pHost = &pRoute->ConnectHosts->Collection.Items[iHostIndex];
				if(pHost->Enabled)
				{
					if(pSock->Connect(pHost->Address, pHost->Port, &pPeer, pThreadInfo))
					{
						if(pRoute->ConnectHosts->PersistBySourceIP)
						{
							pRoute->ConnectHosts->PersistenceMap->Add(pClient->PeerAddress(), iHostIndex);
						}

						break; //Connected!
					}
				}
			}
		}
	}
	else {
		//There is only one defined host record, just connect to it.
		for(int iHostIndex = 0; iHostIndex < pRoute->ConnectHosts->Collection.Count; iHostIndex++)
		{
			CONNECTHOST *pHost = &pRoute->ConnectHosts->Collection.Items[iHostIndex];
			if(pHost->Enabled)
			{
				if(pSock->Connect(pHost->Address, pHost->Port, &pPeer, pThreadInfo))
				{
					if(pRoute->ConnectHosts->PersistBySourceIP)
					{
						pRoute->ConnectHosts->PersistenceMap->Add(pClient->PeerAddress(), iHostIndex);
					}

					break; //Connected!
				}
			}
		}
	}

	//##(ROUTER_MAJOR_STEP:7)##############################################################################
	//## If a connection was made, return it. Otherwise return NULL.
	//#####################################################################################################
	return pPeer;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CCore::CCore(void)
{
	InitializeCriticalSection(&this->_CS);
	this->Load();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCore::Load(void)
{
	char sConfigFile[MAX_PATH];
	char sErrorText[2048];

	memset(&this->Pumps, 0, sizeof(PUMPS));

	this->Routes = new CRoutes(this);

	sprintf_s(sConfigFile, sizeof(sConfigFile), "%s\\Routes.xml", gsDataPath);
	if(!this->Routes->Load(sConfigFile))
	{
		sprintf_s(sErrorText, sizeof(sErrorText), "Could not load \"%s\"", sConfigFile);
		pTrace->Trace(sErrorText);
	}

	for(int i = 0; i < this->Routes->Collection.Count; i++)
	{
		ROUTE *pRoute = &this->Routes->Collection.Items[i];
		if(pRoute->GUID)
		{
			PUMP *pPump = NULL;
			if((pPump = this->AddPump(pRoute)))
			{
				if(pRoute->AutoStart)
				{
					pPump->Server->Start();
				}
			}
		}
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CCore::~CCore(void)
{
	this->Unload();
	DeleteCriticalSection(&this->_CS);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCore::Unload(void)
{
	this->RemoveAllPumps();

	delete this->Routes;
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PUMP *CCore::GetFreePumpSlot(void)
{
	PUMP *pPump = NULL;
	int iSlot = 0;

	for(int i = 0; i < this->Pumps.Count; i++)
	{
		if(!this->Pumps.Items[i].Used)
		{
			pPump = &this->Pumps.Items[i];
			iSlot = i;
			break;
		}
	}

	if(!pPump)
	{
		EnterCriticalSection(&this->_CS);

		if(!(this->Pumps.Items = (PUMP *) pMem->ReAllocate(this->Pumps.Items, sizeof(PUMP), this->Pumps.Count + 1)))
		{
			LeaveCriticalSection(&this->_CS);
			return NULL;
		}

		LeaveCriticalSection(&this->_CS);

		iSlot = this->Pumps.Count++;
		pPump = &this->Pumps.Items[iSlot];
	}

	memset(pPump, 0, sizeof(PUMP));

	pPump->Slot = iSlot;
	pPump->Used = true;

	return pPump;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCore::CreateServer(PUMP *pPump, ROUTE *pRoute)
{
	pPump->Server = new CRouterServer();

	if(pPump->Server->Initialize(
		pRoute->LocalPort, pRoute->MaxConnections, pRoute->DataPumpThreads,
		pRoute->DataPumpSpawnThreshold, pRoute->ResolverThreads, pRoute->ResolverSpawnThreshold))
	{
		if(!pRoute->ListenIPs->ListenOnAll())
		{
			for(int iIP = 0; iIP < pRoute->ListenIPs->Collection.Count; iIP++)
			{
				if(pRoute->ListenIPs->Collection.Items[iIP].Enabled)
				{
					pPump->Server->AddListenAddress(pRoute->ListenIPs->Collection.Items[iIP].IP);
				}
			}
		}

		SOCKETSERVERUSERDATA *pUserData = (SOCKETSERVERUSERDATA *) pMem->Allocate(1, sizeof(SOCKETSERVERUSERDATA));
		pUserData->RouteGUID = pMem->StrDup(pRoute->GUID);
		pUserData->pCore = this;

		pPump->Server->pUserData = pUserData;
		pPump->Server->InitialBufferSize(pRoute->InitialBufferSize);
		pPump->Server->MaxBufferSize(pRoute->MaxBufferSize);
		pPump->Server->MaxInactivityTime(pRoute->IdleDisconnectTimeout);

		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCore::DeleteServer(PUMP *pPump)
{
	if(pPump->Server)
	{
		if(pPump->Server->IsActive())
		{
			pPump->Server->Stop(true);
		}

		if(pPump->Server->pUserData)
		{
			if(((SOCKETSERVERUSERDATA *)pPump->Server->pUserData)->RouteGUID)
			{
				pMem->Free(((SOCKETSERVERUSERDATA *)pPump->Server->pUserData)->RouteGUID);
			}

			pMem->Free(pPump->Server->pUserData);
		}

		pPump->Server->Destroy();
		delete pPump->Server;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PUMP *CCore::AddPump(ROUTE *pRoute)
{
	PUMP *pPump = this->GetFreePumpSlot();
	if(!pPump)
	{
		return NULL;
	}

	int iGUIDLength = (int)strlen(pRoute->GUID);

	pPump->RouteGUID = pMem->StrDup(pRoute->GUID);

	if(this->CreateServer(pPump, pRoute))
	{
		return pPump;
	}

	this->RemovePump(pPump); //An error occured, remove the pump.

	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CCore::RemoveAllPumps(void)
{
	int iFreedCount = 0;

	if(this->Pumps.Items)
	{
		for(int i = 0; i < this->Pumps.Count; i++)
		{
			if(this->Pumps.Items[i].Used)
			{
				if(this->RemovePump(i))
				{
					iFreedCount++;
				}
			}
		}
		pMem->Free(this->Pumps.Items);
	}

	memset(&this->Pumps, 0, sizeof(PUMPS));

	return iFreedCount;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCore::RemovePump(PUMP *pPump)
{
	return this->RemovePump(pPump->Slot);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCore::RemovePump(int iSlot)
{
	if(iSlot >= 0 && iSlot < this->Pumps.Count)
	{
		PUMP *pPump = &this->Pumps.Items[iSlot];
		if(pPump->Used)
		{
			this->DeleteServer(pPump);
			
			if(pPump->RouteGUID)
			{
				pMem->Free(pPump->RouteGUID);
			}

			memset(pPump, 0, sizeof(PUMP));
			return true;
		}
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PUMP *CCore::PumpFromGUID(const char *sGUID)
{
	for(int i = 0; i < this->Pumps.Count; i++)
	{
		if(this->Pumps.Items[i].Used)
		{
			if(strcmp(this->Pumps.Items[i].RouteGUID, sGUID) == 0)
			{
				return &this->Pumps.Items[i];
			}
		}
	}

	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCore::Reload(void)
{
	if(this->Unload())
	{
		return this->Load();
	}
	return false;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCore::AllowRemoteAdministration(void)
{
	return Get_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "AllowRemoteAdministration");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCore::SetPassword(const char *sPassword)
{
	CNASCCL nasccl(MGMT_REGISTRY_KEY);

	char sRegistryPassword[MGMT_MAX_PASS_SZ];

	if(SimpleSHA1(sPassword, (int)strlen(sPassword), sRegistryPassword, sizeof(sRegistryPassword)))
	{
		return Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "Password", sRegistryPassword);
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCore::CheckPassword(const char *sPassword)
{
	CNASCCL nasccl(MGMT_REGISTRY_KEY);

	char sPasswordHash[MGMT_MAX_PASS_SZ];
	char sRegistryHash[MGMT_MAX_PASS_SZ];
	DWORD sRegistryHashLength = sizeof(sRegistryHash);

	if(Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "Password", sRegistryHash, sRegistryHashLength))
	{
		if(SimpleSHA1(sPassword, (int)strlen(sPassword), sPasswordHash, sizeof(sPasswordHash)))
		{
			return (_strcmpi(sRegistryHash, sPasswordHash) == 0);
		}
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CCore::AllowRemoteAdministration(bool bAllow)
{
	return Set_BOOLRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "AllowRemoteAdministration", bAllow);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
