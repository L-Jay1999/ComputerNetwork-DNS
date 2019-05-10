#pragma once

class DNSReceiver   // 接收dns数据包并将其放到工作队列上
{
public:
private:
    DNSPacket dns_packet_[];    // 保存dns数据包的详细信息
    std::deque<char[512]> *jobq; // 保存收到的
    MySocket socRecv;           // 接收dns数据包
    // .......
};