#pragma once

#include <PNet/IncludeMe.h>
#include <iostream>

class MyServer : public Server
{
private:
	void OnConnect(PNet::TCPConnection& newConnection) override;
	void OnDisconnect(PNet::TCPConnection & lostConnection, std::string reason) override;
	bool ProceesPacket(std::shared_ptr<PNet::Packet> packet) override;
};