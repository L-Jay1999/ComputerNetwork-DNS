#include "dns_packet.h"
int main()
{
	/*char a[256];
	a[0] = 0x00;
	a[1] = 0x02;
	a[2] = 0x01;
	a[3] = 0x00;
	a[4] = 0x00;
	a[5] = 0x01;
	a[6] = 0x00;
	a[7] = 0x00;
	a[8] = 0x00;
	a[9] = 0x00;
	a[10] = 0x00;
	a[11] = 0x00;
	a[12] = 0x03;
	a[13] = 0x77;
	a[14] = 0x77;
	a[15] = 0x77;
	a[16] = 0x05;
	a[17] = 0x62;
	a[18] = 0x61;
	a[19] = 0x69;
	a[20] = 0x64;
	a[21] = 0x75;
	a[22] = 0x03;
	a[23] = 0x63;
	a[24] = 0x6f;
	a[25] = 0x6d;
	a[26] = 0x00;
	a[27] = 0x00;
	a[28] = 0x01;
	a[29] = 0x00;
	a[30] = 0x01;
	QueueData q;
	q.len = 31;
	q.data = a;*/
	DNSPacket p;
	//p.Parse(q);
	p.header.ID = 2;
	p.header.Flags = 0x0100;
	p.header.QDCOUNT = 1;
	p.header.ANCOUNT = 0;
	p.header.ARCOUNT = 0;
	p.header.NSCOUNT = 0;
	p.query = new DNSQuery[1];
	p.query[0].QNAME = "www.baidu.com";
	p.query[0].QTYPE = 1;
	p.query[0].QCLASS = 1;
	p.Packet();
	p.print();
	p.printraw();
	return 0;
}