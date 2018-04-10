///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _HTTPHeaders_Types_Cpp
#define _HTTPHeaders_Types_Cpp
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char *HTTP_HANDLER_METHODS[] = {
	"Add/Replace",
	"Replace",
	"Delete",
	NULL
};

const char *HTTP_VERBS[] = {
	"CONNECT",
	"DELETE",
	"GET",
	"HEAD",
	"OPTIONS",
	"POST",
	"PUT",
	"<Any Request>",
	"<Any Response>",
	"<Any>",
	NULL
};

const char *HTTP_HEADER_VARS[] = {
	"[HTTP.Header(<name>)]",
	"[HTTP.Verb]",
	"[HTTP.Version]",
	"[Server.CPU.Count]",
	"[Server.CPU.Name]",
	"[Server.CPU.Speed]",
	"[Server.Host.Name]",
	"[Server.Memory.Free]",
	"[Server.Memory.Total]",
	"[Server.Memory.Used]",
	"[Server.OS.Name]",
	"[Server.Software.Name]",
	"[Server.Software.Version]",
	"[Server.User.Name]",
	"[Time(<format>)]",
	"[Date(<format>)]",
	NULL
};

const char *HTTP_HEADERS[] = {
	"Accept",
	"Accept-Charset",
	"Accept-Encoding",
	"Accept-Language",
	"Accept-Ranges",
	"Age",
	"Allow",
	"Authorization",
	"Cache-Control",
	"Cache-Control",
	"Connection",
	"Content-Encoding",
	"Content-Language",
	"Content-Length",
	"Content-Location",
	"Content-MD5",
	"Content-Range",
	"Content-Type",
	"Cookie",
	"Date",
	"ETag",
	"Expect",
	"Expires",
	"From",
	"Host",
	"If-Match",
	"If-Modified-Since",
	"If-None-Match",
	"If-Range",
	"If-Unmodified-Since",
	"Last-Modified",
	"Location",
	"Max-Forwards",
	"Pragma",
	"Proxy-Authenticate",
	"Proxy-Authorization",
	"Range",
	"Referer",
	"Retry-After",
	"Server",
	"Trailer",
	"Transfer-Encoding",
	"Upgrade",
	"User-Agent",
	"Vary",
	"Via",
	"Warning",
	"WWW-Authenticate",
	NULL
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IsHTTPVerb(const char *sText)
{
	for(int i = 0; HTTP_VERBS[i]; i++)
	{
		if(_strcmpi(HTTP_VERBS[i], sText) == 0)
		{
			return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
