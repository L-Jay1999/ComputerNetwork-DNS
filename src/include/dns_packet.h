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

	//将raw_packet内的dns报文解析并分别读入
	bool Parse(const QueueData &raw_packet);

	//将数据还原为dns报文并将地址等信息写入raw_data
	bool to_packet();

	//输出raw_data内储存的dns报文
	void PrintRawData();

	//输出解析完成的dns报文各部分表示的信息
	void PrintPacket();

	sockaddr_in from{};					 //报文的sockaddr信息
	QueueData raw_data{};				 //报文的原始数据
	DNSHeader header{};					 //报文的头部
	std::unique_ptr<DNSQuery[]> query;   //报文的询问部分
	std::unique_ptr<DNSAnswer[]> answer; //报文的回答部分

	void CopyToCSTR(const std::string &str, char *buffer, int &ptr);

	// 将val的内容以ptr为位置指针写入buffer
	template <typename T>
	void CopyToCSTR(const T val, char *buffer, int &ptr);

	// 将src的内容以ptr为位置指针读入dest
	template <typename T>
	static void ReadFromCSTR(T &dest, const char *src, int &ptr);

	// 将src的内容以ptr为位置指针读取之后len位的内容存入dest
	void ReadFromCSTR(char *dest, const unsigned len, const char *src, int &ptr);
	void ReadFromCSTR(std::string &dest, const unsigned len, const char *src, int &ptr);
};
