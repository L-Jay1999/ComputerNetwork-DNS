#pragma once
#include <string>
#include <WinSock2.h>
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

struct DNSPacket
{
	DNSHeader header;
	DNSQuery query;
	DNSAnswer answer;
};

struct Queuedata
{
	int len;
	char *data;
	sockaddr addr;
};