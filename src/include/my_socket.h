#pragma once
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")
class MySocket
{
public:
    MySocket();

private:
    static constexpr int recvbuflen = 512;
    char recvbuf[recvbuflen];
    sockaddr from;
    int from_len = sizeof(sockaddr);
    // ......
};