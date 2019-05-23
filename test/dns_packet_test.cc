#include "../src/include/dns_packet.h"
int main()
{
	
	char a[256];
	a[0] = 0x00;
	a[1] = 0x02;

	a[2] = 0x81;
	a[3] = 0x80;

	a[4] = 0x00;
	a[5] = 0x01;

	a[6] = 0x00;
	a[7] = 0x03;

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

	a[31] = 0xc0;
	a[32] = 0x0c;

	a[33] = 0x00;
	a[34] = 0x05;

	a[35] = 0x00;
	a[36] = 0x01;

	a[37] = 0x00;
	a[38] = 0x00;
	a[39] = 0x00;
	a[40] = 0x1c;

	a[41] = 0x00;
	a[42] = 0x0f;

	a[43] = 0x03;
	a[44] = 0x77;
	a[45] = 0x77;
	a[46] = 0x77;
	a[47] = 0x01;
	a[48] = 0x61;
	a[49] = 0x06;
	a[50] = 0x73;
	a[51] = 0x68;
	a[52] = 0x69;
	a[53] = 0x66;
	a[54] = 0x65;
	a[55] = 0x6e;
	a[56] = 0xc0;
	a[57] = 0x16;
	a[58] = 0xc0;
	a[59] = 0x2b;
	a[60] = 0x00;
	a[61] = 0x01;
	a[62] = 0x00;
	a[63] = 0x01;
	a[64] = 0x00;
	a[65] = 0x00;
	a[66] = 0x00;
	a[67] = 0x63;
	a[68] = 0x00;
	a[69] = 0x04;
	a[70] = 0x6f;
	a[71] = 0x0d;
	a[72] = 0x64;
	a[73] = 0x5b;
	a[74] = 0xc0;
	a[75] = 0x2b;
	a[76] = 0x00;
	a[77] = 0x01;
	a[78] = 0x00;
	a[79] = 0x01;
	a[80] = 0x00;
	a[81] = 0x00;
	a[82] = 0x00;
	a[83] = 0x63;
	a[84] = 0x00;
	a[85] = 0x04;
	a[86] = 0x6f;
	a[87] = 0x0d;
	a[88] = 0x64;
	a[89] = 0x5c;
	QueueData q;
	q.len = 90;
	q.data = a;
	DNSPacket p;
	p.Parse(q);
	p.PrintPacket();
	p.PrintRawData();
	

	/*
	DNSPacket p;
	p.raw_data.data = new char[256]();
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
	p.to_packet();
	p.PrintPacket();
	p.PrintRawData();
	return 0;
	*/
}