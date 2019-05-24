#include <string>
#include <memory>
#include <utility>
#include <random>

#include "my_queue.h"
#include "dns_packet.h"
#include "job_queue.h"
#include "dns_sender.h"

static int get_quest_port_random()
{
	static constexpr int kSendPortLowerBound = 10000;
	static constexpr int kSendPortUpperBound = 40000;

	static std::random_device rd;
	static std::uniform_int_distribution<int> uid(kSendPortLowerBound, kSendPortUpperBound);

	return uid(rd);
}


DNSSender::DNSSender(JobQueue *job_queue, HostList *host_list, MyMap *my_map, const std::string &address)
	: job_queue_(job_queue), host_list_(host_list), my_map_(my_map), address_(address)
{
	job_queue_->Bind(this);
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
	QueueData temp_qdata = data_queue_->pop_front();
	dns_packet_.Parse(temp_qdata);
}

void DNSSender::set_queue(MyQueue *data_queue)
{
	data_queue_ = data_queue;
}

void DNSSender::Responce()
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
			sockaddr_in temp = dns_packet_.raw_data.addr;
			std::string temp_port = std::to_string(get_quest_port_random());

			MySocket quest_sock(QUEST_SOCKET, temp_port.c_str());
			dns_packet_.raw_data.addr = sockSend_.get_superior_server();
			if (quest_sock.SendTo(dns_packet_.raw_data))
			{
				//log

			}
			else
			{
				//log
			}

			auto temp_packet = quest_sock.RecvFrom();
			temp_packet.addr = temp;
			if (sockSend_.SendTo(temp_packet))
			{
				//log

			}
			else
			{
				//log
			}

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