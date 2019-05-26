#include "dns_receiver.h"
#include "job_queue.h"
void DNSReceiver::Start()
{
	while (true)
	{
		QueueData temp = sockRecv.RecvFrom();
		job_queue_->Push(temp);
	}
}

void DNSReceiver::set_queue(JobQueue *queue)
{
	job_queue_ = queue;
}