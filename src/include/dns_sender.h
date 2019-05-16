#pragma once
#include <string>
#include <deque>
#include <mutex>
#include <map>
#include <WinSock2.h>
#include "my_socket.h"
#include "dns_packet.h"
#include "host_list.h"
class DNSSender
{
public:
	DNSSender(std::deque <Queuedata>* jobq, std::mutex *mutex, std::string address = "10.3.9.5");
	void start();
private:
    std::deque <Queuedata> *jobq; // 从该队列中取出数据并处理发送
	std::mutex *mutex;//锁
	HostList *host_list_;

	MySocket socSend;
	MySocket socQuery;			  // 向上级查询时使用
	std::string address;             // 上级dns服务器地址
	DNSPacket dns_packet;
	std::map <unsigned short, sockaddr> id_ip_;

	void DNSParse();
	unsigned short combine(unsigned char hbyte, unsigned char lbyte);
    // .......
};