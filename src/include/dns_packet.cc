#include "dns_packet.h"


static unsigned short combine(unsigned char hbyte, unsigned char lbyte)
{
	unsigned short high_byte = static_cast<unsigned char>(hbyte);
	unsigned short low_byte = static_cast<unsigned char>(lbyte);
	unsigned short res = ((high_byte << 8) | low_byte);
	return res;
}

bool DNSPacket::Parse(const QueueData &raw_packet)
{
	raw_data = raw_packet;
	char *packet = raw_packet.data;
	int packet_len = raw_packet.len;
	sockaddr packet_addr = raw_packet.addr;


	header.ID = combine(packet[0], packet[1]);
	header.Flags = combine(packet[2], packet[3]);
	header.QDCOUNT = combine(packet[4], packet[5]);
	header.ANCOUNT = combine(packet[6], packet[7]);
	header.NSCOUNT = combine(packet[8], packet[9]);
	header.ARCOUNT = combine(packet[10], packet[11]);

	// 需要修改 ：包中可能同时包含Query和answer报文
	if (!(header.Flags >> 15)) // 询问报文
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
					query.QNAME.push_back('.');
				}
				for (int i = 0; i < char_num; i++)
				{
					++p_question;
					query.QNAME.push_back(packet[p_question]);
				}
				flag = true;
			}
		}
		query.QTYPE = combine(packet[p_question], packet[p_question + 1]);
		p_question += 2;
		query.QCLASS = combine(packet[p_question], packet[p_question + 1]);
	}
}
