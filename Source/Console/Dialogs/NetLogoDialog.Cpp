///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _NetLogoDialog_CPP
#define _NetLogoDialog_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <StdIO.H>

#include "../Source/Entry.H"

#include "NetLogoDialog.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HWND NetLogoDialog_hWnd = NULL;
HINSTANCE hModuleHandle = NULL;
LPABOUTDLGINFO glpADI;

#ifdef _OPENGLLOGO //OpenGL Logo
#include <Math.H>
#include <Time.H>

#include <GL/GL.H>
#include <GL/GLU.H>
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool NetLogo(LPABOUTDLGINFO lpADI)
{
	HWND OwnerHandle = NULL;

    glpADI = lpADI;

	if(glpADI->OwnerHandle == NULL)
        OwnerHandle = GetActiveWindow();
	else OwnerHandle = glpADI->OwnerHandle;

	hModuleHandle = GetModuleHandle(NULL);

	DialogBox(hModuleHandle, MAKEINTRESOURCE(IDD_NETLOGO), OwnerHandle, NetLogoDialog);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HWND NetLogoHandle(void)
{
	return NetLogoDialog_hWnd;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IsNetLogoActive(void)
{
	return (NetLogoDialog_hWnd != NULL);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void EndNetLogo(void)
{
	EndDialog(NetLogoDialog_hWnd, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK NetLogoDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //--------------------------------------------------------------------------

    static HWND AppName_hWnd       = NULL;
    static HWND AppVersion_hWnd    = NULL;
    static HWND AppCopyRight_hWnd  = NULL;
    static HWND DisplayIcon_hWnd   = NULL;
    static HWND BuildTimeDate_hWnd = NULL;
	static HWND NetLink_hWnd       = NULL;

    static HCURSOR HandCursor      = NULL;

    static DWORD BGColorRef        = 0;

    //--------------------------------------------------------------------------

    if(uMsg == WM_INITDIALOG) // Received A Initilize Dialog Message
    {
        char sTempText[255];

        NetLogoDialog_hWnd = hWnd;

		HandCursor = LoadCursor(hModuleHandle, MAKEINTRESOURCE(IDC_HANDCURSOR));
        BGColorRef = GetSysColor(COLOR_3DFACE);

        sprintf_s(sTempText, sizeof(sTempText), "NetworkDLS :: %s", glpADI->TitleCaption);
        SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)sTempText);

        AppName_hWnd       = GetDlgItem(hWnd, IDC_APPNAME);
        AppVersion_hWnd    = GetDlgItem(hWnd, IDC_APPVERSION);
        BuildTimeDate_hWnd = GetDlgItem(hWnd, IDC_BUILDTIMEDATE);
        AppCopyRight_hWnd  = GetDlgItem(hWnd, IDC_APPCOPYRIGHT);
        DisplayIcon_hWnd   = GetDlgItem(hWnd, IDC_ABOUTICON);
        NetLink_hWnd       = GetDlgItem(hWnd, IDC_NETLINK);

        SendMessage(DisplayIcon_hWnd, (UINT)STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)glpADI->DisplayIcon);

        Set_Text(AppName_hWnd, glpADI->TitleCaption);

        Set_Text(AppVersion_hWnd, glpADI->FileVersion);

        sprintf_s(sTempText, sizeof(sTempText), "%s - %s", glpADI->BuildDate, glpADI->BuildTime);
        Set_Text(BuildTimeDate_hWnd, sTempText);

        Set_Text(AppCopyRight_hWnd, glpADI->CopyRight);

        CenterOverOwner(hWnd);

		SetWindowPos(DisplayIcon_hWnd, NULL, 0, 0, 32, 32, SWP_NOMOVE);

		#ifdef _OPENGLLOGO //OpenGL Logo
		LogoCreateGLView(DisplayIcon_hWnd);
		#endif //OpenGL Logo

        return TRUE; // Return TRUE to set the keyboard focus, Otherwise return FALSE
    }

    //--------------------------------------------------------------------------

	if(IsMouseOverHwnd(NetLink_hWnd))
	{
		SetCursor(HandCursor);

		if(uMsg == WM_LBUTTONUP)
        {
			ShellExecute(0, "Open", "Http://www.NetworkDLS.com/", NULL, NULL, SW_SHOWNORMAL);
        }
	}

    //--------------------------------------------------------------------------

    if(uMsg == WM_CTLCOLORSTATIC)
	{
		if((HANDLE)lParam == NetLink_hWnd)
			return (BOOL)Set_Color(RGB(0, 0, 255), BGColorRef, wParam);

        return FALSE;
	}

    //--------------------------------------------------------------------------

    if(uMsg == WM_COMMAND)
    {
        if(wParam == IDC_OKBUTTON)
        {
            EndDialog(hWnd, 0);
            DestroyWindow(hWnd);
            return TRUE;
        }
        return FALSE;
    }

    //--------------------------------------------------------------------------

	if(uMsg == WM_PAINT)
    {
        HDC ThisHDC;
        PAINTSTRUCT ThisPS;

        ThisHDC = BeginPaint(hWnd, &ThisPS);

        // Any painting should be done here

        EndPaint(hWnd, &ThisPS);
        return TRUE;
    }

    //--------------------------------------------------------------------------

	if(uMsg == WM_DESTROY)
    {
		FreeLibrary(hModuleHandle);
		hModuleHandle = NULL;
		NetLogoDialog_hWnd = NULL;
		//LogoDestroyGLView(DisplayIcon_hWnd);
        return TRUE;
    }

    //--------------------------------------------------------------------------

    if(uMsg == WM_CLOSE)  // Received Close Message
    {
        EndDialog(hWnd,0);
        DestroyWindow(hWnd);
        return TRUE;
    }

    //--------------------------------------------------------------------------

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _OPENGLLOGO //OpenGL Logo
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GLuint texture;
//Requires opengl32.lib and glu32.lib
//////////////////////////////////////////////////////////////////////////////////////////////

struct GLData {
	HWND hWnd;		// Owner Of This Structure
	HGLRC hRC;		// Opengl Rendering Context Handle
	HDC hDC;		// Window Handle
	HANDLE hEvent;	// Handle To Signal When To Stop The Thread
	DWORD dwThread; // Thread ID
	HANDLE hThread; // Thread Handle
};

//////////////////////////////////////////////////////////////////////////////////////////////

DWORD WINAPI LogoDrawThread(LPVOID lpParameter)
{
	struct GLData *glData = (struct GLData *)lpParameter;
	RECT rc;

	// you have to create context's in the threads they are used
	glData->hRC = wglCreateContext(glData->hDC); // so we do that here
	wglMakeCurrent(glData->hDC, glData->hRC);	 // and more in-thread context bs

	GetWindowRect(glData->hWnd, &rc); // used for setting up the viewport

	LogoCreate(rc.right - rc.left, rc.bottom - rc.top); // pass it to the user routine

	while(WaitForSingleObject(glData->hEvent, 0) != WAIT_OBJECT_0) // Wait for signal
	{
		Sleep(10);	// give up the slice so we dont choke the cpu with our simple effect
		LogoDraw(); // our drawing routine
		SwapBuffers(glData->hDC); // flip the backbuffer into view
	}

	wglMakeCurrent(NULL, NULL); // unselects any current opengl context
	wglDeleteContext(glData->hRC); // deletes it

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool LogoCreateGLView(HWND hWnd)
{
	PIXELFORMATDESCRIPTOR pfd;
	UINT pixelformat;
	struct GLData *glData = (struct GLData *)LocalAlloc(LPTR, sizeof(struct GLData));

	glData->hWnd = hWnd;
	glData->hEvent = CreateEvent(NULL, TRUE, FALSE, NULL); // create an event we signal the thread with

	ZeroMemory(&pfd, sizeof(pfd)); // clear out the pixelformatdescriptor

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR); // fill in the required fields
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER; // opengl :D~
	pfd.iPixelType = PFD_TYPE_RGBA; // rgba context
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16; // 16 bit depth buffer, after all, this is a small window :|

	glData->hDC = GetDC(hWnd); // get the dc so we can play with it

	pixelformat = ChoosePixelFormat(glData->hDC, &pfd); // choose..

	SetPixelFormat(glData->hDC, pixelformat, &pfd); // .. and set pixel format

	SetWindowLong(hWnd, GWL_USERDATA, (LONG)glData); // put our custom chunk of data in the windows userdata space

	glData->hThread = CreateThread(NULL, 0, LogoDrawThread, glData, 0, &glData->dwThread); // create the rendering thread

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool LogoDestroyGLView(HWND hWnd)
{
	struct GLData *glData = (struct GLData *)GetWindowLong(hWnd, GWL_USERDATA);

	SetEvent(glData->hEvent); // tell the rendering thread to quit
	WaitForSingleObject(glData->hThread, INFINITE); // wait for it to do so

	CloseHandle(glData->hEvent);  // close the event handle we signalled the thread with
	CloseHandle(glData->hThread); // close the thread handle

	ReleaseDC(hWnd, glData->hDC); // release the windows dc we GetDC'd in glLogoCreateGLView

	LocalFree(glData);

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool LogoCreate(int iWidth, int iHeight)
{
	HRSRC hrTex = FindResource(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_NETLOGO), "TEXTURE");
	HGLOBAL hmTex = LoadResource(GetModuleHandle(NULL), hrTex); // load it
	LPVOID texmem = LockResource(hmTex); // get a pointer to the data

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_TEXTURE_2D);

	glGenTextures(1, &texture); // build a texture id for our bitmap
	glBindTexture(GL_TEXTURE_2D, texture); // bind it

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, texmem); // set texture map

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // set filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE); // texture environment

	glViewport(0, 0, iWidth, iHeight);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f, (GLfloat)iWidth / (GLfloat)iHeight, 0.1f, 100.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(0, 0, 0, 0);
	glClearDepth(1);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLightModelf(GL_LIGHT_MODEL_TWO_SIDE,1 );

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool LogoDraw(void) // called whenever it needs drawing
{
	static GLfloat Rotation = 0.0f;
	static GLfloat GBRotate = 0.0f;

	static GLfloat RotateX	= 0.0f;
	static GLfloat RotateY	= 0.0f;
	static GLfloat RotateZ	= 0.0f;

	static BOOL ClockWise	= TRUE;

	glClear(GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -10.0f);
	glRotatef(GBRotate, 0.0f, 0.0f, -1.0f);
	glScalef(7, 7, 7);

	glDisable(GL_TEXTURE_2D);
	glDepthMask(1);

	glBegin(GL_QUADS);
	glColor3ub(0,105,180);
	glTexCoord2f(0, 0);
	glVertex3f(-1.0f, 1.0f, 0.0f);

	glColor3ub(0,105,180);
	glTexCoord2f(1, 0);
	glVertex3f(1.0f,1.0f, 0.0f);

	glColor4f(1, 1, 1, 0.08f);
	glTexCoord2f(1, 1);
	glVertex3f(1.0f, -1.0f, 0.0f);

	glColor4f(1, 1, 1, 0.08f);
	glTexCoord2f(0, 1);
	glVertex3f(-1.0f, -1.0f, 0.0f);
	glEnd();

	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -6.0f);

	RotateX += 0.2f;
	RotateY += 0.9f;
	RotateZ += 0.4f;

	glRotatef(Rotation, RotateX, RotateY, RotateZ);
	glScalef(1.0f,1.0f, 1.0f);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);

	glEnable(GL_DEPTH_TEST);
	//glDepthMask(0);

	glColor4f(1, 1, 1, 1.0f);

	glBegin(GL_QUADS);
	// Front
	glTexCoord2f(0, 0);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1, 0);
	glVertex3f(1.0f,1.0f, 1.0f);
	glTexCoord2f(1, 1);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(0, 1);
	glVertex3f(-1.0f, -1.0f, 1.0f);

	// Bottom
	glTexCoord2f(0, 0);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1, 0);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(1, 1);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(0, 1);
	glVertex3f(1.0f, 1.0f, 1.0f);

	// Top
	glTexCoord2f(0, 0);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1, 0);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(1, 1);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glTexCoord2f(0, 1);
	glVertex3f(1.0f, -1.0f, 1.0f);

	// Back
	glTexCoord2f(0, 0);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(1, 0);
	glVertex3f(1.0f,1.0f, -1.0f);
	glTexCoord2f(1, 1);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glTexCoord2f(0, 1);
	glVertex3f(-1.0f, -1.0f, -1.0f);

	// Right
	glTexCoord2f(0, 0);
	glVertex3f(1.0f,-1.0f,1.0f);
	glTexCoord2f(1, 0);
	glVertex3f(1.0f,-1.0f,-1.0f);
	glTexCoord2f(1, 1);
	glVertex3f(1.0f,1.0f,-1.0f);
	glTexCoord2f(0, 1);
	glVertex3f(1.0f,1.0f,1.0f);

	// Left
	glTexCoord2f(0, 0);
	glVertex3f(-1.0f,-1.0f,1.0f);
	glTexCoord2f(1, 0);
	glVertex3f(-1.0f,-1.0f,-1.0f);
	glTexCoord2f(1, 1);
	glVertex3f(-1.0f,1.0f,-1.0f);
	glTexCoord2f(0, 1);
	glVertex3f(-1.0f,1.0f,1.0f);

	glEnd();

	if(ClockWise)
	{
		GBRotate += 0.5f;
		if(GBRotate > 360)
		{
			GBRotate = 360;
			ClockWise = FALSE;
		}
	}
	else
	{
		GBRotate -= 0.5f;
		if(GBRotate < 0)
		{
			GBRotate = 0;
			ClockWise = TRUE;
		}
	}

	Rotation += 1.0f;
	if(Rotation > 360) Rotation = 0;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif //OpenGL Logo
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
