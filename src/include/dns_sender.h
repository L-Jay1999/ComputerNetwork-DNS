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
class DNSSender
{
public:
	DNSSender(const std::string &address = "10.3.9.5");

	void Start();
	void set_packet();
	// 供job_queue分配
	void set_queue(MyQueue *queue);
	//
	void set_map(MyMap* map);
	// 响应
	void Responce();

private:
	MyQueue *jobq; // 从该队列中取出数据并处理发送
	HostList *host_list_;

	MySocket socSend = MySocket(SEND_SOCKET);	 // 发送

	std::string address; // 上级dns服务器地址
	DNSPacket dns_packet_;
	MyMap *my_map_;

	void set_reply(std::string ip);
	void send_to_client();
	void send_to_client(sockaddr_in addr);
	void send_to_DNS();
};