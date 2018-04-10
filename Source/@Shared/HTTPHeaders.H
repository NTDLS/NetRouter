///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _HTTPHeaders_Types_H
#define _HTTPHeaders_Types_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define HTTPHEADERS_LIST_POS_NAME		0
#define HTTPHEADERS_LIST_POS_METHOD		1
#define HTTPHEADERS_LIST_POS_TEXT	2
#define HTTPHEADERS_LIST_POS_VERBS		3
#define HTTPHEADERS_LIST_POS_DESCR		4

#define HTTPHEADERS_MAX_NAME_LENGTH		1024
#define HTTPHEADERS_MAX_REPLACE_LENGTH	2048
#define HTTPHEADERS_MAX_VERBS_LENGTH	1024
#define HTTPHEADERS_MAX_DESCR_LENGTH	1024
#define HTTPHEADERS_MAX_METHOD_LENGTH	1024

#define HEADER_METHOD_ADDREPLACE		0
#define HEADER_METHOD_REPLACE			1
#define HEADER_METHOD_DELETE			2

/*
	HEADER_METHOD_ADDREPLACE
		If the header exists, is will be replaced. If it does not exist it will be added.
			
	HEADER_METHOD_REPLACE
		If the header exists, it will be replaced. If it does not exist it will NOT be added.
	
	HEADER_METHOD_DELETE
		If the header exists, it will be removed.
*/


extern const char *HTTP_VERBS[];
extern const char *HTTP_HANDLER_METHODS[];
extern const char *HTTP_HEADERS[];
extern const char *HTTP_HEADER_VARS[];

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_HTTP_Header {
	char *Name;
#ifdef ROUTER_SERVICE
	int Method; //Delete, Replace, Leave
#else
	char *MethodName;
#endif
	char *Text;
	char *Description;
	char *Verbs;
	bool Enabled;
} HTTPHEADER, *LPHTTPHEADER;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _tag_HTTP_Headers {
	int Count;
	HTTPHEADER *Items;
} HTTPHEADERS, *LPHTTPHEADERS;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IsHTTPVerb(const char *sText);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
