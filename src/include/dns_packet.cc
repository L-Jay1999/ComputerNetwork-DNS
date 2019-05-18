#include "dns_packet.h"

static unsigned short ctos(unsigned char hbyte, unsigned char lbyte)
{
	unsigned short high_byte = static_cast<unsigned char>(hbyte);
	unsigned short low_byte = static_cast<unsigned char>(lbyte);
	unsigned short res = ((high_byte << 8) | low_byte);
	return res;
}

static unsigned int stoint(unsigned short hbyte, unsigned short lbyte)
{
	unsigned short high_byte = static_cast<unsigned short>(hbyte);
	unsigned short low_byte = static_cast<unsigned short>(lbyte);
	unsigned short res = ((high_byte << 16) | low_byte);
	return res;
}

static unsigned char stoc(unsigned short &x)
{
	unsigned char a = static_cast<unsigned char> (x & 0x00ff);
	unsigned char b = static_cast<unsigned char> (x >> 8);
	x = static_cast <unsigned char> (x);
	x = a;
	return b;
}

static unsigned short inttos(unsigned int& x)
{
	unsigned short a = static_cast<unsigned short> (x & 0x0000ffff);
	unsigned short b = static_cast<unsigned short> (x >> 16);
	x = static_cast <unsigned short> (x);
	x = a;
	return b;
}

bool DNSPacket::Parse(const QueueData &raw_packet)
{
	raw_data = raw_packet;
	char *packet = raw_packet.data;
	int packet_len = raw_packet.len;
	sockaddr packet_addr = raw_packet.addr;

	header.ID = ctos(packet[0], packet[1]);
	header.Flags = ctos(packet[2], packet[3]);
	header.QDCOUNT = ctos(packet[4], packet[5]);
	header.ANCOUNT = ctos(packet[6], packet[7]);
	header.NSCOUNT = ctos(packet[8], packet[9]);
	header.ARCOUNT = ctos(packet[10], packet[11]);

	// query
	query = new DNSQuery[header.QDCOUNT];
	int p_question = 12;
	unsigned short char_num;
	for (int qcnt = 0; qcnt < header.QDCOUNT; qcnt++)
	{
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
					query[qcnt].QNAME.push_back('.');
				}
				for (int i = 0; i < char_num; i++)
				{
					++p_question;
					query[qcnt].QNAME.push_back(packet[p_question]);
				}
				flag = true;
			}
		}
		query[qcnt].QTYPE = ctos(packet[p_question], packet[p_question + 1]);
		p_question += 2;
		query[qcnt].QCLASS = ctos(packet[p_question], packet[p_question + 1]);
		p_question += 2;
	}
	

	//answer
	if (header.Flags >> 15)
	{
		answer = new DNSAnswer[header.ANCOUNT];
		for (int acnt = 0; acnt < header.ANCOUNT; acnt++)
		{
			//answer[acnt].NAME = query.QNAME;//todo
			p_question += 2;
			answer[acnt].RR = ctos(packet[p_question], packet[p_question + 1]);
			p_question += 2;
			answer[acnt].CLASS = ctos(packet[p_question], packet[p_question + 1]);
			p_question += 2;
			answer[acnt].TTL = stoint(ctos(packet[p_question], packet[p_question + 1]), ctos(packet[p_question + 2], packet[p_question + 3]));
			p_question += 4;
			answer[acnt].RDLENGTH = ctos(packet[p_question], packet[p_question + 1]);
			answer[acnt].RDATA = new unsigned int[answer[acnt].RDLENGTH];
			for (int i = 0; i < answer[acnt].RDLENGTH; i++)
			{
				//todo
				answer[acnt].RDATA[i] = stoint(ctos(packet[p_question], packet[p_question + 1]), ctos(packet[p_question + 2], packet[p_question + 3]));
				p_question += 4;
			}
		}
	}
}

bool DNSPacket::Packet()
{
	memset(raw_data.data, '\0', sizeof(raw_data.data));
	
	//头
	unsigned short ID = header.ID;
	unsigned short Flags = header.Flags;
	
	int isans = Flags >> 15;

	unsigned short QDCOUNT = header.QDCOUNT;
	unsigned short ANCOUNT = header.ANCOUNT;
	unsigned short NSCOUNT = header.NSCOUNT;
	unsigned short ARCOUNT = header.ARCOUNT;

	raw_data.data[0] = stoc(ID);
	raw_data.data[1] = ID;

	raw_data.data[2] = stoc(Flags);
	raw_data.data[3] = Flags;

	raw_data.data[4] = stoc(QDCOUNT);
	raw_data.data[5] = QDCOUNT;

	raw_data.data[6] = stoc(ANCOUNT);
	raw_data.data[7] = ANCOUNT;

	raw_data.data[8] = stoc(NSCOUNT);
	raw_data.data[9] = NSCOUNT;

	raw_data.data[10] = stoc(ARCOUNT);
	raw_data.data[11] = ARCOUNT;

	//问题
	int p_question = 12;
	for (int qcnt = 0; qcnt < QDCOUNT; qcnt++)
	{
		int temp, cnt;
		int i = 0;
		while (i < query[qcnt].QNAME.length())
		{
			cnt = 0;
			temp = p_question;
			while (query[qcnt].QNAME[i] != '.' && i < query[qcnt].QNAME.length())
			{
				cnt++;
				p_question++;
				raw_data.data[p_question] = query[qcnt].QNAME[i];
			}
			raw_data.data[temp] = cnt;
		}
		raw_data.data[p_question] = 0;
		p_question++;
	}
	

	if (isans)
	{
		for (int acnt = 0; acnt < ANCOUNT; acnt++)
		{
			unsigned short RR = answer[acnt].RR;
			unsigned short CLASS = answer[acnt].CLASS;
			unsigned int TTL = answer[acnt].TTL;
			unsigned short RDLENGTH = answer[acnt].RDLENGTH;
			//NAME
			raw_data.data[p_question] = 0xc0;
			p_question++;
			raw_data.data[p_question] = 0x0c;
			p_question++;

			//RR
			raw_data.data[p_question] = stoc(RR);
			p_question++;
			raw_data.data[p_question] = RR;
			p_question++;

			//CLASS
			raw_data.data[p_question] = stoc(CLASS);
			p_question++;
			raw_data.data[p_question] = CLASS;
			p_question++;

			//TTL
			unsigned short ttla = inttos(TTL);
			unsigned short ttlb = TTL;
			raw_data.data[p_question] = stoc(ttla);
			p_question++;
			raw_data.data[p_question] = ttla;
			p_question++;
			raw_data.data[p_question] = stoc(ttlb);
			p_question++;
			raw_data.data[p_question] = ttlb;
			p_question++;

			//RDLENGTH = 4
			raw_data.data[p_question] = stoc(RDLENGTH);
			p_question++;
			raw_data.data[p_question] = RDLENGTH;
			p_question++;

			//
			unsigned short da = inttos(TTL);
			unsigned short db = TTL;
		}
	}
}