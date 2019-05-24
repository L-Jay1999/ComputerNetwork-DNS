#include <random>
#include <cstring>

#include <Winsock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#include "my_socket.h"
#include "dns_packet.h"
#include "log.h"

static const char *kDefaultDNSPort = "53";
static const std::string kSuperiorDNSServerAddr = "10.3.9.4";

static int get_send_port_random();

MySocket::MySocket(SocketType sock_type, const std::string& address) : sock_type_(sock_type)
{
	if (InitSock(sock_type_, kDefaultDNSPort, kSuperiorDNSServerAddr) == ERROR_SUCCESS)
	{
		init_success_ = true;
		// log write
	}
	else
	{
		init_success_ = false;
		// log write
	}
}

MySocket::MySocket(SocketType sock_type, const char *port, const std::string &superior_dns) : sock_type_(sock_type)
{
	if (InitSock(sock_type_, port, superior_dns) == ERROR_SUCCESS)
	{
		init_success_ = true;
		// log write
	}
	else
	{
		init_success_ = false;
		// log write
	}
}

MySocket::~MySocket()
{
	if (sock_ != INVALID_SOCKET)
	{
		last_error_ = closesocket(sock_);
		if (last_error_ == SOCKET_ERROR)
		{
			last_error_ = WSAGetLastError();
			// log write
		}
	}
	if (WSACleanup())
	{
		last_error_ = WSAGetLastError();
		// log write
	}
	else
	{
		// log write
	}
}

DWORD MySocket::InitSock(SocketType soc_type, const char *port, const std::string &superior_dns)
{
	WSADATA wsaData;
	int last_error_;

	last_error_ = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (last_error_)
	{
		// log write

		return last_error_;
	}

	addrinfo hint, *result = nullptr;

	ZeroMemory(&hint, sizeof(hint));
	hint.ai_family = AF_INET;
	hint.ai_socktype = SOCK_DGRAM;
	hint.ai_protocol = IPPROTO_UDP;
	hint.ai_flags = AI_PASSIVE;

	if (soc_type == QUEST_SOCKET)
	{
		last_error_ = getaddrinfo(NULL, port, &hint, &result);
	}
	else
	{
		last_error_ = getaddrinfo("localhost", port, &hint, &result);
	}

	if (last_error_)
	{
		// log write
		WSACleanup();
		return last_error_;
	}

	sock_ = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (sock_ == INVALID_SOCKET)
	{
		last_error_ = WSAGetLastError();
		freeaddrinfo(result);
		WSACleanup();
		// log write
		return last_error_;
	}

	DWORD is_reuseaddr = FALSE;
	if (setsockopt(sock_, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, reinterpret_cast<const char *>(&is_reuseaddr), sizeof(is_reuseaddr) == SOCKET_ERROR))
	{
		last_error_ = WSAGetLastError();
		closesocket(sock_);
		freeaddrinfo(result);
		WSACleanup();
		// log write
		return last_error_;
	}

	is_reuseaddr = TRUE;
	if (setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&is_reuseaddr), sizeof(is_reuseaddr) == SOCKET_ERROR))
	{
		last_error_ = WSAGetLastError();
		closesocket(sock_);
		freeaddrinfo(result);
		WSACleanup();
		// log write
		return last_error_;
	}

	last_error_ = bind(sock_, result->ai_addr, static_cast<int>(result->ai_addrlen));
	if (last_error_ == SOCKET_ERROR)
	{
		// log write
		last_error_ = WSAGetLastError();
		freeaddrinfo(result);
		WSACleanup();
		return last_error_;
	}

	my_addr_info_size_ = sizeof(my_addr_info_);
	last_error_ = getsockname(sock_, reinterpret_cast<sockaddr *>(&my_addr_info_), &my_addr_info_size_);
	if (last_error_)
	{
		freeaddrinfo(result);
		WSACleanup();
		return last_error_;
	}

	if (soc_type == RECV_SOCKET)
	{
		freeaddrinfo(result);
	}
	else
	{
		max_msg_size_len_ = sizeof(max_msg_size_);
		last_error_ = getsockopt(sock_, SOL_SOCKET, SO_MAX_MSG_SIZE, reinterpret_cast<char *>(&max_msg_size_), reinterpret_cast<int *>(&max_msg_size_len_));
		if (last_error_ == SOCKET_ERROR)
		{
			// log write
			freeaddrinfo(result);
			WSACleanup();
			return last_error_;
		}

		freeaddrinfo(result);
		last_error_ = getaddrinfo(superior_dns.c_str(), kDefaultDNSPort, &hint, &result);
		if (last_error_)
		{
			WSACleanup();
			return last_error_;
		}

		sockaddr_in *temp = reinterpret_cast<sockaddr_in *>(result->ai_addr);
		superior_server_addr_ = *temp;

		freeaddrinfo(result);
	}
	return ERROR_SUCCESS;
}

DWORD MySocket::_RecvFrom(QueueData &queue_data)
{
	while (1)
	{
		recv_len_ = recvfrom(sock_, recvbuf_, recvbuflen_, 0, reinterpret_cast<sockaddr *>(&from_), &from_len_);
		if (recv_len_ == 0)
		{
			// log write : packet contains no data
			continue;
		}
		else if (recv_len_ < 0)
		{
			last_error_ = WSAGetLastError();
			// log write
			// error handle
		}
		else
		{
			queue_data.addr = from_;
			queue_data.len = recv_len_;
			std::memcpy(queue_data.data, recvbuf_, recv_len_);
			break;
		}
	}
	return ERROR_SUCCESS;
}

DWORD MySocket::_SendTo(const QueueData &queue_data)
{
	last_error_ = sendto(sock_, queue_data.data, queue_data.len, 0, reinterpret_cast<const sockaddr *>(&queue_data.addr), sizeof(queue_data.addr));
	if (last_error_ == SOCKET_ERROR)
	{
		last_error_ = WSAGetLastError();
		// log write
		// error handling
		return last_error_;
	}
	return ERROR_SUCCESS;
}


// 从指定端口接收数据,返回一个QueueData(需要自行管理内存)

QueueData MySocket::RecvFrom()
{
	if (sock_type_ == RECV_SOCKET || sock_type_ == QUEST_SOCKET)
	{
		QueueData res = QueueData();
		last_error_ = _RecvFrom(res);
		if (last_error_ != ERROR_SUCCESS)
		{
			res = QueueData();
		}

		return res;
	}
	else
	{
		// log write
		return QueueData();
	}
}

bool MySocket::SendTo(const QueueData &queue_data)
{
	if (sock_type_ == SEND_SOCKET || sock_type_ == QUEST_SOCKET)
	{
		last_error_ = _SendTo(queue_data);

		if (last_error_ != ERROR_SUCCESS)
		{
			// log write
			return false;
		}

		return true;
	}
	else
	{
		// log write
		return false;
	}
}

