///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _NetLogoDialog_H
#define _NetLogoDialog_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _AboutDialogInfo{
    char *BuildDate;
    char *BuildTime;
    char *CopyRight;
    char *FileVersion;
    char *TitleCaption;
    HICON DisplayIcon;
    HWND OwnerHandle;
} ABOUTDLGINFO, *LPABOUTDLGINFO;

extern LPABOUTDLGINFO glpADI;

extern HWND NetLogoDialog_hWnd;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK NetLogoDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
bool IsNetLogoActive(void);
bool NetLogo(LPABOUTDLGINFO lpADI);
HWND NetLogoHandle(void);
void EndNetLogo(void);

#ifdef _OPENGLLOGO //OpenGL Logo
bool LogoDestroyGLView(HWND hWnd);
bool LogoDraw(void);
bool LogoCreateGLView(HWND hWnd);
bool LogoCreate(int iWidth, int iHeight);
DWORD WINAPI LogoDrawThread(LPVOID);
#endif //OpenGL Logo

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
