#pragma once

#include <PNet/IncludeMe.h>

class MyClient : public Client
{
	bool ProcessPacket(std::shared_ptr<PNet::Packet> packet) override;
	void OnConnect() override;
	//void OnConnectFail();
	//void OnDisconnect(std::string reason);
};