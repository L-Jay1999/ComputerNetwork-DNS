#include <mutex>
#include <map>
#include <random>
#include <condition_variable>

#include "job_queue.h"
#include "dns_packet.h"
#include "dns_sender.h"
#include "log.h"

void JobQueue::Push(const QueueData &packet)
{
	std::random_device rd;

	int upper_bound = pos_;
	if (upper_bound)
		upper_bound--;

	std::uniform_int_distribution<int> uid(0, upper_bound);
	int rand_pos = uid(rd);

	queue_group_[rand_pos].push_back(packet);
	Log::WriteLog(2, __s("JobQueue push packet to queue ") + std::to_string(rand_pos));
}

void JobQueue::Bind(DNSSender *sender)
{
	if (pos_ < group_size_)
	{
		Log::WriteLog(2, __s("JobQueue bind ") + std::to_string(pos_) + __s(" queue to a DNSSender"));
		sender->set_queue(&queue_group_[pos_++]);
	}
	else
	{
		Log::WriteLog(1, __s("JobQueue no avaliable queue to bind, current size: ") + std::to_string(group_size_));
		// throw std::logic_error("可绑定的队列已达到最大值");
	}
}