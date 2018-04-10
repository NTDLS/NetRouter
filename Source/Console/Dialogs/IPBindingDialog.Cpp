///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _IPBindingDialog_Cpp
#define _IPBindingDialog_Cpp
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <ShlObj.H>
#include <Stdio.H>
#include <ShlOBJ.H>
#include <Stdlib.H>

#include "../Source/Entry.H"
#include "../../@Shared/Routes.h"
#include "../../@Shared/ListenIPs.H"

#include "../Dialogs/ConnectDialog.H"
#include "../Dialogs/ManageRouteDialog.H"
#include "../Dialogs/NetLogoDialog.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::String;
using namespace NSWFL::Conversion;
using namespace NSWFL::ListView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopIPAddresses(HWND hList)
{
	char sHostName[1024];
	memset(sHostName, 0, sizeof(sHostName));

	SendMessage(hList, (UINT) CB_ADDSTRING, (WPARAM) NULL, (LPARAM) "127.0.0.1");

	if(gethostname(sHostName, sizeof(sHostName)) == 0)
	{
		struct hostent FAR *pHostent = gethostbyname(sHostName);

		struct in_addr Address;
		for(int i = 0; (*pHostent).h_addr_list[i]; i++)
		{
			memcpy(&Address.s_addr, (*pHostent).h_addr_list[i], (*pHostent).h_length);
			SendMessage(hList, (UINT) CB_ADDSTRING, (WPARAM) NULL, (LPARAM) inet_ntoa(Address));
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK IPBindingDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //--------------------------------------------------------------------------

    static HWND hIP = NULL;
    static HWND hDescr = NULL;

    //--------------------------------------------------------------------------

    if(uMsg == WM_INITDIALOG)
    {
		CenterOverOwner(hWnd);

        HMENU hMenu = GetSystemMenu(hWnd, FALSE);
        AppendMenu(hMenu, MF_SEPARATOR, 0, 0);
        AppendMenu(hMenu, MF_STRING, GENERIC_MENU_ABOUT, "About");

        SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)"IP Binding");
        SendMessage(hWnd, WM_SETICON, TRUE, (LPARAM) LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN)));

		hIP    = GetDlgItem(hWnd, IDC_IPADDRESS);
        hDescr = GetDlgItem(hWnd, IDC_IPDESCR);

		PopIPAddresses(hIP);
		CheckDlgButton(hWnd, IDC_ENABLED, BST_CHECKED);

		if(giEditListIndex != -1)
		{
			char sIP[LISTENIPS_MAX_IP_LENGTH];
			char sDescr[LISTENIPS_MAX_DESCR_LENGTH];

			LVGetText(ghIPAddressList, giEditListIndex, LISTENIPS_LIST_POS_IP, sIP , sizeof(sIP));
			Set_Text(hIP, sIP);
			LVGetText(ghIPAddressList, giEditListIndex, LISTENIPS_LIST_POS_DESCR, sDescr , sizeof(sDescr));
			Set_Text(hDescr, sDescr);

			CheckDlgButton(hWnd, IDC_ENABLED, LVChecked(ghIPAddressList, giEditListIndex));
		}

		return TRUE; // Return TRUE to set the keyboard focus, Otherwise return FALSE.
    }
    //--------------------------------------------------------------------------
    else if(uMsg == WM_COMMAND)
    {
		if(wParam == IDC_OK)
        {
			LISTENIP Item;

			char sIP[LISTENIPS_MAX_IP_LENGTH];
			char sDescr[LISTENIPS_MAX_DESCR_LENGTH];

			Get_Text(hIP, sIP, sizeof(sIP));
			Get_Text(hDescr, sDescr, sizeof(sDescr));

			if(strlen(sIP) == 0 || !IsValidIPAddressV4(sIP))
			{
				MessageBox(hWnd, "You must specify a valid v4 IP Address.", gsTitleCaption, MB_ICONINFORMATION);
				return FALSE;
			}

			Item.IP = sIP;
			Item.Description = sDescr;
			Item.Enabled = (IsDlgButtonChecked(hWnd, IDC_ENABLED) > 0);

			if(giEditListIndex != -1)
			{
				LVDeleteItem(ghIPAddressList, giEditListIndex);
				PopListeningIPsList_AddItem(ghIPAddressList, &Item, giEditListIndex);
				LVAutoSizeColumns(ghIPAddressList, 500);
			}
			else {
				PopListeningIPsList_AddItem(ghIPAddressList, &Item);
				LVAutoSizeColumns(ghIPAddressList, 500);
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
