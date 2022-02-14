#pragma once

#define WIN32_LEAN_AND_MEAN

#include <winsock.h>
#include <vector>

#include "Constants.h"
#include "PacketException.h"
#include "PacketType.h"

namespace PNet
{
	class Packet
	{
	public:
		Packet(PacketType packetType = PacketType::PT_Invalid);

		PacketType GetPacketType();
		void AssignPacketType(PacketType packetType);

		void Clear();
		void Append(const void* data, uint32_t size);

		//Put in
		Packet& operator << (uint32_t data);
		//Get from
		Packet& operator >> (uint32_t& data);

		//Put in
		Packet& operator << (const std::string& data);
		//Get from
		Packet& operator >> (std::string& data);

		uint32_t extractionOffset = 0;
		std::vector<char> buffer;
	private:
	};
}