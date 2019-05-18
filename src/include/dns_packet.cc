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
	char* packet = raw_packet.data;
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
	unsigned char char_num;
	for (int qcnt = 0; qcnt < header.QDCOUNT; qcnt++)
	{
		while (true)
		{
			char_num = static_cast<unsigned char>(packet[p_question]);
			p_question++;
			if (char_num == 0)
			{
				break;
			}
			else
			{
				for (int i = 0; i < char_num; i++)
				{
					query[qcnt].QNAME.push_back(packet[p_question]);
					++p_question;
				}
				query[qcnt].QNAME.push_back('.');
			}
		}
		query[qcnt].QNAME.pop_back();
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
			if (packet[p_question] == 0xc0)
			{
				int p_question_temp = p_question;
				p_question = (int)packet[p_question + 1];
				while (true)
				{
					char_num = static_cast<unsigned char>(packet[p_question]);
					p_question++;
					if (char_num == 0)
					{
						break;
					}
					else
					{
						for (int i = 0; i < char_num; i++)
						{
							answer[acnt].NAME.push_back(packet[p_question]);
							p_question++;
						}
						answer[acnt].NAME.push_back('.');
					}
				}
				answer[acnt].NAME.pop_back();
				p_question = p_question_temp;
			}
			else
			{
				std::cout << "warning" << std::endl;
			}
			p_question += 2;
			answer[acnt].TYPE = ctos(packet[p_question], packet[p_question + 1]);
			p_question += 2;
			answer[acnt].CLASS = ctos(packet[p_question], packet[p_question + 1]);
			p_question += 2;
			answer[acnt].TTL = stoint(ctos(packet[p_question], packet[p_question + 1]), ctos(packet[p_question + 2], packet[p_question + 3]));
			p_question += 4;
			answer[acnt].RDLENGTH = ctos(packet[p_question], packet[p_question + 1]);
			for (int i = 0; i < answer[acnt].RDLENGTH; i++)
			{
				//answer[acnt].RDATA[i] = stoint(ctos(packet[p_question], packet[p_question + 1]), ctos(packet[p_question + 2], packet[p_question + 3]));
				answer[acnt].RDATA.push_back(static_cast<unsigned char>(packet[p_question]));
				p_question++;
			}
		}
	}
	return true;
}

bool DNSPacket::Packet()
{
	memset(raw_data.data, '\0', sizeof(raw_data.data));
	//raw_data.data.clear();
	
	//头
	unsigned short ID = header.ID;
	unsigned short Flags = header.Flags;
	
	int isans = Flags >> 15;

	unsigned short QDCOUNT = header.QDCOUNT;
	unsigned short ANCOUNT = header.ANCOUNT;
	unsigned short NSCOUNT = header.NSCOUNT;
	unsigned short ARCOUNT = header.ARCOUNT;

	raw_data.data[0] = stoc(ID);
	raw_data.data[1] = static_cast<unsigned char>(ID);

	raw_data.data[2] = stoc(Flags);
	raw_data.data[3] = static_cast<unsigned char>(Flags);

	raw_data.data[4] = stoc(QDCOUNT);
	raw_data.data[5] = static_cast<unsigned char>(QDCOUNT);

	raw_data.data[6] = stoc(ANCOUNT);
	raw_data.data[7] = static_cast<unsigned char>(ANCOUNT);

	raw_data.data[8] = stoc(NSCOUNT);
	raw_data.data[9] = static_cast<unsigned char>(NSCOUNT);

	raw_data.data[10] = stoc(ARCOUNT);
	raw_data.data[11] = static_cast<unsigned char>(ARCOUNT);

	//问题
	int p_question = 12;
	for (int qcnt = 0; qcnt < QDCOUNT; qcnt++)
	{
		int temp, cnt = 0;
		p_question++;
		for (int i = 0; i < query[qcnt].QNAME.length(); i++)
		{
			if (query[qcnt].QNAME[i] != '.')
			{
				cnt++;
				raw_data.data[p_question] = query[qcnt].QNAME[i];
			}
			else
			{
				raw_data.data[p_question - (cnt + 1)] = cnt;
				cnt = 0;
			}
			p_question++;
		}
		raw_data.data[p_question] = 0;
		p_question++;
	}

	if (isans)
	{
		unsigned short TYPE;
		unsigned short CLASS;
		unsigned int TTL;
		unsigned short RDLENGTH;
		for (int acnt = 0; acnt < ANCOUNT; acnt++)
		{
			TYPE = answer[acnt].TYPE;
			CLASS = answer[acnt].CLASS;
			TTL = answer[acnt].TTL;
			RDLENGTH = answer[acnt].RDLENGTH;

			//NAME
			raw_data.data[p_question] = 0xc0;
			p_question++;
			raw_data.data[p_question] = 0x0c;
			p_question++;

			//RR
			raw_data.data[p_question] = stoc(TYPE);
			p_question++;
			raw_data.data[p_question] = static_cast<unsigned char>(TYPE);
			p_question++;

			//CLASS
			raw_data.data[p_question] = stoc(CLASS);
			p_question++;
			raw_data.data[p_question] = static_cast<unsigned char>(CLASS);
			p_question++;

			//TTL
			unsigned short ttla = inttos(TTL);
			unsigned short ttlb = TTL;
			raw_data.data[p_question] = stoc(ttla);
			p_question++;
			raw_data.data[p_question] = static_cast<unsigned char>(ttla);
			p_question++;
			raw_data.data[p_question] = stoc(ttlb);
			p_question++;
			raw_data.data[p_question] = static_cast<unsigned char>(ttlb);
			p_question++;

			//RDLENGTH
			raw_data.data[p_question] = stoc(RDLENGTH);
			p_question++;
			raw_data.data[p_question] = static_cast<unsigned char>(RDLENGTH);
			p_question++;

			//answer
			for (int i = 0; i < answer[acnt].RDATA.length(); i++)
			{
				raw_data.data[p_question] = answer[acnt].RDATA[i];
				p_question++;
			}
			p_question++;
		}
	}
	//raw_data.data[p_question] = '\0';
	raw_data.len = strlen(raw_data.data);
	//raw_data.len = raw_data.data.length();
	return true;
}

void DNSPacket::print()
{
	printf("------------------Header------------------\n");
	printf("ID:%X\n", header.ID);
	printf("Flags:%X\n", header.Flags);
	printf("QR = %X, Opcode = %X, AA = %X, TC = %X, RD = %X\n",
		(header.Flags & 0x8000) >> 15, (header.Flags & 0x7800) >> 11, (header.Flags & 0x0400) >> 10, (header.Flags & 0x0200) >> 9,
		(header.Flags & 0x0100) >> 8);
	printf("RA = %X, Z = %X, RCODE = %X\n",
		(header.Flags & 0x0080) >> 7, (header.Flags & 0x0070) >> 4, (header.Flags & 0x000f));
	printf("QDCOUNT = %d, ANCOUNT = %d, NSCOUNT = %d, ARCOUNT = %d\n",
		header.QDCOUNT, header.ANCOUNT, header.NSCOUNT, header.ARCOUNT);

	printf("-------------------Query------------------\n");
	for (int qcnt = 0; qcnt < header.QDCOUNT; qcnt++)
	{
		printf("Query %d:\n", qcnt+1);
		printf("QNAME = %s\n", query[qcnt].QNAME.c_str());
		printf("QTYPE = %d, QCLASS = %d\n", query[qcnt].QTYPE, query[qcnt].QCLASS);
	}

	if ((header.Flags & 0x8000) >> 15)
	{
		printf("------------------Answer------------------\n");
		for (int acnt = 0; acnt < header.ANCOUNT; acnt++)
		{
			printf("Answer %d:\n", acnt+1);
			printf("NAME = %s\n", answer[acnt].NAME.c_str());
			printf("TYPE = %d, CLASS = %d\n", answer[acnt].TYPE, answer[acnt].CLASS);
			printf("TTL = %d\n", answer[acnt].TTL);
			printf("RDLENGTH = %d\n", answer[acnt].RDLENGTH);
			if (answer[acnt].TYPE == 1)
			{
				printf("RDATA = %d.%d.%d.%d", answer[acnt].RDATA[0], answer[acnt].RDATA[1], answer[acnt].RDATA[2], answer[acnt].RDATA[3]);
			}
		}
	}
}

void DNSPacket::printraw()
{
	unsigned temp;
	for (int i = 0; i < raw_data.len; i++)
	{
		temp = static_cast<unsigned char>(raw_data.data[i]);
		printf("%c%X ", "0\a"[temp <= 0xf ? 0 : 1], temp);
		if (i % 8 == 7)printf("\n");
	}
}

void DNSPacket::deleteall()
{
	delete query;
	delete answer;
}