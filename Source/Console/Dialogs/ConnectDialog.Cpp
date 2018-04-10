///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _ConnectDialog_CPP
#define _ConnectDialog_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <ShlObj.H>
#include <Stdio.H>
#include <ShlOBJ.H>
#include <Stdlib.H>

#include "../Source/Entry.H"

#include "../Dialogs/ConnectDialog.H"
#include "../Dialogs/NetLogoDialog.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::String;
using namespace NSWFL::Conversion;
using namespace NSWFL::ListView;
using namespace NSWFL::File;
using namespace NSWFL::Registry;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------(Variable Declarations)
HWND gConnectDialog = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define COLOR_RUNNING		RGB(0,125,0)
#define COLOR_STOPPED		RGB(255,100,100)
#define COLOR_NOTINSTALLED	RGB(100,100,255)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD CheckServiceStatus(HWND hSvcStat, bool bForceRefresh)
{
	DWORD dwForeColor = 0;

	static DWORD dwLastValue = 0;
	char *sText = NULL;

	if(gService.IsInstalled() == _SVC_YES_)
	{
		if(gService.IsActive() == _SVC_YES_)
		{
			sText = "Local service is running.";
			dwForeColor = COLOR_RUNNING;
		}
		else {
			sText = "Local service is not running.";
			dwForeColor = COLOR_STOPPED;
		}
	}
	else {
		sText = "Local service is not installed.";
		dwForeColor = COLOR_NOTINSTALLED;
	}

	if(dwLastValue != dwForeColor || bForceRefresh)
	{
		Set_Text(hSvcStat, sText);
		dwLastValue = dwForeColor;
		InvalidateRect(hSvcStat, NULL, TRUE);
	}

	return dwForeColor;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD CheckServiceStatus(HWND hSvcStat)
{
	return CheckServiceStatus(hSvcStat, false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK ConnectDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static bool bFirstPaint = true;

	static HWND hHost = NULL;
	static HWND hPass = NULL;
	static HWND hSvcStat = NULL;
    static HCURSOR HandCursor = NULL;
	static DWORD dwForeColor = 0;

    //--------------------------------------------------------------------------

    if(uMsg == WM_INITDIALOG)
    {
		bFirstPaint = true;

        gConnectDialog = hWnd;

        HMENU hMenu = GetSystemMenu(hWnd, FALSE);
        AppendMenu(hMenu, MF_SEPARATOR, 0, 0);
        AppendMenu(hMenu, MF_STRING, GENERIC_MENU_ABOUT, "About");

        SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)gsTitleCaption);
        SendMessage(hWnd, WM_SETICON, TRUE, (LPARAM) LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN)));

		HandCursor = LoadCursor(ghAppInstance, MAKEINTRESOURCE(IDC_HANDCURSOR));

		hHost = GetDlgItem(hWnd, IDC_HOST);
		hPass = GetDlgItem(hWnd, IDC_PASSWORD);
		hSvcStat = GetDlgItem(hWnd, IDC_SERVICESTATUS);

		dwForeColor = CheckServiceStatus(hSvcStat, true);

		SetTimer(hWnd, 0, 1000, NULL);

		CenterOverOwner(hWnd);

        return TRUE; // Return TRUE to set the keyboard focus, Otherwise return FALSE.
    }
    //--------------------------------------------------------------------------
	else if (uMsg == WM_CTLCOLORSTATIC)
	{
		if((HANDLE)lParam == hSvcStat)
		{
			return (BOOL)Set_Color(dwForeColor, GetSysColor(COLOR_3DFACE), wParam);
		}

        return FALSE;
	}
    //--------------------------------------------------------------------------
	if(IsMouseOverHwnd(hSvcStat) && dwForeColor != COLOR_NOTINSTALLED)
	{
		SetCursor(HandCursor);

		if(uMsg == WM_LBUTTONUP)
        {
			if(gService.IsInstalled() == _SVC_YES_)
			{
				if(gService.IsActive() == _SVC_YES_)
				{
					if(MessageBox(hWnd, "The local service is currently running, would you like to stop it?", gsTitleCaption, MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2) == IDYES)
					{
						gService.Stop();
					}
				}
				else {
					if(MessageBox(hWnd, "The local service is not running, would you like to start it?", gsTitleCaption, MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2) == IDYES)
					{
						gService.Start();
					}
				}
				dwForeColor = CheckServiceStatus(hSvcStat);
			}
		}
	}
	//--------------------------------------------------------------------------
    else if(uMsg == WM_TIMER)
	{
		dwForeColor = CheckServiceStatus(hSvcStat);
	}
	//--------------------------------------------------------------------------
    else if(uMsg == WM_COMMAND)
    {
		if(wParam == IDC_OK)
		{
			bool bResult = false;

			char sHost[MGMT_MAX_HOST_SZ];
			int iHostLen = Get_Text(hHost, sHost, sizeof(sHost));
			char sPass[MGMT_MAX_PASS_SZ];
			int iPassLen = Get_Text(hPass, sPass, sizeof(sPass));

			if(iHostLen <= 0 || iHostLen >= sizeof(sHost))
			{
				MessageBox(hWnd, "You must specify a hostname.", gsTitleCaption, MB_ICONINFORMATION);
				return 0;
			}

			if(gManage->Connect(sHost))
			{
				BASICHUNK Chunk;
				memset(&Chunk, 0, sizeof(Chunk));
				if(gManage->QueryChunk(&Chunk))
				{
					if(_strcmpi((char *)Chunk.pBuffer, "Ok") == 0)
					{
						CStringBuilder xml;

						xml.AppendF("::Login|<Login><Password>%s</Password></Login>", sPass);

						if(gManage->QueryBoolean(&bResult, xml.Buffer))
						{
							if(bResult)
							{
								Set_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "LastConnectHost", sHost);

								strcpy_s(gManage->CurrentConnectioninfo.Password, sizeof(gManage->CurrentConnectioninfo.Password), sPass);
							}
							else {
								Set_Text(hPass, "");
								MessageBox(hWnd, "Login to the server failed.", gsTitleCaption, MB_ICONWARNING);
								SetFocus(hPass);
							}
						}
						else {
							MessageBox(hWnd, "Failed to query the server. Is remote management disabled?", gsTitleCaption, MB_ICONWARNING);
						}
					}
					else{
						MessageBox(hWnd, (char *)Chunk.pBuffer, gsTitleCaption, MB_ICONWARNING);
					}

					gManage->Socket->Free(&Chunk);
				}
				else {
					MessageBox(hWnd, "Failed to query the server. Is remote management disabled?", gsTitleCaption, MB_ICONWARNING);
				}

				if(!bResult)
				{
					if(gManage->Client->IsConnected())
					{
						gManage->Client->Disconnect();
					}
				}
			}
			else {
				MessageBox(hWnd, "Failed to connect to the specified server.", gsTitleCaption, MB_ICONWARNING);
			}

			if(bResult)
			{
				return EndDialog(hWnd, 1);
			}
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
		if(bFirstPaint)
		{
			bFirstPaint = false;

			DWORD dwBufferSz = 0;
			char sBuffer[255];

			dwBufferSz = sizeof(sBuffer);
			memset(sBuffer, 0, dwBufferSz);
			if(Get_StringRegistryValue(HKEY_LOCAL_MACHINE, gsRegistryKey, "LastConnectHost", sBuffer, dwBufferSz))
			{
				if(dwBufferSz > 0)
				{
					Set_Text(hHost, sBuffer);
					SetFocus(hPass);
				}
			}
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
		return EndDialog(hWnd, 0);
    }
    //--------------------------------------------------------------------------

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
