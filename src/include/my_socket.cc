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

MySocket::MySocket(SocketType sock_type) : sock_type_(sock_type)
{
	if (InitSock(sock_type_, kDefaultDNSPort, kSuperiorDNSServerAddr) == ERROR_SUCCESS)
	{
		init_success_ = true;
		Log::WriteLog(2, __s("MySocket InitSocket success, Port: ") + kDefaultDNSPort + __s(" su_dns addr: ") + kSuperiorDNSServerAddr);
	}
	else
	{
		init_success_ = false;
		Log::WriteLog(2, __s("MySocket InitSocket failed"));
	}
}

MySocket::MySocket(SocketType sock_type, const char *port, const std::string &superior_dns) : sock_type_(sock_type)
{
	if (InitSock(sock_type_, port, superior_dns) == ERROR_SUCCESS)
	{
		init_success_ = true;
		Log::WriteLog(2, __s("MySocket InitSocket success, Port: ") + port + __s(" su_dns addr: ") + superior_dns);
	}
	else
	{
		init_success_ = false;
		Log::WriteLog(2, __s("MySocket InitSocket failed"));
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
			Log::WriteLog(2, __s("MySocket ~ close socket failed, ErrorCode: ") + std::to_string(last_error_));
		}
	}
	if (WSACleanup())
	{
		last_error_ = WSAGetLastError();
		Log::WriteLog(2, __s("MySocket ~ WSACleanup failed, ErrorCode: ") + std::to_string(last_error_));
	}
}

DWORD MySocket::InitSock(SocketType soc_type, const char *port, const std::string &superior_dns)
{
	WSADATA wsaData;

	last_error_ = WSAStartup(MAKEWORD(2, 2), &wsaData);//连接应用程序与winsock
	if (last_error_)//初始化失败，输出错误信息并退出
	{
		Log::WriteLog(2, __s("MySocket InitSock WSAStartup failed, ErrorCode: ") + std::to_string(last_error_));
		return last_error_;
	}

	//建立并配置addrinfo结构用于getaddrinfo函数
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

	if (last_error_)//result指针操作失败时，返回错误信息
	{
		Log::WriteLog(2, __s("MySocket InitSock getaddrinfo failed, ErrorCode: ") + std::to_string(last_error_));
		WSACleanup();
		return last_error_;
	}

	sock_ = socket(result->ai_family, result->ai_socktype, result->ai_protocol);//建立一个监听客户端连接的socket
	if (sock_ == INVALID_SOCKET)
	{
		last_error_ = WSAGetLastError();
		Log::WriteLog(2, __s("MySocket InitSock socket failed, ErrorCode: ") + std::to_string(last_error_));
		freeaddrinfo(result);
		WSACleanup();
		return last_error_;
	}

	DWORD is_reuseaddr = FALSE;
	//防止其他socket被强制绑定到相同的地址和端口
	if (setsockopt(sock_, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, reinterpret_cast<const char *>(&is_reuseaddr), sizeof(is_reuseaddr) == SOCKET_ERROR))
	{
		last_error_ = WSAGetLastError();
		Log::WriteLog(2, __s("MySocket InitSock set SO_EXCLUSIVEADDRUSE to false failed, ErrorCode: ") + std::to_string(last_error_));
		closesocket(sock_);
		freeaddrinfo(result);
		WSACleanup();
		return last_error_;
	}

	//允许将socket绑定到已在使用的地址
	is_reuseaddr = TRUE;
	if (setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&is_reuseaddr), sizeof(is_reuseaddr) == SOCKET_ERROR))
	{
		last_error_ = WSAGetLastError();
		Log::WriteLog(2, __s("MySocket InitSock set SO_REUSEADDR to true failed, ErrorCode: ") + std::to_string(last_error_));
		closesocket(sock_);
		freeaddrinfo(result);
		WSACleanup();
		return last_error_;
	}

	//利用bind检查创建的socket和getaddrinfo函数返回的sockaddr结构是否有误
	last_error_ = bind(sock_, result->ai_addr, static_cast<int>(result->ai_addrlen));
	if (last_error_ == SOCKET_ERROR)
	{
		last_error_ = WSAGetLastError();
		Log::WriteLog(2, __s("MySocket InitSock bind socket failed, ErrorCode: ") + std::to_string(last_error_));
		freeaddrinfo(result);
		WSACleanup();
		return last_error_;
	}

	my_addr_info_size_ = sizeof(my_addr_info_);
	//检索sock_的本地名称，确定与本地关联
	last_error_ = getsockname(sock_, reinterpret_cast<sockaddr *>(&my_addr_info_), &my_addr_info_size_);
	if (last_error_)
	{
		Log::WriteLog(2, __s("MySocket InitSock getsockname failed, ErrorCode: ") + std::to_string(last_error_));
		freeaddrinfo(result);
		WSACleanup();
		return last_error_;
	}

	if (soc_type == RECV_SOCKET)//如果是接收socket不作操作
	{
		freeaddrinfo(result);
	}
	else
	{
		max_msg_size_len_ = sizeof(max_msg_size_);
		//检索socket当前值是否超过最大信息大小
		last_error_ = getsockopt(sock_, SOL_SOCKET, SO_MAX_MSG_SIZE, reinterpret_cast<char *>(&max_msg_size_), reinterpret_cast<int *>(&max_msg_size_len_));
		if (last_error_ == SOCKET_ERROR)
		{
			Log::WriteLog(2, __s("MySocket InitSock get max msg size failed, ErrorCode: ") + std::to_string(last_error_));
			freeaddrinfo(result);
			WSACleanup();
			return last_error_;
		}

		freeaddrinfo(result);
		last_error_ = getaddrinfo(superior_dns.c_str(), kDefaultDNSPort, &hint, &result);//提供从主机名到地址的独立于协议的转换,将result修改为addrinfo结构的链接列表
		if (last_error_)
		{
			Log::WriteLog(2, __s("MySocket InitSock get superior addr info failed, ErrorCode: ") + std::to_string(last_error_));
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
		recv_len_ = recvfrom(sock_, recvbuf_, recvbuflen_, 0, reinterpret_cast<sockaddr *>(&from_), &from_len_);//接收上级返回的数据报并将数据存入recvbuf_，将原地址存入from_
		if (recv_len_ == 0)//连接正常关闭
		{
			Log::WriteLog(2, __s("MySocket recvfrom connectiong is closed gracefully"));
			continue;
		}
		else if (recv_len_ < 0)//连接出错
		{
			last_error_ = WSAGetLastError();
			if (last_error_ == WSAETIMEDOUT)//因为超时而出错
			{
				Log::WriteLog(2, __s("MySocket recvfrom failed: timeout"));
				return WSAETIMEDOUT;
			}
			else if (last_error_ == WSAECONNRESET)//当前socket不可用
			{
				Log::WriteLog(2, __s("MySocket recvfrom failed: previous send destination unreachable, use public socket instead of local socket"));
				// continue;
			}
			else
			{
				Log::WriteLog(2, __s("MySocket recvfrom failed: ErrorCode:") + std::to_string(last_error_));
			}
			// error handle
		}
		else//接收到了数据，recv_len_为接收字节数
		{
			Log::WriteLog(2, __s("MySocket recvfrom success: receive(byte): ") + std::to_string(recv_len_));
			queue_data.addr = from_;
			queue_data.len = recv_len_;
			std::memcpy(queue_data.data, recvbuf_, recv_len_);//将回答信息写入queue_data.data
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
		Log::WriteLog(2, __s("MySocket sendto failed: ErrorCode: ") + std::to_string(last_error_));
		// error handling
		return last_error_;
	}
	Log::WriteLog(2, __s("MySocket sendto success: send size: ") + std::to_string(last_error_));
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
		Log::WriteLog(2, __s("MySocket RecvFrom failed: invalid socket type"));
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
			return false;
		}

		return true;
	}
	else
	{
		Log::WriteLog(2, __s("MySocket SendTo failed: invalid socket type"));
		return false;
	}
}

bool MySocket::set_recv_timeout(const int ms)
{
	if (sock_type_ == QUEST_SOCKET)
	{
		const DWORD time_out = ms;

		last_error_ = setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char *>(&time_out), sizeof(time_out));//设置超时时间
		if (last_error_ == SOCKET_ERROR)
		{
			last_error_ = WSAGetLastError();
			Log::WriteLog(2, __s("MySocket set recv timeout failed: ErrorCode:") + std::to_string(last_error_));
			return false;
		}
		else
		{
			return true;
		}
	}
	else
		return false;
}