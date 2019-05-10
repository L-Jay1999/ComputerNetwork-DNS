#pragma once

class DNSSender
{
private:
    std::deque<char[512]> *jobq; // 从该队列中取出数据并处理发送
    MySocket socSend;
    MySocket socQuery; // 向上级查询时使用
    ADDR;              // 上级dns服务器地址
    DNSPacket dns_packet;
    // .......
};