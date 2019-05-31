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
	std::lock_guard<std::mutex> lock(push_mtx_);
	std::random_device rd;

	int upper_bound = pos_;
	if (upper_bound)
		upper_bound--;

	Log::WriteLog(1, __s("JobQueue push packet to queue ") + std::to_string(push_pos_));
	queue_group_[push_pos_++].push_back(packet);
	if (push_pos_ > upper_bound)
		push_pos_ = 0;
}

void JobQueue::Bind(DNSSender *sender)
{
	std::lock_guard<std::mutex> lock(bind_mtx_);
	if (pos_ < group_size_)
	{
		Log::WriteLog(1, __s("JobQueue bind ") + std::to_string(pos_) + __s(" queue to a DNSSender"));
		sender->set_queue(&queue_group_[pos_++]);
	}
	else
	{
		Log::WriteLog(1, __s("JobQueue no avaliable queue to bind, current size: ") + std::to_string(group_size_));
		// throw std::logic_error("可绑定的队列已达到最大值");
	}
}