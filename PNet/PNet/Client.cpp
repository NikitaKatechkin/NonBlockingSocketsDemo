#include "Client.h"

bool Client::Connect(PNet::IPEndpoint ip)
{
	isConnected = false;

	PNet::Socket socket = PNet::Socket(ip.GetIPVersion());
	if (socket.Create() == PNet::PResult::P_Success)
	{
		if (socket.SetBlocking(true) != PNet::PResult::P_Success)
		{
			return false;
		}

		std::cout << "Socket successfully created." << std::endl;

		if (socket.Connect(ip) == PNet::PResult::P_Success)
		{
			if (socket.SetBlocking(false) == PNet::PResult::P_Success)
			{
				connection = PNet::TCPConnection(socket, ip);

				master_fd.fd = connection.socket.GetHandle();
				master_fd.events = POLLRDNORM | POLLWRNORM;
				master_fd.revents = 0;

				OnConnect();

				isConnected = true;
				return true;
			}
		}
		else
		{
		}

		socket.Close();
	}
	else
	{
		std::cerr << "Socket failed to create." << std::endl;
	}
	OnConnectFail();

	return false;
}

bool Client::IsConnected()
{
	return isConnected;
}

bool Client::Frame()
{
	/*
	PNet::Packet stringPacket(PNet::PacketType::PT_ChatMessage);
	stringPacket << std::string("This is my string packet!");

	PNet::Packet integerPacket(PNet::PacketType::PT_IntegerArray);
	uint32_t arraySize = 6;
	uint32_t integerArray[6] = { 0, 1, 2, 3, 4, 5 };
	integerPacket << arraySize;
	for (auto integer : integerArray)
	{
		integerPacket << integer;
	}


	PNet::PResult result;
	if (rand() % 2 == 0)
	{
		result = socket.Send(stringPacket);
	}
	else
	{
		result = socket.Send(integerPacket);
	}


	if (result != PNet::PResult::P_Success)
	{
		isConnected = false;
		return false;
	}

	std::cout << "Attempting to send chunk of data..." << std::endl;
	Sleep(500);
	*/
	/*PNet::Packet incomingPacket;
	if (socket.Recv(incomingPacket) != PNet::PResult::P_Success)
	{
		std::cout << "Lost connection&" << std::endl;

		isConnected = false;
		return false;
	}

	if (!ProcessPacket(incomingPacket))
	{
		isConnected = false;
		return false;
	}


	return true;*/
	use_fd = master_fd;

	if (WSAPoll(&use_fd, 1, 1) > 0)
	{

		if (use_fd.revents & POLLERR) //If error occured on the socket
		{
			CloseConnection("POLLERR");

			return false;
		}

		if (use_fd.revents & POLLHUP) //If error occured on the socket
		{
			CloseConnection("POLLHUP");

			return false;
		}

		if (use_fd.revents & POLLNVAL) //If error occured on the socket
		{
			CloseConnection("POLLNVAL");

			return false;
		}

		if (use_fd.revents & POLLRDNORM) //If data can be read without blocking
		{

			int bytesRecieved = 0;

			if (connection.pm_incoming.currentTask == PNet::PacketManagerTask::ProcessPacketSize)
			{
				bytesRecieved = recv(use_fd.fd, (char*)&connection.pm_incoming.currentPacketSize + connection.pm_incoming.currentPacketExtractionOffset,
					sizeof(uint16_t) - connection.pm_incoming.currentPacketExtractionOffset, NULL);
			}
			else if (connection.pm_incoming.currentTask == PNet::PacketManagerTask::ProcessPacketContents)
			{
				bytesRecieved = recv(use_fd.fd, (char*)&connection.buffer + connection.pm_incoming.currentPacketExtractionOffset,
					PNet::g_MaxPAcketSize - connection.pm_incoming.currentPacketExtractionOffset, NULL);
			}

			if (bytesRecieved == 0)
			{
				CloseConnection("Recv == 0");

				return false;
			}

			if (bytesRecieved == SOCKET_ERROR)
			{
				int error = WSAGetLastError();
				if (error != WSAEWOULDBLOCK)
				{
					CloseConnection("Recv < 0");

					return false;
				}
			}

			if (bytesRecieved > 0)
			{
				connection.pm_incoming.currentPacketExtractionOffset += bytesRecieved;
				if (connection.pm_incoming.currentTask == PNet::PacketManagerTask::ProcessPacketSize)
				{
					if (connection.pm_incoming.currentPacketExtractionOffset == sizeof(uint16_t))
					{
						connection.pm_incoming.currentPacketSize = ntohs(connection.pm_incoming.currentPacketSize);
						if (connection.pm_incoming.currentPacketSize > PNet::g_MaxPAcketSize)
						{
							CloseConnection("Packet size too large.");

							return false;
						}

						connection.pm_incoming.currentPacketExtractionOffset = 0;
						connection.pm_incoming.currentTask = PNet::PacketManagerTask::ProcessPacketContents;
					}
				}
				else if (connection.pm_incoming.currentTask == PNet::PacketManagerTask::ProcessPacketContents)
				{
					if (connection.pm_incoming.currentPacketExtractionOffset == connection.pm_incoming.currentPacketSize)
					{
						std::shared_ptr<PNet::Packet> packet = std::make_shared<PNet::Packet>();
						packet->buffer.resize(connection.pm_incoming.currentPacketSize);
						memcpy(&packet->buffer[0], connection.buffer, connection.pm_incoming.currentPacketSize);

						connection.pm_incoming.Append(packet);

						connection.pm_incoming.currentPacketSize = 0;
						connection.pm_incoming.currentPacketExtractionOffset = 0;
						connection.pm_incoming.currentTask = PNet::PacketManagerTask::ProcessPacketSize;
					}
				}
			}
		}

		if (use_fd.revents & POLLWRNORM) //If data can be write without blocking
		{
			PNet::PacketManager & pm = connection.pm_outgoing;
			while (pm.HasPendingPackets())
			{
				if (pm.currentTask == PNet::PacketManagerTask::ProcessPacketSize)
				{
					pm.currentPacketSize = uint16_t(pm.Retrieve()->buffer.size());
					uint16_t bigEndianPacketSize = htons(pm.currentPacketSize);

					int bytesSent = send(use_fd.fd, (char*)(&bigEndianPacketSize) + pm.currentPacketExtractionOffset, sizeof(uint16_t) - pm.currentPacketExtractionOffset, NULL);

					if (bytesSent > 0)
					{
						pm.currentPacketExtractionOffset += bytesSent;
					}

					if (pm.currentPacketExtractionOffset == sizeof(uint16_t))
					{
						pm.currentPacketExtractionOffset = 0;
						pm.currentTask = PNet::PacketManagerTask::ProcessPacketContents;
					}
					else
					{
						break;
					}
				}
				else if (pm.currentTask == PNet::PacketManagerTask::ProcessPacketContents)
				{
					char* bufferPtr = &pm.Retrieve()->buffer[0];

					int bytesSent = send(use_fd.fd, (char*)(bufferPtr)+pm.currentPacketExtractionOffset, pm.currentPacketSize - pm.currentPacketExtractionOffset, NULL);

					if (bytesSent > 0)
					{
						pm.currentPacketExtractionOffset += bytesSent;
					}

					if (pm.currentPacketExtractionOffset == pm.currentPacketSize)
					{
						pm.currentPacketExtractionOffset = 0;
						pm.currentTask = PNet::PacketManagerTask::ProcessPacketSize;
						pm.Pop();
					}
					else
					{
						break;
					}
				}
			}
		}
	}

	while (connection.pm_incoming.HasPendingPackets())
	{
		std::shared_ptr<PNet::Packet> frontPacket = connection.pm_incoming.Retrieve();

		if (!ProcessPacket(frontPacket))
		{
			CloseConnection("Failed to process incoming packet.");

			return false;
		}

		connection.pm_incoming.Pop();
	}
	
	return true;
}

bool Client::ProcessPacket(std::shared_ptr<PNet::Packet> packet)
{
	std::cout << "Packet recieved with size:" << packet->buffer.size() << std::endl;
	return true;
}

void Client::OnConnect()
{
	std::cout << "Successfully connected." << std::endl;
}

void Client::OnConnectFail()
{
	std::cout << "Failed to connect." << std::endl;
}

void Client::OnDisconnect(std::string reason)
{
	std::cout << "Lost connection. Reason: " << reason << std::endl;
}

void Client::CloseConnection(std::string reason)
{
	OnDisconnect(reason);
	master_fd.fd = 0;
	isConnected = false;
	connection.Close();
}

/*bool Client::ProcessPacket(PNet::Packet packet)
{
	switch (packet.GetPacketType())
	{
	case PNet::PacketType::PT_ChatMessage:
	{
		std::string chatMessage;
		packet >> chatMessage;

		std::cout << "Chat message from server: " << chatMessage << std::endl;

		break;
	}
	case PNet::PacketType::PT_IntegerArray:
	{
		uint32_t arraySize;
		packet >> arraySize;

		std::cout << "Array sended by client: Array size = " << arraySize << std::endl;;
		for (uint32_t index = 0; index < arraySize; index++)
		{
			uint32_t element = 0;
			packet >> element;
			std::cout << "\t Element[" << index << "]: " << element << std::endl;
		}

		break;
	}
	default:
		return false;
	}

	return true;
}*/