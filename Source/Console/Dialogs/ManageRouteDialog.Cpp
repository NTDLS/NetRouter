///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _ManageRouteDialog_Cpp
#define _ManageRouteDialog_Cpp
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <WindowsX.H>
#include <ShlObj.H>
#include <Stdio.H>
#include <ShlOBJ.H>
#include <Stdlib.H>

#include "../../../../@Libraries/CTabControl/CTabControl.h"

#include "../Source/Entry.H"

#include "../../@Shared/Routes.h"
#include "../../@Shared/ListenIPs.H"
#include "../../@Shared/ConnectHosts.H"
#include "../../@Shared/HTTPHeaders.H"

#include "../Dialogs/ManageRouteDialog.H"
#include "../Dialogs/IPBindingDialog.H"
#include "../Dialogs/NetLogoDialog.H"
#include "../Dialogs/RoutingDialog.H"
#include "../Dialogs/HTTPHeaderDialog.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::String;
using namespace NSWFL::Conversion;
using namespace NSWFL::ListView;
using namespace NSWFL::File;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------(Variable Declarations)
char sManageRouteDialogGUID[ROUTE_MAX_GUID];
HWND ghIPAddressList = NULL;
HWND ghHostsAddressList = NULL;
HWND ghHTTPHeaderList = NULL;
int giEditListIndex = -1;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopListeningIPsList_AddItem(HWND hList, LISTENIP *obj)
{
	int iInsertionIndex = ListView_GetItemCount(hList);
	PopListeningIPsList_AddItem(hList, obj, iInsertionIndex);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopListeningIPsList_AddItem(HWND hList, LISTENIP *obj, int iInsertionIndex)
{
	LV_ITEM ListItem;
    memset(&ListItem, 0, sizeof(ListItem));

	ListItem.pszText = (char *) obj->IP;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = LISTENIPS_LIST_POS_IP;
	ListItem.iItem = iInsertionIndex;
    int iItem = ListView_InsertItem(hList, &ListItem);

    ListItem.pszText = (char *) obj->Description;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = LISTENIPS_LIST_POS_DESCR;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

	ListView_SetCheckState(hList, iItem, obj->Enabled);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopListeningIPsList(CXMLReader *pXML, HWND hDialog, HWND hList)
{
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)FALSE, (LPARAM)0);

	if(LVColumnCount(hList) == 0)
	{
		LV_COLUMN lvColumn;
		memset(&lvColumn, 0, sizeof(LV_COLUMN));

		lvColumn.cx        = 100;
		lvColumn.pszText   = "IP Address";
		lvColumn.iSubItem  = LISTENIPS_LIST_POS_IP;
		lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
		lvColumn.fmt       = LVCFMT_LEFT;
		ListView_InsertColumn(hList, LISTENIPS_LIST_POS_IP, &lvColumn);

		lvColumn.cx        = 300;
		lvColumn.pszText   = "Description";
		lvColumn.iSubItem  = LISTENIPS_LIST_POS_DESCR;
		lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
		lvColumn.fmt       = LVCFMT_LEFT;
		ListView_InsertColumn(hList, LISTENIPS_LIST_POS_DESCR, &lvColumn);

	    SendMessage(hList, (UINT)LVM_SETEXTENDEDLISTVIEWSTYLE, (WPARAM)0,
			(LPARAM)LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP|LVS_EX_CHECKBOXES);

		ListView_SetTextColor(hList, RGB(0,0,100));
	}

	if(pXML)
	{
		int iLength = 0;

		if(pXML->ToBoolean("ListenOnAll", false))
		{
			CheckDlgButton(hDialog, IDC_IP_LISTENONALL, BST_CHECKED);
			SendMessage(hDialog, WM_COMMAND, IDC_IP_LISTENONALL, 0);
		}

		pXML->ProgressiveScan(true);
		CXMLReader xmlItem;

		while(pXML->ToReader("IPAddress", &xmlItem))
		{
			char sIP[LISTENIPS_MAX_IP_LENGTH];
			char sDescr[LISTENIPS_MAX_DESCR_LENGTH];

			xmlItem.ToString("Address", sIP, sizeof(sIP), &iLength);
			xmlItem.ToString("Description", sDescr, sizeof(sDescr), &iLength);

			//Build an object
			LISTENIP obj;
			obj.IP = sIP;
			obj.Description = sDescr;
			obj.Enabled = xmlItem.ToBoolean("Enable", true);

			//Add the object to the list.
			PopListeningIPsList_AddItem(hList, &obj);

			xmlItem.Destroy();
		}
	}

	LVAutoSizeColumns(hList, 500);
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)TRUE, (LPARAM)0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LoadListeningIPsList(HWND hWnd, HWND hGrid, CXMLWriter *pXML)
{
	int iGridItems = LVItemCount(hGrid);

	char sIP[LISTENIPS_MAX_IP_LENGTH];
	char sDescr[LISTENIPS_MAX_DESCR_LENGTH];

	pXML->Initialize("IPAddresses");

	pXML->AddBool("ListenOnAll", IsDlgButtonChecked(hWnd, IDC_IP_LISTENONALL) == BST_CHECKED);

	for(int iItem = 0; iItem < iGridItems; iItem++)
	{
		CXMLWriter Item("IPAddress");

		LVGetText(hGrid, iItem, LISTENIPS_LIST_POS_IP, sIP, sizeof(sIP));
		Item.Add("Address", sIP);

		LVGetText(hGrid, iItem, LISTENIPS_LIST_POS_DESCR, sDescr, sizeof(sDescr));
		Item.Add("Description", sDescr);

		Item.AddBool("Enable", ParseBoolean(LVChecked(hGrid, iItem)));

		pXML->Add(&Item);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopHostsList_AddItem(HWND hList, CONNECTHOST *obj)
{
	int iInsertionIndex = ListView_GetItemCount(hList);
	PopHostsList_AddItem(hList, obj, iInsertionIndex);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopHostsList_AddItem(HWND hList, CONNECTHOST *obj, int iInsertionIndex)
{
	char sInt[64];
	LV_ITEM ListItem;
    memset(&ListItem, 0, sizeof(ListItem));

	ListItem.pszText = (char *) obj->Address;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = CONNECTHOSTS_LIST_POS_ADDRESS;
	ListItem.iItem = iInsertionIndex;
    int iItem = ListView_InsertItem(hList, &ListItem);

    ListItem.pszText = (char *) IntChar(sInt, sizeof(sInt), obj->Port);
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = CONNECTHOSTS_LIST_POS_PORT;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

	ListItem.pszText = (char *) obj->Description;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = CONNECTHOSTS_LIST_POS_DESCR;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

	ListView_SetCheckState(hList, iItem, obj->Enabled);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopHostsList(CXMLReader *pXML, HWND hDialog, HWND hList, HWND hLoadBalanceMethod, HWND hPersistCheckBox)
{
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)FALSE, (LPARAM)0);

	if(LVColumnCount(hList) == 0)
	{
		LV_COLUMN lvColumn;
		memset(&lvColumn, 0, sizeof(LV_COLUMN));

		lvColumn.cx        = 100;
		lvColumn.pszText   = "Address";
		lvColumn.iSubItem  = CONNECTHOSTS_LIST_POS_ADDRESS;
		lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
		lvColumn.fmt       = LVCFMT_LEFT;
		ListView_InsertColumn(hList, CONNECTHOSTS_LIST_POS_ADDRESS, &lvColumn);

		lvColumn.cx        = 100;
		lvColumn.pszText   = "Port";
		lvColumn.iSubItem  = CONNECTHOSTS_LIST_POS_PORT;
		lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
		lvColumn.fmt       = LVCFMT_LEFT;
		ListView_InsertColumn(hList, CONNECTHOSTS_LIST_POS_PORT, &lvColumn);

		lvColumn.cx        = 300;
		lvColumn.pszText   = "Description";
		lvColumn.iSubItem  = CONNECTHOSTS_LIST_POS_DESCR;
		lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
		lvColumn.fmt       = LVCFMT_LEFT;
		ListView_InsertColumn(hList, CONNECTHOSTS_LIST_POS_DESCR, &lvColumn);

	    SendMessage(hList, (UINT)LVM_SETEXTENDEDLISTVIEWSTYLE, (WPARAM)0,
			(LPARAM)LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP|LVS_EX_CHECKBOXES);

		ListView_SetTextColor(hList, RGB(0,0,100));
	}

	if(pXML)
	{
		int iLength = 0;

		char sLoadBalanceMethod[255];
		pXML->ToString("LoadBalanceMethod", sLoadBalanceMethod, sizeof(sLoadBalanceMethod));
		int iIndex = (int)SendMessage(hLoadBalanceMethod, (UINT)CB_FINDSTRING, (WPARAM)-1, (LPARAM) sLoadBalanceMethod);
		SendMessage(hLoadBalanceMethod, (UINT)CB_SETCURSEL, (WPARAM)iIndex, (LPARAM)NULL);

		Button_SetCheck(hPersistCheckBox, pXML->ToBoolean("PersistBySourceIP") ? BST_CHECKED : BST_UNCHECKED);

		pXML->ProgressiveScan(true);
		CXMLReader xmlItem;

		while(pXML->ToReader("Host", &xmlItem))
		{
			char sHost[CONNECTHOSTS_MAX_HOST_LENGTH];
			char sDescr[CONNECTHOSTS_MAX_DESCR_LENGTH];

			xmlItem.ToString("Address", sHost, sizeof(sHost), &iLength);
			xmlItem.ToString("Description", sDescr, sizeof(sDescr), &iLength);

			//Build an object
			CONNECTHOST obj;
			obj.Address = sHost;
			obj.Port = xmlItem.ToInteger("Port");
			obj.Description = sDescr;
			obj.Enabled = xmlItem.ToBoolean("Enable", true);

			//Add the object to the list.
			PopHostsList_AddItem(hList, &obj);

			xmlItem.Destroy();
		}
	}
	else {
		int iIndex = (int)SendMessage(hLoadBalanceMethod, (UINT)CB_FINDSTRING, (WPARAM)-1, (LPARAM) "Failover Only");
		SendMessage(hLoadBalanceMethod, (UINT)CB_SETCURSEL, (WPARAM)iIndex, (LPARAM)NULL);

		Button_SetCheck(hPersistCheckBox, BST_CHECKED);
	}

	LVAutoSizeColumns(hList, 500);
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)TRUE, (LPARAM)0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LoadHostsList(HWND hWnd, HWND hGrid, HWND hLoadBalanceMethod, HWND hPersistCheckBox, CXMLWriter *pXML)
{
	int iGridItems = LVItemCount(hGrid);

	char sHost[CONNECTHOSTS_MAX_HOST_LENGTH];
	char sDescr[CONNECTHOSTS_MAX_DESCR_LENGTH];
	char sPort[64];

	pXML->Initialize("Hosts");

	char sLoadBalanceMethod[255];
	Get_Text(hLoadBalanceMethod, sLoadBalanceMethod, sizeof(sLoadBalanceMethod));
	pXML->Add("LoadBalanceMethod", sLoadBalanceMethod);
	pXML->Add("PersistBySourceIP", Button_GetCheck(hPersistCheckBox) == BST_CHECKED);

	for(int iItem = 0; iItem < iGridItems; iItem++)
	{
		CXMLWriter Item("Host");

		LVGetText(hGrid, iItem, CONNECTHOSTS_LIST_POS_ADDRESS, sHost, sizeof(sHost));
		Item.Add("Address", sHost);

		LVGetText(hGrid, iItem, CONNECTHOSTS_LIST_POS_PORT, sPort, sizeof(sPort));
		Item.Add("Port", sPort);

		LVGetText(hGrid, iItem, CONNECTHOSTS_LIST_POS_DESCR, sDescr, sizeof(sDescr));
		Item.Add("Description", sDescr);

		Item.AddBool("Enable", ParseBoolean(LVChecked(hGrid, iItem)));

		pXML->Add(&Item);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopHTTPHeadersList_AddItem(HWND hList, HTTPHEADER *obj)
{
	int iInsertionIndex = ListView_GetItemCount(hList);
	PopHTTPHeadersList_AddItem(hList, obj, iInsertionIndex);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopHTTPHeadersList_AddItem(HWND hList, HTTPHEADER *obj, int iInsertionIndex)
{
	LV_ITEM ListItem;
    memset(&ListItem, 0, sizeof(ListItem));

	ListItem.pszText = (char *) obj->Name;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = HTTPHEADERS_LIST_POS_NAME;
	ListItem.iItem = iInsertionIndex;
    int iItem = ListView_InsertItem(hList, &ListItem);

	ListItem.pszText = (char *) obj->MethodName;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = HTTPHEADERS_LIST_POS_METHOD;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

	ListItem.pszText = (char *) obj->Text;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = HTTPHEADERS_LIST_POS_TEXT;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

	ListItem.pszText = (char *) obj->Verbs;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = HTTPHEADERS_LIST_POS_VERBS;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

	ListItem.pszText = (char *) obj->Description;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = HTTPHEADERS_LIST_POS_DESCR;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

	ListView_SetCheckState(hList, iItem, obj->Enabled);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopHTTPHeadersList(CXMLReader *pXML, HWND hDialog, HWND hList)
{
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)FALSE, (LPARAM)0);

	if(LVColumnCount(hList) == 0)
	{
		LV_COLUMN lvColumn;
		memset(&lvColumn, 0, sizeof(LV_COLUMN));

		lvColumn.cx        = 200;
		lvColumn.pszText   = "Name";
		lvColumn.iSubItem  = HTTPHEADERS_LIST_POS_NAME;
		lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
		lvColumn.fmt       = LVCFMT_LEFT;
		ListView_InsertColumn(hList, HTTPHEADERS_LIST_POS_NAME, &lvColumn);

		lvColumn.cx        = 150;
		lvColumn.pszText   = "Method";
		lvColumn.iSubItem  = HTTPHEADERS_LIST_POS_METHOD;
		lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
		lvColumn.fmt       = LVCFMT_LEFT;
		ListView_InsertColumn(hList, HTTPHEADERS_LIST_POS_METHOD, &lvColumn);

		lvColumn.cx        = 200;
		lvColumn.pszText   = "Text";
		lvColumn.iSubItem  = HTTPHEADERS_LIST_POS_TEXT;
		lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
		lvColumn.fmt       = LVCFMT_LEFT;
		ListView_InsertColumn(hList, HTTPHEADERS_LIST_POS_TEXT, &lvColumn);

		lvColumn.cx        = 200;
		lvColumn.pszText   = "Verbs";
		lvColumn.iSubItem  = HTTPHEADERS_LIST_POS_VERBS;
		lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
		lvColumn.fmt       = LVCFMT_LEFT;
		ListView_InsertColumn(hList, HTTPHEADERS_LIST_POS_VERBS, &lvColumn);

		lvColumn.cx        = 300;
		lvColumn.pszText   = "Description";
		lvColumn.iSubItem  = HTTPHEADERS_LIST_POS_DESCR;
		lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
		lvColumn.fmt       = LVCFMT_LEFT;
		ListView_InsertColumn(hList, HTTPHEADERS_LIST_POS_DESCR, &lvColumn);

	    SendMessage(hList, (UINT)LVM_SETEXTENDEDLISTVIEWSTYLE, (WPARAM)0,
			(LPARAM)LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP|LVS_EX_CHECKBOXES);

		ListView_SetTextColor(hList, RGB(0,0,100));
	}

	if(pXML)
	{
		int iLength = 0;

		pXML->ProgressiveScan(true);
		CXMLReader xmlItem;

		while(pXML->ToReader("Header", &xmlItem))
		{
			char sName[HTTPHEADERS_MAX_NAME_LENGTH];
			char sText[HTTPHEADERS_MAX_REPLACE_LENGTH];
			char sVerbs[HTTPHEADERS_MAX_VERBS_LENGTH];
			char sMethod[HTTPHEADERS_MAX_METHOD_LENGTH];
			char sDescr[HTTPHEADERS_MAX_DESCR_LENGTH];

			xmlItem.ToString("Verbs", sVerbs, sizeof(sVerbs), &iLength);
			xmlItem.ToString("Method", sMethod, sizeof(sMethod), &iLength);
			xmlItem.ToString("Name", sName, sizeof(sName), &iLength);
			xmlItem.ToString("Text", sText, sizeof(sText), &iLength);
			xmlItem.ToString("Description", sDescr, sizeof(sDescr), &iLength);

			//Build an object
			HTTPHEADER obj;
			obj.Name = sName;
			obj.Text = sText;
			obj.Verbs = sVerbs;
			obj.MethodName = sMethod;
			obj.Description = sDescr;
			obj.Enabled = xmlItem.ToBoolean("Enable", true);

			//Add the object to the list.
			PopHTTPHeadersList_AddItem(hList, &obj);

			xmlItem.Destroy();
		}
	}

	LVAutoSizeColumns(hList, 500);
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)TRUE, (LPARAM)0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool LoadHTTPHeadersList(HWND hWnd, HWND hGrid, CXMLWriter *pXML)
{
	int iGridItems = LVItemCount(hGrid);

	char sName[HTTPHEADERS_MAX_NAME_LENGTH];
	char sText[HTTPHEADERS_MAX_REPLACE_LENGTH];
	char sVerbs[HTTPHEADERS_MAX_VERBS_LENGTH];
	char sMethod[HTTPHEADERS_MAX_METHOD_LENGTH];
	char sDescr[HTTPHEADERS_MAX_DESCR_LENGTH];

	pXML->Initialize("HTTPHeaders");

	for(int iItem = 0; iItem < iGridItems; iItem++)
	{
		CXMLWriter Item("Header");

		LVGetText(hGrid, iItem, HTTPHEADERS_LIST_POS_NAME, sName, sizeof(sName));
		Item.Add("Name", sName);

		LVGetText(hGrid, iItem, HTTPHEADERS_LIST_POS_METHOD, sText, sizeof(sText));
		Item.Add("Method", sText);

		LVGetText(hGrid, iItem, HTTPHEADERS_LIST_POS_TEXT, sVerbs, sizeof(sVerbs));
		Item.Add("Text", sVerbs);

		LVGetText(hGrid, iItem, HTTPHEADERS_LIST_POS_VERBS, sMethod, sizeof(sMethod));
		Item.Add("Verbs", sMethod);

		LVGetText(hGrid, iItem, HTTPHEADERS_LIST_POS_DESCR, sDescr, sizeof(sDescr));
		Item.Add("Description", sDescr);

		Item.AddBool("Enable", ParseBoolean(LVChecked(hGrid, iItem)));

		pXML->Add(&Item);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK ManageRouteDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static CTabControl *TabControl;
	static HWND hTabControl = NULL;
	static HWND hTabBasic = NULL;
	static HWND hTabIPBindings = NULL;
	static HWND hTabRouting = NULL;
	static HWND hTabHTTPHeaders = NULL;
	static HWND hTabLoadBalance = NULL;
	static HWND hThreadingTab = NULL;
	static HWND hTabAdvanced = NULL;

	static HWND hName = NULL;
	static HWND hDescr = NULL;
	static HWND hLocalPort = NULL;
	static HWND hMaxConnections = NULL;
	static HWND hLoadBalanceMethod = NULL;
	static HWND hPersistCheckBox = NULL;
	static HWND hAutoStartCheckBox = NULL;
	static HWND hMaxBufferSize = NULL;
	static HWND hInitialBufferSize = NULL;
	static HWND hIdleTimeout = NULL;
	static HWND hDataPumpSpawnThreshold = NULL;
	static HWND hResolverSpawnThreshold = NULL;
	static HWND hPersistenceTime = NULL;
	static HWND hDataPumpThreads = NULL;
	static HWND hResolverThreads = NULL;

    //--------------------------------------------------------------------------

    if(uMsg == WM_INITDIALOG)
    {
        HMENU hMenu = GetSystemMenu(hWnd, FALSE);
        AppendMenu(hMenu, MF_SEPARATOR, 0, 0);
        AppendMenu(hMenu, MF_STRING, GENERIC_MENU_ABOUT, "About");

		SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)"Route Settings");
        SendMessage(hWnd, WM_SETICON, TRUE, (LPARAM) LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN)));

		static LPSTR tabNames[]= {
			"Basic",
			"Routing",
			"IP Bindings",
			"HTTP Headers",
			"Load Balancing",
			"Threading",
			"Advanced",
			NULL
		};

		static LPSTR dlgNames[] = {
			MAKEINTRESOURCE(IDD_ROUTE_BASIC),
			MAKEINTRESOURCE(IDD_ROUTE_ROUTINGS),
			MAKEINTRESOURCE(IDD_ROUTE_IPBINDINGS),
			MAKEINTRESOURCE(IDD_ROUTE_HEADERS),
			MAKEINTRESOURCE(IDD_ROUTE_LOADBALANCE),
			MAKEINTRESOURCE(IDD_ROUTE_THREADING),
			MAKEINTRESOURCE(IDD_ROUTE_ADVANCED),
			NULL
		};

		TabControl = new CTabControl(GetDlgItem(hWnd, IDC_TABCONTROL), tabNames, dlgNames, &ManageRouteDialog, TRUE);

		hTabBasic				= TabControl->Handle("Basic");
		hName					= GetDlgItem(hTabBasic, IDC_NAME);
		hDescr					= GetDlgItem(hTabBasic, IDC_DESCR);
		hAutoStartCheckBox		= GetDlgItem(hTabBasic, IDC_AUTOSTART);

		hTabIPBindings			= TabControl->Handle("IP Bindings");
		ghIPAddressList			= GetDlgItem(hTabIPBindings, IDC_IPADDRESSLIST);

		hTabRouting				= TabControl->Handle("Routing");
		ghHostsAddressList		= GetDlgItem(hTabRouting, IDC_HOST_LIST);
		hLocalPort				= GetDlgItem(hTabRouting, IDC_LOCALPORT);
		hMaxConnections			= GetDlgItem(hTabRouting, IDC_MAXCONNECTIONS);

		hTabHTTPHeaders			= TabControl->Handle("HTTP Headers");
		ghHTTPHeaderList		= GetDlgItem(hTabHTTPHeaders, IDC_HEADER_LIST);

		hTabLoadBalance			= TabControl->Handle("Load Balancing");
		hLoadBalanceMethod		= GetDlgItem(hTabLoadBalance, IDC_HOST_BALANCE);
		hPersistCheckBox		= GetDlgItem(hTabLoadBalance, IDC_LBMPERSISTBYIP);
		hPersistenceTime		= GetDlgItem(hTabLoadBalance, IDC_PERSISTENCETIME);

		hThreadingTab			= TabControl->Handle("Threading");
		hDataPumpSpawnThreshold	= GetDlgItem(hThreadingTab, IDC_DataPumpSpawnThreshold);
		hResolverSpawnThreshold	= GetDlgItem(hThreadingTab, IDC_ResolverSpawnThreshold);
		hDataPumpThreads		= GetDlgItem(hThreadingTab, IDC_DATAPUMPTHREADS);
		hResolverThreads		= GetDlgItem(hThreadingTab, IDC_RESOLVERTHREADS);

		hTabAdvanced			= TabControl->Handle("Advanced");
		hInitialBufferSize		= GetDlgItem(hTabAdvanced, IDC_INITIALBUFFERSIZE);
		hMaxBufferSize			= GetDlgItem(hTabAdvanced, IDC_MAXBUFFERSIZE);
		hIdleTimeout			= GetDlgItem(hTabAdvanced, IDC_IDLETIMEOUT);

		for(int i = 0; gsLBM[i]; i++)
		{
			SendMessage(hLoadBalanceMethod, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)gsLBM[i]);
		}

		if(strlen(sManageRouteDialogGUID) > 0)
		{
			CXMLReader xmlResult;
			if(gManage->QueryXMLReader(&xmlResult, "::Config:Get:Route|%s", sManageRouteDialogGUID))
			{
				char sText[ROUTE_MAX_MAX];

				xmlResult.ToString("Name", sText, sizeof(sText));
				Set_Text(hName, sText);
				xmlResult.ToString("Description", sText, sizeof(sText));
				Set_Text(hDescr, sText);

				Set_Long(hLocalPort, xmlResult.ToInteger("LocalPort"));
				Set_Long(hMaxConnections, xmlResult.ToInteger("MaxConnections"));

				Set_Long(hMaxBufferSize, xmlResult.ToInteger("MaxBufferSize"));
				Set_Long(hInitialBufferSize, xmlResult.ToInteger("InitialBufferSize"));
				Set_Long(hIdleTimeout, xmlResult.ToInteger("IdleDisconnectTimeout"));
				Set_Long(hDataPumpSpawnThreshold, xmlResult.ToInteger("DataPumpSpawnThreshold"));
				Set_Long(hResolverSpawnThreshold, xmlResult.ToInteger("ResolverSpawnThreshold"));
				Set_Long(hPersistenceTime, xmlResult.ToInteger("PersistenceTime"));
				Set_Long(hDataPumpThreads, xmlResult.ToInteger("DataPumpThreads"));
				Set_Long(hResolverThreads, xmlResult.ToInteger("ResolverThreads"));
				Button_SetCheck(hAutoStartCheckBox, xmlResult.ToBoolean("AutoStart") ? BST_CHECKED : BST_UNCHECKED);

				CXMLReader xmlIPAddresses;
				if(xmlResult.ToReader("IPAddresses", &xmlIPAddresses))
				{
					PopListeningIPsList(&xmlIPAddresses, hTabIPBindings, ghIPAddressList);
					xmlIPAddresses.Destroy();
				}
				else {
					PopListeningIPsList(NULL, hTabIPBindings, ghIPAddressList);
				}

				CXMLReader xmlHTTPHeaders;
				if(xmlResult.ToReader("HTTPHeaders", &xmlHTTPHeaders))
				{
					PopHTTPHeadersList(&xmlHTTPHeaders, hTabHTTPHeaders, ghHTTPHeaderList);
					xmlHTTPHeaders.Destroy();
				}
				else {
					PopHTTPHeadersList(NULL, hTabHTTPHeaders, ghHTTPHeaderList);
				}

				CXMLReader xmlHosts;
				if(xmlResult.ToReader("Hosts", &xmlHosts))
				{
					PopHostsList(&xmlHosts, hTabRouting, ghHostsAddressList, hLoadBalanceMethod, hPersistCheckBox);

					xmlHosts.ProgressiveScan(true);

					char sLoadBalanceMethod[255];
					xmlHosts.ToString("LoadBalanceMethod", sLoadBalanceMethod, sizeof(sLoadBalanceMethod));

					xmlHosts.Destroy();
				}
				else	{
					PopHostsList(NULL, hTabRouting, ghHostsAddressList, hLoadBalanceMethod, hPersistCheckBox);
				}
			}
		}
		else {
			Set_Text(hName, "");
			Set_Text(hDescr, "");
			Set_Text(hLocalPort, "");
			Set_Long(hMaxConnections, 1024);

			Set_Long(hInitialBufferSize, 16 * 1024);
			Set_Long(hMaxBufferSize, 256 * 1024);
			Set_Long(hIdleTimeout, 3600);
			Set_Long(hDataPumpSpawnThreshold, 75);
			Set_Long(hResolverSpawnThreshold, 200);
			Set_Long(hPersistenceTime, 3600);
			Set_Long(hDataPumpThreads, 16);
			Set_Long(hResolverThreads, 1);
			Button_SetCheck(hAutoStartCheckBox, BST_CHECKED);

			CheckDlgButton(hTabIPBindings, IDC_IP_LISTENONALL, BST_CHECKED);
			SendMessage(hWnd, WM_COMMAND, IDC_IP_LISTENONALL, 0);

			PopListeningIPsList(NULL, hTabIPBindings, ghIPAddressList);
			PopHostsList(NULL, hTabRouting, ghHostsAddressList, hLoadBalanceMethod, hPersistCheckBox);
			PopHTTPHeadersList(NULL, hTabHTTPHeaders, ghHTTPHeaderList);
		}

		SendMessage(hWnd, WM_COMMAND, IDC_LBMPERSISTBYIP, 0);

		CenterOverOwner(hWnd);

        return TRUE; // Return TRUE to set the keyboard focus, Otherwise return FALSE.
    }
	else if(uMsg == WM_CHAR)
	{
		if(wParam == VK_RETURN)
		{
			return 0;
		}
		else if(wParam == VK_TAB)
		{
			return 0;
		}
		else if(wParam == VK_ESCAPE)
		{
			return 0;
		}
	}
	else if(uMsg == WM_KEYDOWN)
	{
		if(wParam == VK_TAB)
		{
			//PostMessage(hOwner, WM_NEXTDLGCTL, 0, 0L);
			return 0;
		}
	}
	//--------------------------------------------------------------------------
    else if(uMsg == WM_COMMAND)
    {
		if(wParam == IDC_OK || wParam == IDOK)
		{
			bool bIsUpdate = strlen(sManageRouteDialogGUID) > 0;
			int iLength = 0;
			char sName[ROUTE_MAX_NAME];
			char sDescription[ROUTE_MAX_DESCRIPTION];
			int iLocalPort;
			int iMaxConnections;
			int iInitialBufferSize;
			int iMaxBufferSize;
			int iIdleDisconnectTimeout;
			int iDataPumpSpawnThreshold;
			int iResolverSpawnThreshold;
			int iPersistenceTime;
			int iDataPumpThreads;
			int iResolverThreads;
			bool bAutoStart;
			bool bError = false;

			//----------Basic--------------------------------------------------

			iLength = Get_Text(hName, sName, sizeof(sName));
			if(iLength <= 0 || iLength >= sizeof(sName))
			{
				TabControl->SetControlFocus(hName);
				MessageBox(hWnd, "\"Name\" is required.", gsTitleCaption, MB_ICONINFORMATION);
				return TRUE;
			}

			iLength = Get_Text(hDescr, sDescription, sizeof(sDescription));
			if(iLength < 0 || iLength >= sizeof(sDescription))
			{
				TabControl->SetControlFocus(hDescr);
				MessageBox(hWnd, "\"Description\" length is invalid.", gsTitleCaption, MB_ICONINFORMATION);
				return TRUE;
			}

			bAutoStart = Button_GetCheck(hAutoStartCheckBox) == BST_CHECKED;

			//----------Routing--------------------------------------------------

			iLocalPort = Get_Long(hLocalPort);
			if(iLocalPort <= 0 || iLocalPort > 65535)
			{
				TabControl->SetControlFocus(hLocalPort);
				MessageBox(hWnd, "\"Local Port\" is required and must be between 1 and 65,535.", gsTitleCaption, MB_ICONINFORMATION);
				return TRUE;
			}

			iMaxConnections = Get_Long(hMaxConnections);
			if(iMaxConnections <= 0 || iMaxConnections > 5000)
			{
				TabControl->SetControlFocus(hMaxConnections);
				MessageBox(hWnd, "\"Max Connections\" must be between 1 and 5,000.", gsTitleCaption, MB_ICONINFORMATION);
				return TRUE;
			}

			//----------Load Balancing--------------------------------------------------

			iPersistenceTime = Get_Long(hPersistenceTime);
			if(iPersistenceTime <= 0 || iPersistenceTime > 86400)
			{
				TabControl->SetControlFocus(hPersistenceTime);
				MessageBox(hWnd, "\"Persistence Timeout\" is required and must be between 1 and 86,400 seconds.", gsTitleCaption, MB_ICONINFORMATION);
				return TRUE;
			}

			//----------Threading--------------------------------------------------

			iDataPumpThreads = Get_Long(hDataPumpThreads);
			if(iDataPumpThreads <= 0 || iDataPumpThreads > 1000)
			{
				TabControl->SetControlFocus(hDataPumpThreads);
				MessageBox(hWnd, "\"DataPump Threads\" is required and must be between 1 and 1,000.", gsTitleCaption, MB_ICONINFORMATION);
				return TRUE;
			}

			iDataPumpSpawnThreshold = Get_Long(hDataPumpSpawnThreshold);
			if(iDataPumpSpawnThreshold <= 0 || iDataPumpSpawnThreshold > 100)
			{
				TabControl->SetControlFocus(hDataPumpSpawnThreshold);
				MessageBox(hWnd, "\"Data-Pump Thread Spawn Threshold\" is required and must be between 1 and 100 percent.", gsTitleCaption, MB_ICONINFORMATION);
				return TRUE;
			}

			iResolverThreads = Get_Long(hResolverThreads);
			if(iResolverThreads <= 0 || iResolverThreads > iDataPumpThreads)
			{
				TabControl->SetControlFocus(hResolverThreads);
				MessageBox(hWnd, "\"Connection Threads\" is required and must be between 1 and \"DataPump Threads\".", gsTitleCaption, MB_ICONINFORMATION);
				return TRUE;
			}

			iResolverSpawnThreshold = Get_Long(hResolverSpawnThreshold);
			if(iResolverSpawnThreshold <= 0 || iResolverSpawnThreshold > 10000)
			{
				TabControl->SetControlFocus(hResolverSpawnThreshold);
				MessageBox(hWnd, "\"Connection Thread Spawn Threshold\" is required and must be between 100 and 10,000 percent.", gsTitleCaption, MB_ICONINFORMATION);
				return TRUE;
			}

			iInitialBufferSize = Get_Long(hInitialBufferSize);
			if(iInitialBufferSize <= 0 || iInitialBufferSize > 1073741824)
			{
				TabControl->SetControlFocus(hInitialBufferSize);
				MessageBox(hWnd, "\"Initial Buffer Size\" is required and must be between 1 and 1,073,741,824 bytes.", gsTitleCaption, MB_ICONINFORMATION);
				return TRUE;
			}

			iMaxBufferSize = Get_Long(hMaxBufferSize);
			if(iMaxBufferSize <= 0 || iMaxBufferSize > 1073741824)
			{
				TabControl->SetControlFocus(hMaxBufferSize);
				MessageBox(hWnd, "\"Max Buffer Size\" is required and must be between 1 and 1,073,741,824 bytes.", gsTitleCaption, MB_ICONINFORMATION);
				return TRUE;
			}

			//----------Advanced--------------------------------------------------

			iIdleDisconnectTimeout = Get_Long(hIdleTimeout);
			if(iIdleDisconnectTimeout < 0 || iIdleDisconnectTimeout > 86400)
			{
				TabControl->SetControlFocus(hIdleTimeout);
				MessageBox(hWnd, "\"Idle Timeout\" is required and must be between 0 (disabled) and 86,400 seconds.", gsTitleCaption, MB_ICONINFORMATION);
				return TRUE;
			}

			//----------Finished--------------------------------------------------

			if(LVCheckedItemCount(ghHostsAddressList) == 0)
			{
				MessageBox(hWnd, "No routings have been defined or they are all disabled."
					" This route will start but will not be functional until at least one end-point is defined.", gsTitleCaption, MB_ICONINFORMATION);
			}

			CXMLWriter Item("Route");
			if(bIsUpdate)
			{
				Item.Add("GUID", sManageRouteDialogGUID);
			}
			Item.Add("Name", sName);
			Item.Add("Description", sDescription);
			Item.Add("LocalPort", iLocalPort);
			Item.Add("MaxConnections", iMaxConnections);
			Item.Add("InitialBufferSize", iInitialBufferSize);
			Item.Add("MaxBufferSize", iMaxBufferSize);
			Item.Add("IdleDisconnectTimeout", iIdleDisconnectTimeout);
			Item.Add("DataPumpSpawnThreshold", iDataPumpSpawnThreshold);
			Item.Add("ResolverSpawnThreshold", iResolverSpawnThreshold);
			Item.Add("PersistenceTime", iPersistenceTime);
			Item.Add("DataPumpThreads", iDataPumpThreads);
			Item.Add("ResolverThreads", iResolverThreads);
			Item.AddBool("AutoStart", bAutoStart);

			CXMLWriter xmlListenIPs("IPAddresses");
			if(LoadListeningIPsList(hTabIPBindings, ghIPAddressList, &xmlListenIPs))
			{
				Item.Add(&xmlListenIPs);
			}

			CXMLWriter xmlHosts("Hosts");
			if(LoadHostsList(hTabRouting, ghHostsAddressList, hLoadBalanceMethod, hPersistCheckBox, &xmlHosts))
			{
				Item.Add(&xmlHosts);
			}

			CXMLWriter xmlHTTPHeaders("HTTPHeaders");
			if(LoadHTTPHeadersList(hTabHTTPHeaders, ghHTTPHeaderList,  &xmlHTTPHeaders))
			{
				Item.Add(&xmlHTTPHeaders);
			}

			if(bIsUpdate)
			{
				CXMLReader xmlResult;
				if(gManage->QueryXMLReader(&xmlResult, "::Config:Update:Route|%s", Item.Text()))
				{
					if(!xmlResult.ToBoolean("Value"))
					{
						CStringBuilder errorText;
						xmlResult.ToString("Text", &errorText);
						MessageBox(hWnd, errorText.Buffer, "Failed to update the configuration", MB_ICONERROR);
					}

					delete TabControl;
					return EndDialog(hWnd, 1);
				}
			}
			else {
				CXMLReader xmlResult;
				if(gManage->QueryXMLReader(&xmlResult, "::Config:Add:Route|%s", Item.Text()))
				{
					if(!xmlResult.ToBoolean("Value"))
					{
						CStringBuilder errorText;
						xmlResult.ToString("Text", &errorText);
						MessageBox(hWnd, errorText.Buffer, gsTitleCaption, MB_ICONERROR);
					}

					delete TabControl;
					return EndDialog(hWnd, 1);
				}
			}

			return TRUE;
		}
		else if(wParam == IDC_CANCEL || wParam == IDCANCEL)
		{
			delete TabControl;
			return EndDialog(hWnd, 0);
		}
		else if(wParam == IDC_HEADER_UP)
		{
			LVMoveItemUp(ghHTTPHeaderList);
		}
		else if(wParam == IDC_HEADER_DOWN)
		{
			LVMoveItemDown(ghHTTPHeaderList);
		}
		else if(wParam == IDC_HEADER_ADD)
		{
			giEditListIndex = -1;
			DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_ROUTE_HEADER), hWnd, HTTPHeaderDialog);
		}
		else if(wParam == IDC_HEADER_DELETE)
		{
			LVDeleteItemFriendly(ghHTTPHeaderList);
		}
		else if(wParam == IDC_IP_UP)
		{
			LVMoveItemUp(ghIPAddressList);
		}
		else if(wParam == IDC_IP_DOWN)
		{
			LVMoveItemDown(ghIPAddressList);
		}
		else if(wParam == IDC_IP_ADD)
		{
			giEditListIndex = -1;
			DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_ROUTE_IPBINDING), hWnd, IPBindingDialog);
		}
		else if(wParam == IDC_IP_DELETE)
		{
			LVDeleteItemFriendly(ghIPAddressList);
		}
		else if(wParam == IDC_LBMPERSISTBYIP)
		{
			bool bChecked = Button_GetCheck(hPersistCheckBox) == BST_CHECKED;
			EnableWindow(hPersistenceTime, bChecked);
		}
		else if(wParam == IDC_IP_LISTENONALL)
		{
			bool bChecked = IsDlgButtonChecked(hTabIPBindings, IDC_IP_LISTENONALL) > 0;
			EnableWindow(GetDlgItem(hTabIPBindings, IDC_IP_ADD), !bChecked);
			EnableWindow(GetDlgItem(hTabIPBindings, IDC_IP_UP), !bChecked);
			EnableWindow(GetDlgItem(hTabIPBindings, IDC_IP_DOWN), !bChecked);
			EnableWindow(GetDlgItem(hTabIPBindings, IDC_IP_DELETE), !bChecked);
			EnableWindow(GetDlgItem(hTabIPBindings, IDC_IPADDRESSLIST), !bChecked);
		}
		else if(wParam == IDC_HOST_UP)
		{
			LVMoveItemUp(ghHostsAddressList);
		}
		else if(wParam == IDC_HOST_DOWN)
		{
			LVMoveItemDown(ghHostsAddressList);
		}
		else if(wParam == IDC_HOST_ADD)
		{
			giEditListIndex = -1;
			DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_ROUTE_ROUTING), hWnd, RoutingDialog);
		}
		else if(wParam == IDC_HOST_DELETE)
		{
			LVDeleteItemFriendly(ghHostsAddressList);
		}

		return FALSE;
    }
	//--------------------------------------------------------------------------
	else if(uMsg == WM_NOTIFY)
	{
		LPNMHDR lpnmhdr = (LPNMHDR)lParam;
		if(lpnmhdr->code == NM_DBLCLK && lpnmhdr->hwndFrom == ghIPAddressList)
		{
			int iSelectedIndex = LVGetFirstSelectedItemIndex(ghIPAddressList);
			if(iSelectedIndex >= 0)
			{
				giEditListIndex = iSelectedIndex;
				DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_ROUTE_IPBINDING), hWnd, IPBindingDialog);
			}
		}
		else if(lpnmhdr->code == NM_DBLCLK && lpnmhdr->hwndFrom == ghHostsAddressList)
		{
			int iSelectedIndex = LVGetFirstSelectedItemIndex(ghHostsAddressList);
			if(iSelectedIndex >= 0)
			{
				giEditListIndex = iSelectedIndex;
				DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_ROUTE_ROUTING), hWnd, RoutingDialog);
			}
		}
		else if(lpnmhdr->code == NM_DBLCLK && lpnmhdr->hwndFrom == ghHTTPHeaderList)
		{
			int iSelectedIndex = LVGetFirstSelectedItemIndex(ghHTTPHeaderList);
			if(iSelectedIndex >= 0)
			{
				giEditListIndex = iSelectedIndex;
				DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_ROUTE_HEADER), hWnd, HTTPHeaderDialog);
			}
		}
	}
	//--------------------------------------------------------------------------
	else if(uMsg == WM_PAINT)
    {
		HDC ThisHDC;
        PAINTSTRUCT ThisPS;

        ThisHDC = BeginPaint(hWnd, &ThisPS);

        //- Any painting should be done here.

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
		delete TabControl;
		return EndDialog(hWnd, 0);
    }
    //--------------------------------------------------------------------------

	if(TabControl)
	{
		TabControl->ProcessParentMessages(hWnd, uMsg, wParam, lParam);
	}

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
