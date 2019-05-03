#include <winsock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <stdlib.h>

#include <string>
#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_PORT "53"
#define DEFAULT_BUFLEN 512

struct DNSHeader
{
	// byte 0 - 1
	unsigned short ID;

	// byte 2 - 3
	unsigned short Flags;
	// unsigned short QR;	// query = 0, responce = 1
	// unsigned short Opcode;
	// unsigned short AA;
	// unsigned short TC;
	// unsigned short RD;
	// unsigned short RA;
	// unsigned short Z;
	// unsigned short RCODE;

	// byte 3 - 4
	unsigned short QDCOUNT;

	// byte 5 - 6
	unsigned short ANCOUNT;

	// byte 7 - 8
	unsigned short NSCOUNT;

	// byte 9 - 10
	unsigned short ARCOUNT;
};

struct DNSQuestion
{
	std::string QNAME;
	unsigned short QTYPE;
	unsigned short QCLASS;
};

struct DNSAnswer
{
	unsigned short NAME; // 0xc00c
	unsigned int TYPE;
	unsigned int CLASS;
	unsigned int TTL;
	unsigned int RDLENGTH;
	unsigned int RDATA;
};

struct DNSPacket
{
	// header
	struct DNSHeader header;

	// questions
	// struct DNSQuestion *questions;
	struct DNSQuestion question;

	// answers
	struct DNSAnswer answer;

	// records

	// additional records

};

void PrintHEX(char* buf, int len)
{
	unsigned temp;
	for (int i = 0; i < len; i++)
	{
		temp = static_cast<unsigned char>(buf[i]);
		printf("%c%X ", "0\a"[temp <= 0xf ? 0 : 1], temp);
	}
	printf("\n");
}

void PrintChar(char* buf, int len)
{
	for (int i = 0; i < len; i++)
	{
		printf("%c", buf[i]);
	}
	printf("\n");
}

unsigned short combine(unsigned char hbyte, unsigned char lbyte)
{
	unsigned short h = static_cast<unsigned char>(hbyte);
	unsigned short l = static_cast<unsigned char>(lbyte);
	unsigned short res = ((hbyte << 8) | lbyte);
	return res;
}

struct DNSPacket DNSParse(char* packet, int len)
{
	struct DNSPacket res;
	unsigned short hbyte;
	unsigned short lbyte;
	
	res.header.ID = combine(packet[0], packet[1]);
	res.header.Flags = combine(packet[2], packet[3]);
	res.header.QDCOUNT = combine(packet[4], packet[5]);
	res.header.ANCOUNT = combine(packet[6], packet[7]);
	res.header.NSCOUNT = combine(packet[8], packet[9]);
	res.header.ARCOUNT = combine(packet[10], packet[11]);

	int p_question = 12;
	unsigned short char_num;

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
			for (int i = 0; i < char_num; i++)
			{
				++p_question;
				res.question.QNAME.push_back(packet[p_question]);
			}
			++p_question;
			res.question.QNAME.push_back('.');
		}
	}

	res.question.QTYPE = combine(packet[p_question], packet[p_question + 1]);
	
	p_question += 2;

	res.question.QCLASS = combine(packet[p_question], packet[p_question + 1]);
	return res;
}

void PrintDNSPacket(const DNSPacket& packet)
{
	unsigned int flags = packet.header.Flags;
	printf("ID = %X\n", packet.header.ID);
	printf("Flags = %X\n", flags);
	printf("QR = %X, Opcode = %X, AA = %X, TC = %X, RD = %X\n",
		(flags & 0x8000) >> 15, (flags & 0x7800) >> 11, (flags & 0x0400) >> 10, (flags & 0x0200) >> 9,
		(flags & 0x0100) >> 8);
	printf("RA = %X, Z = %X, RCODE = %X\n",
		(flags & 0x0080) >> 7, (flags & 0x0070) >> 4, (flags & 0x000f));
	printf("QDCOUNT = %d, ANCOUNT = %d, NSCOUNT = %d, ARCOUNT = %d\n",
		packet.header.QDCOUNT, packet.header.ANCOUNT, packet.header.NSCOUNT, packet.header.ARCOUNT);

	printf("Questions:\n");
	printf("QNAME = %s\n", packet.question.QNAME.c_str());
	printf("QTYPE = %d, QCLASS = %d\n\n", packet.question.QTYPE, packet.question.QCLASS);
}

int get_answer_packet(const DNSPacket& packet, char* buf, int len)
{
	// flag
	buf[2] = 0x81;
	buf[3] = 0x80;

	// answer count
	buf[7] = 0x01;

	buf[len] = 0xc0;
	buf[len + 1] = 0x0c;

	buf[len + 2] = 0x00;
	buf[len + 3] = 0x01;

	buf[len + 4] = 0x00;
	buf[len + 5] = 0x01;

	buf[len + 6] = 0x00;
	buf[len + 7] = 0x00;
	buf[len + 8] = 0x02;
	buf[len + 9] = 0x58;

	buf[len + 10] = 0x00;
	buf[len + 11] = 0x04;

	buf[len + 12] = 127;
	buf[len + 13] = 0;
	buf[len + 14] = 0;
	buf[len + 15] = 1;
	return len + 16;
}

int main() {
	WSADATA wsaData;
	int iResult;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	struct addrinfo *result = NULL, *ptr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}
	
	SOCKET socSrv = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (socSrv == INVALID_SOCKET)
	{
		printf("socket failed: %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	iResult = bind(socSrv, result->ai_addr, result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(socSrv);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	char recvbuf[DEFAULT_BUFLEN];
	int iSendResult;
	int recvbuflen = DEFAULT_BUFLEN;
	sockaddr from;
	int from_len = sizeof(sockaddr);
	struct DNSPacket dns_packet;
	// Receive until the peer shuts down the connection
	do {

		iResult = recvfrom(socSrv, recvbuf, recvbuflen, 0, &from, &from_len);
		if (iResult > 0) {
			printf("Bytes received: %d\n", iResult);

			PrintHEX(recvbuf, iResult);
			// PrintChar(recvbuf, iResult);
			
			dns_packet = DNSParse(recvbuf, iResult);
			PrintDNSPacket(dns_packet);

			iResult = get_answer_packet(dns_packet, recvbuf, iResult);

			// Echo the buffer back to the sender
			iSendResult = sendto(socSrv, recvbuf, iResult, 0, &from, from_len);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed: %d\n", WSAGetLastError());
				closesocket(socSrv);
				WSACleanup();
				return 1;
			}
			printf("Bytes sent: %d\n", iSendResult);
		}
		else if (iResult == 0)
			printf("Connection closing...\n");
		else {
			printf("recv failed: %d\n", WSAGetLastError());
			closesocket(socSrv);
			WSACleanup();
			return 1;
		}

	} while (true);

	iResult = shutdown(socSrv, SD_BOTH);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(socSrv);
		WSACleanup();
		return 1;
	}

	closesocket(socSrv);
	WSACleanup();

	return 0;
}