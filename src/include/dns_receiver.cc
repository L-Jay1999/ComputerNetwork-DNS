#include "dns_receiver.h"
#include "job_queue.h"
void DNSReceiver::Start()
{
	while (true)
	{
		QueueData temp = sockRecv.RecvFrom(); // 接收数据包
		job_queue_->Push(temp);
	}
}