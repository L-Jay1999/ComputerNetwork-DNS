#pragma once
#include <string>
#include <deque>
#include <mutex>
#include <map>

#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

#include "my_socket.h"
#include "dns_packet.h"
#include "host_list.h"

class MyQueue;
class JobQueue;

class DNSSender
{
public:
	DNSSender() = delete;
	explicit DNSSender(JobQueue *job_queue, HostList *host_list, const std::string &address);
	DNSSender(const DNSSender *other) = delete;
	~DNSSender() = default;

	// 开始运行
	void Start();
	// 从工作队列获取一个报文
	void set_packet();

	void set_queue(MyQueue *data_queue) noexcept { data_queue_ = data_queue; }

	void Responce();

private:
	JobQueue *job_queue_ = nullptr; // 控制器
	MyQueue *data_queue_ = nullptr; // 工作队列
	HostList *host_list_ = nullptr; // 对照表

	MySocket sockSend_;
	MySocket sockQuest_;

	std::string address_;  // 上级dns服务器地址
	DNSPacket dns_packet_; // 将数据放到该结构体

	void set_reply_normal(const std::string &ip);
	void set_reply_banned();
	void send_to_client() { sockSend_.SendTo(dns_packet_.raw_data); } // 在socket上写入传出数据raw_data
	void send_to_client(const sockaddr_in &addr);
};