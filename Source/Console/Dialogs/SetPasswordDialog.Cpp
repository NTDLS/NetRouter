///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _SetPasswordDialog_CPP
#define _SetPasswordDialog_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <ShlObj.H>
#include <Stdio.H>
#include <ShlOBJ.H>
#include <Stdlib.H>

#include "../Source/Entry.H"

#include "../Dialogs/SetPasswordDialog.H"
#include "../Dialogs/NetLogoDialog.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::String;
using namespace NSWFL::Conversion;
using namespace NSWFL::ListView;
using namespace NSWFL::File;
using namespace NSWFL::Registry;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define INITHANDLERPROC (WM_USER + 1000) //Can be used to init all the custom procs.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct TAG_PWDWeightHandler{
	WNDPROC lpOldProc;
	HWND hPwdWeight;
}INITPWDWEIGHTHANDLERINFO, *LPINITPWDWEIGHTHANDLERINFO;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK PWDWeightHandlerProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static WNDPROC lpOldProc;
	static HWND hOwner;
	static INITPWDWEIGHTHANDLERINFO IPWHI;

    if(uMsg == WM_GETDLGCODE)
	{
		return (DLGC_WANTCHARS | CallWindowProc(IPWHI.lpOldProc, hWnd, uMsg, wParam, lParam));
	}
	else if(uMsg == INITHANDLERPROC)
	{
		memcpy(&IPWHI, (WNDPROC *)lParam, sizeof(INITPWDWEIGHTHANDLERINFO));
		memcpy(&hOwner, (WNDPROC *)wParam, sizeof(HWND));
	}
	else if(uMsg == WM_CHAR)
	{
		if(IPWHI.lpOldProc)
		{
			CNASCCL MyCode;
			char sPwd[1024 + 1];
			int iPwdLen = Get_Text(hWnd, sPwd, sizeof(sPwd));
			if(iPwdLen < sizeof(sPwd))
			{
				Set_Text(IPWHI.hPwdWeight, MyCode.KeyWeightString(sPwd, iPwdLen));
			}
		}
	}

	if(IPWHI.lpOldProc)
	{
		return CallWindowProc(IPWHI.lpOldProc, hWnd, uMsg, wParam, lParam);
	}
	else{
		return 0;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AddPWDWeightHandler(HWND hOwner, HWND hWnd, HWND hPwdWeight)
{
	INITPWDWEIGHTHANDLERINFO IPWHI;

	IPWHI.lpOldProc = (WNDPROC) SetWindowLongPtr(hWnd, GWLP_WNDPROC, (INT_PTR) &PWDWeightHandlerProc);
	IPWHI.hPwdWeight = hPwdWeight;

	SendMessage(hWnd, INITHANDLERPROC, (WPARAM)&hOwner, (LPARAM) &IPWHI);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK SetPasswordDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hPassword1 = NULL;
    static HWND hPassword2 = NULL;
    static HWND hPwdWeight = NULL;

    //--------------------------------------------------------------------------

    if(uMsg == WM_INITDIALOG)
    {
        HMENU hMenu = GetSystemMenu(hWnd, FALSE);
        AppendMenu(hMenu, MF_SEPARATOR, 0, 0);
        AppendMenu(hMenu, MF_STRING, GENERIC_MENU_ABOUT, "About");

        SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)gsTitleCaption);
        SendMessage(hWnd, WM_SETICON, TRUE, (LPARAM) LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN)));

        hPassword1 = GetDlgItem(hWnd, IDC_PASSWORD1);
        hPassword2 = GetDlgItem(hWnd, IDC_PASSWORD2);
        hPwdWeight = GetDlgItem(hWnd, IDC_PWDWEIGHT);

		SendMessage(hPassword1, EM_LIMITTEXT, 1024, 0);
		SendMessage(hPassword2, EM_LIMITTEXT, 1024, 0);

		AddPWDWeightHandler(hWnd, hPassword1, hPwdWeight);
		AddPWDWeightHandler(hWnd, hPassword2, hPwdWeight);

        Set_Text(hPassword1, "");
        Set_Text(hPassword2, "");

		CXMLReader xmlResult;
		if(gManage->QueryXMLReader(&xmlResult, "::Config:Get:AllowRemoteAdministration"))
		{
			CheckDlgButton(hWnd, IDC_REMOTEADMIN, xmlResult.ToBoolean("Value"));
		}

		CenterOverOwner(hWnd);

        return TRUE; // Return TRUE to set the keyboard focus, Otherwise return FALSE.
    }
	//--------------------------------------------------------------------------
    else if(uMsg == WM_COMMAND)
    {
		if(wParam == IDC_OK || wParam == IDOK)
		{
			char sPwd1[1024 + 1];
			char sPwd2[1024 + 1];

			int iSz1 = Get_Text(hPassword1, sPwd1, sizeof(sPwd1));
			int iSz2 = Get_Text(hPassword2, sPwd2, sizeof(sPwd2));

			if(iSz1 < sizeof(sPwd1) && iSz1 < sizeof(sPwd1))
			{
				if(strcmp(sPwd1, sPwd2) == 0)
				{
					if(iSz1 == 0 && iSz2 == 0)
					{
						if(MessageBox(hWnd, "Are you sure you want to use a blank password?", gsTitleCaption,
							MB_YESNO| MB_ICONQUESTION | MB_DEFBUTTON2) != IDYES)
						{
							return TRUE;
						}
					}

					CXMLWriter xmlLoginInfo("LoginInfo");
					
					xmlLoginInfo.Add("Password", sPwd1);
					xmlLoginInfo.AddBool("AllowRemoteAdministration", IsDlgButtonChecked(hWnd, IDC_REMOTEADMIN) == BST_CHECKED);
					
					CXMLReader xmlResult;
					if(gManage->QueryXMLReader(&xmlResult, "::Security:AlterLogin|%s", xmlLoginInfo.Text()))
					{
						if(!xmlResult.ToBoolean("Value"))
						{
							CStringBuilder errorText;
							xmlResult.ToString("Text", &errorText);
							MessageBox(hWnd, errorText.Buffer, gsTitleCaption, MB_ICONERROR);
						}
						return EndDialog(hWnd, 1);
					}
				}
				else{
					MessageBox(hWnd, "The passwords you entered do not match.", gsTitleCaption, MB_ICONINFORMATION);
					return TRUE;
				}
			}

			return TRUE;
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
