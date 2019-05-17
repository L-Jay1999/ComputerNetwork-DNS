#include "dns_sender.h"

DNSSender::DNSSender(std::deque <Queuedata>* jobq, std::mutex *mutex, std::string address)
{
	this->address = address;
	this->jobq = jobq;
	this->mutex = mutex;
}

void DNSSender::start()
{

}

void DNSSender::DNSParse()
{
	if (!jobq->empty())
	{
		mutex->lock();
		char* packet = jobq->front().data;
		int packet_len = jobq->front().len;
		sockaddr packet_addr = jobq->front().addr;
		jobq->pop_front();
		mutex->unlock();
		
		dns_packet.header.ID = combine(packet[0], packet[1]);
		dns_packet.header.Flags = combine(packet[2], packet[3]);
		dns_packet.header.QDCOUNT = combine(packet[4], packet[5]);
		dns_packet.header.ANCOUNT = combine(packet[6], packet[7]);
		dns_packet.header.NSCOUNT = combine(packet[8], packet[9]);
		dns_packet.header.ARCOUNT = combine(packet[10], packet[11]);

		if (!(dns_packet.header.Flags>>15)) // 询问报文
		{
			int p_question = 12;
			unsigned short char_num;
			bool flag = false;
			while (true)
			{
				char_num = static_cast<unsigned char>(packet[p_question]);
				if (char_num == 0)
				{
					++p_question;
					break;
				}
				else
				{
					if (flag)
					{
						++p_question;
						dns_packet.query.QNAME.push_back('.');
					}
					for (int i = 0; i < char_num; i++)
					{
						++p_question;
						dns_packet.query.QNAME.push_back(packet[p_question]);
					}
					flag = true;
				}
			}
			dns_packet.query.QTYPE = combine(packet[p_question], packet[p_question + 1]);
			p_question += 2;
			dns_packet.query.QCLASS = combine(packet[p_question], packet[p_question + 1]);

			//调试信息

			auto temp = host_list_->find(dns_packet.query.QNAME);
			if (temp == FIND)
			{
				std::string ip_addr = host_list_->get(dns_packet.query.QNAME);
				//发回
			}
			else if (temp == BANNED)
			{
				std::string ip_addr = "0.0.0.0";
				//发回
			}
			else
			{
				id_ip_.insert(std::pair<unsigned short, sockaddr>(dns_packet.header.ID, packet_addr));
				//转发
			}
		}

		else//回答
		{
			auto iter = id_ip_.find(dns_packet.header.ID);
			if (iter == id_ip_.end())
			{
				return;
			}
			sockaddr addr = iter->second;
			id_ip_.erase(iter);

			
		}
	}
}

unsigned short DNSSender::combine(unsigned char hbyte, unsigned char lbyte)
{
	unsigned short h = static_cast<unsigned char>(hbyte);
	unsigned short l = static_cast<unsigned char>(lbyte);
	unsigned short res = ((hbyte << 8) | lbyte);
	return res;
}