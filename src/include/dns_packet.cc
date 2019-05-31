#include <cstdio>
#include <string>
#include <vector>
#include <iostream>
#include <cstdio>
#include <memory>
#include <utility>
#include <cassert>

#include "dns_packet.h"

// 将val的内容以ptr为位置指针写入buffer
template<typename T>
void DNSPacket::CopyToCSTR(const T val, char *buffer, int &ptr)
{
	assert(buffer != nullptr);
	char temp_LE[12];	// 小端序
	const char *p_char = reinterpret_cast<const char *>(&val);

	for (int i = sizeof(T) - 1; i >= 0; i--)
	{
		temp_LE[i] = *p_char;
		p_char++;
	}

	T *p_HE = reinterpret_cast<T *>(buffer + ptr);	// 大端序
	T *p_LE = reinterpret_cast<T *>(temp_LE);
	*p_HE = *p_LE;
	ptr += sizeof(T);
}

void DNSPacket::CopyToCSTR(const std::string &str, char *buffer, int &ptr)
{
	assert(buffer != nullptr);
	for (const auto c : str)
		CopyToCSTR(c, buffer, ptr);
}

// 将src的内容以ptr为位置指针读入dest
template<typename T>
void DNSPacket::ReadFromCSTR(T &dest, const  char *src, int &ptr)
{
	assert(src != nullptr);
	char temp_HE[12];
	const char *p_char = src + ptr;

	for (int i = sizeof(T) - 1; i >= 0; i--)
	{
		temp_HE[i] = *p_char;
		p_char++;
	}

	T *p_HE = reinterpret_cast<T *>(temp_HE);
	dest = *p_HE;
	ptr += sizeof(T);
}

// 将src的内容以ptr为位置指针读取之后len位的内容存入dest
void DNSPacket::ReadFromCSTR(char *dest, const unsigned len, const char *src, int &ptr)
{
	assert(src != nullptr);
	for (unsigned i = 0; i < len; i++)
		ReadFromCSTR(dest[i], src, ptr);
}

void DNSPacket::ReadFromCSTR(std::string &dest, const unsigned len, const char *src, int &ptr)
{
	assert(src != nullptr);
	for (unsigned i = 0; i < len; i++)
		dest.push_back(src[ptr++]);
}

bool DNSPacket::Parse(const QueueData &raw_packet)
{
	raw_data = raw_packet;
	std::memcpy(raw_data.data, raw_packet.data, sizeof(raw_packet.data));
	const char *packet = raw_packet.data;
	int ptr = 0;

	// char packet[256];

	from = raw_packet.addr;
	ReadFromCSTR(header.ID, packet, ptr);
	ReadFromCSTR(header.Flags, packet, ptr);
	ReadFromCSTR(header.QDCOUNT, packet, ptr);
	ReadFromCSTR(header.ANCOUNT, packet, ptr);
	ReadFromCSTR(header.NSCOUNT, packet, ptr);
	ReadFromCSTR(header.ARCOUNT, packet, ptr);

	// query
	query = std::make_unique<DNSQuery[]>(header.QDCOUNT);

	unsigned char char_num;
	for (int query_cnt = 0; query_cnt < header.QDCOUNT; query_cnt++)
	{
		while (true)
		{
			ReadFromCSTR(char_num, packet, ptr);
			if (char_num == 0)
				break;
			ReadFromCSTR(query[query_cnt].QNAME, char_num, packet, ptr);
			query[query_cnt].QNAME.push_back('.');
		}
		query[query_cnt].QNAME.pop_back();
		ReadFromCSTR(query[query_cnt].QTYPE, packet, ptr);
		ReadFromCSTR(query[query_cnt].QCLASS, packet, ptr);
	}

	// answer
	if (header.Flags >> 15)	// 读取QR用于判断
	{
		answer = std::make_unique<DNSAnswer[]>(header.ANCOUNT);
		for (int answer_cnt = 0; answer_cnt < header.ANCOUNT; answer_cnt++)
		{
			if (static_cast<unsigned char>(packet[ptr]) == 0xc0)	// ptr指向0xc0时，ptr接下来一位会指向QNAME首位的地址，据此进行读取
			{
				const int ptr_temp = ptr;
				ptr = static_cast<int>(packet[ptr + 1]);
				while (true)
				{
					ReadFromCSTR(char_num, packet, ptr);
					if (char_num == 0)
						break;
					ReadFromCSTR(answer[answer_cnt].NAME, char_num, packet, ptr);
					answer[answer_cnt].NAME.push_back('.');
				}
				answer[answer_cnt].NAME.pop_back();
				ptr = ptr_temp;
				ptr += sizeof(unsigned short);
			}
			else
			{
				std::cout << "warning" << std::endl;
			}
			ReadFromCSTR(answer[answer_cnt].TYPE, packet, ptr);
			ReadFromCSTR(answer[answer_cnt].CLASS, packet, ptr);
			ReadFromCSTR(answer[answer_cnt].TTL, packet, ptr);
			ReadFromCSTR(answer[answer_cnt].RDLENGTH, packet, ptr);
			ReadFromCSTR(answer[answer_cnt].RDATA, answer[answer_cnt].RDLENGTH, packet, ptr);
		}
	}
	return true;
}

bool DNSPacket::to_packet()
{
	// char data[2048];
	char *data = raw_data.data;
	int ptr = 0;

	// 头
	const int isans = header.Flags >> 15;

	CopyToCSTR(header.ID, data, ptr);
	CopyToCSTR(header.Flags, data, ptr);
	CopyToCSTR(header.QDCOUNT, data, ptr);
	CopyToCSTR(header.ANCOUNT, data, ptr);
	CopyToCSTR(header.NSCOUNT, data, ptr);
	CopyToCSTR(header.ARCOUNT, data, ptr);

	// 问题
	unsigned short QTYPE;
	unsigned short QCLASS;
	for (int query_cnt = 0; query_cnt < header.QDCOUNT; query_cnt++)
	{
		QTYPE = query[query_cnt].QTYPE;
		QCLASS = query[query_cnt].QCLASS;
		int cnt = 0;
		ptr++;	// QNAME转化前ptr + 1预留出网址的位数符空间
		for (int i = 0; i < query[query_cnt].QNAME.length(); i++)
		{
			if (query[query_cnt].QNAME[i] != '.')
			{
				cnt++;
				data[ptr] = query[query_cnt].QNAME[i];
			}
			else
			{
				data[ptr - (cnt + 1)] = cnt;
				cnt = 0;
			}
			ptr++;
		}
		data[ptr - (cnt + 1)] = cnt;
		// end of name
		CopyToCSTR('\0', data, ptr);

		CopyToCSTR(query[query_cnt].QTYPE, data, ptr);
		CopyToCSTR(query[query_cnt].QCLASS, data, ptr);
	}

	// 回答
	if (isans)
	{
		unsigned short TYPE;
		unsigned short CLASS;
		unsigned int TTL;
		unsigned short RDLENGTH;
		for (int acnt = 0; acnt < header.ANCOUNT; acnt++)
		{
			TYPE = answer[acnt].TYPE;
			CLASS = answer[acnt].CLASS;
			TTL = answer[acnt].TTL;
			RDLENGTH = answer[acnt].RDLENGTH;

			// NAME (PTR)
			CopyToCSTR(static_cast<unsigned char>(0xc0), data, ptr);
			CopyToCSTR(static_cast<unsigned char>(0x0c), data, ptr);

			// RR
			CopyToCSTR(TYPE, data, ptr);
			CopyToCSTR(CLASS, data, ptr);
			CopyToCSTR(TTL, data, ptr);
			CopyToCSTR(RDLENGTH, data, ptr);
			CopyToCSTR(answer[acnt].RDATA, data, ptr);

			//answer
			/*
			for (int i = 0; i < answer[answer_cnt].RDATA.length(); i++)
			{
				raw_data.data[p_question] = answer[answer_cnt].RDATA[i];
				p_question++;
			}
			*/
			ptr++;
		}
	}
	//raw_data.data[p_question] = '\0';
	raw_data.addr = from;//目的地址默认为源地址（即用户）
	raw_data.len = ptr;
	//raw_data.len = raw_data.data.length();
	return true;
}

#ifndef NDEBUG
void DNSPacket::PrintPacket()
{
	printf("------------------Header------------------\n");
	printf("ID:%X\n", header.ID);
	printf("Flags:%.4X\n", header.Flags);
	printf("QR = %X, Opcode = %X, AA = %X, TC = %X, RD = %X\n",
		(header.Flags & 0x8000) >> 15, (header.Flags & 0x7800) >> 11, (header.Flags & 0x0400) >> 10, (header.Flags & 0x0200) >> 9,
		   (header.Flags & 0x0100) >> 8);
	printf("RA = %X, Z = %X, RCODE = %X\n",
		(header.Flags & 0x0080) >> 7, (header.Flags & 0x0070) >> 4, (header.Flags & 0x000f));
	printf("QDCOUNT = %u, ANCOUNT = %u, NSCOUNT = %u, ARCOUNT = %u\n",
		   header.QDCOUNT, header.ANCOUNT, header.NSCOUNT, header.ARCOUNT);

	printf("-------------------Query------------------\n");
	for (int qcnt = 0; qcnt < header.QDCOUNT; qcnt++)
	{
		printf("Query %d:\n", qcnt + 1);
		printf("QNAME = %s\n", query[qcnt].QNAME.c_str());
		printf("QTYPE = %u, QCLASS = %u\n", query[qcnt].QTYPE, query[qcnt].QCLASS);
		printf("\n");
	}

	if ((header.Flags & 0x8000) >> 15)
	{
		printf("------------------Answer------------------\n");
		for (int acnt = 0; acnt < header.ANCOUNT; acnt++)
		{
			printf("Answer %d:\n", acnt + 1);
			printf("NAME = %s\n", answer[acnt].NAME.c_str());
			printf("TYPE = %u, CLASS = %u\n", answer[acnt].TYPE, answer[acnt].CLASS);
			printf("TTL = %u\n", answer[acnt].TTL);
			printf("RDLENGTH = %u\n", answer[acnt].RDLENGTH);
			if (answer[acnt].TYPE == 1)
			{
				printf("RDATA = %u.%u.%u.%u\n", answer[acnt].RDATA[0], answer[acnt].RDATA[1], answer[acnt].RDATA[2], answer[acnt].RDATA[3]);
			}
			else
			{
				std::cout << answer[acnt].RDATA << std::endl;
			}
			printf("\n");
		}
	}
}

void DNSPacket::PrintRawData()
{
	printf("-----------------Raw Data-----------------\n");
	unsigned temp;
	for (int i = 0; i < raw_data.len; i++)
	{
		temp = static_cast<unsigned char>(raw_data.data[i]);
		printf("%c%X ", "0\a"[temp <= 0xf ? 0 : 1], temp);
		if (i % 8 == 7)printf("\n");
	}
}
#endif