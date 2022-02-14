#include "Socket.h"

namespace PNet
{
	Socket::Socket(IPVersion ip_version, SocketHandle handle)
		: ip_version(ip_version), handle(handle)
	{
		assert(ip_version == IPVersion::IPv4 || ip_version == IPVersion::IPv6);
	}

	PResult PNet::Socket::Create()
	{
		assert(ip_version == IPVersion::IPv4 || ip_version == IPVersion::IPv6);

		if (handle != INVALID_SOCKET)
		{
			return PResult::P_GenericError;
		}

		handle = socket((ip_version == IPVersion::IPv4) ? AF_INET : AF_INET6, 
						SOCK_STREAM, IPPROTO_TCP);
		if (handle == INVALID_SOCKET)
		{
			int error = WSAGetLastError();
			return PResult::P_GenericError;
		}

		if (SetBlocking(false) != PResult::P_Success)
		{
			return PResult::P_GenericError;
		}

		if (SetSocketOption(SocketOption::TCP_NoDelay, TRUE) != PResult::P_Success)
		{
			return PResult::P_GenericError;
		}

		return PResult::P_Success;
	}

	PResult Socket::Close()
	{
		if (handle == INVALID_SOCKET)
		{
			return PResult::P_GenericError;
		}

		int result = closesocket(handle);
		if (result != 0)
		{
			int error = WSAGetLastError();
			return PResult::P_GenericError;
		}

		handle = INVALID_SOCKET;
		return PResult::P_Success;
	}

	PResult Socket::Bind(IPEndpoint endpoint)
	{
		assert(ip_version == endpoint.GetIPVersion());

		if (ip_version == IPVersion::IPv4)
		{
			sockaddr_in addr = endpoint.GetSockaddrIPv4();
			int result = bind(handle, (sockaddr *)(&addr), sizeof(sockaddr_in));

			if (result != 0)
			{
				int error = WSAGetLastError();
				return PResult::P_GenericError;
			}
		}
		else if (ip_version == IPVersion::IPv6)
		{
			sockaddr_in6 addr6 = endpoint.GetSockaddrIPv6();
			int result = bind(handle, (sockaddr *)(&addr6), sizeof(sockaddr_in6));

			if (result != 0)
			{
				int error = WSAGetLastError();
				return PResult::P_GenericError;
			}
		}

		

		return PResult::P_Success;
	}

	PResult Socket::Listen(IPEndpoint endpoint, int backlog)
	{
		if (ip_version == IPVersion::IPv6)
		{
			if (SetSocketOption(SocketOption::IPv6_Only, FALSE) != PResult::P_Success)
			{
				return PResult::P_GenericError;
			}
		}

		if (Bind(endpoint) != PResult::P_Success)
		{
			return PResult::P_GenericError;
		}

		int result = listen(handle, backlog);
		if (result != 0)
		{
			int error = WSAGetLastError();
			return PResult::P_GenericError;
		}

		return PResult::P_Success;
	}

	PResult Socket::Accept(Socket & outSocket, IPEndpoint * endpoint)
	{
		assert(ip_version == IPVersion::IPv4 || ip_version == IPVersion::IPv6);

		if (ip_version == IPVersion::IPv4)
		{
			sockaddr_in addr = {};
			int addr_len = sizeof(sockaddr_in);
			SocketHandle acceptedConnectionHandle = accept(handle, (sockaddr *)(&addr), &addr_len);

			if (acceptedConnectionHandle == INVALID_SOCKET)
			{
				int error = WSAGetLastError();
				return PResult::P_GenericError;
			}

			if (endpoint != nullptr)
			{
				*endpoint = IPEndpoint((sockaddr*)(&addr));
			}

			outSocket = Socket(IPVersion::IPv4, acceptedConnectionHandle);
		}
		else if (ip_version == IPVersion::IPv6)
		{
			sockaddr_in6 addr = {};
			int addr_len = sizeof(sockaddr_in6);
			SocketHandle acceptedConnectionHandle = accept(handle, (sockaddr *)(&addr), &addr_len);

			if (acceptedConnectionHandle == INVALID_SOCKET)
			{
				int error = WSAGetLastError();
				return PResult::P_GenericError;
			}

			if (endpoint != nullptr)
			{
				*endpoint = IPEndpoint((sockaddr*)(&addr));
			}

			outSocket = Socket(IPVersion::IPv6, acceptedConnectionHandle);
		}

		return PResult::P_Success;
	}

	PResult Socket::Connect(IPEndpoint endpoint)
	{
		assert(ip_version = endpoint.GetIPVersion());

		if (ip_version == IPVersion::IPv4)
		{
			sockaddr_in addr = endpoint.GetSockaddrIPv4();
			int result = connect(handle, (sockaddr *)(&addr), sizeof(sockaddr_in));
			if (result != 0)
			{
				int error = WSAGetLastError();
				return PResult::P_GenericError;
			}
		}
		else
		{
			sockaddr_in6 addr = endpoint.GetSockaddrIPv6();
			int result = connect(handle, (sockaddr *)(&addr), sizeof(sockaddr_in6));
			if (result != 0)
			{
				int error = WSAGetLastError();
				return PResult::P_GenericError;
			}
		}

		return PResult::P_Success;
	}

	PResult Socket::Send(const void * data, int numberOfBytes, int & bytesSent)
	{
		bytesSent = send(handle, (const char *)data, numberOfBytes, NULL);

		if (bytesSent == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			return PResult::P_GenericError;
		}

		return PResult::P_Success;
	}

	PResult Socket::Recv(void * destination, int numberOfBytes, int & bytesReceived)
	{
		bytesReceived = recv(handle, (char *)destination, numberOfBytes, NULL);

		if (bytesReceived == 0)
		{
			return PResult::P_GenericError;
		}
		if (bytesReceived == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			return PResult::P_GenericError;
		}

		return PResult::P_Success;
	}

	PResult Socket::SendAll(const void * data, int numberOfBytes)
	{
		int totalBytesSent = 0;
		while (totalBytesSent < numberOfBytes)
		{
			int bytesRemaining = numberOfBytes - totalBytesSent;
			int bytesSent = 0;
			char * bufferOffset = (char*)data + totalBytesSent;
			PResult result = Send(bufferOffset, bytesRemaining, bytesSent);
			if (result != PResult::P_Success)
			{
				return PResult::P_GenericError;
			}

			totalBytesSent += bytesSent;
		}

		return PResult::P_Success;
	}

	PResult Socket::RecvAll(void * destination, int numberOfBytes)
	{
		int totalBytesRecieved = 0;
		while (totalBytesRecieved < numberOfBytes)
		{
			int bytesRemaining = numberOfBytes - totalBytesRecieved;
			int bytesRecieved = 0;
			char * bufferOffset = (char*)destination + totalBytesRecieved;
			PResult result = Recv(bufferOffset, bytesRemaining, bytesRecieved);
			if (result != PResult::P_Success)
			{
				return PResult::P_GenericError;
			}

			totalBytesRecieved += bytesRecieved;
		}

		return PResult::P_Success;
	}

	PResult Socket::Send(Packet & packet)
	{
		uint16_t encodedPacketSize = htons(packet.buffer.size());
		PResult result = SendAll(&encodedPacketSize, sizeof(uint16_t));
		if (result != PResult::P_Success)
		{
			return PResult::P_GenericError;
		}

		result = SendAll(packet.buffer.data(), packet.buffer.size());
		if (result != PResult::P_Success)
		{
			return PResult::P_GenericError;
		}

		return PResult::P_Success;
	}

	PResult Socket::Recv(Packet & packet)
	{
		packet.Clear();

		uint16_t encodedSize = 0;
		PResult result = RecvAll(&encodedSize, sizeof(uint16_t));
		if (result != PResult::P_Success)
		{
			return PResult::P_GenericError;
		}

		uint16_t bufferSize = ntohs(encodedSize);
		if (bufferSize > g_MaxPAcketSize)
		{
			return PResult::P_GenericError;
		}

		packet.buffer.resize(bufferSize);
		result = RecvAll(&packet.buffer[0], bufferSize);
		if (result != PResult::P_Success)
		{
			return PResult::P_GenericError;
		}

		return PResult::P_Success;
	}

	SocketHandle Socket::GetHandle()
	{
		return handle;
	}

	IPVersion Socket::GetIPVersion()
	{
		return ip_version;
	}

	PResult Socket::SetBlocking(bool isBlocking)
	{
		unsigned long nonBlocking = 1;
		unsigned long blocking = 0;
		int result = ioctlsocket(handle, FIONBIO, isBlocking ? &blocking : &nonBlocking);
		if (result == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			return PResult::P_GenericError;
		}

		return PResult::P_Success;
	}

	PResult Socket::SetSocketOption(SocketOption option, BOOL value)
	{
		int result;
		switch (option)
		{
		case SocketOption::TCP_NoDelay:
			result = setsockopt(handle, IPPROTO_TCP, TCP_NODELAY, (const char*)&value, sizeof(value));
			break;
		case SocketOption::IPv6_Only:
			result = setsockopt(handle, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&value, sizeof(value));
			break;
		default:
			return PResult::P_GenericError;
		}

		if (result != 0)
		{
			int error = WSAGetLastError();
			return PResult::P_GenericError;
		}

		return PResult::P_Success;
	}

}

