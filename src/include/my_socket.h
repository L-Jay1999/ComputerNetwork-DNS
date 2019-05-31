#pragma once
#include <string>

#include <WS2tcpip.h>
#include <WinSock2.h>

#pragma comment(lib, "Ws2_32.lib")

enum SocketType
{
	SEND_SOCKET = 0,
	RECV_SOCKET,
	QUEST_SOCKET,
};

struct QueueData;

class MySocket
{
public:
	MySocket() = delete;
	MySocket(SocketType sock_type);
	MySocket(SocketType sock_type, const char *port, const std::string &superior_dns);
	MySocket(const MySocket &soc) = delete;
	MySocket &operator=(const MySocket &soc) = delete;
	~MySocket();

	QueueData RecvFrom(); // 从指定端口接收数据,返回一个QueueData(需要自行管理内存)

	//将queue_data内的数据queue_data.data向地址为queue_data.addr的目标传送
	bool SendTo(const QueueData &queue_data);
	//设置接收定时器以判定是否超时
	bool set_recv_timeout(const int ms);
	//将my_addr_info_.sin_family转换为点十进制表示法ip地址并存于buffer
	std::string get_ip_recver_str() const
	{
		static char buffer[20];
		return std::string(inet_ntop(my_addr_info_.sin_family, &(my_addr_info_.sin_addr), buffer, 20));
	}
	unsigned short get_port_recver() const noexcept { return my_addr_info_.sin_port; }
	std::string get_port_recver_str() const { return std::to_string(htons(my_addr_info_.sin_port)); }

	sockaddr_in get_superior_server() const noexcept { return superior_server_addr_; }

private:
	static constexpr int recvbuflen_ = 512; // 默认缓冲区大小

	DWORD InitSock(SocketType soc_type, const char *port, const std::string &superior_dns);

	DWORD _RecvFrom(QueueData &queue_data); // 从指定端口接收数据,对传入的空QueueData初始化,返回错误信息
	DWORD _SendTo(const QueueData &queue_data);

	DWORD last_error_ = 0;
	bool init_success_ = false;

	SocketType sock_type_;
	SOCKET sock_ = INVALID_SOCKET;
	sockaddr_in my_addr_info_;
	sockaddr_in superior_server_addr_; //上级DNS信息
	int my_addr_info_size_ = sizeof(my_addr_info_);
	sockaddr_in from_;
	int from_len_ = sizeof(from_);
	unsigned max_msg_size_ = 0; // 用于发送数据时每个数据报文的最大大小
	unsigned max_msg_size_len_ = sizeof(max_msg_size_);

	int recv_len_ = 0;
	char recvbuf_[recvbuflen_];
};