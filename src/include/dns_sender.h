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
class DNSSender
{
public:
	DNSSender(const std::string &address = "10.3.9.5");

	void Start();
	void set_packet();
	// 供job_queue分配
	void set_queue(MyQueue *queue);
	// 响应
	void Responce();

private:
	MyQueue *jobq; // 从该队列中取出数据并处理发送
	HostList *host_list_;

	MySocket socSend;	 // 发回给用户
	MySocket socQuery;   // 向上级查询时使用
	std::string address; // 上级dns服务器地址
	DNSPacket dns_packet_;
	std::map<unsigned short, sockaddr> id_ip_;
};