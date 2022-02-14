//Server Part
#include "MyServer.h"

#include <iostream>

int main()
{
	if (PNet::NetWork::Initialize())
	{
		std::cout << "WinSock API successfully initialized." << std::endl;

		MyServer server;
		if (server.Initialize(PNet::IPEndpoint("::", 1111)))
		{
			while (true)
			{
				server.Frame();
			}
		}
	}

	PNet::NetWork::Shutdown();
	system("pause");
	return 0;
}