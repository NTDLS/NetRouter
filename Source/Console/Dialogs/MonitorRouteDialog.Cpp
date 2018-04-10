///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _MonitorRouteDialog_Cpp
#define _MonitorRouteDialog_Cpp
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <ShlObj.H>
#include <Stdio.H>
#include <ShlOBJ.H>
#include <Stdlib.H>
#include <Process.H>
#include <time.h>

#include <list>

#include "../../../../@Libraries/CGraph/CGraph.H"
#include "../../../../@Libraries/CStack/CStack.H"
#include "../Source/Entry.H"

#include "../../@Shared/Routes.h"
#include "../../@Shared/ListenIPs.H"

#include "../Dialogs/MonitorRouteDialog.H"
#include "../Dialogs/IPBindingDialog.H"
#include "../Dialogs/NetLogoDialog.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::String;
using namespace NSWFL::Conversion;
using namespace NSWFL::ListView;
using namespace NSWFL::File;
using namespace std;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct {
	CGraph *pGraph;
	char GUID[ROUTE_MAX_GUID];
	DWORD LastTickCount;
	__int64 LastBytesTx;
	HANDLE hEvent;
	bool bShutdown;
	HWND hWnd;
} MONITORDIALOGPROPS;

typedef struct _MonitorRouteThreadInfo {
	HANDLE hThread;
	HANDLE hEvent;
} MONITORROUTETHREADINFO, *LPMONITORROUTETHREADINFO;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------(Variable Declarations)
char sMonitorRouteDialogGUID[ROUTE_MAX_GUID];
std::list<MONITORDIALOGPROPS *> ActiveMonitorDialogs;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Graph is populated in this thread.
unsigned __stdcall MonitorGraphThreadProc(void *pVoid)
{
	HWND hWnd = (HWND) pVoid;

	MONITORDIALOGPROPS *pMDP = (MONITORDIALOGPROPS *) GetProp(hWnd, "Props");

	while(!pMDP->bShutdown)
	{
		CXMLReader xmlResult;
		if(gManage->QueryXMLReader(&xmlResult, "::Config:Get:Route|%s", pMDP->GUID))
		{
			xmlResult.ProgressiveScan(false);

			CXMLReader xml;
			if(xmlResult.ToReader("Route", &xml))
			{
				char sInt1[32];
				char sInt2[32];
				char sInt3[32];
				char sText[1024];
				char sName[ROUTE_MAX_NAME];
				char sLoadBalanceMethod[254];

				xml.ToString("Name", sName, sizeof(sName));
				xml.ToString("LoadBalanceMethod", sLoadBalanceMethod, sizeof(sLoadBalanceMethod));

				sprintf_s(sText, "Monitor %s", sName);
				SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)sText);

				CXMLReader xmlStatistics;
				CXMLReader xmlStatus;
				if(xml.ToReader("Status", &xmlStatus) && xml.ToReader("Statistics", &xmlStatistics))
				{
					DWORD dwThisTickCount = GetTickCount();
					DWORD dwLastTickCount = pMDP->LastTickCount;
					pMDP->LastTickCount = dwThisTickCount;
					
					__int64 i64ThisBytesTx = xmlStatistics.ToI64("BytesTransferred");
					__int64 i64LastBytesTx = pMDP->LastBytesTx;
					pMDP->LastBytesTx = i64ThisBytesTx;

					//Calculate the transfer rate.
					double dTxRate = 0;
					if(dwLastTickCount != 0)
					{
						double dElapsed = ((double)(dwThisTickCount - dwLastTickCount)) / 1000.0;
						double dDelta = ((double)i64ThisBytesTx - i64LastBytesTx);
						dTxRate = dDelta * dElapsed;
					}

					if(dTxRate >= 0)
					{
						double dValues[2];
						dValues[0] = xmlStatus.ToInteger("CurrentConnections");
						dValues[1] = dTxRate;
						pMDP->pGraph->Update(dValues);
						pMDP->pGraph->Redraw();
					
						Set_Text(GetDlgItem(hWnd, IDC_ROUTENAME), sName);
						Set_LongFormatted(GetDlgItem(hWnd, IDC_SOURCEPORT), xml.ToInteger("LocalPort"));

						int iThreads = xml.ToInteger("DataPumpThreads");
						int iResolvers = xml.ToInteger("ResolverThreads");

						sprintf_s(sText, sizeof(sText), "%s / %s thread%s (%s Resolver%s)",
							FormatInteger(sInt1, sizeof(sInt1), xml.ToInteger("MaxConnections")),
							FormatInteger(sInt2, sizeof(sInt2), iThreads),
							iThreads != 1 ? "s" : "",
							FormatInteger(sInt3, sizeof(sInt3), iResolvers),
							iResolvers != 1 ? "s" : ""
						);

						Set_Text(GetDlgItem(hWnd, IDC_MAXCONNECTIONS), sText);
						Set_Text(GetDlgItem(hWnd, IDC_LOADBALANCEMETHOD), sLoadBalanceMethod);
						Set_LongFormatted(GetDlgItem(hWnd, IDC_CURRENTCONNECTIONS), xml.ToInteger("CurrentConnections"));
						Set_LongFormatted(GetDlgItem(hWnd, IDC_TOTALCONNECTIONS), xml.ToInteger("TotalConnections"));
						Set_Text(GetDlgItem(hWnd, IDC_TOTALTRANSFERRED), FileSizeFriendly(i64ThisBytesTx, sText, sizeof(sText)));

						FileSizeFriendly(dTxRate, sText, sizeof(sText));
						strcat_s(sText, sizeof(sText), "/s");
						Set_Text(GetDlgItem(hWnd, IDC_TRANSFERRATE), sText);
					}

					xmlStatus.Destroy();
					xmlStatistics.Destroy();
				}
				xml.Destroy();
			}
			else {
				break;
			}
		}
		else {
			break;
		}
		
		WaitForSingleObject(pMDP->hEvent, INFINITE);
	}

	RemoveProp(pMDP->hWnd, "Props");

	EndDialog(pMDP->hWnd, 0);

	while(IsWindow(pMDP->hWnd))
	{
		Sleep(1);
	}

	delete pMDP->pGraph;
	CloseHandle(pMDP->hEvent);
	free(pMDP);
	
	CloseHandle(GetCurrentThread());

	_endthreadex(0);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Monitor dialog runs in this thread.
unsigned __stdcall MonitorRouteThreadProc(void *pVoid)
{
	MONITORROUTETHREADINFO mrti;
	memcpy(&mrti, pVoid, sizeof(MONITORROUTETHREADINFO));
	SetEvent(mrti.hEvent);

	DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_MONITORROUTE), NULL, MonitorRouteDialog);

	CloseHandle(GetCurrentThread());

	_endthreadex(0);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MonitorRoute(const char *sGUID)
{
	char sEventName[255];
	sprintf_s(sEventName, sizeof(sEventName), "MRT_%s", sGUID);

	for (list<MONITORDIALOGPROPS *>::const_iterator it = ActiveMonitorDialogs.begin(); it != ActiveMonitorDialogs.end(); it++)
	{
		if (_strcmpi((*it)->GUID, sGUID) == 0)
		{
			SetForegroundWindow((*it)->hWnd);
			SetFocus((*it)->hWnd);
			return;
		}
	}

    strcpy_s(sMonitorRouteDialogGUID, sizeof(sMonitorRouteDialogGUID), sGUID);

	MONITORROUTETHREADINFO mrti;
	mrti.hEvent = CreateEvent(NULL, FALSE, FALSE, sEventName);
	mrti.hThread = (HANDLE) _beginthreadex(NULL, 0, MonitorRouteThreadProc, &mrti, 0, NULL);

	WaitForSingleObject(mrti.hEvent, INFINITE);

	CloseHandle(mrti.hEvent);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool RouteMonitorGraphTextFormatCallback(CGraph *pGraph, GRAPHSERIES *pSeries, GRAPHTEXTATTRIBUTES *pTextAttributes)
{
	pTextAttributes->BGMode = TRANSPARENT;
	pTextAttributes->FGColor = pSeries->LineColor;
	pTextAttributes->FontSize = 10;

	char sSize[128];
	char *sPart = NULL;

	if(pSeries->Index == 0)
	{
		pTextAttributes->x = pGraph->Width();
		pTextAttributes->x = 5;
		pTextAttributes->y = 5;
		sPart = "Active ";
		FormatDouble(sSize, sizeof(sSize), pSeries->LastValue, 0);
	}
	else if(pSeries->Index == 1)
	{
		pTextAttributes->x = 5;
		pTextAttributes->y = 20;
		sPart = "Tx/Rate";
		FileSizeFriendly(pSeries->LastValue, 2, sSize, sizeof(sSize));
		strcat_s(sSize, sizeof(sSize), "/s");
	}

	sprintf_s(pTextAttributes->Text, sizeof(pTextAttributes->Text), "%s:%s", sPart, sSize);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK MonitorRouteDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //--------------------------------------------------------------------------

    if(uMsg == WM_INITDIALOG)
    {
        HMENU hMenu = GetSystemMenu(hWnd, FALSE);
        AppendMenu(hMenu, MF_SEPARATOR, 0, 0);
        AppendMenu(hMenu, MF_STRING, GENERIC_MENU_ABOUT, "About");

        SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)gsTitleCaption);
        SendMessage(hWnd, WM_SETICON, TRUE, (LPARAM) LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN)));

		Set_Text(GetDlgItem(hWnd, IDC_ROUTENAME), "");
		Set_Text(GetDlgItem(hWnd, IDC_SOURCEPORT), "");
		Set_Text(GetDlgItem(hWnd, IDC_MAXCONNECTIONS), "");
		Set_Text(GetDlgItem(hWnd, IDC_LOADBALANCEMETHOD), "");
		Set_Text(GetDlgItem(hWnd, IDC_CURRENTCONNECTIONS), "");
		Set_Text(GetDlgItem(hWnd, IDC_TOTALCONNECTIONS), "");
		Set_Text(GetDlgItem(hWnd, IDC_TOTALTRANSFERRED), "");
		Set_Text(GetDlgItem(hWnd, IDC_TRANSFERRATE), "");

		MONITORDIALOGPROPS *pMDP = (MONITORDIALOGPROPS *) calloc(sizeof(MONITORDIALOGPROPS), 1);
		memset(pMDP, 0, sizeof(MONITORDIALOGPROPS));
		pMDP->hWnd = hWnd;

		pMDP->pGraph = new CGraph(GetDlgItem(hWnd, IDC_GRAPH), RGB(0, 0, 0), RGB(0, 100, 0), 10, GRAPH_RIGHT, false);  
		pMDP->pGraph->SpacingTop(50);
		pMDP->pGraph->AddSeries(RGB(255, 0, 0), 10, RouteMonitorGraphTextFormatCallback);
		pMDP->pGraph->AddSeries(RGB(255, 255, 0), 10, RouteMonitorGraphTextFormatCallback);

		strcpy_s(pMDP->GUID, sizeof(pMDP->GUID), sMonitorRouteDialogGUID);

		char sEventName[64];
		sprintf_s(sEventName, sizeof(sEventName), "MRT_%s", pMDP->GUID);
		pMDP->hEvent = CreateEvent(NULL, FALSE, FALSE, sEventName);

		ActiveMonitorDialogs.push_back(pMDP);

		SetProp(hWnd, "Props", pMDP);

		_beginthreadex(NULL, 0, MonitorGraphThreadProc, hWnd, 0, NULL);		

		CenterOverOwner(hWnd);

		SetTimer(hWnd, 0, 700, NULL);

        return TRUE; // Return TRUE to set the keyboard focus, Otherwise return FALSE.
    }
	//--------------------------------------------------------------------------
    else if(uMsg == WM_TIMER)
    {
		MONITORDIALOGPROPS *pMDP = (MONITORDIALOGPROPS *) GetProp(hWnd, "Props");
		if(pMDP && !pMDP->bShutdown)
		{
			SetEvent(pMDP->hEvent);
		}
	}
	//--------------------------------------------------------------------------
	else if(uMsg == WM_PAINT)
    {
		HDC ThisHDC;
        PAINTSTRUCT ThisPS;

        ThisHDC = BeginPaint(hWnd, &ThisPS);

        //- Any painting should be done here.
		MONITORDIALOGPROPS *pMDP = (MONITORDIALOGPROPS *) GetProp(hWnd, "Props");
		if(pMDP && !pMDP->bShutdown)
		{
			if(pMDP->pGraph)
			{
				pMDP->pGraph->Redraw();
			}
		}

        EndPaint(hWnd, &ThisPS);
        return TRUE;
    }
    //--------------------------------------------------------------------------
    else if(uMsg == WM_SYSCOMMAND) //- Received a system menu message.
    {
        if(LOWORD(wParam) == GENERIC_MENU_ABOUT) //- About.
        {
            _AboutDialogInfo ADI;
            ADI.DisplayIcon  = LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN));
            ADI.TitleCaption = gsTitleCaption;
            ADI.FileVersion  = gsFileVersion;
            ADI.BuildDate    = __DATE__;
            ADI.BuildTime    = __TIME__;
            ADI.CopyRight    = gsAppCopyRight;
            ADI.OwnerHandle  = hWnd;
            NetLogo(&ADI);

            return TRUE;
        }

        return FALSE;
    }
    //--------------------------------------------------------------------------
    else if(uMsg == WM_CLOSE) //- Received close message.
    {
		KillTimer(hWnd, 0);

		MONITORDIALOGPROPS *pMDP = (MONITORDIALOGPROPS *) GetProp(hWnd, "Props");
		if(pMDP)
		{
			pMDP->bShutdown = true;
			SetEvent(pMDP->hEvent);
			ActiveMonitorDialogs.remove(pMDP);
		}

		return TRUE;
    }
    //--------------------------------------------------------------------------

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
