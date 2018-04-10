///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _ManageRouteDialog_H
#define _ManageRouteDialog_H

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//---------------------(Includes)
#include "../../@Shared/ListenIPs.H"
#include "../../@Shared/ConnectHosts.H"
#include "../../@Shared/HTTPHeaders.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//---------------------(Constant Definitions)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//---------------------(Variable Declarations)
extern char sManageRouteDialogGUID[ROUTE_MAX_GUID];
extern HWND ghIPAddressList;
extern HWND ghHostsAddressList;
extern HWND ghHTTPHeaderList;
extern int giEditListIndex;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//---------------------(Function Prototypes)
INT_PTR CALLBACK ManageRouteDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void PopListeningIPsList_AddItem(HWND hList, LISTENIP *obj);
void PopListeningIPsList_AddItem(HWND hList, LISTENIP *obj, int iInsertionIndex);
void PopHostsList_AddItem(HWND hList, CONNECTHOST *obj);
void PopHostsList_AddItem(HWND hList, CONNECTHOST *obj, int iInsertionIndex);
void PopHTTPHeadersList_AddItem(HWND hList, HTTPHEADER *obj, int iInsertionIndex);
void PopHTTPHeadersList_AddItem(HWND hList, HTTPHEADER *obj);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
