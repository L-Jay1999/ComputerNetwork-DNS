
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
	if (dns_packet_.header.Flags >> 15 == 0)
	{
		for (int qcnt = 0; qcnt < dns_packet_.header.QDCOUNT; qcnt++)
		{
			HostState state = host_list_->get_host_state(dns_packet_.query[qcnt].QNAME);
			if (state == FIND)
			{
				std::string ip_addr = host_list_->get_ip_str(dns_packet_.query[qcnt].QNAME);
				set_reply(ip_addr);
				dns_packet_.Packet();
				//todo
			}
			else if (state == BANNED)
			{
				std::string ip_addr = "0.0.0.0";
				set_reply(ip_addr);
				dns_packet_.Packet();
				//todo
			}
			else
			{
				id_ip_->insert(std::pair<unsigned short, sockaddr>(dns_packet_.header.ID, dns_packet_.raw_data.addr));
				//todo
			}
		}
	}
	else
	{
		auto iter = id_ip_->find(dns_packet_.header.ID);
		if (iter == id_ip_->end())
		{
			return;
		}
		sockaddr addr = iter->second;
		id_ip_->erase(iter);
		//todo
	}
}

void DNSSender::set_reply(std::string ip)
{
	dns_packet_.header.Flags = 0x8180;
	dns_packet_.header.ANCOUNT = 1;

	dns_packet_.answer = new DNSAnswer;
	//dns_packet_.answer->NAME = name;
	dns_packet_.answer->TYPE = 1;
	dns_packet_.answer->CLASS = 1;
	dns_packet_.answer->TTL = 168;
	dns_packet_.answer->RDLENGTH = 4;

	std::string temp;
	for (int i = 0; i < ip.length(); i++)
	{
		if (ip[i] == '.')
		{
			dns_packet_.answer->RDATA.push_back(static_cast<unsigned char>(std::stoi(temp)));
			temp.clear();
		}
		else temp.push_back(ip[i]);
	}
	dns_packet_.answer->RDATA.push_back(static_cast<unsigned char>(std::stoi(temp)));
	temp.clear();
}