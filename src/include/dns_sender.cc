#include <string>
#include <memory>
#include <utility>
#include <random>
#include <iostream>

#include "my_queue.h"
#include "dns_packet.h"
#include "job_queue.h"
#include "dns_sender.h"
#include "log.h"

static int get_quest_port_random()
{
	static constexpr int kSendPortLowerBound = 10000;
	static constexpr int kSendPortUpperBound = 40000;

	static std::random_device rd;
	static std::uniform_int_distribution<int> uid(kSendPortLowerBound, kSendPortUpperBound);

	return uid(rd);
}

DNSSender::DNSSender(JobQueue *job_queue, HostList *host_list, MyMap *my_map, const std::string &address)
	: job_queue_(job_queue), host_list_(host_list), my_map_(my_map), address_(address), sockSend_(SEND_SOCKET, "53", address), sockQuest_(QUEST_SOCKET, std::to_string(get_quest_port_random()).c_str(), "")
{
	job_queue_->Bind(this);
	sockQuest_.set_recv_timeout(1000);
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

void DNSSender::set_queue(MyQueue *data_queue) noexcept
{
	data_queue_ = data_queue;
}

void DNSSender::Responce()
{
	for (int query_cnt = 0; query_cnt < dns_packet_.header.QDCOUNT; query_cnt++)
	{
		HostState state = host_list_->get_host_state(dns_packet_.query[query_cnt].QNAME);
		char client_ip[24];
		inet_ntop(AF_INET, &dns_packet_.from.sin_addr, client_ip, sizeof(client_ip));
		Log::WriteLog(1, __s("Sender get packet querying for ") + dns_packet_.query[query_cnt].QNAME + __s(" from ") + __s(client_ip));
		if (state == FIND)
		{
			Log::WriteLog(1, __s("Sender find ip address: ") + host_list_->get_ip_str(dns_packet_.query[query_cnt].QNAME) + __s(" for host: ") + dns_packet_.query[query_cnt].QNAME);
			set_reply(host_list_->get_ip_str(dns_packet_.query[query_cnt].QNAME));
			send_to_client();
		}
		else if (state == BANNED)
		{
			Log::WriteLog(1, __s("Sender host is banned"));
			set_reply("0.0.0.0");
			send_to_client();
		}
		else
		{
			Log::WriteLog(1, __s("Sender cannot find host, query ip address from ") + address_);
			sockaddr_in temp = dns_packet_.raw_data.addr;

			int resend = 0;
			int ptr = 0;
			auto id_expected = dns_packet_.header.ID;
			decltype(id_expected) id_recv;
			DNSPacket temp_dns_packet;
			QueueData temp_packet;

			dns_packet_.raw_data.addr = sockSend_.get_superior_server();
			while (true)
			{
				if (!sockQuest_.SendTo(dns_packet_.raw_data))
					Log::WriteLog(2, __s("Sender send to superior dns server failed, ErrorCode: ") + std::to_string(WSAGetLastError()));

				temp_packet = sockQuest_.RecvFrom();
				if (temp_packet.len)
				{
					temp_dns_packet.ReadFromCSTR(id_recv, temp_packet.data, ptr);
					if (id_recv == id_expected)
					{
						break;
					}
					else
					{
						ptr = 0;
						Log::WriteLog(2, __s("Sender recv answer with wrong id"));
						continue;
					}
				}
				else if (resend > 1)
					return;
				else
					resend++;
			}
			temp_packet.addr = temp;

			if (!sockSend_.SendTo(temp_packet))
				Log::WriteLog(2, __s("Sender send to client failed, ErrorCode: ") + std::to_string(WSAGetLastError()));
			else
				Log::WriteLog(2, __s("Sender send to client success"));
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