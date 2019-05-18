#pragma once
#include <string>
#include <iostream>
#include <cstdio>
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")
// DNSHeader
// DNSQuery
// DNSAnswer
// ......

struct DNSHeader
{
	unsigned short ID;
	unsigned short Flags;
	unsigned short QDCOUNT;
	unsigned short ANCOUNT;
	unsigned short NSCOUNT;
	unsigned short ARCOUNT;
};

struct DNSQuery
{
	std::string QNAME;
	unsigned short QTYPE;
	unsigned short QCLASS;
};

struct DNSAnswer
{
	std::string NAME;
	unsigned short TYPE;
	unsigned short CLASS;
	unsigned int TTL;
	unsigned short RDLENGTH;
	std::string RDATA;
};

struct QueueData
{
	int len;
	char* data;
	sockaddr addr;
};

struct DNSPacket
{
public:
	bool Parse(const QueueData &raw_packet);
	bool Packet();
	void printraw();
	void print();
	void deleteall();
	QueueData raw_data;
	DNSHeader header;
	DNSQuery *query;
	DNSAnswer *answer;
};
