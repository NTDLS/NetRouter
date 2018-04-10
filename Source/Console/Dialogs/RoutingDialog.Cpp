///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _RoutingDialog_Cpp
#define _RoutingDialog_Cpp
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <ShlObj.H>
#include <Stdio.H>
#include <ShlOBJ.H>
#include <Stdlib.H>

#include "../Source/Entry.H"
#include "../../@Shared/Routes.h"
#include "../../@Shared/ConnectHosts.H"

#include "../Dialogs/ConnectDialog.H"
#include "../Dialogs/ManageRouteDialog.H"
#include "../Dialogs/NetLogoDialog.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::String;
using namespace NSWFL::Conversion;
using namespace NSWFL::ListView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK RoutingDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //--------------------------------------------------------------------------

    static HWND hHost = NULL;
    static HWND hDescr = NULL;
    static HWND hPort = NULL;

    //--------------------------------------------------------------------------

    if(uMsg == WM_INITDIALOG)
    {
		CenterOverOwner(hWnd);

        HMENU hMenu = GetSystemMenu(hWnd, FALSE);
        AppendMenu(hMenu, MF_SEPARATOR, 0, 0);
        AppendMenu(hMenu, MF_STRING, GENERIC_MENU_ABOUT, "About");

        SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)"Routing");
        SendMessage(hWnd, WM_SETICON, TRUE, (LPARAM) LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN)));

		hHost	= GetDlgItem(hWnd, IDC_HOST);
        hDescr	= GetDlgItem(hWnd, IDC_IPDESCR);
        hPort	= GetDlgItem(hWnd, IDC_PORT);

		CheckDlgButton(hWnd, IDC_ENABLED, BST_CHECKED);

		if(giEditListIndex != -1)
		{
			char sHost[CONNECTHOSTS_MAX_HOST_LENGTH];
			char sDescr[CONNECTHOSTS_MAX_DESCR_LENGTH];
			char sPort[64];

			LVGetText(ghHostsAddressList, giEditListIndex, CONNECTHOSTS_LIST_POS_ADDRESS, sHost , sizeof(sHost));
			Set_Text(hHost, sHost);

			LVGetText(ghHostsAddressList, giEditListIndex, CONNECTHOSTS_LIST_POS_DESCR, sDescr , sizeof(sDescr));
			Set_Text(hDescr, sDescr);

			LVGetText(ghHostsAddressList, giEditListIndex, CONNECTHOSTS_LIST_POS_PORT, sPort , sizeof(sPort));
			Set_Text(hPort, sPort);

			CheckDlgButton(hWnd, IDC_ENABLED, LVChecked(ghHostsAddressList, giEditListIndex));
		}
	
		return TRUE; // Return TRUE to set the keyboard focus, Otherwise return FALSE.
    }
    //--------------------------------------------------------------------------
    else if(uMsg == WM_COMMAND)
    {
		if(wParam == IDC_OK)
        {
			CONNECTHOST Item;

			char sHost[CONNECTHOSTS_MAX_HOST_LENGTH];
			char sDescr[LISTENIPS_MAX_DESCR_LENGTH];
			int iPort = Get_Int(hPort);
			int iLength = 0;

			Get_Text(hDescr, sDescr, sizeof(sDescr));

			iLength = Get_Text(hHost, sHost, sizeof(sHost));
			if(iLength == 0 || iLength > sizeof(sHost))
			{
				MessageBox(hWnd, "You must specify a host name or valid v4 IP Address.", gsTitleCaption, MB_ICONINFORMATION);
				return FALSE;
			}

			if(iPort <= 0)
			{
				MessageBox(hWnd, "You must specify a valid port number (between 1 and in most cases 65,535).", gsTitleCaption, MB_ICONINFORMATION);
				return FALSE;
			}

			Item.Address = sHost;
			Item.Description = sDescr;
			Item.Enabled = (IsDlgButtonChecked(hWnd, IDC_ENABLED) > 0);
			Item.Port = iPort;

			if(giEditListIndex != -1)
			{
				LVDeleteItem(ghHostsAddressList, giEditListIndex);
				PopHostsList_AddItem(ghHostsAddressList, &Item, giEditListIndex);
				LVAutoSizeColumns(ghHostsAddressList, 500);
			}
			else {
				PopHostsList_AddItem(ghHostsAddressList, &Item);
				LVAutoSizeColumns(ghHostsAddressList, 500);
			}

            EndDialog(hWnd, TRUE);
            DestroyWindow(hWnd);
        }
        else if(wParam == IDC_CANCEL || wParam == IDCANCEL)
        {
            EndDialog(hWnd, FALSE);
            DestroyWindow(hWnd);
        }
        return FALSE;
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
        EndDialog(hWnd, 0);
        DestroyWindow(hWnd);
        return TRUE;
    }
    //--------------------------------------------------------------------------

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
