#include "dns_receiver.h"

void DNSReceiver::Start()
{
	while (true)
	{
		QueueData temp = socRecv.RecvFrom();
		jobq->push_back(temp);
	}
}

void DNSReceiver::set_queue(MyQueue* queue)
{
	jobq = queue;
}