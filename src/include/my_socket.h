#pragma once
#include <string>

#include <WS2tcpip.h>
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

enum SocketType
{
	SEND_SOCKET = 0,
	RECV_SOCKET,
};

struct QueueData;

class MySocket
{
public:
    MySocket() = delete;
	MySocket(SocketType sock_type);
	MySocket(SocketType sock_type, const char *port);
	MySocket(const MySocket &soc) = delete;
	MySocket &operator=(const MySocket &soc) = delete;
	~MySocket();

	// 从指定端口接收数据,返回一个QueueData(需要自行管理内存)
	QueueData RecvFrom();

	bool SendTo(const QueueData &queue_data);

	std::string get_ip_recver_str() const 
	{
		static char buffer[20];
		return std::string(inet_ntop(my_addr_info_.sin_family, &(my_addr_info_.sin_addr), buffer, 20)); 
	}
	unsigned short get_port_recver() const { return my_addr_info_.sin_port; }
	std::string get_port_recver_str() const { return std::to_string(htons(my_addr_info_.sin_port)); }
	
	sockaddr_in get_superior_server() const { return superior_server_addr_; }

private:
	static constexpr int recvbuflen_ = 512;
	

	DWORD InitSock(SocketType soc_type, const char *port);

	// 从指定端口接收数据,对传入的空QueueData初始化,返回错误信息
	DWORD _RecvFrom(QueueData &queue_data);	
	DWORD _SendTo(const QueueData &queue_data);


	DWORD last_error_ = 0;
	bool init_success_ = false;

	SocketType sock_type_;
	SOCKET sock_ = INVALID_SOCKET;
	sockaddr_in my_addr_info_;
	sockaddr_in superior_server_addr_;
	int my_addr_info_size_ = sizeof(my_addr_info_);
	sockaddr_in from_;
	int from_len_ = sizeof(from_);
	unsigned max_msg_size_ = 0; // 用于发送数据时每个数据报文的最大大小
	unsigned max_msg_size_len_ = sizeof(max_msg_size_);

	std::size_t recv_len_ = 0;
    char recvbuf_[recvbuflen_];
};