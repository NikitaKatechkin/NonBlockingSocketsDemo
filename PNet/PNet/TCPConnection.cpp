#include "TCPConnection.h"

namespace PNet
{
	TCPConnection::TCPConnection()
		:socket(Socket())
	{

	}

	TCPConnection::TCPConnection(Socket socket, IPEndpoint endpoint):
		socket(socket), endpoint(endpoint)
	{
		stringRepresentation = "[" + endpoint.GetIPString();
		stringRepresentation += ":" + std::to_string(endpoint.GetPort()) + "]";
	}

	std::string TCPConnection::ToString()
	{
		return stringRepresentation;
	}

	void TCPConnection::Close()
	{
		socket.Close();
	}

}