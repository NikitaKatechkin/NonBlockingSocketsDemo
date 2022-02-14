#pragma once

#include "TCPConnection.h"
#include "Network.h"

#include <iostream>

class Client
{
public:
	bool Connect(PNet::IPEndpoint ip);

	bool IsConnected();
	bool Frame();
protected:
	virtual bool ProcessPacket(std::shared_ptr<PNet::Packet> packet);
	virtual void OnConnect();
	virtual void OnConnectFail();
	virtual void OnDisconnect(std::string reason);

	void CloseConnection(std::string reason);

	PNet::TCPConnection connection;
private:
	bool isConnected = false;

	WSAPOLLFD master_fd;
	WSAPOLLFD use_fd;
};