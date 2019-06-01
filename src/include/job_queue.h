#pragma once
#include <atomic>
#include <map>
#include <mutex>

#include "my_queue.h"

class DNSSender;
struct QueueData;

class JobQueue
{
public:
	JobQueue() = default;

	//将packet置入工作队列
	void Push(const QueueData &packet);

	// 将给定的sender与某一队列绑定, 之后直接从绑定后的队列Pop即可
	void Bind(DNSSender *sender); 

private:
	static constexpr int group_size_ = 64;

	std::atomic<int> pos_ = 0;
	int push_pos_ = 0;
	MyQueue queue_group_[group_size_]; // queue数组
	std::mutex push_mtx_;			   // push操作的互斥锁
	std::mutex bind_mtx_;			   // bind操作的互斥锁
};