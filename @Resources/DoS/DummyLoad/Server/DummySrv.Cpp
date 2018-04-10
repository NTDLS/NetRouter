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
	if(argc != 3)
	{
		printf("Usage: DummySrv.exe <Port_Number> <Use_Packet_Mode>\n");
		printf("Example: DummySrv.exe 80 0\n");
		return 1;
	}

	int iPort = atol(argv[1]);
	bool bUsePacketMode = (atol(argv[2]) != 0);

	CSocketServer Sock;

	printf("Initializing Dummy Server on port %d.\n", iPort);
	if(!Sock.Initialize(iPort, 1, &OnError))
	{
		return 0;
	}

	Sock.PacketMode(bUsePacketMode);
	Sock.MaxBufferSize(1024*1024*25);
	Sock.InitialBufferSize(1024*1024*25);

	if(Sock.Start())
	{
		while(true)
		{
			if(Sock.CurrentConnections() > 0)
			{
				double dTotalBytes = 0;
				double dLastTime = 0;
				double dLastBytes = 0;

				CSocketClient *pClient = Sock.Client(0);

				BASICHUNK data;
				memset(&data, 0, sizeof(data));

				while(pClient->IsConnected())
				{
					if(pClient->GetClientData(&data))
					{
						dTotalBytes += data.iLength;
						double dDeltaBytes = (dTotalBytes - dLastBytes);
						if(dDeltaBytes > 1048576)
						{
							if(dLastTime >= 0)
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
	
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
