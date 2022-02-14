#pragma once

namespace PNet
{
	enum SocketOption
	{
		TCP_NoDelay, // TRUE = Disable Nagle's algorithm
		IPv6_Only, // TRUE = Disable IPv6 backward compatibility
	};
}