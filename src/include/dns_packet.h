#pragma once
#include <string>

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
	unsigned short NAME;
	unsigned int TYPE;
	unsigned int CLASS;
	unsigned int TTL;
	unsigned int RDLENGTH;
	unsigned int RDATA;
};

struct QueueData
{
	int len;
	char *data;
	sockaddr addr;
};

struct DNSPacket
{
public:
	bool Parse(const QueueData &raw_packet);
	QueueData raw_data;
	DNSHeader header;
	DNSQuery query;
	DNSAnswer answer;
};
