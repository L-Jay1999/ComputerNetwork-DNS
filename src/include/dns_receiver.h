#pragma once
#include "dns_packet.h"
#include "my_socket.h"
#include <deque>
#include <mutex>
class DNSReceiver   // 接收dns数据包并将其放到工作队列上
{
public:
	DNSReceiver(std::deque <Queuedata>* jobq, std::mutex *mutex);
	void start();
private:
    //DNSPacket dns_packet_;    // 保存dns数据包的详细信息

    std::deque <Queuedata> *jobq; // 保存收到的
	std::mutex *mutex;

    MySocket socRecv;           // 接收dns数据包
	// .......
};