///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _HTTPHeaderDialog_Cpp
#define _HTTPHeaderDialog_Cpp
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <ShlObj.H>
#include <Stdio.H>
#include <ShlOBJ.H>
#include <Stdlib.H>

#include "../Source/Entry.H"
#include "../../@Shared/Routes.h"
#include "../../@Shared/ListenIPs.H"

#include "../Dialogs/HTTPHeaderDialog.H"
#include "../Dialogs/ManageRouteDialog.H"
#include "../Dialogs/NetLogoDialog.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::String;
using namespace NSWFL::Conversion;
using namespace NSWFL::ListView;
using namespace NSWFL::File;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopHTTPHeaderNames(HWND hList)
{
	for(int i = 0; HTTP_HEADERS[i]; i++)
	{
		SendMessage(hList, (UINT) CB_ADDSTRING, (WPARAM) NULL, (LPARAM) HTTP_HEADERS[i]);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopHTTPHeaderMethods(HWND hList)
{
	for(int i = 0; HTTP_HANDLER_METHODS[i]; i++)
	{
		SendMessage(hList, (UINT) CB_ADDSTRING, (WPARAM) NULL, (LPARAM) HTTP_HANDLER_METHODS[i]);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopHTTPHeaderVerbs(HWND hList)
{
	for(int i = 0; HTTP_VERBS[i]; i++)
	{
		SendMessage(hList, (UINT) CB_ADDSTRING, (WPARAM) NULL, (LPARAM) HTTP_VERBS[i]);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PopHTTPHeaderText(HWND hList)
{
	for(int i = 0; HTTP_HEADER_VARS[i]; i++)
	{
		SendMessage(hList, (UINT) CB_ADDSTRING, (WPARAM) NULL, (LPARAM) HTTP_HEADER_VARS[i]);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK HTTPHeaderDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HWND hHeaderName = NULL;
	static HWND hHeaderVerbs = NULL;
	static HWND hHeaderMethod = NULL;
	static HWND hHeaderText = NULL;
	static HWND hHeaderDescr = NULL;

    //--------------------------------------------------------------------------

    if(uMsg == WM_INITDIALOG)
    {
        HMENU hMenu = GetSystemMenu(hWnd, FALSE);
        AppendMenu(hMenu, MF_SEPARATOR, 0, 0);
        AppendMenu(hMenu, MF_STRING, GENERIC_MENU_ABOUT, "About");

        SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)"HTTP Header");
        SendMessage(hWnd, WM_SETICON, TRUE, (LPARAM) LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN)));

		hHeaderName		= GetDlgItem(hWnd, IDC_HEADER_NAME);
		hHeaderVerbs	= GetDlgItem(hWnd, IDC_HEADER_VERBS);
		hHeaderMethod	= GetDlgItem(hWnd, IDC_HEADER_METHOD);
		hHeaderText		= GetDlgItem(hWnd, IDC_HEADER_TEXT);
		hHeaderDescr	= GetDlgItem(hWnd, IDC_HEADER_DESCR);

		PopHTTPHeaderNames(hHeaderName);
		PopHTTPHeaderMethods(hHeaderMethod);
		PopHTTPHeaderVerbs(hHeaderVerbs);
		PopHTTPHeaderText(hHeaderText);

		CheckDlgButton(hWnd, IDC_ENABLED, BST_CHECKED);

		if(giEditListIndex != -1)
		{
			char sName[HTTPHEADERS_MAX_NAME_LENGTH];
			char sText[HTTPHEADERS_MAX_REPLACE_LENGTH];
			char sVerbs[HTTPHEADERS_MAX_VERBS_LENGTH];
			char sMethod[HTTPHEADERS_MAX_METHOD_LENGTH];
			char sDescr[HTTPHEADERS_MAX_DESCR_LENGTH];

			LVGetText(ghHTTPHeaderList, giEditListIndex, HTTPHEADERS_LIST_POS_NAME, sName, sizeof(sName));
			Set_Text(hHeaderName, sName);

			LVGetText(ghHTTPHeaderList, giEditListIndex, HTTPHEADERS_LIST_POS_VERBS, sVerbs, sizeof(sVerbs));
			Set_Text(hHeaderVerbs, sVerbs);

			LVGetText(ghHTTPHeaderList, giEditListIndex, HTTPHEADERS_LIST_POS_METHOD, sMethod, sizeof(sMethod));
			int iIndex = (int)SendMessage(hHeaderMethod, (UINT)CB_FINDSTRING, (WPARAM)-1, (LPARAM) sMethod);
			SendMessage(hHeaderMethod, (UINT)CB_SETCURSEL, (WPARAM)iIndex, (LPARAM)NULL);

			LVGetText(ghHTTPHeaderList, giEditListIndex, HTTPHEADERS_LIST_POS_TEXT, sText, sizeof(sText));
			Set_Text(hHeaderText, sText);

			LVGetText(ghHTTPHeaderList, giEditListIndex, HTTPHEADERS_LIST_POS_DESCR, sDescr, sizeof(sDescr));
			Set_Text(hHeaderDescr, sDescr);

			CheckDlgButton(hWnd, IDC_ENABLED, LVChecked(ghHTTPHeaderList, giEditListIndex));
		}
		else {
			int iIndex = (int)SendMessage(hHeaderMethod, (UINT)CB_FINDSTRING, (WPARAM)-1, (LPARAM) "Replace");
			SendMessage(hHeaderMethod, (UINT)CB_SETCURSEL, (WPARAM)iIndex, (LPARAM)NULL);
		}

		//IDC_ENABLED

		CenterOverOwner(hWnd);

        return TRUE; // Return TRUE to set the keyboard focus, Otherwise return FALSE.
    }
	//--------------------------------------------------------------------------
    else if(uMsg == WM_COMMAND)
    {
		if(wParam == IDC_OK || wParam == IDOK)
		{
			int iLength = 0;
			char sName[HTTPHEADERS_MAX_NAME_LENGTH];
			char sText[HTTPHEADERS_MAX_REPLACE_LENGTH];
			char sVerbs[HTTPHEADERS_MAX_VERBS_LENGTH];
			char sMethod[HTTPHEADERS_MAX_METHOD_LENGTH];
			char sDescr[HTTPHEADERS_MAX_DESCR_LENGTH];

			HTTPHEADER Item;

			iLength = Get_Text(hHeaderName, sName, sizeof(sName));
			if(iLength <= 0 || iLength > sizeof(sName) )
			{
				MessageBox(hWnd, "You must specify a header name.", gsTitleCaption, MB_ICONINFORMATION);
				return FALSE;
			}

			Get_Text(hHeaderVerbs, sVerbs, sizeof(sVerbs));

			iLength = Get_Text(hHeaderMethod, sMethod, sizeof(sMethod));
			if(iLength <= 0 || iLength > sizeof(sName) )
			{
				MessageBox(hWnd, "You must specify a header manipulation method.", gsTitleCaption, MB_ICONINFORMATION);
				return FALSE;
			}

			Get_Text(hHeaderText, sText, sizeof(sText));
			Get_Text(hHeaderDescr, sDescr, sizeof(sDescr));

			Item.Name = sName;
			Item.Text = sText;
			Item.Verbs = sVerbs;
			Item.MethodName = sMethod;
			Item.Description = sDescr;
			Item.Enabled = (IsDlgButtonChecked(hWnd, IDC_ENABLED) > 0);

			if(giEditListIndex != -1)
			{
				LVDeleteItem(ghHTTPHeaderList, giEditListIndex);
				PopHTTPHeadersList_AddItem(ghHTTPHeaderList, &Item, giEditListIndex);
				LVAutoSizeColumns(ghHTTPHeaderList, 500);
			}
			else {
				PopHTTPHeadersList_AddItem(ghHTTPHeaderList, &Item);
				LVAutoSizeColumns(ghHTTPHeaderList, 500);
			}

            EndDialog(hWnd, TRUE);
            DestroyWindow(hWnd);
		}
		else if(wParam == IDC_CANCEL || wParam == IDCANCEL)
		{
			return EndDialog(hWnd, 0);
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
		return EndDialog(hWnd, 0);
    }
    //--------------------------------------------------------------------------

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
