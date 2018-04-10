///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _MainDialog_CPP
#define _MainDialog_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <ShlObj.H>
#include <Stdio.H>
#include <ShlOBJ.H>
#include <Stdlib.H>
#include <tlhelp32.h>

#include "../Source/Entry.H"
#include "../../@Shared/Routes.h"

#include "MainDialog.H"
#include "ManageRouteDialog.H"
#include "NetLogoDialog.H"
#include "ConnectDialog.H"
#include "SetPasswordDialog.H"
#include "MonitorRouteDialog.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::String;
using namespace NSWFL::Conversion;
using namespace NSWFL::ListView;
using namespace NSWFL::File;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK MainDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

//---------------------(Variable Declarations)
HWND gMainDialog;
HIMAGELIST hEnableDisableImageList = NULL;
WNDPROC gpOldRouteListProc;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam)
{
	if(hWnd != (HWND)lParam)
	{
		if(GetWindowLongPtr(hWnd, GWL_STYLE) &WS_CAPTION)
		{
			SendMessage(hWnd, WM_CLOSE, 0, 0);
		}
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CloseThreadWindows(DWORD dwThreadId, HWND hExcept)
{
	EnumThreadWindows(dwThreadId, EnumChildProc, (LPARAM)hExcept);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CloseAllWindows(HWND hExcept) 
{ 
	HANDLE hThreadSnap = INVALID_HANDLE_VALUE; 
	THREADENTRY32 te32; 
 
	// Take a snapshot of all running threads  
	hThreadSnap = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0 ); 
	if(hThreadSnap == INVALID_HANDLE_VALUE)
	{
		return false; 
	}
 
	// Fill in the size of the structure before using it. 
	te32.dwSize = sizeof(THREADENTRY32); 
 
	// Retrieve information about the first thread,
	// and exit if unsuccessful
	if(!Thread32First( hThreadSnap, &te32)) 
	{
		CloseHandle(hThreadSnap); // clean the snapshot object
		return false;
	}

	DWORD dwProcessId = GetCurrentProcessId();
	do 
	{ 
		if(te32.th32OwnerProcessID == dwProcessId)
		{
			CloseThreadWindows(te32.th32ThreadID, hExcept);
		}
	}
	while(Thread32Next(hThreadSnap, &te32)); 

	CloseHandle(hThreadSnap);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopRoutes_AddItem(HWND hList, ROUTE *obj)
{
	char sText[1024];
	LV_ITEM ListItem;
    memset(&ListItem, 0, sizeof(ListItem));
	int iInsertionIndex = ListView_GetItemCount(hList);
	int iImage = obj->IsRunning;

    ListItem.pszText = (char *) obj->Name;
    ListItem.mask = LVIF_TEXT|LVIF_IMAGE;
	ListItem.iImage = iImage;
    ListItem.iSubItem = ROUTE_LIST_POS_NAME;
	ListItem.iItem = iInsertionIndex;
    int iItem = ListView_InsertItem(hList, &ListItem);

    ListItem.pszText = (char *) FormatInteger(sText, sizeof(sText), obj->LocalPort);
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = ROUTE_LIST_POS_LOCALPORT;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

    ListItem.pszText = (char *) FormatInteger(sText, sizeof(sText), obj->CurrentConnections);
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = ROUTE_LIST_POS_CURRENTCONNECTS;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

	/*
    ListItem.pszText = (char *) FormatInteger(sText, sizeof(sText), obj->TotalConnections);
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = ROUTE_LIST_POS_TOTALCONNECTS;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);
	*/

	ListItem.pszText = (char *) FileSizeFriendly(obj->BytesTransferred, sText, sizeof(sText));
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = ROUTE_LIST_POS_BYTESTRANSFERRED;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

    ListItem.pszText = (char *) obj->Description;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = ROUTE_LIST_POS_DESCRIPTION;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);

    ListItem.pszText = (char *) obj->GUID;
    ListItem.mask = LVIF_TEXT;
    ListItem.iSubItem = ROUTE_LIST_POS_GUID;
	ListItem.iItem = iInsertionIndex;
    ListView_SetItem(hList, &ListItem);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopRoutes(HWND hList)
{
	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)FALSE, (LPARAM)0);

	bool bAddedColumns = false;

	if(LVColumnCount(hList) == 0)
	{
		bAddedColumns = true;

		LV_COLUMN lvColumn;
		memset(&lvColumn, 0, sizeof(LV_COLUMN));

		lvColumn.cx        = 200;
		lvColumn.pszText   = "Name";
		lvColumn.iSubItem  = ROUTE_LIST_POS_NAME;
		lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
		lvColumn.fmt       = LVCFMT_LEFT;
		ListView_InsertColumn(hList, ROUTE_LIST_POS_NAME, &lvColumn);

		lvColumn.cx        = 100;
		lvColumn.pszText   = "Local Port";
		lvColumn.iSubItem  = ROUTE_LIST_POS_LOCALPORT;
		lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
		lvColumn.fmt       = LVCFMT_LEFT;
		ListView_InsertColumn(hList, ROUTE_LIST_POS_LOCALPORT, &lvColumn);

		lvColumn.cx        = 100;
		lvColumn.pszText   = "Connections";
		lvColumn.iSubItem  = ROUTE_LIST_POS_CURRENTCONNECTS;
		lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
		lvColumn.fmt       = LVCFMT_LEFT;
		ListView_InsertColumn(hList, ROUTE_LIST_POS_CURRENTCONNECTS, &lvColumn);

		lvColumn.cx        = 100;
		lvColumn.pszText   = "Bytes Tx.";
		lvColumn.iSubItem  = ROUTE_LIST_POS_BYTESTRANSFERRED;
		lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
		lvColumn.fmt       = LVCFMT_LEFT;
		ListView_InsertColumn(hList, ROUTE_LIST_POS_BYTESTRANSFERRED, &lvColumn);

		lvColumn.cx        = 300;
		lvColumn.pszText   = "Description";
		lvColumn.iSubItem  = ROUTE_LIST_POS_DESCRIPTION;
		lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
		lvColumn.fmt       = LVCFMT_LEFT;
		ListView_InsertColumn(hList, ROUTE_LIST_POS_DESCRIPTION, &lvColumn);

		lvColumn.cx        = 0;
		lvColumn.pszText   = "GUID";
		lvColumn.iSubItem  = ROUTE_LIST_POS_GUID;
		lvColumn.mask      = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
		lvColumn.fmt       = LVCFMT_LEFT;
		ListView_InsertColumn(hList, ROUTE_LIST_POS_GUID, &lvColumn);

		SendMessage(hList, (UINT)LVM_SETEXTENDEDLISTVIEWSTYLE, (WPARAM)0,
			(LPARAM)LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP);

		ListView_SetTextColor(hList, RGB(0,0,100));
	}

	if(!hEnableDisableImageList)
	{
		HICON hIcon = NULL;

		hEnableDisableImageList = ImageList_Create(
			GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_MASK|ILC_COLOR32, 1, 1);

		hIcon = LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_STOPPED));
		ImageList_AddIcon(hEnableDisableImageList, hIcon);

		hIcon = LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_RUNNING));
		ImageList_AddIcon(hEnableDisableImageList, hIcon);
	}

	ListView_SetImageList(hList, hEnableDisableImageList, LVSIL_SMALL);
	ListView_DeleteAllItems(hList);

	CXMLReader xmlResult;
	if(gManage->QueryXMLReader(&xmlResult, "::Config:Get:Routes"))
	{
		char sGUID[ROUTE_MAX_GUID];
		char sName[ROUTE_MAX_NAME];
		char sDescription[ROUTE_MAX_DESCRIPTION];
		char sRemoteHost[ROUTE_MAX_REMOTEHOST];

		xmlResult.ScanPosition(0);
		bool bParentEnabled = xmlResult.ToBoolean("Enable");
		xmlResult.ProgressiveScan(true);

		//Pop the list-view.
		for(CXMLReader xml; xmlResult.ToReader("Route", &xml);)
		{
			//Read XML.
			xml.ToString("Name", sName, sizeof(sName));
			xml.ToString("RemoteHost", sRemoteHost, sizeof(sRemoteHost));
			xml.ToString("Description", sDescription, sizeof(sDescription));
			xml.ToString("GUID", sGUID, sizeof(sGUID));

			//Build an object
			ROUTE obj;
			memset(&obj, 0, sizeof(obj));
			obj.Name        = sName;
			obj.LocalPort   = xml.ToInteger("LocalPort");
			obj.Description = sDescription;
			obj.GUID        = sGUID;

			CXMLReader xmlStatus;
			CXMLReader xmlStatistics;
			if(xml.ToReader("Status", &xmlStatus) && xml.ToReader("Statistics", &xmlStatistics))
			{
				obj.IsRunning = xmlStatus.ToBoolean("IsRunning");
				obj.TotalConnections = xmlStatus.ToInteger("TotalConnections");
				obj.BytesTransferred = xmlStatistics.ToU64("BytesTransferred");
				obj.CurrentConnections = xmlStatistics.ToInteger("CurrentConnections");

				xmlStatus.Destroy();
				xmlStatistics.Destroy();
			}

			xml.Destroy();

			//Add the object to the list.
			PopRoutes_AddItem(hList, &obj);
		}
	}

	if(bAddedColumns)
	{
		LVAutoSizeColumns(hList, 500);
	}

	SendMessage(hList, (UINT)WM_SETREDRAW , (WPARAM)TRUE, (LPARAM)0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UpdateStats(HWND hList)
{
	if(LVItemCount(hList) <= 0 || gManage->Socket->CurrentConnections() <= 0)
	{
		return;
	}

	CXMLReader xmlResult;
	if(gManage->QueryXMLReader(&xmlResult, "::Config:Get:Routes"))
	{
		char sGUID[ROUTE_MAX_GUID];
		char sGridGUID[ROUTE_MAX_GUID];

		xmlResult.ScanPosition(0);
		bool bParentEnabled = xmlResult.ToBoolean("Enable");
		xmlResult.ProgressiveScan(true);

		//Pop the list-view.
		for(CXMLReader xml; xmlResult.ToReader("Route", &xml);)
		{
			//Read XML.
			xml.ToString("GUID", sGUID, sizeof(sGUID));

			CXMLReader xmlStatus;
			CXMLReader xmlStatistics;
			if(xml.ToReader("Status", &xmlStatus) && xml.ToReader("Statistics", &xmlStatistics))
			{
				for(int iItem = 0; iItem < LVItemCount(hList); iItem++)
				{
					memset(sGridGUID, 0, sizeof(sGridGUID));
					LVGetText(hList, iItem, ROUTE_LIST_POS_GUID, sGridGUID , sizeof(sGridGUID));
					if(_strcmpi(sGUID, sGridGUID) == 0)
					{
						char sText[1024];
						LV_ITEM ListItem;
						memset(&ListItem, 0, sizeof(ListItem));
						int iImage = xmlStatus.ToBoolean("IsRunning");

						ListItem.mask = LVIF_IMAGE;
						ListItem.iImage = iImage;
						ListItem.iSubItem = ROUTE_LIST_POS_NAME;
						ListItem.iItem = iItem;
						ListView_SetItem(hList, &ListItem);

						ListItem.pszText = (char *) FormatInteger(sText, sizeof(sText), xmlStatus.ToInteger("CurrentConnections"));
						ListItem.mask = LVIF_TEXT;
						ListItem.iSubItem = ROUTE_LIST_POS_CURRENTCONNECTS;
						ListItem.iItem = iItem;
						ListView_SetItem(hList, &ListItem);

						/*
						ListItem.pszText = (char *) FormatInteger(sText, sizeof(sText), xmlStatistics.ToInteger("TotalConnections"));
						ListItem.mask = LVIF_TEXT;
						ListItem.iSubItem = ROUTE_LIST_POS_TOTALCONNECTS;
						ListItem.iItem = iItem;
						ListView_SetItem(hList, &ListItem);
						*/

						ListItem.pszText = (char *) FileSizeFriendly(xmlStatistics.ToI64("BytesTransferred"), sText, sizeof(sText));
						ListItem.mask = LVIF_TEXT;
						ListItem.iSubItem = ROUTE_LIST_POS_BYTESTRANSFERRED;
						ListItem.iItem = iItem;
						ListView_SetItem(hList, &ListItem);

						break;
					}
				}

				xmlStatus.Destroy();
				xmlStatistics.Destroy();
			}

			xml.Destroy();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define LIST_CMD_DUMMY   WM_USER + 10
#define LIST_CMD_ADDNEW  WM_USER + 11
#define LIST_CMD_START   WM_USER + 12
#define LIST_CMD_STOP    WM_USER + 13
#define LIST_CMD_DELETE  WM_USER + 14
#define LIST_CMD_MONITOR WM_USER + 15

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK NewRouteListProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = CallWindowProc(gpOldRouteListProc, hWnd, uMsg, wParam, lParam);

	if(uMsg == WM_COMMAND)
	{
		if(LOWORD(wParam) == LIST_CMD_START)
        {
			char sGUID[ROUTE_MAX_GUID];
			if(LVGetItemText(hWnd, LVGetFirstSelectedItemIndex(hWnd), ROUTE_LIST_POS_GUID, sGUID, sizeof(sGUID)))
			{
				CXMLReader xmlResult;
				if(gManage->QueryXMLReader(&xmlResult, "::Start:Route|%s", sGUID))
				{
					if(!xmlResult.ToBoolean("Value"))
					{
						CStringBuilder errorText;
						xmlResult.ToString("Text", &errorText);
						MessageBox(hWnd, errorText.Buffer, gsTitleCaption, MB_ICONERROR);
					}
				}
			}
			PopRoutes(hWnd);
		}
		else if(wParam == LIST_CMD_STOP)
        {
			char sGUID[ROUTE_MAX_GUID];
			if(LVGetItemText(hWnd, LVGetFirstSelectedItemIndex(hWnd), ROUTE_LIST_POS_GUID, sGUID, sizeof(sGUID)))
			{
				CXMLReader xmlResult;
				if(gManage->QueryXMLReader(&xmlResult, "::Stop:Route|%s", sGUID))
				{
					if(!xmlResult.ToBoolean("Value"))
					{
						CStringBuilder errorText;
						xmlResult.ToString("Text", &errorText);
						MessageBox(hWnd, errorText.Buffer, gsTitleCaption, MB_ICONERROR);
					}
				}
			}
			PopRoutes(hWnd);
		}
		if(LOWORD(wParam) == LIST_CMD_DELETE)
        {
			if(MessageBox(hWnd, "Are you sure you want to delete this route?",
				gsTitleCaption, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDYES)
			{
				char sGUID[ROUTE_MAX_GUID];
				if(LVGetItemText(hWnd, LVGetFirstSelectedItemIndex(hWnd), ROUTE_LIST_POS_GUID, sGUID, sizeof(sGUID)))
				{
					CXMLReader xmlResult;
					if(gManage->QueryXMLReader(&xmlResult, "::Config:Delete:Route|%s", sGUID))
					{
						if(!xmlResult.ToBoolean("Value"))
						{
							CStringBuilder errorText;
							xmlResult.ToString("Text", &errorText);
							MessageBox(hWnd, errorText.Buffer, gsTitleCaption, MB_ICONERROR);
						}
					}
				}
				PopRoutes(hWnd);
			}
		}
		else if(wParam == LIST_CMD_MONITOR)
        {
			char sGUID[ROUTE_MAX_GUID];
			if(LVGetItemText(hWnd, LVGetFirstSelectedItemIndex(hWnd), ROUTE_LIST_POS_GUID, sGUID, sizeof(sGUID)))
			{
				MonitorRoute(sGUID);
			}
		}
		else if(wParam == LIST_CMD_ADDNEW)
        {
			SendMessage(gMainDialog, WM_COMMAND, ID_SERVER_ADDNEWROUTE, 0);
		}
	}
	else if(uMsg == WM_RBUTTONDOWN)
    {
		int iSelectedIndex = LVGetFirstSelectedItemIndex(hWnd);

		HMENU hParent = CreateMenu();
		HMENU hChild = CreateMenu();
		HMENU hSelect = CreateMenu();

		AppendMenu(hChild, MF_STRING, LIST_CMD_ADDNEW, "Add new");
		AppendMenu(hChild, MF_SEPARATOR, LIST_CMD_DUMMY, "");
		AppendMenu(hChild, MF_STRING, LIST_CMD_START, "Start");
		AppendMenu(hChild, MF_STRING, LIST_CMD_STOP, "Stop");
		AppendMenu(hChild, MF_SEPARATOR, LIST_CMD_DUMMY, "");
		AppendMenu(hChild, MF_STRING, LIST_CMD_DELETE, "Delete");
		AppendMenu(hChild, MF_SEPARATOR, LIST_CMD_DUMMY, "");
		AppendMenu(hChild, MF_STRING, LIST_CMD_MONITOR, "Monitor Route");

		if(iSelectedIndex >= 0)
		{
			LV_ITEM lvItem;
			memset(&lvItem, 0, sizeof(lvItem));

			lvItem.iItem = iSelectedIndex;
			lvItem.iSubItem = 0;
			lvItem.mask = LVIF_IMAGE;
			if(ListView_GetItem(hWnd, &lvItem) > 0)
			{
				if(lvItem.iImage == 1)
				{
					ModifyMenu(hChild, LIST_CMD_START, MF_BYCOMMAND | MF_GRAYED, LIST_CMD_START, "Start");
				}
				else if(lvItem.iImage == 0)
				{
					ModifyMenu(hChild, LIST_CMD_STOP, MF_BYCOMMAND | MF_GRAYED, LIST_CMD_STOP, "Stop");
					ModifyMenu(hChild, LIST_CMD_MONITOR, MF_BYCOMMAND | MF_GRAYED, LIST_CMD_MONITOR, "Monitor Route");
				}
			}
		}
		else {
			ModifyMenu(hChild, LIST_CMD_START, MF_BYCOMMAND | MF_GRAYED, LIST_CMD_START, "Start");
			ModifyMenu(hChild, LIST_CMD_STOP, MF_BYCOMMAND | MF_GRAYED, LIST_CMD_STOP, "Stop");
			ModifyMenu(hChild, LIST_CMD_DELETE, MF_BYCOMMAND | MF_GRAYED, LIST_CMD_DELETE, "Delete");
			ModifyMenu(hChild, LIST_CMD_MONITOR, MF_BYCOMMAND | MF_GRAYED, LIST_CMD_MONITOR, "Monitor Route");
		}

		InsertMenu(hParent, 0, MF_POPUP|MF_BYPOSITION, (UINT_PTR)hChild, "Router Main Menu");

		POINT xy;
		GetCursorPos(&xy);
		TrackPopupMenuEx(hChild, TPM_LEFTALIGN|TPM_RIGHTBUTTON, (xy.x) + 1, xy.y, hWnd, NULL);
		DestroyMenu(hChild);
		DestroyMenu(hParent);
    }
	else if(uMsg == WM_KEYUP || uMsg == WM_RBUTTONDOWN || uMsg == WM_LBUTTONDOWN)
	{
		/*
		if(wParam == PCE_KEY_DELETE)
		{
			SendMessage(hMainDialog, WM_COMMAND, IDI_TB_DELETE, 0);
		}
		else{
			RecalcSelectedFiles();
		}
		*/
    }

	//--------------------------------------------------------------------------

	return lResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK MainDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //--------------------------------------------------------------------------

	static HWND hListView = NULL;
    static HWND hStatusBar = NULL;
	static bool bFirstPaint = true;
	static bool bTryingToConnect = false;

    //--------------------------------------------------------------------------

    if(uMsg == WM_INITDIALOG)
    {
        gMainDialog = hWnd;

        HMENU hMenu = GetSystemMenu(hWnd, FALSE);
        AppendMenu(hMenu, MF_SEPARATOR, 0, 0);
        AppendMenu(hMenu, MF_STRING, GENERIC_MENU_ABOUT, "About");

        SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)gsTitleCaption);
        SendMessage(hWnd, WM_SETICON, TRUE, (LPARAM) LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN)));

        hListView  = GetDlgItem(hWnd, IDC_DETAIL);
		hStatusBar = CreateStatusWindow(WS_CHILD|WS_VISIBLE, "", hWnd, 1);

        int Widths[] = {200, 400, 500, -1};
        SendMessage(hStatusBar, (UINT)SB_SETPARTS, (WPARAM)4, (LPARAM)&Widths);

		RECT rcWindow;
		memset(&rcWindow, 0, sizeof(rcWindow));
        GetClientRect(hWnd, &rcWindow);
		SendMessage(hWnd, WM_SIZE, 0, MAKELONG(rcWindow.right, rcWindow.bottom));

		CenterOverOwner(hWnd);

		SetTimer(hWnd, 0, 5000, NULL);

		gpOldRouteListProc = (WNDPROC) SetWindowLongPtr(hListView, GWLP_WNDPROC, (LRESULT) &NewRouteListProc);

        return TRUE; // Return TRUE to set the keyboard focus, Otherwise return FALSE.
    }
    //--------------------------------------------------------------------------
    else if(uMsg == WM_TIMER)
    {
		UpdateStats(hListView);
	}
    //--------------------------------------------------------------------------
    else if(uMsg == WM_COMMAND)
    {
		if(wParam == ID_HELP_ABOUT)
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
		}
		else if(wParam == ID_FILE_EXIT)
		{
			return EndDialog(hWnd, 0);
		}
		else if(wParam == ID_FILE_CONNECTTO)
		{
			gManage->Disconnect();
		}
		else if(wParam == ID_SERVER_SERVICECONSOLE)
		{
			gManage->SendCommand("::Server:OpenConsole");
		}
		else if(wParam == ID_SERVER_ADDNEWROUTE)
		{
			strcpy_s(sManageRouteDialogGUID, sizeof(sManageRouteDialogGUID), "");
			if(DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_ROUTE), hWnd, ManageRouteDialog) == 1)
			{
				PopRoutes(hListView);
				LVAutoSizeColumns(hListView, 500);
			}
		}
		else if(wParam == ID_SERVER_RESTARTSERVER)
		{
			if(MessageBox(hWnd, "Are you sure you want to restart the router?", gsTitleCaption, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDYES)
			{
				CXMLReader xmlResult;
				if(gManage->QueryXMLReader(&xmlResult, "::Server:Restart"))
				{
					if(!xmlResult.ToBoolean("Value"))
					{
						CStringBuilder errorText;
						xmlResult.ToString("Text", &errorText);
						MessageBox(hWnd, errorText.Buffer, gsTitleCaption, MB_ICONERROR);
					}
					else {
						MessageBox(hWnd, "Restart complete.", gsTitleCaption, MB_ICONINFORMATION);
					}
					PopRoutes(hListView);
				}
			}
		}
		else if(wParam == ID_SERVER_SETPASSWORD)
		{
			DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_SETPASSWORD), hWnd, SetPasswordDialog);
		}

		return FALSE;
    }
	//--------------------------------------------------------------------------
	else if(uMsg == UM_MGMT_DISCONNECTED)
	{
		if(!bTryingToConnect)
		{
			CloseAllWindows(hWnd);

			bTryingToConnect = true;

			LVClear(hListView);

			if(DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_CONNECT), hWnd, ConnectDialog) == 1)
			{
				PopRoutes(hListView);
			}
			else {
				EndDialog(hWnd, 0);
			}
			bTryingToConnect = false;
		}
	}
    //--------------------------------------------------------------------------
	else if(uMsg == WM_SIZE)
    {
		int nWidth = LOWORD(lParam);
        int nHeight = HIWORD(lParam);

		MoveWindow(hListView, 10, 5, nWidth - 20, nHeight - 30, TRUE);
		MoveWindow(hStatusBar, nWidth-20, nHeight-20, nWidth, nHeight, TRUE);

        return FALSE;
    }
	//---------------------------------------------------------------------------------------------------------------------
	else if(uMsg == WM_GETMINMAXINFO)
	{
		MINMAXINFO *pMMI = (MINMAXINFO *)lParam;
		pMMI->ptMinTrackSize.x = 550;
		pMMI->ptMinTrackSize.y = 350;
		return 0;
	}
	//--------------------------------------------------------------------------
	else if(uMsg == WM_NOTIFY)
    {
        LPNMHDR lpnmhdr = (LPNMHDR)lParam;
		if(lpnmhdr->code == NM_DBLCLK && lpnmhdr->hwndFrom == hListView)
		{
			int iSelectedIndex = LVGetFirstSelectedItemIndex(hListView);
			if(iSelectedIndex >= 0)
			{
				if(LVGetItemText(hListView, iSelectedIndex, ROUTE_LIST_POS_GUID, sManageRouteDialogGUID, sizeof(sManageRouteDialogGUID)))
				{
					if(DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_ROUTE), hWnd, ManageRouteDialog) == 1)
					{
						PopRoutes(hListView);
						LVAutoSizeColumns(hListView, 500);
					}
				}
			}
		}
	}
    //--------------------------------------------------------------------------
	else if(uMsg == WM_PAINT)
    {
		if(bFirstPaint)
		{
			bFirstPaint = false;
			SendMessage(hWnd, UM_MGMT_DISCONNECTED, 0, 0);
		}

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
		CloseAllWindows(hWnd);
		return EndDialog(hWnd, 0);
    }
    //--------------------------------------------------------------------------

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
