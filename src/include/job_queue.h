#pragma once

#include <atomic>
#include <map>

#include "my_queue.h"

class DNSSender;
struct QueueData;

class JobQueue
{
public:

	JobQueue() = default;
	void Push(const QueueData &packet);
	// 将给定的sender与某一队列绑定, 之后直接从绑定后的队列Pop即可
	void Bind(DNSSender *sender);
private:
	static constexpr int group_size_ = 256;
	std::atomic<int> pos_ = 0;
	MyQueue queue_group_[group_size_];
};