#pragma once

#include "Socket.h"
#include "PacketManager.h"

namespace PNet
{

	class TCPConnection
	{
	public:
		TCPConnection();
		TCPConnection(Socket socket, IPEndpoint endpoint);

		std::string ToString();
		void Close();

		Socket socket;

		PacketManager pm_incoming;
		PacketManager pm_outgoing;
		char buffer[PNet::g_MaxPAcketSize];
	private:
		IPEndpoint endpoint;
		std::string stringRepresentation = "";
	};
}