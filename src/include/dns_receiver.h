#pragma once
#include "dns_packet.h"
#include "my_socket.h"
#include "job_queue.h"
#include <deque>
#include <mutex>
class DNSReceiver // 接收dns数据包并将其放到工作队列上
{
public:
	DNSReceiver(JobQueue* jobque);
	void start();

private:
	//DNSPacket dns_packet_;    // 保存dns数据包的详细信息

	//std::deque<QueueData> *jobq; // 保存收到的
	JobQueue* jobque;

	MySocket socRecv; // 接收dns数据包
					  // .......
};