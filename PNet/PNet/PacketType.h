#pragma once
#include <cstdint>

namespace PNet
{
	enum PacketType : uint16_t
	{
		PT_Invalid,
		PT_ChatMessage,
		PT_IntegerArray,
		PT_Test
	};
}