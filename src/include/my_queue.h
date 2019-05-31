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

	//将qdata压入data_queue_队列
	void push_back(const QueueData &qdata);
	//将data_queue_队列第一个个体弹出并返回
	QueueData pop_front();

private:
	std::deque<QueueData> data_queue_{};
	std::mutex mutex_;
	std::condition_variable cond_;
	int size_ = 0;
};