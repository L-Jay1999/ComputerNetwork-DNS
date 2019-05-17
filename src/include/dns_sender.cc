
#include "my_queue.h"
#include "dns_packet.h"
#include "dns_sender.h"

DNSSender::DNSSender(const std::string &address)
{
	this->address = address;
}

void DNSSender::Start()
{
	while (true)
	{
		set_packet();
		Responce();
	}
}

void DNSSender::set_packet()
{
	QueueData temp_qdata = jobq->pop_front();	// 自动阻塞
	dns_packet_.Parse(temp_qdata);
}

void DNSSender::set_queue(MyQueue *queue)
{
	jobq = queue;
}

void DNSSender::Responce()
{
	/*
	if (!(dns_packet_.header.Flags >> 15))
	{
		HostState state = host_list_->get_host_state(dns_packet_.query.QNAME);
		if (state == FIND)
		{
			std::string ip_addr = host_list_->get_ip_str(dns_packet_.query.QNAME);
		}
		else if (state == BANNED)
		{
			std::string ip_addr = "0.0.0.0";
		}
		else
		{
			id_ip_.insert(std::pair<unsigned short, sockaddr>(dns_packet_.header.ID, temp_qdata.addr));
		}
	}
	else
	{
		auto iter = id_ip_.find(dns_packet.header.ID);
		if (iter == id_ip_.end())
		{
			return;
		}
		sockaddr addr = iter->second;
		id_ip_.erase(iter);
	}
	*/
}