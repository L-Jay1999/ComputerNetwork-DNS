#pragma once
#include <string>
#include <iostream>
#include <cstdio>
#include <memory>
#include <utility>

#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

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
	char data[1024];
	sockaddr_in addr;
};

struct DNSPacket
{
	DNSPacket() = default;
	DNSPacket(const DNSPacket &other) = delete;
	DNSPacket &operator=(const DNSPacket &rhs) = delete;
	~DNSPacket() = default;

	bool Parse(const QueueData &raw_packet);
	bool to_packet();
	void PrintRawData();
	void PrintPacket();

	sockaddr_in from{};
	QueueData raw_data{};
	DNSHeader header{};
	std::unique_ptr<DNSQuery[]> query;
	std::unique_ptr<DNSAnswer[]> answer;

	void CopyToCSTR(const std::string &str, char *buffer, int &ptr);

	template <typename T>
	void CopyToCSTR(const T val, char *buffer, int &ptr);

	template <typename T>
	static void ReadFromCSTR(T &dest, const char *src, int &ptr);

	void ReadFromCSTR(char *dest, const unsigned len, const char *src, int &ptr);
	void ReadFromCSTR(std::string &dest, const unsigned len, const char *src, int &ptr);
};
