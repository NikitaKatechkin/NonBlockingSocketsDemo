#pragma once

#include "TCPConnection.h"
#include "Network.h"

#include <iostream>

class Server
{
public:
	bool Initialize(PNet::IPEndpoint ip);
	void Frame();
protected:
	virtual void OnConnect(PNet::TCPConnection& newConnection);
	virtual void OnDisconnect(PNet::TCPConnection & lostConnection, std::string reason);
	virtual bool ProceesPacket(std::shared_ptr<PNet::Packet> packet);

	void CloseConnection(int connectionIndex, std::string reason);

	PNet::Socket listeninigSocket;
	std::vector<PNet::TCPConnection> connections;
	std::vector<WSAPOLLFD> master_fd;
	std::vector<WSAPOLLFD> use_fd;
};