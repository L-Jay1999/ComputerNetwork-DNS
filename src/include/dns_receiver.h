#pragma once
#include <deque>
#include <mutex>

#include "my_socket.h"

class JobQueue;
class DNSReceiver // 接收DNS数据包并将其放到工作队列上
{
public:
	DNSReceiver() = delete;
	DNSReceiver(JobQueue *job_queue) : job_queue_(job_queue) {}
	DNSReceiver(const DNSReceiver &other) = delete;
	~DNSReceiver() = default;

	void Start();
	void set_queue(JobQueue *queue) noexcept { job_queue_ = queue; }

private:
	JobQueue *job_queue_ = nullptr; // 工作队列

	MySocket sockRecv{RECV_SOCKET}; // 接收dns数据包所使用的MySocket
};