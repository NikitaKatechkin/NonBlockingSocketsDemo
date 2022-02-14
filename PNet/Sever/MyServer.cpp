#include "MyServer.h"

void MyServer::OnConnect(PNet::TCPConnection & newConnection)
{
	std::cout << newConnection.ToString() << " - New connection accepted." << std::endl;

	std::shared_ptr<PNet::Packet> welcomeMessagePacket = std::make_shared<PNet::Packet>(PNet::PacketType::PT_ChatMessage);
	*welcomeMessagePacket << std::string("This is welcome server string packet.");

	newConnection.pm_outgoing.Append(welcomeMessagePacket);

	std::shared_ptr<PNet::Packet> newUserMessagepacket = std::make_shared<PNet::Packet>(PNet::PacketType::PT_ChatMessage);
	*newUserMessagepacket << std::string("A user " + newConnection.ToString() + " connected");
	for (auto& connection : connections)
	{
		if (&connection == &newConnection)
			continue;

		connection.pm_outgoing.Append(newUserMessagepacket);
	}
}

void MyServer::OnDisconnect(PNet::TCPConnection & lostConnection, std::string reason)
{
	std::cout << "[" << reason << "] Connection lost:" << lostConnection.ToString() << "." << std::endl;

	std::shared_ptr<PNet::Packet> connectionLostPacket = std::make_shared<PNet::Packet>(PNet::PacketType::PT_ChatMessage);
	*connectionLostPacket << std::string("A user " + lostConnection.ToString() + " disconnected");
	for (auto& connection : connections)
	{
		if (&connection == &lostConnection)
			continue;

		connection.pm_outgoing.Append(connectionLostPacket);
	}
}

bool MyServer::ProceesPacket(std::shared_ptr<PNet::Packet> packet)
{
	switch (packet->GetPacketType())
	{
	case PNet::PacketType::PT_ChatMessage:
	{
		std::string chatMessage;
		*packet >> chatMessage;

		std::cout << "Chat message from client: " << chatMessage << std::endl;

		break;
	}
	case PNet::PacketType::PT_IntegerArray:
	{
		uint32_t arraySize;
		*packet >> arraySize;

		std::cout << "Array sended by client: Array size = " << arraySize << std::endl;;
		for (uint32_t index = 0; index < arraySize; index++)
		{
			uint32_t element = 0;
			*packet >> element;
			std::cout << "\t Element[" << index << "]: " << element << std::endl;
		}

		break;
	}
	default:
		std::cout << "Unrecogmized packet type: " << packet->GetPacketType() << std::endl;
		return false;
	}

	return true;
}
