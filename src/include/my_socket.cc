#include <random>
#include <cstring>

#include <Winsock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#include "my_socket.h"
#include "dns_packet.h"
#include "log.h"

static const char *kDefaultRecvPort = "53";

static int get_send_port_random();

enum SocketType
{
	SEND_SOCKET = 0,
	RECV_SOCKET,
};

inline MySocket::MySocket(SocketType sock_type) : sock_type_(sock_type)
{
	if (InitSock(soc_type_) == ERROR_SUCCESS)
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
		if (last_error_)
		{
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

DWORD MySocket::InitSock(SocketType soc_type)
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

	last_error_ = getaddrinfo(NULL, kDefaultRecvPort, &hint, &result);
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

	if (soc_type == RECV_SOCKET)
	{
		last_error_ = bind(sock_, result->ai_addr, static_cast<int>(result->ai_addrlen));
		if (last_error_ == SOCKET_ERROR)
		{
			// log write
			freeaddrinfo(result);
			WSACleanup();
			return last_error_;
		}

		last_error_ = getsockname(sock_, reinterpret_cast<sockaddr *>(&my_addr_info_), reinterpret_cast<int *>(sizeof(my_addr_info_)));
		if (last_error_)
		{
			freeaddrinfo(result);
			WSACleanup();
			return last_error_;
		}

		freeaddrinfo(result);
	}
	else
	{
		last_error_ = getsockopt(sock_, SOL_SOCKET, SO_MAX_MSG_SIZE, reinterpret_cast<char *>(&max_msg_size_), reinterpret_cast<int *>(sizeof(max_msg_size_)));
		if (last_error_ == SOCKET_ERROR)
		{
			// log write
			WSACleanup();
			return last_error_;
		}
	}
	return ERROR_SUCCESS;
}

DWORD MySocket::_RecvFrom(QueueData &queue_data)
{
	while (1)
	{
		recv_len_ = recvfrom(sock_, recvbuf_, recvbuflen_, 0, reinterpret_cast<sockaddr *>(&from_), reinterpret_cast<int *>(sizeof(from_)));
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
			queue_data.data = new char[recv_len_]();
			if (queue_data.data == nullptr)
			{
				// log write cannot allocate memory for queue_data
			}
			else
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
	if (sock_type_ == RECV_SOCKET)
	{
		QueueData res;
		last_error_ = _RecvFrom(res);
		if (last_error_ != ERROR_SUCCESS)
		{
			// log write
			if (res.data)
				delete res.data;
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
	if (sock_type_ == SEND_SOCKET)
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

int get_send_port_random()
{
	static constexpr int kSendPortLowerBound = 10000;
	static constexpr int kSendPortUpperBound = 20000;

	static std::random_device rd;
	static std::mt19937 g(rd);
	static std::uniform_int_distribution<int> uid(kSendPortLowerBound, kSendPortUpperBound);

	return uid(g);
}
