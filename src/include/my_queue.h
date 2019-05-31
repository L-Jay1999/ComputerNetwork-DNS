#pragma once

#include <deque>
#include <mutex>
#include <utility>
#include <condition_variable>

#include "dns_packet.h"
// using QueueData = long long;

class MyQueue
{
public:
	MyQueue() = default;

	void push_back(const QueueData &qdata);
	QueueData pop_front();

private:
	std::deque<QueueData> data_queue_{}; //数据队列
	std::mutex mutex_;                   //同步锁
	std::condition_variable cond_;
	int size_ = 0;
};