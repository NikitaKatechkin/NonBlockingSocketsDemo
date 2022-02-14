//Client Part
#include "MyClient.h"

#include <iostream>
#include <string>

int main()
{
	if (PNet::NetWork::Initialize())
	{
		std::cout << "WinSock API successfully initialized." << std::endl;
		MyClient client;
		if (client.Connect(PNet::IPEndpoint("::1", 1111)))
		{
			while (client.IsConnected())
			{
				client.Frame();
			}
		}
	}

	PNet::NetWork::Shutdown();
	system("pause");
	return 0;
}