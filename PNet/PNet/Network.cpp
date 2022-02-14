#include "Network.h"
#include <iostream>

bool PNet::NetWork::Initialize()
{
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (result != 0)
	{
		std::cerr << "Failed to start up the WinSock API." << std::endl;
		return false;
	}

	if (LOBYTE(wsa_data.wVersion) != 2 || HIBYTE(wsa_data.wVersion) != 2)
	{
		std::cerr << "Could not find a usable version of the WinSock API DLL." << std::endl;
		return false;
	}

	return true;
}

void PNet::NetWork::Shutdown()
{
	WSACleanup();
}
