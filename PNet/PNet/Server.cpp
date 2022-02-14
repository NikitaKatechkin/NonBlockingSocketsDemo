#include "Server.h"

bool Server::Initialize(PNet::IPEndpoint ip)
{
	master_fd.clear();
	connections.clear();
	

	listeninigSocket = PNet::Socket(ip.GetIPVersion());
	if (listeninigSocket.Create() == PNet::PResult::P_Success)
	{
		std::cout << "Socket successfully created." << std::endl;

		if (listeninigSocket.Listen(ip) == PNet::PResult::P_Success)
		{
			WSAPOLLFD listeningSocketFD = {};
			listeningSocketFD.fd = listeninigSocket.GetHandle();
			listeningSocketFD.events = POLLRDNORM;
			listeningSocketFD.revents = 0;

			master_fd.push_back(listeningSocketFD);

			std::cout << "Socket succesfully listening." << std::endl;

			return true;
		}
		else
		{
			std::cerr << "Failed to listen socket to port 1111." << std::endl;
		}

		listeninigSocket.Close();
	}
	else
	{
		std::cerr << "Socket failed to create." << std::endl;
	}
	

	return false;
}

void Server::Frame()
{
	use_fd = master_fd;

	if (WSAPoll(use_fd.data(), use_fd.size(), 1) > 0)
	{
#pragma region listener
		WSAPOLLFD& listeningSocketFD = use_fd[0];
		if (listeningSocketFD.revents & POLLRDNORM)
		{
			PNet::Socket newConnectionSocket;
			PNet::IPEndpoint newConnectionEndpoint;
			if (listeninigSocket.Accept(newConnectionSocket, &newConnectionEndpoint) == PNet::PResult::P_Success)
			{
				connections.emplace_back(PNet::TCPConnection(newConnectionSocket, newConnectionEndpoint));
				PNet::TCPConnection& acceptedConnection = connections[connections.size() - 1];
				OnConnect(acceptedConnection);

				WSAPOLLFD newConnectionFD = {};
				newConnectionFD.fd = newConnectionSocket.GetHandle();
				newConnectionFD.events = POLLRDNORM | POLLWRNORM;
				newConnectionFD.revents = 0;

				master_fd.push_back(newConnectionFD);

				
			}
			else
			{
				std::cerr << "Failed to accept new connection." << std::endl;
			}
		}
#pragma endregion Code specific for the listening socket

		for (int index = int(use_fd.size()) - 1; index > 0; index--)
		{
			int connectionIndex = index - 1;
			PNet::TCPConnection& connection = connections[connectionIndex];

			if (use_fd[index].revents & POLLERR) //If error occured on the socket
			{
				CloseConnection(connectionIndex, "POLLERR");

				continue;
			}

			if (use_fd[index].revents & POLLHUP) //If error occured on the socket
			{
				CloseConnection(connectionIndex, "POLLHUP");

				continue;
			}

			if (use_fd[index].revents & POLLNVAL) //If error occured on the socket
			{
				CloseConnection(connectionIndex, "POLLNVAL");

				continue;
			}

			if (use_fd[index].revents & POLLRDNORM) //If data can be read without blocking
			{

				int bytesRecieved = 0;

				if (connection.pm_incoming.currentTask == PNet::PacketManagerTask::ProcessPacketSize)
				{
					bytesRecieved = recv(use_fd[index].fd, (char*)&connection.pm_incoming.currentPacketSize + connection.pm_incoming.currentPacketExtractionOffset,
						sizeof(uint16_t) - connection.pm_incoming.currentPacketExtractionOffset, NULL);
				}
				else if (connection.pm_incoming.currentTask == PNet::PacketManagerTask::ProcessPacketContents)
				{
					bytesRecieved = recv(use_fd[index].fd, (char*)&connection.buffer + connection.pm_incoming.currentPacketExtractionOffset,
						PNet::g_MaxPAcketSize - connection.pm_incoming.currentPacketExtractionOffset, NULL);
				}

				if (bytesRecieved == 0)
				{
					CloseConnection(connectionIndex, "Recv == 0");

					continue;
				}

				if (bytesRecieved == SOCKET_ERROR)
				{
					int error = WSAGetLastError();
					if (error != WSAEWOULDBLOCK)
					{
						CloseConnection(connectionIndex, "Recv < 0");

						continue;
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
								CloseConnection(connectionIndex, "Packet size too large.");

								continue;
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

			if (use_fd[index].revents & POLLWRNORM) //If data can be write without blocking
			{
				PNet::PacketManager & pm = connection.pm_outgoing;
				while (pm.HasPendingPackets())
				{
					if (pm.currentTask == PNet::PacketManagerTask::ProcessPacketSize)
					{
						pm.currentPacketSize = uint16_t(pm.Retrieve()->buffer.size());
						uint16_t bigEndianPacketSize = htons(pm.currentPacketSize);

						int bytesSent = send(use_fd[index].fd, (char*)(&bigEndianPacketSize) + pm.currentPacketExtractionOffset, sizeof(uint16_t) - pm.currentPacketExtractionOffset, NULL);

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

						int bytesSent = send(use_fd[index].fd, (char*)(bufferPtr)+pm.currentPacketExtractionOffset, pm.currentPacketSize - pm.currentPacketExtractionOffset, NULL);

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
	}

	for (int index = connections.size() - 1; index >= 0; index--)
	{
		while (connections[index].pm_incoming.HasPendingPackets())
		{
			std::shared_ptr<PNet::Packet> frontPacket = connections[index].pm_incoming.Retrieve();

			if (!ProceesPacket(frontPacket))
			{
				CloseConnection(index, "Failed to process incoming packet.");

				break;
			}

			connections[index].pm_incoming.Pop();
		}
	}
}

void Server::OnConnect(PNet::TCPConnection & newConnection)
{
	std::cout << newConnection.ToString() << " - New connection accepted." << std::endl;

}

void Server::OnDisconnect(PNet::TCPConnection & lostConnection, std::string reason)
{
	std::cout << "[" << reason << "] Connection lost:" << lostConnection.ToString() << "." << std::endl;
}

void Server::CloseConnection(int connectionIndex, std::string reason)
{
	PNet::TCPConnection& connection = connections[connectionIndex];

	OnDisconnect(connection, reason);

	master_fd.erase(master_fd.begin() + (connectionIndex + 1));
	use_fd.erase(use_fd.begin() + (connectionIndex + 1));
	connection.Close();
	connections.erase(connections.begin() + connectionIndex);
}

bool Server::ProceesPacket(std::shared_ptr<PNet::Packet> packet)
{
	/*
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
	*/

	std::cout << "Packet recieved with size:" << packet->buffer.size() << std::endl;

	return true;
}
