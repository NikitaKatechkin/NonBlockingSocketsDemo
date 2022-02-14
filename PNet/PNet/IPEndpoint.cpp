#include "IPEndpoint.h"

namespace PNet
{
	IPEndpoint::IPEndpoint()
	{
	}

	IPEndpoint::IPEndpoint(const char * ip, unsigned short port)
	{
		this->port = port;
		
		//Ipv4
		in_addr addr; //location so store the IPv4 internet address
		int result = inet_pton(AF_INET, ip, &addr);

		if (result == 1)
		{
			if (addr.S_un.S_addr != INADDR_NONE)
			{
				ip_string = ip;
				hostname = ip;
				Helpers::trim(ip_string);
				Helpers::trim(hostname);

				ip_bytes.resize(sizeof(ULONG));
				memcpy(&ip_bytes[0], &addr.S_un.S_addr, sizeof(ULONG));

				ip_version = IPVersion::IPv4;
				return;
			}
		}

		//Attempt to resolve hostname to IPv4 address
		addrinfo hints = {}; //hints will filter the results from getaddrinfo()
		hints.ai_family = AF_INET; //IPv4 addresses only
		addrinfo * hostinfo = nullptr;
		result = getaddrinfo(ip, NULL, &hints, &hostinfo); //Get address from a hostname
		if (result == 0)
		{
			sockaddr_in * host_addr = reinterpret_cast<sockaddr_in*>(hostinfo->ai_addr);

			//host_addr->sin_addr.S_un.S_addr
			ip_string.resize(16);
			inet_ntop(AF_INET, &host_addr->sin_addr, &ip_string[0], 16);
			hostname = ip;

			Helpers::trim(ip_string);
			Helpers::trim(hostname);

			ULONG ip_long = host_addr->sin_addr.S_un.S_addr;
			ip_bytes.resize(sizeof(ULONG));
			memcpy(&ip_bytes[0], &ip_long, sizeof(ULONG));

			ip_version = IPVersion::IPv4;

			freeaddrinfo(hostinfo); //memory cleanuo after getaddrinfo call
			return;
		}

		//Ipv6
		in6_addr addr6; //location so store the IPv4 internet address
		result = inet_pton(AF_INET6, ip, &addr6);

		if (result == 1)
		{
			ip_string = ip;
			hostname = ip;

			Helpers::trim(ip_string);
			Helpers::trim(hostname);

			ip_bytes.resize(16);
			memcpy(&ip_bytes[0], &addr6.u, 16);

			ip_version = IPVersion::IPv6;
			return;
		}

		//Attempt to resolve hostname to IPv4 address
		addrinfo hintsv6 = {}; //hints will filter the results from getaddrinfo()
		hintsv6.ai_family = AF_INET6; //IPv4 addresses only
		addrinfo * hostinfov6 = nullptr;
		result = getaddrinfo(ip, NULL, &hintsv6, &hostinfov6); //Get address from a hostname
		if (result == 0)
		{
			sockaddr_in6 * host_addr = reinterpret_cast<sockaddr_in6*>(hostinfov6->ai_addr);

			//host_addr->sin_addr.S_un.S_addr
			ip_string.resize(46);
			inet_ntop(AF_INET6, &host_addr->sin6_addr, &ip_string[0], 46);
			hostname = ip;

			Helpers::trim(ip_string);
			Helpers::trim(hostname);

			ip_bytes.resize(16);
			memcpy(&ip_bytes[0], &host_addr->sin6_addr, 16);

			ip_version = IPVersion::IPv6;

			freeaddrinfo(hostinfov6); //memory cleanuo after getaddrinfo call
			return;
		}
	}

	IPEndpoint::IPEndpoint(sockaddr * addr)
	{
		assert(addr->sa_family == AF_INET || addr->sa_family == AF_INET6);

		if (addr->sa_family == AF_INET)
		{
			sockaddr_in * addrv4 = reinterpret_cast<sockaddr_in*>(addr);
			ip_version = IPVersion::IPv4;
			port = ntohs(addrv4->sin_port);

			ip_bytes.resize(sizeof(ULONG));
			memcpy(&ip_bytes[0], &addrv4->sin_addr, sizeof(ULONG));

			ip_string.resize(16);
			inet_ntop(AF_INET, &addrv4->sin_addr, &ip_string[0], 16);

			hostname = ip_string;
		}
		else if (addr->sa_family == AF_INET6)
		{
			sockaddr_in6 * addrv6 = reinterpret_cast<sockaddr_in6*>(addr);
			ip_version = IPVersion::IPv6;
			port = ntohs(addrv6->sin6_port);

			ip_bytes.resize(16);
			memcpy(&ip_bytes[0], &addrv6->sin6_addr, 16);

			ip_string.resize(46);
			inet_ntop(AF_INET6, &addrv6->sin6_addr, &ip_string[0], 46);

			hostname = ip_string;
		}

		Helpers::trim(ip_string);
		Helpers::trim(hostname);
	}

	IPVersion IPEndpoint::GetIPVersion()
	{
		return ip_version;
	}

	std::string IPEndpoint::GetHostname()
	{
		return hostname;
	}

	std::string IPEndpoint::GetIPString()
	{
		return ip_string;
	}

	std::vector<uint8_t> IPEndpoint::GetIPBytes()
	{
		return ip_bytes;
	}

	unsigned short IPEndpoint::GetPort()
	{
		return port;
	}

	sockaddr_in IPEndpoint::GetSockaddrIPv4()
	{
		assert(ip_version == IPVersion::IPv4);

		sockaddr_in addr = {};
		addr.sin_family = AF_INET;
		memcpy(&addr.sin_addr, &ip_bytes[0], sizeof(ULONG));
		addr.sin_port = htons(port);
		
		return addr;
	}
	sockaddr_in6 IPEndpoint::GetSockaddrIPv6()
	{
		assert(ip_version == IPVersion::IPv6);

		sockaddr_in6 addr6 = {};
		addr6.sin6_family = AF_INET6;
		memcpy(&addr6.sin6_addr, &ip_bytes[0], 16);
		addr6.sin6_port = htons(port);

		return addr6;
	}
	void IPEndpoint::Print()
	{
		switch (ip_version)
		{
		case IPVersion::IPv4:
			std::cout << "IP Version: IPv4" << std::endl;
			break;
		case IPVersion::IPv6:
			std::cout << "IP Version: IPv6" << std::endl;
			break;
		default:
			std::cout << "IP Version: Unknown" << std::endl;
			break;
		}

		std::cout << "Hostname: " << hostname << std::endl;
		std::cout << "IP: " << ip_string << std::endl;
		std::cout << "Port: " << port << std::endl;
		std::cout << "IP Bytes..." << std::endl;
		for (auto & digit : ip_bytes)
		{
			std::cout << (int)digit << std::endl;
		}
	}
}


