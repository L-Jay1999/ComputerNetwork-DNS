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
#include "my_map.h"

class MyQueue;
class JobQueue;
class DNSSender
{
public:
	DNSSender() = delete;
	explicit DNSSender(JobQueue *job_queue, HostList *host_list, MyMap *my_map, const std::string &address);
	DNSSender(const DNSSender *other) = delete;
	~DNSSender() = default;

	void Start();
	void set_packet();

	// 供job_queue分配
	void set_queue(MyQueue *queue);

	// 响应
	void Responce();

private:
	JobQueue *job_queue_ = nullptr;
	MyQueue *data_queue_ = nullptr; // 从该队列中取出数据并处理发送
	HostList *host_list_ = nullptr;

	//MySocket sockSend_{SEND_SOCKET};	 // 发送
	MySocket sockSend_;

	std::string address_; // 上级dns服务器地址
	MyMap *my_map_ = nullptr;
	DNSPacket dns_packet_;

	void set_reply(const std::string &ip);
	void send_to_client();
	void send_to_client(const sockaddr_in &addr);
};