#include "MyClient.h"

bool MyClient::ProcessPacket(std::shared_ptr<PNet::Packet> packet)
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

void MyClient::OnConnect()
{
	std::cout << "Successfully connected to server." << std::endl;

	std::shared_ptr<PNet::Packet> helloPacket = std::make_shared<PNet::Packet>(PNet::PacketType::PT_ChatMessage);
	*helloPacket << std::string("Hello from client " + connection.ToString());
	connection.pm_outgoing.Append(helloPacket);
}
