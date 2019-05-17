#include <mutex>
#include <map>
#include <random>
#include <condition_variable>

#include "job_queue.h"
#include "dns_packet.h"
#include "dns_sender.h"

void JobQueue::Push(const QueueData &packet)
{
	std::random_device rd;

	int upper_bound = pos_;
	if (upper_bound)
		upper_bound--;

	std::uniform_int_distribution<int> uid(0, upper_bound);
	queue_group_[uid(rd)].push_back(packet);
}

void JobQueue::Bind(DNSSender *sender)
{
	if (pos_ < group_size_)
		sender->set_queue(&queue_group_[pos_]);
	else
		throw std::logic_error("可绑定的队列已达到最大值");
}
