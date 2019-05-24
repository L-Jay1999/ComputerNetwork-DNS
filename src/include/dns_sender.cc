#include <string>
#include <memory>
#include <utility>
#include "my_queue.h"
#include "dns_packet.h"
#include "job_queue.h"
#include "dns_sender.h"

DNSSender::DNSSender(JobQueue *job_queue, HostList *host_list, const std::string &address)
	: job_queue_(job_queue), host_list_(host_list), address_(address)
{
	job_queue_->Bind(this);
}

void DNSSender::Start()
{
	while (true)
	{
		set_packet();
		Responce();
		delete[] dns_packet_.raw_data.data;
	}
}

void DNSSender::set_packet()
{
	QueueData temp_qdata = data_queue_->pop_front();	// 自动阻塞
	dns_packet_.Parse(temp_qdata);
}

void DNSSender::set_queue(MyQueue *data_queue)
{
	data_queue_ = data_queue;
}

void DNSSender::Responce()
{
	if (dns_packet_.header.Flags >> 15 == 0)
	{
		for (int query_cnt = 0; query_cnt < dns_packet_.header.QDCOUNT; query_cnt++)
		{
			HostState state = host_list_->get_host_state(dns_packet_.query[query_cnt].QNAME);
			if (state == FIND)
			{
				set_reply(host_list_->get_ip_str(dns_packet_.query[query_cnt].QNAME));
				send_to_client();
			}
			else if (state == BANNED)
			{
				set_reply("0.0.0.0");
				send_to_client();
			}
			else
			{
				my_map_->insert(dns_packet_.header.ID, dns_packet_.raw_data.addr);
				send_to_DNS();
			}
		}
	}
	else
	{
		if (my_map_->find(dns_packet_.header.ID))
		{
			auto temp = my_map_->get(dns_packet_.header.ID);
			my_map_->erase(dns_packet_.header.ID);
			send_to_client(temp);
		}
		else
		{
			//有错
			return;
		}
	}
}

void DNSSender::set_reply(const std::string &ip)
{
	dns_packet_.header.Flags = 0x8180;
	dns_packet_.header.ANCOUNT = 1;

	dns_packet_.answer = std::make_unique<DNSAnswer[]>(1);
	// dns_packet_.answer->NAME = name;
	dns_packet_.answer[0].TYPE = 1;
	dns_packet_.answer[0].CLASS = 1;
	dns_packet_.answer[0].TTL = 168;
	dns_packet_.answer[0].RDLENGTH = 4;

	std::string temp;

	for (int i = 0; i < ip.length(); i++)
	{
		if (ip[i] == '.')
		{
			dns_packet_.answer[0].RDATA.push_back(static_cast<unsigned char>(std::stoi(temp)));
			temp.clear();
		}
		else temp.push_back(ip[i]);
	}
	dns_packet_.answer[0].RDATA.push_back(static_cast<unsigned char>(std::stoi(temp)));
	temp.clear();
	dns_packet_.to_packet();
}

void DNSSender::send_to_client()
{
	if (sockSend_.SendTo(dns_packet_.raw_data))
	{
		//log

	}
	else
	{
		//log
	}
}

void DNSSender::send_to_client(const sockaddr_in &addr)
{
	dns_packet_.raw_data.addr = addr;
	if (sockSend_.SendTo(dns_packet_.raw_data))
	{
		//log

	}
	else
	{
		//log
	}
}

void DNSSender::send_to_DNS()
{
	dns_packet_.raw_data.addr = sockSend_.get_superior_server();
	if (sockSend_.SendTo(dns_packet_.raw_data))
	{
		//log

	}
	else
	{
		//log
	}
}