
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

void DNSSender::set_map(MyMap* mymap)
{
	my_map_ = mymap;
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
				set_reply(host_list_->get_ip_str(dns_packet_.query[qcnt].QNAME));
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
		}
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
	dns_packet_.to_packet();
}

void DNSSender::send_to_client()
{
	if (socSend.SendTo(dns_packet_.raw_data))
	{
		//log

	}
	else
	{
		//log
	}
}

void DNSSender::send_to_client(sockaddr_in addr)
{
	dns_packet_.raw_data.addr = addr;
	if (socSend.SendTo(dns_packet_.raw_data))
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
	dns_packet_.raw_data.addr = socSend.get_superior_server();
	if (socSend.SendTo(dns_packet_.raw_data))
	{
		//log

	}
	else
	{
		//log
	}
}