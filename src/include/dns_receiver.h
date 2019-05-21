#pragma once
#include "dns_packet.h"
#include "my_socket.h"
#include "job_queue.h"
#include <deque>
#include <mutex>
class DNSReceiver // 接收dns数据包并将其放到工作队列上
{
public:
	DNSReceiver() = default;
	void Start();
	void set_queue(MyQueue* queue);

private:
	//DNSPacket dns_packet_;    // 保存dns数据包的详细信息

	//std::deque<QueueData> *jobq; // 保存收到的
	MyQueue* jobq;

	MySocket socRecv = MySocket(RECV_SOCKET); // 接收dns数据包
					  // .......
};