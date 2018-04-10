///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <WinSock2.H>
#include <WinSock.H>
#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <Time.H>

#include "../../../../@Libraries/CSocketServer/CSocketServer.H"
#include "../../../../@Libraries/CStringBuilder/CStringBuilder.H"
#include "../../../../@Libraries/NSWFL/NSWFL.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::String;
using namespace NSWFL::File;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void OnError(CSocketServer *lpCSocketServer, CSocketClient *pClient, int iErrorNumber, const char *sErrorMsg)
{
	if(pClient)
	{
		printf("Error: %d, Client: %d, Msg: %s\n", iErrorNumber, pClient->PeerID(), sErrorMsg);
	}
	else {
		printf("Error: %d, Msg: %s\n", iErrorNumber, sErrorMsg);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
	if(argc != 5)
	{
		printf("Usage: DummyClt.exe <IP_Address> <Port_number> <Use_Packet_Mode> <Buffer_Size>\n");
		printf("Example: DummyClt.exe 127.0.0.1 80 0 1048576\n");
		return 1;
	}

	char *sIPAddress = argv[1];
	int iPort = atol(argv[2]);
	bool bUsePacketMode = (atol(argv[3]) != 0);
	int iBufferSize = atol(argv[4]);

	CSocketServer Sock;

	printf("Initializing Dummy Server on port %d.\n", iPort);
	if(!Sock.Initialize(&OnError))
	{
		return 0;
	}

	Sock.PacketMode(bUsePacketMode);

	CStringBuilder data;
	while(data.Length < iBufferSize)
	{
		char guid[255];
		NewGUID(guid, sizeof(guid));
		data.Append(guid);
	}
	data.Terminate(iBufferSize);

	if(Sock.Start())
	{
		while(true)
		{
			CSocketClient *pClient;
			if(Sock.ConnectByIPAndCreateThread(sIPAddress, iPort, &pClient))
			{
				double dTotalBytes = 0;
				double dLastTime = 0;
				double dLastBytes = 0;

				while(pClient->IsConnected())
				{
					if(pClient->SetNextSendData(data.Buffer, data.Length))
					{
						dTotalBytes += data.Length;
						double dDeltaBytes = (dTotalBytes - dLastBytes);
						if(dDeltaBytes >= 1048576)
						{
							if(dLastTime > 0)
							{
								char sSize1[64];
								char sSize2[64];
								char sSize3[64];

								double dDeltaTime = (GetTickCount() - dLastTime) / 1000.0;
								printf("Total: %s, Delta: %s, Rate: %s/s\n",
									FileSizeFriendly(dTotalBytes, 2, sSize1, sizeof(sSize1)),
									FileSizeFriendly(dDeltaBytes, 2, sSize2, sizeof(sSize2)),
									FileSizeFriendly(dDeltaBytes / dDeltaTime, 2, sSize3, sizeof(sSize3))
								);
							}

							dLastBytes = dTotalBytes;
							dLastTime = GetTickCount();
						}

					}
				}
			}
			Sleep(100); //Waiting on a connection.
		}
	}

	Sock.Destroy();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
