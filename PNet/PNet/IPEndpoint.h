#pragma once

#define WIND32_LEAN_AND_MEAN

#include "IPVersion.h"
#include "Helpers.h"

#include <WS2tcpip.h>
#include <vector>
#include <string>
#include <cassert>
#include <iostream>

namespace PNet
{
	class IPEndpoint
	{
	public:
		IPEndpoint();
		IPEndpoint(const char* ip, unsigned short port);
		IPEndpoint(sockaddr * addr);

		IPVersion GetIPVersion();
		std::string GetHostname();
		std::string GetIPString();
		std::vector<uint8_t> GetIPBytes();
		unsigned short GetPort();

		sockaddr_in GetSockaddrIPv4();
		sockaddr_in6 GetSockaddrIPv6();

		void Print();
	private:
		IPVersion ip_version = IPVersion::Unknown;
		std::string hostname = "";
		std::string ip_string = "";
		std::vector<uint8_t> ip_bytes;
		unsigned short port = 0;
	};
}